#include "grafting.h"
#include <SCBW/Storm.h>
#include <hook_tools.h>

// For scbwdata.h
BUTTON_SET* buttonSetTable = (BUTTON_SET*)(0x005187E8);
UnitStatFuncArrayEntry* unitStatFuncs = (UnitStatFuncArrayEntry*)(0x005193A0);
u32 orderDatReqScript = 0x00514CF8;

namespace {

  const u32 load_offsets_unit[] = {
    0,  0x0046E134 + 4, 0x0046E19A + 4, 0x0046E2D4 + 4, 0x0046E382 + 4, 0x0046E560 + 4, 0x0046E58F + 4, 0x0048E247 + 4,  0,
    2,  0x0046E100 + 3, 0x0046E13E + 4, 0x0046E1A6 + 4, 0x0046E320 + 4, 0x0046E4DB + 4, 0x0046E51B + 4, 0x0046E56C + 4, 0x0048E253 + 4,  0,
    0,0
  };
  const u32 load_offsets_upgrade[] = {
    0,  0x0046D5D2 + 4, 0x0046E0C6 + 1,  0,
    2,  0x0046D5A0 + 3, 0x0046D5DC + 4,  0,
    0,0
  };
  const u32 load_offsets_research[] = {
    0,  0x0046D564 + 4, 0x0046DF95 + 1,  0,
    2,  0x0046D530 + 3, 0x0046D56E + 4,  0,
    0,0
  };
  const u32 load_offsets_useTech[] = {
    0,  0x0046D4F4 + 4, 0x0046DE75 + 1,  0,
    2,  0x0046D4C0 + 3, 0x0046D4FE + 4,  0,
    0,0
  };
  const u32 load_offsets_order[] = {
    0,  0x0042961F + 1, 0x0046D484 + 4, 0x0046DD6C + 1, 0x004C11AB + 1, 0x004C139B + 1, 0x004C158B + 1, 0x004C1797 + 1, 0x004C1E12 + 1, 0x004C22C4 + 1, 0x004CC591 + 1,  0,
    2,  0x0046D450 + 3, 0x0046D48E + 4,  0,
    0,0
  };
  const u32 load_offsets_buttons[] = {
    0,  0x004591ED + 3,  0,
    4,  0x00458C51 + 2, 0x00458C5A + 2,  0,
    8,  0x00458C62 + 3, 0x00458C6B + 3,  0,
    0,0
  };
  const u32 load_offsets_stat[] = {
    4,  0x00458182 + 3,  0,
    8,  0x0045819F + 3,  0,
    0,0
  };

  const u8 buttonReqParamOpcode[] = { 0x90, 0x66, 0x8B }; // 0x4F, 0x0C }; // MOV CX,WORD PTR DS[EDI+C]

  void loadData(void* pointer, const u32* load_offsets) {
    u32 value;
    for (u32 i = 0; load_offsets[i] != 0 || load_offsets[i + 1] != 0; i++) {
      value = (u32)(pointer) + load_offsets[i];
      for (i++; load_offsets[i] != 0; i++) {
        memoryPatch(load_offsets[i], value);
      }
    }
  }

};

namespace hooks {

  void injectGraftingData(void* graftData, u32 type) {
    if (graftData == NULL) return;
    switch (type) {
    case GraftType::units_req:
      loadData(graftData, load_offsets_unit);
      break;

    case GraftType::upgrades_req:
      loadData(graftData, load_offsets_upgrade);
      break;

    case GraftType::research_req:
      loadData(graftData, load_offsets_research);
      break;

    case GraftType::tech_req:
      loadData(graftData, load_offsets_useTech);
      break;

    case GraftType::orders_req:
      loadData(graftData, load_offsets_order);
      orderDatReqScript = (u32)(graftData);
      break;

    case GraftType::buttons:
      loadData(graftData, load_offsets_buttons);
      buttonSetTable = (BUTTON_SET*)graftData;
      
      // Replace MOVZX BYTE with MOV WORD
      memoryPatch(0x0045925C, buttonReqParamOpcode, 3);
      break;

    case GraftType::statInfo:
      loadData(graftData, load_offsets_stat);
      unitStatFuncs = (UnitStatFuncArrayEntry*)graftData;
      break;
    }
  }

}
