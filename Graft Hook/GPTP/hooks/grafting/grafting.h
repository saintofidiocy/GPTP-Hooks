#pragma once
#include <SCBW/api.h>

typedef u16 REQ_SCRIPT;

namespace GraftType {
  enum Enum : u32 {
    units_req,
    upgrades_req,
    research_req,
    tech_req,
    orders_req,
    buttons,
    statInfo,
  };
};

namespace hooks {

  void loadReqGrafts();
  void loadButtonGrafts();

  void injectGraftingData(void* graftData, u32 type);
};
