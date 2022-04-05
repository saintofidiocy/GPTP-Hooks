This hook provides a way for defining custom images.dat drawing functions, and comes with a draw function to enable full alpha transparency using a GRP in Images.dat->specialOverlay as the alpha layer.

### To Install
- Copy the contents of \/hooks\/ to your GPTP project.
- Merge the \/SCBW\/ directory with your project (modifications to base GPTP files are listed below)
- Include `hooks/draw_funcs.h` and add `hooks::injectDrawFuncHook();` to initialize.cpp.
- If you want the included alpha transparency drawing function, install [Alpha Blending](https://github.com/saintofidiocy/GPTP-Hooks/tree/main/Alpha%20Blending)
- If you do not want the alpha transparency draw function:
  - In draw_funcs.cpp, remove `#include <graphics/alpha_blend.h>` and delete/comment `DrawFunc_Alpha` and `DrawFunc_Alpha_F`
  - In draw_funcs_inject.cpp, remove the associated lines from `imgUpdateFuncs` and `imgDrawFuncs`

### Creating New Drawing Functions
1. Copy either the Alpha or Normal draw functions in draw_funcs.cpp (normal draw is included in a comment)
2. If needed, create an update function (the function type definition is in draw_funcs.h) to update the image right before the iscript parser is called (typically used to decrement timers in coloringData)
3. Add the function prototypes to draw_funcs.h
4. In draw_funcs_inject.cpp, add your update function (or `UpdateFunc_Null` for none) to a new entry in `imgUpdateFuncs`
5. Similarly, add your draw functions to a new entry in `imgDrawFuncs` (needs a normal and flipped function, but they can point to the same function for no flipping)


#### Modifications to Base GPTP Files
Changes to base GPTP files done by this hook are listed here to aid in merging if your plugin has already modified any of these files or you are attempting to merge with a newer version of GPTP.
- **GPTP\SCBW\structures.h**:
  - `GrpFrame` members need to be u8 instead of s8
- **GPTP\SCBW\structures\CImage.cpp**:
  - Includes `hooks/draw_funcs.h`
  - `void CImage::initializeData` now calls `hooks::initializeColoringData(this);` instead of setting the coloringData itself
