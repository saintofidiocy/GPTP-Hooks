#include "races.h"
#include "../hook_tools.h"

namespace hooks {
  // values from races.cpp
  extern RaceTable CustomRaceList[];
  extern u32 CustomRaceCount;
  extern u8 RandomRaceList[];
  extern u32 RandomRaceMax;
} // hooks

namespace {
  
  void __declspec(naked) GetRandomRace_Wrapper() {
    static u32 index;
    static u32 count;
    static u32 race;
    __asm {
      MOV index,EBX
      MOV ECX,[ESP+4]
      MOV count,ECX
      PUSHAD
    }
    race = hooks::getRandomRace(index, count);
    __asm {
      POPAD
      MOV EAX,race
      RETN 4
    }
  }

  const u32 Func_RandBetween = 0x004DC550;
  void __declspec(naked) randomizeRace() {
    __asm {
      PUSH EBP
      MOV EBP, ESP
      MOV EAX, hooks::RandomRaceMax
      PUSH EAX
      MOV EAX, 0x90
      CALL Func_RandBetween
      MOVZX EAX, [hooks::RandomRaceList + EAX]
      POP EBP
      RETN 4
    }
  }



  const u32 loadMapRaces_Return = 0x004BF6EA;
  void __declspec(naked) loadMapRaces_Wrapper() {
    __asm {
      PUSHAD
    }
    hooks::loadMapRaces();
    __asm {
      POPAD
      JMP loadMapRaces_Return
    }
  }

} //unnamed namespace

namespace hooks {

  const u32 gluChat_RaceDropdown_TableRef = 0x00450B31 + 1;
  const u32 gluChat_RaceDropdown_TableRefEnd = 0x00450B9F + 2;
  const u32 gluCustm_RaceDropdown_TableRef = 0x004ADF36 + 1;
  const u32 gluCustm_RaceDropdown_TableRefEnd = 0x004ADF8B + 2;
  void injectCustomizedRaces() {
    u32 TableStart = (u32)(CustomRaceList);
    u32 TableEnd = TableStart + CustomRaceCount * sizeof(RaceTable);
    memoryPatch(gluChat_RaceDropdown_TableRef, TableStart);
    memoryPatch(gluChat_RaceDropdown_TableRefEnd, TableEnd);
    memoryPatch(gluCustm_RaceDropdown_TableRef, TableStart);
    memoryPatch(gluCustm_RaceDropdown_TableRefEnd, TableEnd);
    
    callPatch(randomizeRace, 0x004A9AB7);
    callPatch(GetRandomRace_Wrapper, 0x004A9AC5);
    jmpPatch(loadMapRaces_Wrapper, 0x004BF6A1);
  }

} //hooks
