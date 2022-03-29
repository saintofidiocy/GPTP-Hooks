Provides a (relatively) quick way to do RGB color blending & palette matching. This may lag if there are lots of blended graphics on screen, but for basic effects or a reasonable number of images it should work fine.

Originally this used a k-d tree for the color lookup, but that proved to be even slower. Instead, it divides up the RGB colorspace into small chunks and generates a sub-list of colors for each chunk to reduce the number of color comparisons required. An alternative method is to keep a 16777216-byte array to cache the color lookups, which would indeed be much faster during rendering but also use a lot of memory.

### To Install
- Put the files in in the \/graphics\/ directory of your GPTP project (or wherever you would like)
- Include `graphics/alpha_blend.h` in \/hooks\/main\/game_hooks.cpp and call `graphics::initPalData();` in GameOn() to initialize the palette lookup data
- Call `blend` or `getColorId` as desired
