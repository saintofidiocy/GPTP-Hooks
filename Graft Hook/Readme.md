Loads .graft files from the MPQ to modify .dat requirement scripts and unit button information as a replacement for FireGraft.

**This is still a work in progress, so file names and formats may change.**

MPQ Files:  
`DatReq\Buttons.graft` -- Button & Unit Information  
`DatReq\Units.graft` -- Unit Requirements  
`DatReq\Upgrades.graft` -- Upgrade Requirements  
`DatReq\Research.graft` -- Tech Research Requirements  
`DatReq\Tech.graft` -- Tech Use Requirements  
`DatReq\Orders.graft` -- Order Requirements  
All of these files are optional, and each patch will only be applied if the associated file exists.


### To Install
- Install [Storm Functions](https://github.com/saintofidiocy/GPTP-Hooks/tree/main/Storm%20Functions) and [Loading Hook](https://github.com/saintofidiocy/GPTP-Hooks/tree/main/Loading%20Hook) first
- Merge the GPTP directory with your project (modifications to base GPTP files are listed below) 
- Apart from Loading Hook, there is nothing to be added to initialize.cpp as changes are made when each file is loaded

Alternatively, start with the [DatExtender](https://github.com/saintofidiocy/GPTP/tree/DatExtender) fork of GPTP which comes with the Graft Hook already integrated.
  

#### Modifications to Base GPTP Files
Changes to base GPTP files done by this hook are listed here to aid in merging if your plugin has already modified any of these files, your plugin contains something requiring similar modifications, or you are attempting to merge with a newer version of GPTP.
- **GPTP\SCBW\scbwdata.h**:
  - Now contains `unitStatFuncs` definition from stats_display_main.cpp
  - Defines `orderDatReqScript` for use in CMDRECV_LiftOff.cpp and CMDRECV_Stop.cpp
  - Both of these values plus `buttonSetTable` are set to extern so that grafting_inject.cpp may modify them
- **GPTP\SCBW\structures.h**:
  - Now contains `UnitStatFuncArrayEntry` struct from stats_display_main.cpp
- **GPTP\hooks\interface\status_display\stats_display_main.cpp**:
  - `UnitStatFuncArrayEntry` struct is removed and now in structures.h
  - `unitStatFuncs` definition is removed and now in scbwdata.h
- **GPTP\hooks\recv_commands\CMDRECV_LiftOff.cpp** and **GPTP\hooks\recv_commands\CMDRECV_Stop.cpp**:
  - All instances of the constant `0x00514CF8` have been replaced with `orderDatReqScript`
  

### Creating .graft Files
Until PyGRAFT is made, the graft compiler can be used to convert FireGraft patches into .graft files.
1. Make your desired changes in FireGraft and save the project. There is no need to add any plugins or MPQ data.
2. Open the FireGraft mod in an MPQ editor and extract the `Firegraft\[mod name].fgp` file.
3. Drag & Drop the .fgp file on `datext fg helper.bat`, which will create .txt file scripts.
4. Confirm that the .txt files accurately contain the correct modifications. *Note that FG EXE edits are not supported and will not be reflected anywhere.*
5. Run `datext fg helper.bat` by itself, enter `C` to compile, then leave unit count blank (unless this is a DatExt mod) and hit Enter.
6. Import the resulting .graft files to `DatReq\` in your final MPQ to be loaded by the grafting hook.
