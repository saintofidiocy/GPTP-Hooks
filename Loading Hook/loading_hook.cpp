#include "loading_hook.h"
#include <SCBW/api.h>
#include <cstdio>

namespace hooks {

// Called at the end of init_program (0x004DAF30): MPQs are open, but images.dat GRPs are not loaded yet
void loadingData() {
  // load some mpq files, or whatever
  
}

} //hooks
