#include "alpha_blend.h"
#include <SCBW/api.h>

namespace {

  typedef struct {
    u8 count;
    u8 single;
    u16 offset;
  } rgbcell;

  const s32 CELL_SIZE  = 16;
  const s32 CELL_COUNT = 256 / CELL_SIZE;
  const s32 CELL_WIDTH = CELL_SIZE - 1;
  const u8  CELL_MASK  = 0xFF ^ CELL_WIDTH;
  const u32 DATA_MAX   = 32768;

  const u8  PAL_TEST   = 1;
  const u8  PAL_REJECT = 2;
  const u8  PAL_IGNORE = 255;

  const u16 DATA_NONE  = 0xFFFF;

  rgbcell cells[CELL_COUNT][CELL_COUNT][CELL_COUNT] = { 0 };
  u8 cellData[DATA_MAX];
  u32 cellDataCount = 0;

};

namespace graphics {

  const RGBA* palette = (const RGBA*)(0x005994E0);


  ColorId blend(RGBA rgb1, RGBA rgb2, u8 a) {
    u8 r = (a*rgb1.r + (255 - a)*rgb2.r) / 255;
    u8 g = (a*rgb1.g + (255 - a)*rgb2.g) / 255;
    u8 b = (a*rgb1.b + (255 - a)*rgb2.b) / 255;
    return getColorId(r, g, b);
  }

  ColorId blend(RGBA rgba1, RGBA rgb2) {
    u8 r = (rgba1.a*rgba1.r + (255 - rgba1.a)*rgb2.r) / 255;
    u8 g = (rgba1.a*rgba1.g + (255 - rgba1.a)*rgb2.g) / 255;
    u8 b = (rgba1.a*rgba1.b + (255 - rgba1.a)*rgb2.b) / 255;
    return getColorId(r, g, b);
  }

  ColorId blend(u8 r1, u8 g1, u8 b1, u8 r2, u8 g2, u8 b2, u8 a) {
    RGBA rgb1 = { r1, g1, b1, 0 };
    RGBA rgb2 = { r2, g2, b2, 0 };
    return blend(rgb1, rgb2, a);
  }

  ColorId blend(ColorId pal1, ColorId pal2, u8 a) {
    if (a == 0) return pal2;
    if (a == 255) return pal1;

    ColorId result = blend(palette[pal1], palette[pal2], a);

    // try to make color cycling nicer
    if (result != pal2 && ((pal2 >= 1 && pal2 <= 13) || (pal2 >= 248 && pal2 <= 254))) {
      if (palette[result].r == palette[pal2].r && palette[result].g == palette[pal2].g && palette[result].b == palette[pal2].b) {
        return pal2;
      }
    }
    return result;
  }

  ColorId blend(u8* rgb1, u8* rgb2, u8 a) {
    RGBA col1 = { rgb1[0], rgb1[1], rgb1[2], 0 };
    RGBA col2 = { rgb2[0], rgb2[1], rgb2[2], 0 };
    return blend(rgb1, rgb2, a);
  }


  ColorId getColorId(u32 rgb) {
    return getColorId((u8)(rgb), (u8)(rgb >> 8), (u8)(rgb >> 16));
  }

  ColorId getColorId(RGBA rgb) {
    return getColorId(rgb.r, rgb.g, rgb.b);
  }

  ColorId getColorId(u8 r, u8 g, u8 b) {
    rgbcell* cell = &(cells[r / CELL_COUNT][g / CELL_COUNT][b / CELL_COUNT]);
    if (cell->count == 0) return cell->single;
    if (cell->count == 1) return cellData[cell->offset];

    s32 calc, dist;
    s32 best = LONG_MAX;
    ColorId data;
    ColorId color = 0;
    
    for (u32 i = 0; i < cell->count; i++) {
      data = cellData[cell->offset + i];

      calc = palette[data].r - r;
      dist = calc*calc;
      calc = palette[data].g - g;
      dist += calc*calc;
      calc = palette[data].b - b;
      dist += calc*calc;

      if (dist == 0) return data;
      if (dist < best) {
        color = data;
        best = dist;
      }
    }

    return color;
  }


  void initPalData() {
    u32 i, j, k;
    u32 cellR, cellG, cellB;
    u8 maskR, maskG, maskB;
    s32 deltaR, deltaG, deltaB;
    s32 testR, testG, testB;
    s32 mag2[256];
    s32 near[256];
    s32 far[256];
    u8 flags[256] = { 0 };
    s32 calc;
    s32 arg;
    u32 count;
    bool result;
    bool test;
    bool pass;

    cellDataCount = 0;

    // set excluded palette indexes and pre-compute magnitudes
    for (i = 0; i < 256; i++) {
      // skips color cycle colors and extra blue colors
      if (i >= 1 && i <= 13) {
        if (i < 7 || (*CURRENT_TILESET != TilesetType::Space_Platform && *CURRENT_TILESET != TilesetType::Installation)) { // 1 to 7 for platform or installation
          flags[i] = PAL_IGNORE;
          continue;
        }
      }else if(i >= 247 && i <= 254){
        if (i > 247 || *CURRENT_TILESET != TilesetType::Badlands) { // keep 247 for badlands
          flags[i] = PAL_IGNORE;
          continue;
        }
      }

      // mag2_i = ||pal_i||^2
      mag2[i] = palette[i].r*palette[i].r + palette[i].g*palette[i].g + palette[i].b*palette[i].b;
    }

    // iterate cells and generate color lists for each
    for (cellB = 0; cellB < CELL_COUNT; cellB++) {
      maskB = cellB * CELL_SIZE;
      for (cellG = 0; cellG < CELL_COUNT; cellG++) {
        maskG = cellG * CELL_SIZE;
        for (cellR = 0; cellR < CELL_COUNT; cellR++) {
          maskR = cellR * CELL_SIZE;

          count = 0;

          // Calculate nearest and farthest points within the cell for each palette index
          for (i = 0; i < 256; i++) {
            if (flags[i] == PAL_IGNORE) continue;

            // nearest cell point
            near[i] = 0;
            if (palette[i].r < maskR) {
              calc = palette[i].r - maskR;
              near[i] = calc*calc;
            } else if (palette[i].r > (maskR + CELL_WIDTH)){
              calc = palette[i].r - (maskR + CELL_WIDTH);
              near[i] = calc*calc;
            }
            if (palette[i].g < maskG) {
              calc = palette[i].g - maskG;
              near[i] += calc*calc;
            } else if (palette[i].g > (maskG + CELL_WIDTH)){
              calc = palette[i].g - (maskG + CELL_WIDTH);
              near[i] += calc*calc;
            }
            if (palette[i].b < maskB) {
              calc = palette[i].b - maskB;
              near[i] += calc*calc;
            } else if (palette[i].b > (maskB + CELL_WIDTH)){
              calc = palette[i].b - (maskB + CELL_WIDTH);
              near[i] += calc*calc;
            }

            // farthest cell point
            far[i] = 0;
            if (palette[i].r <= (maskR + CELL_WIDTH) / 2) {
              calc = palette[i].r - (maskR + CELL_WIDTH);
              far[i] = calc*calc;
            } else {
              calc = palette[i].r - maskR;
              far[i] = calc*calc;
            }
            if (palette[i].g <= (maskG + CELL_WIDTH) / 2) {
              calc = palette[i].g - (maskG + CELL_WIDTH);
              far[i] += calc*calc;
            } else {
              calc = palette[i].g - maskG;
              far[i] += calc*calc;
            }
            if (palette[i].b <= (maskB + CELL_WIDTH) / 2) {
              calc = palette[i].b - (maskB + CELL_WIDTH);
              far[i] += calc*calc;
            } else {
              calc = palette[i].b - maskB;
              far[i] += calc*calc;
            }
          }

          // near-far point test
          for (i = 0; i < 256; i++) {
            if (flags[i] == PAL_IGNORE) continue;
            pass = true;
            for (j = 0; j < 256; j++) {
              if (flags[j] == PAL_IGNORE || flags[j] == PAL_REJECT) continue;
              if (near[i] > far[j]) {
                pass = false;
                break;
              }
            }
            if (pass) {
              flags[i] = PAL_TEST;
            }
          }

          // boundary plane test
          for (i = 0; i < 256; i++) {
            if (flags[i] != PAL_TEST) continue;
            pass = true;
            for (j = 0; j < 256; j++) {
              if (i == j || flags[j] != PAL_TEST) continue;
              deltaR = 2 * ((s32)palette[i].r - (s32)palette[j].r);
              deltaG = 2 * ((s32)palette[i].g - (s32)palette[j].g);
              deltaB = 2 * ((s32)palette[i].b - (s32)palette[j].b);
              arg = mag2[i] - mag2[j];

              // test cell corners
              for (k = 0; k < 8; k++) {
                testR = (k & 1) ? maskR : maskR + CELL_WIDTH;
                testG = (k & 2) ? maskG : maskG + CELL_WIDTH;
                testB = (k & 4) ? maskB : maskB + CELL_WIDTH;
                test = (testR*deltaR + testG*deltaG + testB*deltaB < arg);
                if (k == 0) {
                  result = test;
                } else {
                  // different sign --> test plane intersects cell and both colors are valid
                  if (result != test) break;
                }
              }

              // entire cell is on one side of the boundary plane
              if (result == test) {
                test = (((s32)palette[j].r)*deltaR + ((s32)palette[j].g)*deltaG + ((s32)palette[j].b)*deltaB < arg);
                if (test == result) {
                  // entire cell is on [j] side --> [i] does not influence this cell
                  pass = false;
                  break;
                }
              }
            }

            // passes all the tests -- add to the cell list
            if (pass) {
              cellData[cellDataCount + count] = i;
              count++;
            }
          }

          if (count == 0) {
            // error ???
            cells[cellR][cellG][cellB].single = 0;
            cells[cellR][cellG][cellB].offset = DATA_NONE;
          } else if (count == 1) {
            count = 0;
            cells[cellR][cellG][cellB].single = cellData[cellDataCount];
            cells[cellR][cellG][cellB].offset = DATA_NONE;
          } else {
            cells[cellR][cellG][cellB].single = 0;
            cells[cellR][cellG][cellB].offset = cellDataCount;
            cellDataCount += count;
            //if(cellDataCount >= 65536) error ???
          }
          cells[cellR][cellG][cellB].count = count;
        }
      }
    }
  }


};
