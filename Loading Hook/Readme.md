Provides a hook that is called once during Starcraft's loading screen, after GRPs and other regular files have already been loaded (at the end of function 0x004DAF30).

This allows for loading MPQ data before map load but after plugin initialization.


### To Install
- Put the files somewhere in your GPTP project (e.g. \/hooks\/main\/ alongside game_hooks)
- Include `loading_hook.h` and call `hooks::injectLoadingHook();` in initialize.cpp
