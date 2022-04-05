#pragma once
#include "../SCBW/scbwdata.h"

typedef void (__fastcall *UpdateFunction)(CImage* image);
typedef void (__fastcall *DrawFunction)(s32 screenX, s32 screenY, GrpFrame* frame, Box32* grpBounds, void* coloringData);

namespace hooks {

  void initializeColoringData(CImage* image);

  void __fastcall DrawFunc_Alpha(s32 screenX, s32 screenY, GrpFrame* frame, Box32* grpBounds, void* coloringData);
  void __fastcall DrawFunc_Alpha_F(s32 screenX, s32 screenY, GrpFrame* frame, Box32* grpBounds, void* coloringData);

  void injectDrawFuncHook();
  
}; // hooks


// ===== GRP Decoding Macros =====
// Makes for *significantly* nicer-looking draw functions

// in an ifdef so they only exist when needed
#ifdef GRP_DECODING

// RLE Transparent Pixels (0x80)
#define GRP_RLE_TRANSPARENT(DST, DRAWFUNC, XDIR) \
  x += len;

#define GRP_RLE_TRANSPARENT_FLIP(DST, DRAWFUNC, XDIR) \
  x -= len;

#define GRP_RLE_BLACK(DST, DRAWFUNC, XDIR) \
  for (; len > 0; len--) { \
    (DST)[x] = 0;          \
    XDIR;                  \
  }

// RLE Single Color Copy (0x40)
#define GRP_RLE_SINGLE(DST, DRAWFUNC, XDIR) \
  for (; len > 0; len--) { \
    (DST)[x] = (DRAWFUNC); \
    XDIR;                  \
  }                        \
  data++;

// RLE Block Copy (0x00)
#define GRP_RLE_BLOCK(DST, DRAWFUNC, XDIR) \
  for (; len > 0; len--) { \
    (DST)[x] = (DRAWFUNC); \
    XDIR;                  \
    data++;                \
  }


// Draws pixels to the destination buffer using the specified RLE macro
#define GRP_DRAW(RLE_MACRO, DST, DRAWFUNC, XDIR) \
  drawRight -= len;                    \
  if (drawRight < 0) len += drawRight; \
  RLE_MACRO(DST, DRAWFUNC, XDIR);



// Decodes GRP outside bounding rectangle, discarding pixels or drawing as needed
#define GRP_DECODE_SKIP(TRANSFUNC, DST, DRAWFUNC, XDIR) \
  while (drawLeft != 0) { \
    len = *data; \
    data++; \
    if (len & 0x80) { /* transparent */ \
      len &= ~0x80; \
      drawLeft -= len; \
      if (drawLeft < 0) { \
        len = -drawLeft; \
        drawRight -= len; \
        TRANSFUNC(DST, DRAWFUNC, XDIR); \
        break; \
      } \
    }  else if (len & 0x40) { /* single-color copy */ \
      len &= ~0x40; \
      data++; \
      drawLeft -= len; \
      if (drawLeft < 0) { \
        len = -drawLeft; \
        data--; \
        GRP_DRAW(GRP_RLE_SINGLE, DST, DRAWFUNC, XDIR); \
        break; \
      } \
    } else { /* block copy */ \
      data += len; \
      drawLeft -= len; \
      if (drawLeft < 0) { \
        len = -drawLeft; \
        data -= len; \
        GRP_DRAW(GRP_RLE_BLOCK, DST, DRAWFUNC, XDIR); \
        break; \
      } \
    } \
  }

// Decodes & draws GRP data
#define GRP_DECODE_DRAW(TRANSFUNC, DST, DRAWFUNC, XDIR) \
  while (drawRight > 0) { \
    len = *data; \
    data++; \
    if (len & 0x80) { /* transparent */ \
      len &= ~0x80; \
      GRP_DRAW(TRANSFUNC, DST, DRAWFUNC, XDIR); \
    } else if (len & 0x40) { /* single-color copy */ \
      len &= ~0x40; \
      GRP_DRAW(GRP_RLE_SINGLE, DST, DRAWFUNC, XDIR); \
    } else { /* block copy */ \
      GRP_DRAW(GRP_RLE_BLOCK, DST, DRAWFUNC, XDIR); \
    } \
  }


// Decodes normal GRP line
//   GRPFRAME  -- pointer to GRP frame
//   GRPRECT   -- pointer to RECT defining GRP bounds
//   TRANSFUNC -- Transparency mode GRP_RLE_TRANSPARENT, GRP_RLE_TRANSPARENT_FLIP, or GRP_RLE_BLACK
//   DST       -- Destination buffer
//   DRAWFUNC  -- Expression to write to destination buffer: (DST)[x] = (DRAWFUNC); -- can use *data, x with whatever DST is, function calls, or other variables
#define GRP_DECODE_LINE(GRPFRAME, GRPRECT, TRANSFUNC, DST, DRAWFUNC) \
  do { \
    s32 x = 0; \
    u16* lines = (u16*)((GRPFRAME)->dataOffset + (GRPRECT)->top * 2); \
    u8* data = (u8*)((GRPFRAME)->dataOffset + lines[y]); \
    u8 len; \
    s32 drawLeft = (GRPRECT)->left; \
    s32 drawRight = (GRPRECT)->right; \
    \
    /* skip pixels outside of GRP bounds */ \
    GRP_DECODE_SKIP(TRANSFUNC, DST, DRAWFUNC, x++); \
    \
    if (drawRight <= 0) { /* bytes overdrawn -- does this even do anything? */ \
      x += drawRight; \
    } else { \
      /* draw pixels */ \
      GRP_DECODE_DRAW(TRANSFUNC, DST, DRAWFUNC, x++); \
    } \
  } while (0)


// Decodes flipped GRP line
//   -- see GRP_DECODE_LINE for arguments
#define GRP_DECODE_LINE_FLIP(GRPFRAME, GRPRECT, TRANSFUNC, DST, DRAWFUNC) \
  do { \
    s32 x = (GRPRECT)->right - 1; \
    u16* lines = (u16*)((GRPFRAME)->dataOffset + (GRPRECT)->top * 2); \
    u8* data = (u8*)((GRPFRAME)->dataOffset + lines[y]); \
    u8 len; \
    s32 drawLeft = (GRPFRAME)->width - (GRPRECT)->right - (GRPRECT)->left; \
    s32 drawRight = (GRPRECT)->right; \
    \
    /* skip pixels outside of GRP bounds */ \
    GRP_DECODE_SKIP(TRANSFUNC, DST, DRAWFUNC, x--); \
    \
    /* draw pixels */ \
    GRP_DECODE_DRAW(TRANSFUNC, DST, DRAWFUNC, x--); \
  } while (0)


#endif
