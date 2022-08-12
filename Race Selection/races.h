#pragma once
#include "../SCBW/api.h"

typedef struct {
  u8 raceId; // Race ID
  u8 network_tblId; // 1-based network.tbl string index
} RaceTable;

namespace hooks {

  // used to access virtual races elsewhere
  u32 getVirtualRace(u8 player);

  void setVirtualRaces();
  void playerShuffle(u8 newID, u8 oldID);
  u32 getRandomRace(u32 index, u32 count);
  void loadMapRaces();
  bool isValidRace(u8 raceID);

	void injectCustomizedRaces();
 
} //hooks
