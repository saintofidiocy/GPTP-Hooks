typedef unsigned char  u8;
typedef   signed char  s8;
typedef unsigned short u16;
typedef   signed short s16;
typedef unsigned int   u32;
typedef   signed int   s32;
typedef enum { false, true } bool;


#define ADDR_BASE          (0x00400000+0x00114178)

// sc.exe data addresses
#define ADDR_UNITS_REQ     0x00514178
#define ADDR_UPGRADE_REQ   0x005145C0
#define ADDR_RESEARCH_REQ  0x00514908
#define ADDR_TECH_REQ      0x00514A48
#define ADDR_ORDERS_REQ    0x00514CF8
#define ADDR_BUTTONS       0x005187E8
#define ADDR_STATINFO      0x005193A0

// default data sizes
#define SIZE_UNITS_REQ     1090
#define SIZE_UPGRADE_REQ   838
#define SIZE_RESEARCH_REQ  318
#define SIZE_TECH_REQ      688
#define SIZE_ORDERS_REQ    1316

// data types
#define NONE     0
#define UNIT     1
#define NOT_UNIT 2 // avail type
#define TECH     2
#define UPGRADE  3
#define ORDER    4
#define ICON     5
#define BSET     6

// file types
#define BUTTONS       0
#define UNITS_REQ     1
#define UPGRADE_REQ   2
#define RESEARCH_REQ  3
#define TECH_REQ      4
#define ORDERS_REQ    5

#define SC_BUTTONS    0x01
#define FG_BUTTONS    0x02
#define TXT_BUTTONS   0x04
#define FG_EXTENDED   0x08

typedef struct {
  u16 position;
  u16 iconID;
  u32 reqFunc;
  u32 actFunc;
  u16 reqVar;
  u16 actVar;
  u16 enableStr;
  u16 disableStr;
} BUTTON;

typedef struct {
  u32 buttonsInSet;
  BUTTON* firstButton;
  u16 connectedUnit;
  u16 padding;
} BUTTON_SET;

typedef struct {
  u32 unitId;
  u32 condFunc;
  u32 actFunc;
} STATFUNC;


typedef struct {
  u8* name;
  u8  paramType;
  u8  avail;
} OPCODE;

typedef struct {
  u8* name;
  u32 func;
  u8  type;
} BUTTON_FUNC;

typedef struct {
  u8* name;
  u8  stat;
  u8  disp;
} STAT_PRESET;



typedef struct {
  u32 name;
  u32 size;
} FG_CHUNK;
typedef struct __attribute__((packed)) {
  u8  id;
  u8  buttonCount;
  u8  setID; // 1-based; 0 = none
  u16 connectedUnit;
  u8  hasStatInfo;
} FG_UNIT;
typedef struct __attribute__((packed)) {
  u8  debugId;
  u8  statFuncId;
  u8  dispFuncId;
} FG_STATINFO;
typedef struct __attribute__((packed)) {
  u8  id; // 0-based
  u8  count;
  BUTTON buttons[1];
} FG_BUTTONSET;
typedef struct __attribute__((packed)) {
  u8  id;
  u8  count;
  u16 opcodes[1];
  //u8  used;
} FG_REQSCRIPT;
typedef struct __attribute__((packed)) {
  s32 id; // -1 = no id
  u8  isFolder;
  u32 depth;
  char string[1];
} FG_TREENODE;
