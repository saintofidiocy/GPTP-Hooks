#include "draw_funcs.h"
#include <hook_tools.h>

namespace {

// Default update function pointers
const UpdateFunction UpdateFunc_Null      = (UpdateFunction)(0x004D55F0);
const UpdateFunction UpdateFunc_Cloak     = (UpdateFunction)(0x004D5D70);
const UpdateFunction UpdateFunc_Decloak   = (UpdateFunction)(0x004D5D10);
const UpdateFunction UpdateFunc_WarpFlash = (UpdateFunction)(0x004D8590);

// Default draw function pointers
const DrawFunction DrawFunc_Normal        = (DrawFunction)(0x0040ABBE); // 0_0
const DrawFunction DrawFunc_Normal_F      = (DrawFunction)(0x0040BF60); // 0_1
const DrawFunction DrawFunc_EnCloaking    = (DrawFunction)(0x004D55B0); // 2_0
const DrawFunction DrawFunc_EnCloaking_F  = (DrawFunction)(0x004D5570); // 2_1
const DrawFunction DrawFunc_EnCloaked     = (DrawFunction)(0x0040B155); // 3_0
const DrawFunction DrawFunc_EnCloaked_F   = (DrawFunction)(0x0040B9A9); // 3_1
const DrawFunction DrawFunc_Cloaking      = (DrawFunction)(0x004D5530); // 5_0
const DrawFunction DrawFunc_Cloaking_F    = (DrawFunction)(0x004D54F0); // 5_1
const DrawFunction DrawFunc_Cloaked       = (DrawFunction)(0x004D54D0); // 6_0
const DrawFunction DrawFunc_Cloaked_F     = (DrawFunction)(0x004D54B0); // 6_1
const DrawFunction DrawFunc_EMPEffect     = (DrawFunction)(0x0040B2D3); // 8_0
const DrawFunction DrawFunc_EMPEffect_F   = (DrawFunction)(0x0040BB2E); // 8_1
const DrawFunction DrawFunc_Remapping     = (DrawFunction)(0x0040B596); // 9_0
const DrawFunction DrawFunc_Remapping_F   = (DrawFunction)(0x0040BCA3); // 9_1
const DrawFunction DrawFunc_Shadow        = (DrawFunction)(0x0040B6F0); // 10_0
const DrawFunction DrawFunc_Shadow_F      = (DrawFunction)(0x0040C0AE); // 10_1
const DrawFunction DrawFunc_HPBar         = (DrawFunction)(0x0047A820); // 11
const DrawFunction DrawFunc_WarpTex       = (DrawFunction)(0x004D5CD0); // 12_0
const DrawFunction DrawFunc_WarpTex_F     = (DrawFunction)(0x004D5C90); // 12_1
const DrawFunction DrawFunc_SelCircle     = (DrawFunction)(0x004D5390); // 13
const DrawFunction DrawFunc_PColor        = (DrawFunction)(0x004D61A0); // 14_0
const DrawFunction DrawFunc_PColor_F      = (DrawFunction)(0x004D6120); // 14_1
const DrawFunction DrawFunc_UpdateRect    = (DrawFunction)(0x004D5280); // 15
const DrawFunction DrawFunc_Halluc        = (DrawFunction)(0x004D5210); // 16_0
const DrawFunction DrawFunc_Halluc_F      = (DrawFunction)(0x004D51A0); // 16_1
const DrawFunction DrawFunc_WarpFlash     = (DrawFunction)(0x004D5480); // 17_0
const DrawFunction DrawFunc_WarpFlash_F   = (DrawFunction)(0x004D5450); // 17_1


// Update functions get called right before an image's iscript to update CImage->coloringData
const struct {
 u32 id;
 UpdateFunction func;
} imgUpdateFuncs[] = {
  { PaletteType::RLE_NORMAL,       UpdateFunc_Null },
  { PaletteType::RLE_NOSHIFT,      UpdateFunc_Null },
  { PaletteType::RLE_ENEMYCLOAK,   UpdateFunc_Cloak },
  { PaletteType::RLE_ENEMYCLOAKED, UpdateFunc_Null },
  { PaletteType::RLE_ENEMYDECLOAK, UpdateFunc_Decloak },
  { PaletteType::RLE_CLOAK,        UpdateFunc_Cloak },
  { PaletteType::RLE_CLOAKED,      UpdateFunc_Null },
  { PaletteType::RLE_DECLOAK,      UpdateFunc_Decloak },
  { PaletteType::RLE_EMP,          UpdateFunc_Null },
  { PaletteType::RLE_EFFECT,       UpdateFunc_Null },
  { PaletteType::RLE_SHADOW,       UpdateFunc_Null },
  { PaletteType::RLE_HPFLOATDRAW,  UpdateFunc_Null },
  { PaletteType::RLE_WARP,         UpdateFunc_Null },
  { PaletteType::RLE_OUTLINE,      UpdateFunc_Null },
  { PaletteType::RLE_PLAYER_SIDE,  UpdateFunc_Null },
  { PaletteType::RLE_RECT,         UpdateFunc_Null },
  { PaletteType::RLE_SHIFT,        UpdateFunc_Null },
  { PaletteType::RLE_FIRE,         UpdateFunc_WarpFlash },

// ----- CUSTOM DRAW FUNCTIONS -----
  { 18, UpdateFunc_Null },
};


// Drawing functions which actually render & process the grp
const struct {
 u32 id;
 DrawFunction func;
 DrawFunction flip;
} imgDrawFuncs[] = {
  { PaletteType::RLE_NORMAL,       DrawFunc_Normal,     DrawFunc_Normal_F     },
  { PaletteType::RLE_NOSHIFT,      DrawFunc_Normal,     DrawFunc_Normal_F     },
  { PaletteType::RLE_ENEMYCLOAK,   DrawFunc_EnCloaking, DrawFunc_EnCloaking_F },
  { PaletteType::RLE_ENEMYCLOAKED, DrawFunc_EnCloaked,  DrawFunc_EnCloaked_F  },
  { PaletteType::RLE_ENEMYDECLOAK, DrawFunc_EnCloaking, DrawFunc_EnCloaking_F },
  { PaletteType::RLE_CLOAK,        DrawFunc_Cloaking,   DrawFunc_Cloaking_F   },
  { PaletteType::RLE_CLOAKED,      DrawFunc_Cloaked,    DrawFunc_Cloaked_F    },
  { PaletteType::RLE_DECLOAK,      DrawFunc_Cloaking,   DrawFunc_Cloaking_F   },
  { PaletteType::RLE_EMP,          DrawFunc_EMPEffect,  DrawFunc_EMPEffect_F  },
  { PaletteType::RLE_EFFECT,       DrawFunc_Remapping,  DrawFunc_Remapping_F  },
  { PaletteType::RLE_SHADOW,       DrawFunc_Shadow,     DrawFunc_Shadow_F     },
  { PaletteType::RLE_HPFLOATDRAW,  DrawFunc_HPBar,      DrawFunc_HPBar        }, // no flip
  { PaletteType::RLE_WARP,         DrawFunc_WarpTex,    DrawFunc_WarpTex_F    },
  { PaletteType::RLE_OUTLINE,      DrawFunc_SelCircle,  DrawFunc_SelCircle    }, // no flip
  { PaletteType::RLE_PLAYER_SIDE,  DrawFunc_PColor,     DrawFunc_PColor_F     },
  { PaletteType::RLE_RECT,         DrawFunc_UpdateRect, DrawFunc_UpdateRect   }, // no flip
  { PaletteType::RLE_SHIFT,        DrawFunc_Halluc,     DrawFunc_Halluc_F     },
  { PaletteType::RLE_FIRE,         DrawFunc_WarpFlash,  DrawFunc_WarpFlash_F  },

// ----- CUSTOM DRAW FUNCTIONS -----
  { 18, hooks::DrawFunc_Alpha, hooks::DrawFunc_Alpha_F },
};


// Function table access pointers (don't modify these)
u32 updatePtrs[] = {
  0,  0x00497E98 + 2, 0x00497EFD + 3, 0x00498A46 + 3, 0x00498AC7 + 3, 0x00498B9D + 3, 0x00499302 + 3, 0x004D562A + 3, 0x004D58B6 + 3, 0x004D5973 + 3, 0x004D5D33 + 1, 0x004D66C4 + 3,  0,
  2,  0x004980B4 + 2, 0x00498115 + 2, 0x004D59C4 + 2,  0,
  3,  0x00498014 + 2,  0,
  4,  0x00498065 + 2,  0,
  6,  0x00497F44 + 2,  0,
  11, 0x004D68F8 + 2,  0,
  12, 0x004D854F + 1,  0,
  13, 0x004D686B + 1,  0,
  15, 0x004D5B33 + 2, 0x004D6A8D + 2, 0x004D6B6B + 2, 0x004D6BF0 + 2, 0x004D6C81 + 2,  0,
  16, 0x00497DD7 + 2, 0x004D6E46 + 1,  0,
  0
};
u32 drawPtrs[] = {
  0,  0x00497EB0 +2, 0x00497F1C +3, 0x00498A65 +3, 0x00498AE5 +3, 0x00498BB5 +3, 0x0049931A +3, 0x004D561E +3, 0x004D58D5 +3, 0x004D5992 +3, 0x004D5D49 +2, 0x004D66E3 +3,  0,
      0x00497EA8 +2, 0x00497F13 +3, 0x00498A5C +3, 0x00498ADC +3, 0x00498BAC +3, 0x00499311 +3, 0x004D5615 +3, 0x004D58CC +3, 0x004D5989 +3, 0x004D5D41 +2, 0x004D66DA +3,  0,
  2,  0x004980C5 +2, 0x0049812D +2, 0x004D59DC +2,  0,
      0x004980BD +2, 0x00498125 +2, 0x004D59D4 +2,  0,
  3,  0x0049802C +2,  0,
      0x00498024 +2,  0,
  4,  0x00498076 +2,  0,
      0x0049806E +2,  0,
  6,  0x00497F5C +2,  0,
      0x00497F54 +2,  0,
  11, 0x004D690A +2,  0,
      0x004D6901 +2,  0,
  12, 0x004D8565 +2,  0,
      0x004D855D +2,  0,
  13, 0x004D6881 +2,  0,
      0x004D6879 +2,  0,
  15, 0x004D5B44 +2, 0x004D6AF4 +2, 0x004D6B97 +2, 0x004D6C1C +2, 0x004D6CBF +2,  0,
      0x004D5B3C +2, 0x004D6AE6 +2, 0x004D6B7C +2, 0x004D6C01 +2, 0x004D6C95 +2,  0,
  16, 0x00497DEF +2, 0x004D6E5C +2,  0,
      0x00497DE7 +2, 0x004D6E54 +2,  0,
  0
};


// End of CImage::initializeData @ 0x004D5A50
void __declspec(naked) initializeColoringData_Wrapper() {
  static CImage* image;
  __asm {
    MOV image, EAX
    PUSHAD
  }
  hooks::initializeColoringData(image);
  __asm {
    POPAD
    POP EBX
    POP EBP
    RETN 8
  }
}

};


namespace hooks {

  void injectDrawFuncHook() {
    u32 i, id;

    // update functions
    for (i = 0; (id = updatePtrs[i]) != 0 || i == 0; i++) {
      for (i++; updatePtrs[i] != 0; i++) memoryPatch(updatePtrs[i], &(imgUpdateFuncs[id].func));
    }

    // draw functions
    for (i = 0; (id = drawPtrs[i]) != 0 || i == 0; i++) {
      for (i++; drawPtrs[i] != 0; i++) memoryPatch(drawPtrs[i], &(imgDrawFuncs[id].func));
      for (i++; drawPtrs[i] != 0; i++) memoryPatch(drawPtrs[i], &(imgDrawFuncs[id].flip));
    }

    jmpPatch(initializeColoringData_Wrapper, 0x004D5ACC);
  }

}; // hooks