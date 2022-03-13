General Storm.dll function definitions, since GPTP is notably lacking them.

Also includes a wrapper for FastFileRead to simplify reading MPQ files.


### To Install
- Put somewhere in your GPTP project (I put mine in \/SCBW\/) and include `Storm.h` wherever necessary
- There is nothing to put in initialize.cpp as the functions are self-initializing.
