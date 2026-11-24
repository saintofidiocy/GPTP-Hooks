Provides a hook that is called once during Starcraft's startup, at the end of function 0x004DAF30 (init_program). At that point the MPQs are open, but the video and the images.dat GRPs are not loaded yet; those are loaded later by 0x004E0820 (0x004DB060 and 0x004D7390).

This allows for loading MPQ data before map load but after plugin initialization.


### To Install
- Put the files somewhere in your GPTP project (e.g. \/hooks\/main\/ alongside game_hooks)
- Include `loading_hook.h` and call `hooks::injectLoadingHook();` in initialize.cpp
