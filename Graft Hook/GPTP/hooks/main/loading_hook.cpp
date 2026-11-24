#include "loading_hook.h"
#include <SCBW/api.h>
#include <cstdio>
#include "hooks/grafting/grafting.h"

namespace hooks {

// Called at the end of init_program (0x004DAF30): MPQs are open, but images.dat GRPs are not loaded yet
void loadingData() {
  // load some mpq files, or whatever
  
  // Load graft patch files -- can be called in gameOn to re-load data if it gets modified in-game
  loadReqGrafts();
  loadButtonGrafts();
}

} //hooks
