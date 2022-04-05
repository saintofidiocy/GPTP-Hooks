#define GRP_DECODING // get macro definitions
#include "draw_funcs.h"
#include <SCBW/api.h>
#include <graphics/alpha_blend.h>

//Helper functions definitions
namespace {

  typedef struct {
    u16 width, height;
    u8* data;
  } Bitmap; // because the bitmap.h one doesn't work for some reason, and doesn't have data access anyway

  u8* palette = (u8*)(0x005994E0); // active game palette
  u8* grpReindexing = (u8*)(0x0050CDC1); // reindexing table for player, icon, button, wireframe, etc. colors
  Bitmap** screenBitmap = (Bitmap**)(0x006CF4A8); // screen draw buffer

}; // unnamed namespace


namespace hooks {
  
  bool calcMaskBounds(Box32* grpBounds, Box32* maskBounds, GrpHead* grp, GrpHead* mask, u32 grpFrame, u32 maskFrame, bool flip);


  // Called by CImage constructor
  void initializeColoringData(CImage* image) {
    switch (images_dat::RLE_Function[image->id]) {
      case PaletteType::RLE_PLAYER_SIDE:
        *(u32*)(&(image->coloringData)) = image->parentSprite->playerId;
        break;

      case PaletteType::RLE_EFFECT:
        image->coloringData = colorShift[images_dat::Remapping[image->id]].data;
        break;

      case 18:
        // low word = frame -- 0xFFFF to mimic main image
        // high word = image ID
        *(u32*)(&(image->coloringData)) = 0xFFFF | (image->id << 16);
        break;
    }
  }


  /* ---- Samples for making custom draw functions: ----

  // Copy of Normal draw function:
  void __fastcall DrawFunc_Normal(s32 screenX, s32 screenY, GrpFrame* frame, Box32* grpBounds, void* coloringData) {
    u8* dst = (*screenBitmap)->data + (*screenBitmap)->width*screenY + screenX;
    for (s32 y = 0; y < grpBounds->bottom; y++) {
      GRP_DECODE_LINE(frame, grpBounds, GRP_RLE_TRANSPARENT, dst, grpReindexing[*data]);
      dst += (*screenBitmap)->width;
    }
  }
  void __fastcall DrawFunc_Normal_F(s32 screenX, s32 screenY, GrpFrame* frame, Box32* grpBounds, void* coloringData) {
    u8* dst = (*screenBitmap)->data + (*screenBitmap)->width*screenY + screenX;
    for (s32 y = 0; y < grpBounds->bottom; y++) {
      GRP_DECODE_LINE_FLIP(frame, grpBounds, GRP_RLE_TRANSPARENT_FLIP, dst, grpReindexing[*data]);
      dst += (*screenBitmap)->width;
    }
  }

  // for the Remapping draw function, decoding becomes:
  GRP_DECODE_LINE(frame, grpBounds, GRP_RLE_TRANSPARENT, dst, remapTable[*data * 256 + dst[x]]);
  // where remapTable is the decompressed PCX palette

  // ripple effects do something related to dst[x + some value], though I don't know the specifics

  */


  // Draws GRP clipped to a mask GRP referenced by images.dat->specialOverlay which contains alpha blending data
  void __fastcall DrawFunc_Alpha(s32 screenX, s32 screenY, GrpFrame* frame, Box32* grpBounds, void* coloringData) {
    Box32 maskBounds;
    u32 maskFrame = (u32)(coloringData) & 0xFFFF;
    u32 imgID = (u32)(coloringData) >> 16;
    GrpHead* grp = imageGrpGraphics[imgID];
    GrpHead* maskgrp = (GrpHead*)(lo_files->specialOverlays[imgID]);
    if (maskgrp == NULL || maskgrp->width == 0) return;
    u32 grpFrame = ((u32)(frame) - (u32)(&(grp->frames[0]))) / sizeof(GrpFrame);

    // let maskFrame copy grpFrame
    if (maskFrame == 0xFFFF) maskFrame = grpFrame;

    if (maskFrame >= maskgrp->frameCount) maskFrame = 0;
    GrpFrame mask = maskgrp->frames[maskFrame];
    mask.dataOffset += (u32)maskgrp;
    
    // Update bounds rects
    screenX -= grpBounds->left;
    screenY -= grpBounds->top;
    if (calcMaskBounds(grpBounds, &maskBounds, grp, maskgrp, grpFrame, maskFrame, false) == false) return;
    screenX += grpBounds->left;
    screenY += grpBounds->top;

    u8* dst = (*screenBitmap)->data + (*screenBitmap)->width*screenY + screenX;
    u8 maskData[256];
    

    for (s32 y = 0; y < grpBounds->bottom; y++) {
      // decode mask
      GRP_DECODE_LINE(&mask, &maskBounds, GRP_RLE_BLACK, maskData, *data);

      // decode main GRP
      GRP_DECODE_LINE(frame, grpBounds, GRP_RLE_TRANSPARENT, dst, graphics::blend(grpReindexing[*data], dst[x], maskData[x]));

      dst += (*screenBitmap)->width;
    }
  }


  // Flipped version of alpha draw function
  void __fastcall DrawFunc_Alpha_F(s32 screenX, s32 screenY, GrpFrame* frame, Box32* grpBounds, void* coloringData) {
    Box32 maskBounds;
    u32 maskFrame = (u32)(coloringData) & 0xFFFF;
    u32 imgID = (u32)(coloringData) >> 16;
    GrpHead* grp = imageGrpGraphics[imgID];
    GrpHead* maskgrp = (GrpHead*)(lo_files->specialOverlays[imgID]);
    if (maskgrp == NULL || maskgrp->width == 0) return;
    u32 grpFrame = ((u32)(frame) - (u32)(&(grp->frames[0]))) / sizeof(GrpFrame);

    // let maskFrame copy grpFrame
    if (maskFrame == 0xFFFF) maskFrame = grpFrame;

    if (maskFrame >= maskgrp->frameCount) maskFrame = 0;
    GrpFrame mask = maskgrp->frames[maskFrame];
    mask.dataOffset += (u32)maskgrp; // correct GRP offset since game load only does it for main GRPs
    
    // Update bounds rects
    screenX -= grpBounds->left;
    screenY -= grpBounds->top;
    if (calcMaskBounds(grpBounds, &maskBounds, grp, maskgrp, grpFrame, maskFrame, true) == false) return;
    screenX += grpBounds->left;
    screenY += grpBounds->top;

    u8* dst = (*screenBitmap)->data + (*screenBitmap)->width*screenY + screenX;
    u8 maskData[256];
    

    for (s32 y = 0; y < grpBounds->bottom; y++) {
      // decode mask
      GRP_DECODE_LINE_FLIP(&mask, &maskBounds, GRP_RLE_BLACK, maskData, *data);

      // decode main GRP
      GRP_DECODE_LINE_FLIP(frame, grpBounds, GRP_RLE_TRANSPARENT_FLIP, dst, graphics::blend(grpReindexing[*data], dst[x], maskData[x]));
      
      dst += (*screenBitmap)->width;
    }
  }


  // Helper function for alpha drawing -- calculates the intersected area of grpBounds and mask frame
  // Returns true if such an area is nonzero, false if there is no intersection (nothing to draw)
  bool calcMaskBounds(Box32* grpBounds, Box32* maskBounds, GrpHead* grp, GrpHead* mask, u32 grpFrame, u32 maskFrame, bool flip) {
    s32 maskFrameX, maskFrameY;
    s32 intersectL, intersectR;
    s32 intersectU, intersectD;

    // Mask frame position relative to top left corner of grp frame
    if (flip) {
      maskFrameX = grp->frames[grpFrame].width - mask->frames[maskFrame].width + grp->frames[grpFrame].x - mask->frames[maskFrame].x + (mask->width - grp->width) / 2;
    }else{
      maskFrameX = (mask->frames[maskFrame].x - grp->frames[grpFrame].x) + (grp->width - mask->width) / 2;
    }
    maskFrameY = (mask->frames[maskFrame].y - grp->frames[grpFrame].y) + (grp->height - mask->height) / 2;

    grpBounds->right += grpBounds->left;
    grpBounds->bottom += grpBounds->top;

    // Intersection rectangle between GRP frame and mask frame
    intersectL = MAX(maskFrameX, grpBounds->left);
    intersectR = MIN(maskFrameX + mask->frames[maskFrame].width, grpBounds->right);
    intersectU = MAX(maskFrameY, grpBounds->top);
    intersectD = MIN(maskFrameY + mask->frames[maskFrame].height, grpBounds->bottom);

    // No intersection exists -- image is fully masked out
    if (intersectL >= intersectR || intersectU >= intersectD) return false;

    // Restrict GRP bounds to intersection rectangle
    grpBounds->left = MAX(grpBounds->left, intersectL);
    grpBounds->right = MIN(grpBounds->right, intersectR) - grpBounds->left;
    grpBounds->top = MAX(grpBounds->top, intersectU);
    grpBounds->bottom = MIN(grpBounds->bottom, intersectD) - grpBounds->top;

    // Restrict mask bounds to intersection rectangle
    maskBounds->left = MAX(grpBounds->left - maskFrameX, 0);
    maskBounds->right = MIN(grpBounds->right, mask->frames[maskFrame].width);
    maskBounds->top = MAX(grpBounds->top - maskFrameY, 0);
    maskBounds->bottom = MIN(grpBounds->bottom, mask->frames[maskFrame].height);

    // Final bounds check, just to be safe
    if (grpBounds->right < 0 || grpBounds->bottom < 0 || maskBounds->right < 0 || maskBounds->bottom < 0) return false;
    return true;
  }

}; // hooks

