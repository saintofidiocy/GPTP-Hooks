#include "loading_hook.h"
#include <hook_tools.h>

/******** GAME LOADING HOOK ********/

const u32 Hook_loadingDataReturn = 0x004DB056;
void __declspec(naked) loadingDataHook() {

  __asm {
    PUSHAD
  }
  hooks::loadingData();
  __asm {
    POPAD
    RETN
  }
};

//-------- Hook injector --------//

namespace hooks {

void injectLoadingHook() {
  jmpPatch(loadingDataHook, Hook_loadingDataReturn);
}

} //hooks
