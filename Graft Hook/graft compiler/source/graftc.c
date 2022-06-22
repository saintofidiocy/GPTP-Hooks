#include <stdio.h>
#include "genscripts.h"


// Opcode & Function Definitions
OPCODE opcodes[256] = {0};
BUTTON_FUNC* buttonConds = NULL;
BUTTON_FUNC* buttonActs = NULL;
BUTTON_FUNC* displayFuncs = NULL;
BUTTON_FUNC* statusFuncs = NULL;
STAT_PRESET* statPresets = NULL;
u32 buttonCondCount = 0;
u32 buttonActCount = 0;
u32 displayFuncCount = 0;
u32 statusFuncCount = 0;
u32 statPresetCount = 0;

// String and other buffers
char* strData[32] = {0};
u32 strDataCount = 0;

u8* FGData = NULL;
u32 FGDataSize = 0;

// String tables
char** unitNames = NULL;
char** techNames = NULL;
char** upgdNames = NULL;
char** orderNames = NULL;
char** iconNames = NULL;
char** bsetNames = NULL;

// string counts
u32 unitStrCount = 0;
u32 techStrCount = 0;
u32 upgdStrCount = 0;
u32 orderStrCount = 0;
u32 iconStrCount = 0;
u32 bsetStrCount = 0;

// req data
u16** units_req = NULL;
u16** upgrades_req = NULL;
u16** research_req = NULL;
u16** tech_req = NULL;
u16** orders_req = NULL;

// button data
BUTTON_SET* buttonSets = NULL; // connectedUnit == if it's from FGData
BUTTON_SET* unitButtons = NULL;
STATFUNC* unitStats = NULL;

// default counts
u32 unitCount = 250;
u32 statCount = 228;
u32 techCount = 44;
u32 upgdCount = 61;
u32 orderCount = 189;
u32 bsetCount = 98;
u32 bsetMax = 98*2; // FG doesn't seem to save the number of buttonsets anywhere, so add some extra space



void saveButtons(const char* filename);
void saveReq(const char* filename, u16** reqData, u32 count, u8 availType);

void saveTXTButtons(const char* filename);
void saveTXTReq(const char* filename, const char* header, u16** reqData, u32 count, char** labels, u32 labelCount, u8 availType);

void loadSCData();
void loadSCButtons(FILE * f);
void loadSCReqList(FILE * f, u32 offset, u32 size, u16** reqData, u32 count, u8 availType);

void loadFGData(const char* filename);
void loadFGReq(const char* chunk, u16** reqData, u32 maxCount);
void loadFGStrings(const char* chunk, char** names, u32 maxCount);
u8* findFGChunk(const char* name, u32* size);

void parseTXTButtons(const char* filename);
void parseTXTReq(const char* filename, u16** reqData, u32 maxCount);

bool loadDefaultData();
void unloadData();
u8* loadFile(const char* filename, u32* r_size);
char** loadStringTable(const char* filename, u32* strCount);
u32 getLineCount(u8* buf);
u8* parseLine(u8* buf, u8 delim, u32* dataCount, char*** data);
u8 getParamType(char* str);
char* getIDName(u32 id, u32 type);

int main(int argc, char* argv[]){
  bool loadSC = false;
  u32 loadFG = 0;
  u32 loadArgs[6] = {0};
  u32 saveArgs[6] = {0};
  u32 counts[6] = {0};
  bool saveTXT = false;
  int i,j;
  
  
  if(argc == 1){
    puts("arguments:\n"
         "  -c          Compile mode (Outputs graft files)\n"
         "  -d          Decompile mode (Output txt files)\n"
         "  -s          Load Starcraft.exe data\n"
         "  -f  [path]  Load specified FGP file\n"
         "  -n? [num]   Sets number of IDs for each type (for datext files)\n"
         "  -i? [path]  Specifies input files for each type\n"
         "  -o? [path]  Specifies output files for each type\n"
         "\n"
         "Where '?' is one of:           Default Count:\n"
         "  b  Buttons                   250 (unit IDs with buttons beyond 228)\n"
         "  u  Units                     228\n"
         "  p  Upgrades                  61\n"
         "  t  Tech                      44\n"
         "  r  Research                  (Alias of Tech for counts)\n"
         "  o  Orders                    189\n"
         "\n"
         "Examples:\n"
         "graftc -d -f sample.fgp -ob Buttons.txt -ou Units.txt -op Upgrades.txt\n"
         "graftc -c -s -ib Buttons.txt -iu Units.txt -ob Buttons.graft -ou Buttons.graft\n");
    return 0;
  }
  
  // parse arguments
  for(i = 1; i < argc; i++){
    if(argv[i][0] != '-'){
      printf("Unknown argument '%s'\n", argv[i]);
      return 0;
    }
    switch(argv[i][1]){
      case 'c':
        saveTXT = false;
        break;
      case 'd':
        saveTXT = true;
        break;
      case 's':
        loadSC = true;
        break;
      case 'f':
        i++;
        loadFG = i;
        break;
      case 'n':
      {
        i++;
        switch(argv[i-1][2]){
          case 'b':
            counts[BUTTONS] = atoi(argv[i]);
            break;
          case 'u':
            counts[UNITS_REQ] = atoi(argv[i]);
            break;
          case 'p':
            counts[UPGRADE_REQ] = atoi(argv[i]);
            break;
          case 't':
          case 'r':
            counts[TECH_REQ] = atoi(argv[i]);
            break;
          case 'o':
            counts[ORDERS_REQ] = atoi(argv[i]);
            break;
          default:
            i--;
            printf("Unknown data type '%c'\n", argv[i][2]);
            break;
        }
        break;
      }
      case 'i':
      {
        i++;
        switch(argv[i-1][2]){
          case 'b':
            loadArgs[BUTTONS] = i;
            break;
          case 'u':
            loadArgs[UNITS_REQ] = i;
            break;
          case 'p':
            loadArgs[UPGRADE_REQ] = i;
            break;
          case 't':
            loadArgs[TECH_REQ] = i;
            break;
          case 'r':
            loadArgs[RESEARCH_REQ] = i;
            break;
          case 'o':
            loadArgs[ORDERS_REQ] = i;
            break;
          default:
            i--;
            printf("Unknown data type '%c'\n", argv[i][2]);
            break;
        }
        break;
      }
      case 'o':
      {
        i++;
        switch(argv[i-1][2]){
          case 'b':
            saveArgs[BUTTONS] = i;
            break;
          case 'u':
            saveArgs[UNITS_REQ] = i;
            break;
          case 'p':
            saveArgs[UPGRADE_REQ] = i;
            break;
          case 't':
            saveArgs[TECH_REQ] = i;
            break;
          case 'r':
            saveArgs[RESEARCH_REQ] = i;
            break;
          case 'o':
            saveArgs[ORDERS_REQ] = i;
            break;
          default:
            i--;
            printf("Unknown data type '%c'\n", argv[i][2]);
            break;
        }
        break;
      }
      default:
        printf("Unknown argument '%s'\n", argv[i]);
        return 0;
    }
  }
  
  // load function and ID names
  loadDefaultData();
  
  // Update counts
  if(counts[BUTTONS]     > unitCount) unitCount = counts[BUTTONS];
  if(counts[UNITS_REQ]   > statCount) statCount = counts[UNITS_REQ];
  if(counts[UPGRADE_REQ] > upgdCount) upgdCount = counts[UPGRADE_REQ];
  if(counts[TECH_REQ]    > techCount) techCount = counts[TECH_REQ];
  if(counts[ORDERS_REQ] > orderCount) orderCount = counts[ORDERS_REQ];
  
  // Allocate data
  units_req    = calloc(statCount, sizeof(u16*));
  upgrades_req = calloc(upgdCount, sizeof(u16*));
  research_req = calloc(techCount, sizeof(u16*));
  tech_req     = calloc(techCount, sizeof(u16*));
  orders_req   = calloc(orderCount, sizeof(u16*));
  buttonSets   = calloc(bsetMax, sizeof(BUTTON_SET));
  unitButtons  = calloc(unitCount, sizeof(BUTTON_SET));
  unitStats    = calloc(statCount, sizeof(STATFUNC));
  
  // Increase string counts
  if(unitCount > unitStrCount){
    unitNames = (char**)realloc(unitNames, unitCount * sizeof(char*));
    memset(&unitNames[unitStrCount], 0, (unitCount - unitStrCount) * sizeof(char*));
    unitStrCount = unitCount;
  }
  if(techCount > techStrCount){
    techNames = (char**)realloc(techNames, techCount * sizeof(char*));
    memset(&techNames[techStrCount], 0, (techCount - techStrCount) * sizeof(char*));
    techStrCount = techCount;
  }
  if(upgdCount > upgdStrCount){
    upgdNames = (char**)realloc(upgdNames, upgdCount * sizeof(char*));
    memset(&upgdNames[upgdStrCount], 0, (upgdCount - upgdStrCount) * sizeof(char*));
    upgdStrCount = upgdCount;
  }
  if(orderCount > orderStrCount){
    orderNames = (char**)realloc(orderNames, orderCount * sizeof(char*));
    memset(&orderNames[orderStrCount], 0, (orderCount - orderStrCount) * sizeof(char*));
    orderStrCount = orderCount;
  }
  if(bsetMax > bsetStrCount){
    bsetNames = (char**)realloc(bsetNames, bsetMax * sizeof(char*));
    memset(&bsetNames[bsetStrCount], 0, (bsetMax - bsetStrCount) * sizeof(char*));
    bsetStrCount = bsetMax;
  }
  
  
  // load data
  if(loadSC){
    puts("Loading SC data ...");
    loadSCData();
  }
  if(loadFG){
    puts("Loading FG data ...");
    loadFGData(argv[loadFG]);
  }
  
  // load txt files
  if(loadArgs[UNITS_REQ])    parseTXTReq(argv[loadArgs[UNITS_REQ]],    units_req,    statCount);
  if(loadArgs[UPGRADE_REQ])  parseTXTReq(argv[loadArgs[UPGRADE_REQ]],  upgrades_req, upgdCount);
  if(loadArgs[RESEARCH_REQ]) parseTXTReq(argv[loadArgs[RESEARCH_REQ]], research_req, techCount);
  if(loadArgs[TECH_REQ])     parseTXTReq(argv[loadArgs[TECH_REQ]],     tech_req,     techCount);
  if(loadArgs[ORDERS_REQ])   parseTXTReq(argv[loadArgs[ORDERS_REQ]],   orders_req,   orderCount);
  if(loadArgs[BUTTONS])      parseTXTButtons(argv[loadArgs[BUTTONS]]);
  
  
  // save data
  if(saveTXT){
    if(saveArgs[UNITS_REQ])    saveTXTReq(argv[saveArgs[UNITS_REQ]],    "Units",    units_req,    statCount,  unitNames,  unitStrCount, UNIT);
    if(saveArgs[UPGRADE_REQ])  saveTXTReq(argv[saveArgs[UPGRADE_REQ]],  "Upgrades", upgrades_req, upgdCount,  upgdNames,  upgdStrCount, NOT_UNIT);
    if(saveArgs[RESEARCH_REQ]) saveTXTReq(argv[saveArgs[RESEARCH_REQ]], "Research", research_req, techCount,  techNames,  techStrCount, NOT_UNIT);
    if(saveArgs[TECH_REQ])     saveTXTReq(argv[saveArgs[TECH_REQ]],     "Tech",     tech_req,     techCount,  techNames,  techStrCount, NOT_UNIT);
    if(saveArgs[ORDERS_REQ])   saveTXTReq(argv[saveArgs[ORDERS_REQ]],   "Orders",   orders_req,   orderCount, orderNames, orderStrCount, NOT_UNIT);
    if(saveArgs[BUTTONS])      saveTXTButtons(argv[saveArgs[BUTTONS]]);
  }else{
    if(saveArgs[UNITS_REQ])    saveReq(argv[saveArgs[UNITS_REQ]],    units_req,    statCount, UNIT);
    if(saveArgs[UPGRADE_REQ])  saveReq(argv[saveArgs[UPGRADE_REQ]],  upgrades_req, upgdCount, NOT_UNIT);
    if(saveArgs[RESEARCH_REQ]) saveReq(argv[saveArgs[RESEARCH_REQ]], research_req, techCount, NOT_UNIT);
    if(saveArgs[TECH_REQ])     saveReq(argv[saveArgs[TECH_REQ]],     tech_req,     techCount, NOT_UNIT);
    if(saveArgs[ORDERS_REQ])   saveReq(argv[saveArgs[ORDERS_REQ]],   orders_req,   orderCount, NOT_UNIT);
    if(saveArgs[BUTTONS])      saveButtons(argv[saveArgs[BUTTONS]]);
  }
  
  puts("Done :)");
  
  unloadData();
  return 0;
}




void saveButtons(const char* filename){
  u32 i,j;
  printf("Writing %s...\n", filename);
  FILE * fOut = fopen(filename, "wb");
  if(fOut == NULL){
    printf("Could not save to file %s\n", filename);
    return;
  }
  s32 butID;
  
  // Count buttons
  u32 buttCount = 0;
  for(i = 0; i < bsetCount; i++){
    if(buttonSets[i].padding == 0) continue; // nothing wrote to this ID
    if(buttCount >= 65535){
      puts("Thats a lotta buttons");
    }
    buttonSets[i].connectedUnit = buttCount;
    buttCount += buttonSets[i].buttonsInSet;
  }
  
  // Write header
  fwrite(&buttCount, sizeof(u32), 1, fOut);
  fwrite(&unitCount, sizeof(u32), 1, fOut);
  fwrite(&statCount, sizeof(u32), 1, fOut);
  // TODO: Anything else?
  
  
  // Write buttons
  for(i = 0; i < bsetCount; i++){
    if(buttonSets[i].padding == 0 || buttonSets[i].buttonsInSet == 0) continue; // nothing to write
    fwrite(buttonSets[i].firstButton, sizeof(BUTTON), buttonSets[i].buttonsInSet, fOut);
  }
  
  // Write units
  for(i = 0; i < unitCount; i++){
    butID = (s32)(unitButtons[i].firstButton);
    if(butID == -1 || butID >= bsetCount || buttonSets[butID].buttonsInSet == 0){
      unitButtons[i].buttonsInSet = 0;
      unitButtons[i].firstButton = NULL;
    }else{
      unitButtons[i].buttonsInSet = buttonSets[butID].buttonsInSet;
      unitButtons[i].firstButton = (BUTTON*)((u32)(buttonSets[butID].connectedUnit));
    }
    unitButtons[i].padding = 0;
  }
  fwrite(unitButtons, sizeof(BUTTON_SET), unitCount, fOut);
  
  // Write stat info
  fwrite(unitStats, sizeof(STATFUNC), statCount, fOut);
  
  // TODO: Strings?
  // TODO: Anything else?
  
  fclose(fOut);
}


void saveReq(const char* filename, u16** reqData, u32 count, u8 availType){
  u32 i,j;
  u16 v;
  bool sublist;
  u8 paramType;
  char pad[10];
  printf("Writing %s...\n", filename);
  FILE * fOut = fopen(filename, "wb");
  if(fOut == NULL){
    printf("Could not save to file %s\n", filename);
    return;
  }
  
  v = 0;
  fwrite(&v, sizeof(u16), 1, fOut); // idk if this is actually used for anything
  
  for(i = 0; i < count; i++){
    if(reqData[i] == NULL) continue;
    
    fwrite(&i, sizeof(u16), 1, fOut);
    
    sublist = false;
    j = 0;
    do {
      v = reqData[i][j++];
      fwrite(&v, sizeof(u16), 1, fOut);
      if(sublist || v != 0xFFFF){
        if(v == 0xFFFF){ // end of sublist
          sublist = false;
          v = 0; // so it doesn't exit the loop
        }else if(availType == NOT_UNIT && (v == 0xFF1F || v == 0xFF20)){
          sublist = true;
        }
      }
    } while(v != 0xFFFF);
  }
  
  v = 0xFFFF;
  fwrite(&v, sizeof(u16), 1, fOut);
  
  fclose(fOut);
}


void saveTXTButtons(const char* filename){
  u32 i,j;
  BUTTON* but;
  bool hasComment;
  printf("Writing %s...\n", filename);
  FILE * fOut = fopen(filename, "w");
  if(fOut == NULL){
    printf("Could not save to file %s\n", filename);
    return;
  }
  
  fputs("#### Button Sets ####\n\n", fOut);
  for(i = 0; i < bsetCount; i++){
    if(buttonSets[i].padding == 0) continue; // nothing wrote to this set
    
    if(i < bsetStrCount && bsetNames[i] != NULL){
      fprintf(fOut, "%d:\t# %s\n", i, bsetNames[i]);
    }else{
      fprintf(fOut, "%d:\n", i, bsetNames[i]);
    }
    if(buttonSets[i].buttonsInSet == 0){
      fputs("# No buttons\n\n", fOut);
      continue;
    }
    
    for(j = 0; j < buttonSets[i].buttonsInSet; j++){
      but = &(buttonSets[i].firstButton[j]);
      fprintf(fOut, "  %d, %d, %d, %d, %d, %d, %d, %d  # ", but->position, but->iconID, but->reqFunc, but->actFunc, but->reqVar, but->actVar, but->enableStr, but->disableStr);
      if(but->iconID < iconStrCount && iconNames[but->iconID] != NULL){
        fprintf(fOut, "%s, ", iconNames[but->iconID]);
      }
      fprintf(fOut, "%s (%s), ", buttonConds[but->reqFunc].name, getIDName(but->reqVar, buttonConds[but->reqFunc].type));
      fprintf(fOut, "%s (%s)\n", buttonActs[but->actFunc].name, getIDName(but->actVar, buttonActs[but->actFunc].type));
    }
    fputc('\n', fOut);
  }
  
  fputs("\n\n#### Unit Settings ####\n\n", fOut);
  for(i = 0; i < unitCount; i++){
    if(unitButtons[i].padding == 0) continue; // nothing wrote to this unit
    
    if((unitButtons[i].padding & FG_EXTENDED) || (unitButtons[i&0xFF].padding & FG_EXTENDED)){
      fputs("# WARNING: Duplicate FG Unit ID -- Verify correctness before compiling\n", fOut);
      if(unitButtons[i].padding == FG_EXTENDED){
        fprintf(fOut, "Unit %d: # %s\n\n", i, getIDName(i, UNIT));
        continue; // no actual data here
      }
    }
    fprintf(fOut, "Unit %d: # %s\n  %d, %d", i, getIDName(i, UNIT), (u32)(unitButtons[i].firstButton), unitButtons[i].connectedUnit);
    if(i < statCount){
      fprintf(fOut, ", %d, %d", unitStats[i].condFunc, unitStats[i].actFunc);
    }
    j = (u32)(unitButtons[i].firstButton);
    if(j < bsetStrCount && bsetNames[j] != NULL){
      fprintf(fOut, "  # Button Set: %s", bsetNames[j]);
      hasComment = true;
    }else{
      hasComment = false;
    }
    if(i < statCount){
      for(j = 0; j < statPresetCount; j++){
        if(unitStats[i].condFunc == statPresets[j].stat && unitStats[i].actFunc == statPresets[j].disp){
          if(hasComment){
            fprintf(fOut, "\n                   # Stat Preset: %s", statPresets[j].name);
          }else{
            fprintf(fOut, "  # Stat Preset: %s", statPresets[j].name);
          }
          break;
        }
      }
    }
    fputs("\n\n", fOut);
  }
  
  fclose(fOut);
}


void saveTXTReq(const char* filename, const char* header, u16** reqData, u32 count, char** labels, u32 labelCount, u8 availType){
  u32 i,j;
  u16 v;
  bool sublist;
  u8 paramType;
  char pad[10];
  printf("Writing %s...\n", filename);
  FILE * fOut = fopen(filename, "w");
  if(fOut == NULL){
    printf("Could not save to file %s\n", filename);
    return;
  }
  
  fprintf(fOut, "# %s Requirement Scripts\n\n", header);
  
  for(i = 0; i < count; i++){
    if(reqData[i] == NULL) continue;
    
    if(availType == UNIT && ((unitButtons[i].padding & FG_EXTENDED) || (unitButtons[i&0xFF].padding & FG_EXTENDED))){
      fputs("# WARNING: Duplicate FG Unit ID -- Verify correctness before compiling\n", fOut);
    }
    if(i < labelCount && labels[i] != NULL){
      sprintf(pad, "%d:", i);
      fprintf(fOut, "%-20s# %s\n", pad, labels[i]);
    }else{
      fprintf(fOut, "%d:\n", i);
    }
    
    sublist = false;
    j = 0;
    do {
      v = reqData[i][j++];
      if(v < 0xFF00){
        if(paramType == NONE){
          fprintf(fOut, "  %-18d# Has Unit: %s\n", v, getIDName(v, UNIT));
        }else{
          fprintf(fOut, "  %-18d# %s\n", v, getIDName(v, paramType));
        }
        paramType = NONE;
      }else if(sublist || v != 0xFFFF){
        v &= 0xFF;
        if(opcodes[v].avail & availType){
          paramType = opcodes[v].paramType;
          fprintf(fOut, "  0xFF%-14.2X# %s\n", v, opcodes[v].name);
        }else{
          fprintf(fOut, "  0xFF%-14.2X# unused\n", v);
        }
        if(v == 0xFF){
          sublist = false;
        }
        if(availType == NOT_UNIT && (v == 0x1F || v == 0x20)){
          sublist = true;
        }
      }
    } while(v != 0xFFFF);
    fprintf(fOut, "  0x%-16.4X# ==End of List==\n\n", v);
  }
  
  fclose(fOut);
}



void loadSCData(){
  FILE * f = fopen("data\\starcraft.dat", "rb");
  if(f == NULL) {
    puts("Could not load starcraft.dat");
    return;
  }
  
  // load req data
  loadSCReqList(f, ADDR_UNITS_REQ, SIZE_UNITS_REQ, units_req, unitCount, UNIT);
  loadSCReqList(f, ADDR_UPGRADE_REQ, SIZE_UPGRADE_REQ, upgrades_req, upgdCount, NOT_UNIT);
  loadSCReqList(f, ADDR_RESEARCH_REQ, SIZE_RESEARCH_REQ, research_req, techCount, NOT_UNIT);
  loadSCReqList(f, ADDR_TECH_REQ, SIZE_TECH_REQ, tech_req, techCount, NOT_UNIT);
  loadSCReqList(f, ADDR_ORDERS_REQ, SIZE_ORDERS_REQ, orders_req, orderCount, NOT_UNIT);
  
  // load button data
  loadSCButtons(f);
  
  fclose(f);
}

void loadSCReqList(FILE * f, u32 offset, u32 size, u16** reqData, u32 count, u8 availType){
  u16* data;
  u32 i;
  u16 v;
  bool sublist;
  
  fseek(f, offset - ADDR_BASE, SEEK_SET);
  data = malloc(size);
  fread(data, size, 1, f);
  strData[strDataCount++] = (char*)data;
  
  i = 1;
  sublist = false;
  do {
    v = data[i++];
    if(v == 0xFFFF) break;
    if(v >= count){
      printf("ID %d greater than maximum %d\n", v, count);
    }else{
      reqData[v] = &data[i];
    }
    do {
      v = data[i++];
      if(v != 0xFFFF){
        if(availType == NOT_UNIT && (v == 0xFF1F || v == 0xFF20)){
          sublist = true;
        }
      }else{
        if(sublist){
          sublist = false;
        }else{
          break;
        }
      }
    } while(1);
  } while(1);
}

u32 getButtonFuncID(BUTTON_FUNC* list, u32 listCount, u32 func){
  u32 i;
  for(i = 0; i < listCount; i++){
    if(list[i].func == func) return i;
  }
  return 0xFFFFFFFF;
}

void loadSCButtons(FILE * f){
  u32 i,j;
  
  BUTTON butt;
  u32 offsets[250] = {0};
  u32 numButtons[250] = {0};
  u32 reID[250] = {0};
  u32 count = 0;
  u32 min, minid;
  u32 condid, actid;
  
  fseek(f, ADDR_BUTTONS - ADDR_BASE, SEEK_SET);
  fread(unitButtons, sizeof(BUTTON_SET), 250, f);
  
  fseek(f, ADDR_STATINFO - ADDR_BASE, SEEK_SET);
  fread(unitStats, sizeof(STATFUNC), 228, f);
  
  // Find button sets
  for(i = 0; i < 250; i++){
    unitButtons[i].padding = SC_BUTTONS;
    if(unitButtons[i].firstButton == NULL){
      unitButtons[i].firstButton = (BUTTON*)(-1);
      continue;
    }
    for(j = 0; j < count; j++){
      if(offsets[j] == (u32)(unitButtons[i].firstButton)){
        if(numButtons[j] < unitButtons[i].buttonsInSet){
          printf("%d\n", j);
          numButtons[j] = unitButtons[i].buttonsInSet;
        }
        unitButtons[i].firstButton = (BUTTON*)j;
        break;
      }
    }
    if(j == count){
      offsets[j] = (u32)(unitButtons[i].firstButton);
      numButtons[j] = unitButtons[i].buttonsInSet;
      unitButtons[i].firstButton = (BUTTON*)j;
      count++;
    }
  }
  
  //printf("Debug: %d == %d\n", count, bsetCount);
  
  // Loading buttons
  for(i = 0; i < count; i++){
    min = 0xFFFFFFFF;
    minid = 0xFFFFFFFF;
    for(j = 0; j < count; j++){
      if(offsets[j] < min){
        minid = j;
        min = offsets[j];
      }
    }
    if(minid == 0xFFFFFFFF) break;
    offsets[minid] = 0xFFFFFFFF;
    
    reID[minid] = i;
    
    fseek(f, min - ADDR_BASE, SEEK_SET);
    buttonSets[i].buttonsInSet = numButtons[minid];
    buttonSets[i].firstButton = malloc(numButtons[minid] * sizeof(BUTTON));
    fread(buttonSets[i].firstButton, sizeof(BUTTON), numButtons[minid], f);
    buttonSets[i].connectedUnit = 0;
    buttonSets[i].padding = SC_BUTTONS;
    
    // Convert functions to IDs
    for(j = 0; j < numButtons[minid]; j++){
      buttonSets[i].firstButton[j].reqFunc = getButtonFuncID(buttonConds, buttonCondCount, buttonSets[i].firstButton[j].reqFunc);
      buttonSets[i].firstButton[j].actFunc = getButtonFuncID(buttonActs, buttonActCount, buttonSets[i].firstButton[j].actFunc);
    }
  }
  
  // reID
  for(i = 0; i < 250; i++){
    if((s32)(unitButtons[i].firstButton) != -1){
      unitButtons[i].firstButton = (BUTTON*)(reID[(u32)(unitButtons[i].firstButton)]);
    }
  }
  
  // Convert functions to IDs
  for(i = 0; i < 228; i++){
    unitStats[i].condFunc = getButtonFuncID(statusFuncs, statusFuncCount, unitStats[i].condFunc);
    unitStats[i].actFunc = getButtonFuncID(displayFuncs, displayFuncCount, unitStats[i].actFunc);
  }
}




void loadFGData(const char* filename){
  FGData = loadFile(filename, &FGDataSize);
  if(FGData == NULL) return;
  
  u8* data = NULL;
  u32 size;
  u32 count;
  u32 i;
  u32 prevID;
  u32 curID;
  u32 addID;
  FG_BUTTONSET* bs;
  FG_UNIT* unit;
  FG_STATINFO* stat;
  
  // check version
  data = findFGChunk("FgPa", NULL);
  if(data == NULL || strcmp(data, "1.16.1") != 0){
    printf("Invalid or unsupported FireGraft data. (%s)", filename);
    return;
  }
  
  // load button sets
  data = findFGChunk("Buts", &size);
  if(data == NULL){
    puts("Could not find FG button data.");
    return;
  }
  count = *(u16*)(data);
  data += sizeof(u16);
  for(i = 0; i < count; i++){
    bs = (FG_BUTTONSET*)data;
    if(bs->id >= bsetCount){
      if(bs->id >= bsetMax){
        // enlarge buffer
        u32 newMax = bsetMax*2;
        while(bs->id >= newMax) newMax += bsetMax; // ensure new buffer is always large enough
        buttonSets = (BUTTON_SET*)realloc(buttonSets, newMax*sizeof(BUTTON_SET));
        memset(&buttonSets[bsetMax], 0, (newMax-bsetMax)*sizeof(BUTTON_SET)); // set new memory to 0
        bsetMax = newMax;
      }
      bsetCount = bs->id + 1;
    }
    
    if((buttonSets[bs->id].padding & FG_BUTTONS) == 0 && buttonSets[bs->id].firstButton != NULL){ // is not an FG button set
      free(buttonSets[bs->id].firstButton); // this set is no longer referenced
      buttonSets[bs->id].firstButton = NULL;
      buttonSets[bs->id].padding &= ~(SC_BUTTONS|TXT_BUTTONS); // is FG data
    }
    buttonSets[bs->id].buttonsInSet = bs->count;
    buttonSets[bs->id].firstButton = bs->buttons;
    buttonSets[bs->id].connectedUnit = 0;
    buttonSets[bs->id].padding |= FG_BUTTONS; // is FG data
    
    // next button
    data += sizeof(FG_BUTTONSET) + (bs->count-1)*sizeof(BUTTON);
  }
  // Increase number of button set strings
  if(bsetCount > bsetStrCount){
    bsetNames = (char**)realloc(bsetNames, bsetCount * sizeof(char*));
    memset(&bsetNames[bsetStrCount], 0, (bsetCount - bsetStrCount) * sizeof(char*));
    bsetStrCount = bsetCount;
  }
  
  // load unit info
  prevID = 0;
  addID = 0;
  data = findFGChunk("Unit", &size);
  if(data == NULL){
    puts("Could not find FG unit data.");
    return;
  }
  count = *(u16*)(data);
  data += sizeof(u16);
  for(i = 0; i < count; i++){
    unit = (FG_UNIT*)data;
    data += sizeof(FG_UNIT);
    if(unit->hasStatInfo){
      stat = (FG_STATINFO*)data;
      data += sizeof(FG_STATINFO);
    }
    
    curID = unit->id + addID;
    if(curID < prevID){
      if(addID == 0) puts("WARNING: Extended Unit IDs detected. Verify scripts before merging.");
      while(curID < prevID){
        addID += 256;
        curID += 256;
      }
    }
    //printf("unit %d\n", curID);
    if(addID != 0){
      if(curID < unitCount) unitButtons[curID].padding |= FG_EXTENDED;
      unitButtons[unit->id].padding |= FG_EXTENDED;
    }
    prevID = curID;
    
    if(curID >= unitCount){
      printf("Unit ID %d greater than maximum %d\n", curID, unitCount);
      continue;
    }
    unitButtons[curID].buttonsInSet = unit->buttonCount;
    unitButtons[curID].firstButton = (BUTTON*)(unit->setID - 1);
    unitButtons[curID].connectedUnit = unit->connectedUnit;
    unitButtons[curID].padding |= FG_BUTTONS;
    if(unit->hasStatInfo){
      if(curID >= statCount){
        printf("Unit ID %d greater than maximum for stat info %d\n", curID, statCount);
        continue;
      }
      unitStats[curID].unitId = stat->debugId;
      unitStats[curID].condFunc = stat->statFuncId;
      unitStats[curID].actFunc = stat->dispFuncId;
    }
  }
  
  // load req data
  loadFGReq("UntR", units_req, unitCount);
  loadFGReq("UpgR", upgrades_req, upgdCount);
  loadFGReq("TecR", research_req, techCount);
  loadFGReq("TecU", tech_req, techCount);
  loadFGReq("OrdR", orders_req, orderCount);
  
  // load strings
  loadFGStrings("SUni", unitNames, unitStrCount);
  loadFGStrings("SRes", techNames, techStrCount);
  loadFGStrings("SUpg", upgdNames, upgdStrCount);
  loadFGStrings("SOrd", orderNames, orderStrCount);
  loadFGStrings("SBut", bsetNames, bsetStrCount);
}

void loadFGReq(const char* chunk, u16** reqData, u32 maxCount){
  u8* data = NULL;
  u32 size;
  u32 count;
  u32 i;
  FG_REQSCRIPT* req;
  u8 used;
  u32 curID;
  u32 prevID = 0;
  u32 addID = 0;
  
  data = findFGChunk(chunk, &size);
  if(data == NULL){
    printf("Could not find FG \"%s\" chunk\n", chunk);
    return;
  }
  count = *(u16*)(data);
  data += sizeof(u16);
  for(i = 0; i < count; i++){
    req = (FG_REQSCRIPT*)data;
    data += sizeof(FG_REQSCRIPT) + (req->count-1)*sizeof(u16);
    used = *data;
    data++;
    
    curID = req->id;
    prevID = curID;
    if(used != 0){
      if(curID >= maxCount){
        printf("Dat req ID %d greater than maximum %d\n", curID, maxCount);
        continue;
      }
      while((u32)(reqData[curID]) >= (u32)FGData && (u32)(reqData[curID]) < (u32)FGData + FGDataSize){
        curID += 256;
      }
      reqData[curID] = req->opcodes;
    }else{
      if(curID < maxCount){
        while((u32)(reqData[curID]) >= (u32)FGData && (u32)(reqData[curID]) < (u32)FGData + FGDataSize){
          curID += 256;
        }
        reqData[curID] = NULL;
      }
    }
    if(curID > req->id){
      //printf("req %08X %d -> %d\n", reqData, req->id, curID);
      if(addID == 0) puts("WARNING: Extended unit req IDs detected. Verify scripts before merging.");
      if(curID < maxCount) unitButtons[curID].padding |= FG_EXTENDED;
      unitButtons[req->id].padding |= FG_EXTENDED;
    }
  }
}

void loadFGStrings(const char* chunk, char** names, u32 maxCount){
  u8* data = NULL;
  u32 size;
  u32 count;
  u32 i;
  FG_TREENODE* node;
  
  data = findFGChunk(chunk, &size);
  if(data == NULL){
    printf("Could not find FG \"%s\" chunk\n", chunk);
    return;
  }
  count = *(u32*)(data);
  data += sizeof(u32);
  for(i = 0; i < count; i++){
    node = (FG_TREENODE*)data;
    data += sizeof(FG_TREENODE)-1;
    
    if(node->id != -1 && node->isFolder != 1){
      if(node->id >= maxCount){
        printf("ID %d greater than maximum %d\n", node->id, maxCount);
      }else{
        names[node->id] = data;
      }
    }
    
    data += strlen(data) + 1;
  }
}

u8* findFGChunk(const char* name, u32* size){
  u32 chunkID = *(u32*)(name);
  FG_CHUNK* chunk;
  u32 offset;
  for(offset = 0; offset < FGDataSize; offset += chunk->size + sizeof(FG_CHUNK)){
    chunk = (FG_CHUNK*)(&FGData[offset]);
    if((offset + sizeof(FG_CHUNK) + chunk->size) > FGDataSize) break;
    if(chunk->name == chunkID){
      if(size != NULL) *size = chunk->size;
      return &FGData[offset + sizeof(FG_CHUNK)];
    }
  }
  if(size != NULL) *size = 0;
  return NULL;
}


void parseTXTButtons(const char* filename){
  u8* buf;
  u16* data;
  u32 count;
  u32 cols;
  char** lineData;
  u32 i;
  u32 v;
  u32 line;
  s32 args[9];
  
  BUTTON butbuf[32];
  u32 butcount = 0;
  s32 butID = -1;
  bool readButt = false;
  bool readUnit = false;
  
  buf = loadFile(filename, NULL);
  if(buf == NULL) return;
  
  count = getLineCount(buf)+1;
  data = malloc(count * sizeof(u16));
  strData[strDataCount++] = (u8*)data;
  
  line = 0;
  while(buf[0] != 0){
    buf = parseLine(buf, ':', &cols, &lineData);
    line++;
    if(cols == 0 || (cols == 1 && lineData[0][0] == 0)) continue; // nothing on this line
    if(cols > 2){
      printf("Unexpected \":\" in data on line %d\n", line);
      continue;
    }
    
    if(cols == 2){ // line has a ':'
      // store previously defined buttons
      if(readButt == true && butID != -1){
        if(butcount == 0){
          buttonSets[butID].buttonsInSet = 0;
        }else{
          if(butcount > buttonSets[butID].buttonsInSet){
            if((buttonSets[butID].padding & FG_BUTTONS) == 0){ // not FG data -- must be freed
              if(buttonSets[butID].firstButton != NULL) free(buttonSets[butID].firstButton);
            }
            buttonSets[butID].firstButton = malloc(butcount * sizeof(BUTTON));
            buttonSets[butID].padding &= ~FG_BUTTONS; // not FG data
          }
          memcpy(buttonSets[butID].firstButton, butbuf, butcount * sizeof(BUTTON));
          buttonSets[butID].buttonsInSet = butcount;
          buttonSets[butID].padding |= TXT_BUTTONS;
        }
      }
      readButt = false;
      
      if(sscanf(lineData[0], "Unit %i %1s", &v, &args[8]) == 1){
        if(v >= unitCount){
          printf("Unit ID %d greater than maximum %d on line %d\n", v, unitCount, line);
          continue;
        }
        butID = v;
        readButt = false;
        readUnit = true;
      }else if(sscanf(lineData[0], "%i %1s", &v, &args[8]) == 1){
        // get new ID
        butID = v;
        // TODO: validate ID?
        butcount = 0;
        if(butID >= bsetCount){
          if(butID >= bsetMax){
            // enlarge buffer
            u32 newMax = bsetMax*2;
            while(butID >= newMax) newMax += bsetMax; // ensure new buffer is always large enough
            buttonSets = (BUTTON_SET*)realloc(buttonSets, newMax*sizeof(BUTTON_SET));
            memset(&buttonSets[bsetMax], 0, (newMax-bsetMax)*sizeof(BUTTON_SET)); // set new memory to 0
            bsetMax = newMax;
          }
          bsetCount = butID + 1;
        }
        readButt = true;
        readUnit = false;
      }else{
        printf("Unexpected data \"%s\" on line %d\n", lineData[0], line);
        butID = -1;
        readButt = false;
        readUnit = false;
        continue;
      }
      
      // trim trailing spaces
      for(i = strlen(lineData[1]); i != 0 && (isspace(lineData[1][i]) || lineData[1][i] == 0); i--){
        lineData[1][i] = 0;
      }
      // no data after ':'
      if(lineData[1][0] == 0) continue;
    }
    cols--; // allow reading data to the right of ':'
    
    if(readButt){
      if(sscanf(lineData[cols], " %i , %i , %i , %i , %i , %i , %i , %i %1s", &args[0], &args[1], &args[2], &args[3], &args[4], &args[5], &args[6], &args[7], &args[8]) != 8){
        printf("Incorrect number of arguments on line %d\n", line);
        continue;
      }
      
      // validate data
      if(args[0] < 1 || args[0] > 9){
        printf("Button position %d on line %d\n", args[0], line);
      }
      if(args[2] < 0 || args[2] >= buttonCondCount){
        printf("Invalid button condition ID %d on line %d\n", args[2], line);
        continue;
      }
      if(args[3] < 0 || args[3] >= buttonActCount){
        printf("Invalid button action ID %d on line %d\n", args[3], line);
        continue;
      }
      
      // add button
      butbuf[butcount].position = args[0];
      butbuf[butcount].iconID = args[1];
      butbuf[butcount].reqFunc = args[2];
      butbuf[butcount].actFunc = args[3];
      butbuf[butcount].reqVar = args[4];
      butbuf[butcount].actVar = args[5];
      butbuf[butcount].enableStr = args[6];
      butbuf[butcount].disableStr = args[7];
      butcount++;
    }else if(readUnit){
      v = sscanf(lineData[cols], " %i , %i , %i , %i %1s", &args[0], &args[1], &args[2], &args[3], &args[8]);
      if(v != 1 && v != 2 && v != 4){
        printf("Incorrect number of arguments on line %d\n", line);
        continue;
      }
      
      // validate data
      if(args[0] < -1 || args[0] >= (s32)bsetCount){
        printf("Undefined button set %d (currently %d defined) on line %d\n", args[0], bsetCount, line);
        continue;
      }
      if(v == 4 && butID >= statCount){
        printf("Stat info ID %d greater than maximum %d on line %d\n", butID, statCount, line);
        v = 2;
      }
      if(v == 4 && (args[2] < 0 || args[2] >= statusFuncCount)){
        printf("Invalid status function ID %d on line %d\n", args[2], line);
        continue;
      }
      if(v == 4 && (args[3] < 0 || args[3] >= displayFuncCount)){
        printf("Invalid display function ID %d on line %d\n", args[3], line);
        continue;
      }
      
      unitButtons[butID].padding = TXT_BUTTONS;
      if(args[0] < 0){
        unitButtons[butID].firstButton = (BUTTON*)(-1);
        unitButtons[butID].buttonsInSet = 0;
      }else{
        unitButtons[butID].firstButton = (BUTTON*)(args[0]);
        unitButtons[butID].buttonsInSet = buttonSets[args[0]].buttonsInSet;
      }
      if(v > 1){
        unitButtons[butID].connectedUnit = args[1];
      }else{
        unitButtons[butID].connectedUnit = 65535;
      }
      if(v == 4){
        unitStats[butID].unitId = butID;
        unitStats[butID].condFunc = args[2];
        unitStats[butID].actFunc = args[3];
      }
    }else{
      // error checking, maybe
    }
  }
  
  // store active button if there is one
  if(readButt && butID != -1){
    if(butcount == 0){
      buttonSets[butID].buttonsInSet = 0;
    }else{
      if(butcount > buttonSets[butID].buttonsInSet){
        if((buttonSets[butID].connectedUnit & FG_BUTTONS) == 0){ // not FG data
          if(buttonSets[butID].firstButton != NULL) free(buttonSets[butID].firstButton);
        }
        buttonSets[butID].firstButton = malloc(butcount * sizeof(BUTTON));
        buttonSets[butID].connectedUnit &= ~FG_BUTTONS; // not FG data
      }
      memcpy(buttonSets[butID].firstButton, butbuf, butcount * sizeof(BUTTON));
      buttonSets[butID].buttonsInSet = butcount;
      buttonSets[butID].connectedUnit |= TXT_BUTTONS;
    }
  }
  
  free(buf);
}

void parseTXTReq(const char* filename, u16** reqData, u32 maxCount){
  u8* buf;
  u16* data;
  u32 count;
  u32 cols;
  char** lineData;
  u32 i;
  u32 v;
  u32 line;
  
  buf = loadFile(filename, NULL);
  if(buf == NULL) return;
  
  count = getLineCount(buf)+1;
  data = malloc(count * sizeof(u16));
  strData[strDataCount++] = (u8*)data;
  
  i = 0;
  line = 0;
  while(buf[0] != 0){
    buf = parseLine(buf, ':', &cols, &lineData);
    line++;
    if(cols == 0 || (cols == 1 && lineData[0][0] == 0)) continue; // nothing on this line
    if(cols > 2){
      printf("Unexpected \":\" in data on line %d\n", line);
      continue;
    }
    
    if(cols == 2){ // line has a ':'
      if(sscanf(lineData[0], "%i", &v) == 0){
        printf("Unexpected data \"%s\" on line %d\n", lineData[0], line);
        continue;
      }
      if(v >= maxCount){
        printf("ID %d greater than maximum %d on line %d\n", v, maxCount, line);
        continue;
      }
      reqData[v] = &data[i];
      // optionally read a value here
      if(sscanf(lineData[1], " %i", &v) == 1){
        data[i++] = v;
      }
    }else{ // does not have a :
      if(sscanf(lineData[0], " %i", &v) == 1){
        data[i++] = v;
      }
    }
    data[i] = 0xFFFF; // ensure script is terminated
  }
  
  free(buf);
}




bool loadDefaultData(){
  // load ID names
  unitNames = loadStringTable("data\\Units.txt", &unitStrCount);
  techNames = loadStringTable("data\\Techdata.txt", &techStrCount);
  upgdNames = loadStringTable("data\\Upgrades.txt", &upgdStrCount);
  orderNames = loadStringTable("data\\Orders.txt", &orderStrCount);
  iconNames = loadStringTable("data\\Icons.txt", &iconStrCount);
  bsetNames = loadStringTable("data\\Button Sets.txt", &bsetStrCount);
  
  char** data;
  char* buf;
  u32 count;
  u32 cols;
  u32 i,j;
  u32 v;
  
  // load opcodes
  buf = loadFile("data\\opcodes.txt", NULL);
  strData[strDataCount++] = buf;
  while(buf[0] != 0){
    buf = parseLine(buf, '\t', &cols, &data);
    if(cols == 0) continue;
    if(cols != 4){
      printf("Malformed opcode data %d %s\n", cols, data[0]);
      continue;
    }
    
    v = 0;
    sscanf(data[0], "%x", &v);
    if((v & 0xFF00) != 0xFF00){
      printf("Invalid opcode %x\n", v);
      continue;
    }
    v &= 0xFF;
    opcodes[v].paramType = getParamType(data[1]);
    if(strcmpi(data[2], "unit") == 0) opcodes[v].avail = UNIT;
    if(strcmpi(data[2], "no unit") == 0) opcodes[v].avail = NOT_UNIT;
    if(strcmpi(data[2], "any") == 0) opcodes[v].avail = UNIT | NOT_UNIT;
    opcodes[v].name = data[3];
  }
  
  // load button conditions
  buf = loadFile("data\\button conditions.txt", NULL);
  strData[strDataCount++] = buf;
  count = getLineCount(buf);
  buttonConds = calloc(count, sizeof(BUTTON_FUNC));
  while(buf[0] != 0){
    buf = parseLine(buf, '\t', &cols, &data);
    if(cols == 0) continue;
    if(cols != 3){
      printf("Malformed condition data %d %s\n", cols, data[0]);
      continue;
    }
    
    v = 0;
    sscanf(data[0], "%x", &v);
    if(v == 0) printf("Error on ID %d\n", buttonCondCount);
    buttonConds[buttonCondCount].func = v;
    buttonConds[buttonCondCount].type = getParamType(data[1]);
    buttonConds[buttonCondCount].name = data[2];
    buttonCondCount++;
  }
  
  // load button actions
  buf = loadFile("data\\button actions.txt", NULL);
  strData[strDataCount++] = buf;
  count = getLineCount(buf);
  buttonActs = calloc(count, sizeof(BUTTON_FUNC));
  while(buf[0] != 0){
    buf = parseLine(buf, '\t', &cols, &data);
    if(cols == 0){
      continue;
    }
    if(cols != 3){
      printf("Malformed action data %d %s\n", cols, data[0]);
      continue;
    }
    
    v = 0;
    sscanf(data[0], "%x", &v);
    if(v == 0) printf("Error on ID %d\n", buttonActCount);
    buttonActs[buttonActCount].func = v;
    buttonActs[buttonActCount].type = getParamType(data[1]);
    buttonActs[buttonActCount].name = data[2];
    buttonActCount++;
  }
  
  
  // load display functions
  buf = loadFile("data\\Display Functions.txt", NULL);
  strData[strDataCount++] = buf;
  count = getLineCount(buf);
  displayFuncs = calloc(count, sizeof(BUTTON_FUNC));
  while(buf[0] != 0){
    buf = parseLine(buf, '\t', &cols, &data);
    if(cols == 0) continue;
    if(cols != 2){
      printf("Malformed display data %d %s\n", cols, data[0]);
      continue;
    }
    
    v = 0;
    sscanf(data[0], "%x", &v);
    if(v == 0) printf("Error on ID %d\n", displayFuncCount);
    displayFuncs[displayFuncCount].func = v;
    displayFuncs[displayFuncCount].type = 0;
    displayFuncs[displayFuncCount].name = data[1];
    displayFuncCount++;
  }
  
  // load stat functions
  buf = loadFile("data\\Status Functions.txt", NULL);
  strData[strDataCount++] = buf;
  count = getLineCount(buf);
  statusFuncs = calloc(count, sizeof(BUTTON_FUNC));
  while(buf[0] != 0){
    buf = parseLine(buf, '\t', &cols, &data);
    if(cols == 0) continue;
    if(cols != 2){
      printf("Malformed status data %d %s\n", cols, data[0]);
      continue;
    }
    
    v = 0;
    sscanf(data[0], "%x", &v);
    if(v == 0) printf("Error on ID %d\n", statusFuncCount);
    statusFuncs[statusFuncCount].func = v;
    statusFuncs[statusFuncCount].type = 0;
    statusFuncs[statusFuncCount].name = data[1];
    statusFuncCount++;
  }
  
  // load stat  presets
  buf = loadFile("data\\stat presets.txt", NULL);
  strData[strDataCount++] = buf;
  count = getLineCount(buf);
  statPresets = calloc(count, sizeof(STAT_PRESET));
  while(buf[0] != 0){
    buf = parseLine(buf, '\t', &cols, &data);
    if(cols == 0) continue;
    if(cols != 3){
      printf("Malformed preset data %d %s\n", cols, data[0]);
      continue;
    }
    
    statPresets[statPresetCount].name = data[2];
    statPresets[statPresetCount].stat = atoi(data[0]);
    statPresets[statPresetCount].disp = atoi(data[1]);
    statPresetCount++;
  }
  
  //printf("Debug: %d %d %d %d %d %d\nDebug: 250 45 62 190 390 98\n", unitStrCount, techStrCount, upgdStrCount, orderStrCount, iconStrCount, bsetStrCount);
  //printf("Debug: %d %d %d %d %d\nDebug: 67 60 9 9 9\n", buttonCondCount, buttonActCount, displayFuncCount, statusFuncCount, statPresetCount);
}

void unloadData(){
  u32 i;
  for(i = 0; i < strDataCount; i++){
    if(strData[i] != NULL) free(strData[i]);
  }
  if(unitNames != NULL) free(unitNames);
  if(techNames != NULL) free(techNames);
  if(upgdNames != NULL) free(upgdNames);
  if(orderNames != NULL) free(orderNames);
  if(iconNames != NULL) free(iconNames);
  if(bsetNames != NULL) free(bsetNames);
  if(buttonConds != NULL) free(buttonConds);
  if(buttonActs != NULL) free(buttonActs);
  if(statPresets != NULL) free(statPresets);
  if(FGData != NULL) free(FGData);
  if(units_req != NULL) free(units_req);
  if(upgrades_req != NULL) free(upgrades_req);
  if(research_req != NULL) free(research_req);
  if(tech_req != NULL) free(tech_req);
  if(orders_req != NULL) free(orders_req);
  if(buttonSets != NULL){
    for(i = 0; i < bsetCount; i++){
      if(buttonSets[i].connectedUnit == 0 && buttonSets[i].firstButton != NULL){
        free(buttonSets[i].firstButton);
      }
    }
    free(buttonSets);
  }
  if(unitButtons != NULL) free(unitButtons);
  if(unitStats != NULL) free(unitStats);

}

u8 getParamType(char* str){
  if(str[0] == 0 || strcmpi(str, "none") == 0) return NONE;
  if(strcmpi(str, "unit") == 0) return UNIT;
  if(strcmpi(str, "tech") == 0) return TECH;
  if(strcmpi(str, "upgrade") == 0) return UPGRADE;
  if(strcmpi(str, "order") == 0) return ORDER;
  printf("Unknown param type %s\n", str);
  return NONE;
}

char* getIDName(u32 id, u32 type){
  static u8 buf[16] = "";
  switch(type){
    case UNIT:
      if(id >= unitStrCount && unitNames[id] != NULL) break;
      return unitNames[id];
    case TECH:
      if(id >= techStrCount && techNames[id] != NULL) break;
      return techNames[id];
    case UPGRADE:
      if(id >= upgdStrCount && upgdNames[id] != NULL) break;
      return upgdNames[id];
    case ORDER:
      if(id >= orderStrCount && orderNames[id] != NULL) break;
      return orderNames[id];
    case ICON:
      if(id >= iconStrCount && iconNames[id] != NULL) break;
      return iconNames[id];
    case BSET:
      if(id >= bsetStrCount && bsetNames[id] != NULL) break;
      return bsetNames[id];
    case NONE:
    default:
      break;
  }
  sprintf(buf, "%d", id);
  return buf;
}

u8* loadFile(const char* filename, u32* r_size){
  char* buf;
  u32 size;
  FILE * f = fopen(filename, "rb");
  if(f == NULL){
    printf("Could not open %s\n", filename);
    return NULL;
  }
  fseek(f, 0, SEEK_END);
  size = ftell(f);
  rewind(f);
  buf = malloc(size+1);
  fread(buf, size, 1, f);
  fclose(f);
  buf[size] = 0;
  if(r_size != NULL) *r_size = size;
  return buf;
}

char** loadStringTable(const char* filename, u32* strCount){
  char* buf;
  char** table;
  u32 size;
  u32 count;
  u32 i,j;
  bool copying;
  
  buf = loadFile(filename, &size);
  if(buf == NULL) return NULL;
  
  // parse each line & count strings
  copying = false;
  count = 0;
  j = 0;
  for(i = 0; i <= size; i++){
    if(buf[i] != '\n' && buf[i] != '\r' && buf[i] != 0){
      copying = true;
      buf[j] = buf[i];
      j++;
    }else{
      if(copying){
        buf[j] = 0;
        j++;
        count++;
        copying = false;
      }
    }
  }
  for( ; j <= size; j++){
    buf[j] = 0;
  }
  
  // store buffer
  strData[strDataCount] = buf;
  strDataCount++;
  
  // generate table
  table = calloc(count, sizeof(char*));
  for(i = 0; i < count; i++){
    table[i] = buf;
    buf += strlen(buf)+1;
  }
  
  if(strCount != NULL) *strCount = count;
  return table;
}

u32 getLineCount(u8* buf){
  u32 count;
  u32 i;
  count = 1;
  for(i = 0; buf[i] != 0; i++){
    if(buf[i] == '\n') count++;
  }
  return count;
}

u8* parseLine(u8* buf, u8 delim, u32* dataCount, char*** data){
  static char* s_data[16];
  u32 count;
  u32 i,j;
  
  count = 0;
  for(i = 0; buf[i] != '\n' && buf[i] != '\r' && buf[i] != 0; i++){
    s_data[count] = &buf[i];
    count++;
    for(; buf[i] != delim && buf[i] != '#' && buf[i] != '\n' && buf[i] != '\r' && buf[i] != 0; i++);
    if(buf[i] == '#'){ // comment character
      for(; buf[i-1] != 0 && (buf[i-1] == ' ' || buf[i-1] == '\t'); i--); // remove trailing spaces
      buf[i] = 0;
      for(i++; buf[i] != '\n' && buf[i] != '\r' && buf[i] != 0; i++); // ignore rest of the line
      break;
    }
    
    // remove trailing spaces
    for(j = i; j > 0 && buf[j-1] != 0 && buf[j-1] != delim && (buf[j-1] == ' ' || buf[j-1] == '\t'); j--);
    if(j != i) buf[j] = 0;
    
    if(buf[i] != delim) break;
    buf[i] = 0;
  }
  if(buf[i] == '\r' && buf[i+1] == '\n'){
    buf[i] = 0;
    i++;
  }
  buf[i] = 0;
  i++;
  
  *dataCount = count;
  *data = s_data;
  return &buf[i];
}
