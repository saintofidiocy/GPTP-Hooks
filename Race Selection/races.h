#pragma once
#include "../SCBW/api.h"

typedef struct {
  u8 raceId; // Race ID
  u8 network_tblId; // 1-based network.tbl string index
} RaceTable;

namespace hooks {
  u32 getRandomRace(u32 index, u32 count);
  void loadMapRaces();

	void injectCustomizedRaces();
 
} //hooks
