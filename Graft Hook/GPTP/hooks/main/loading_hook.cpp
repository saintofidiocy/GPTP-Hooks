#include "loading_hook.h"
#include <SCBW/api.h>
#include <cstdio>
#include "hooks/grafting/grafting.h"

namespace hooks {

// Called the end of the data loading function, after GRPs and such are loaded
void loadingData() {
  // load some mpq files, or whatever
  
  // Load graft patch files -- can be called in gameOn to re-load data if it gets modified in-game
  loadReqGrafts();
  loadButtonGrafts();
}

} //hooks
