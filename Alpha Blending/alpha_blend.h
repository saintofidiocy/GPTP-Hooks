#pragma once
#include "graphics.h"

//#define RGB(r,g,b) ((r) | ((g)<<8) | ((b)<<16))

namespace graphics {
  typedef struct {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
  } RGBA;

  // Mixes rgb1 and rgb2. a = 0 returns rgb1, a = 255 returns rgb2
  ColorId blend(RGBA rgb1, RGBA rgb2, u8 a);
  ColorId blend(u8 r1, u8 g1, u8 b1, u8 r2, u8 g2, u8 b2, u8 a);
  ColorId blend(ColorId pal1, ColorId pal2, u8 a);
  ColorId blend(u8* rgb1, u8* rgb2, u8 a);

  // Mixes rgba1 and rgb2. Alpha is from rgba1
  ColorId blend(RGBA rgba1, RGBA rgb2);

  // Returns the closest palette color to the given RGB color
  ColorId getColorId(u8 r, u8 g, u8 b);
  ColorId getColorId(RGBA rgb);
  ColorId getColorId(u32 rgb);

  // Initializes palette matching data
  void initPalData();
};
