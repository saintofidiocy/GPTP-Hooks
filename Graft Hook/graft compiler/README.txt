Quick & dirty compiler/decompiler to generate .graft files from .txt scripts.

!!! WARNING !!!
Graft file formats are still a work in progress! This tool is mostly just to help before PyGRAFT exists.




Use batch files to make it easy:

generate default scripts.bat -- Dumps entire default scripts from starcraft.exe
compile scripts.bat          -- Compiles scripts as they are to .graft files
merge scripts.bat            -- Merges scripts with starcraft.exe data to generate .graft files
datext fg helper.bat         -- Drag & Drop *.fgp file to decompile (doesn't have to be a datext fgp),
                                  or run on its own and follow the prompts for DatExt graft files


Batch files uses the following filenames for inputs and outputs:
Buttons.txt
Units.txt
Upgrades.txt
Research.txt
Tech.txt
Orders.txt
Buttons.graft
Units.graft
Upgrades.graft
Research.graft
Tech.graft
Orders.graft






Or you can try command-line, but it's pretty messy:

arguments:
  -c          Compile mode (Outputs graft files)
  -d          Decompile mode (Output txt files)
  -s          Load Starcraft.exe data
  -f  [path]  Load specified FGP file
  -n? [num]   Sets number of IDs for each type (for datext files)
  -i? [path]  Specifies input files for each type
  -o? [path]  Specifies output files for each type

Where '?' is one of:           Default Count:
  b  Buttons                   250 (unit IDs with buttons beyond 228)
  u  Units                     228
  p  Upgrades                  61
  t  Tech                      44
  r  Research                  (Alias of Tech for counts)
  o  Orders                    189

Examples:
graftc -d -f sample.fgp -ob Buttons.txt -ou Units.txt -op Upgrades.txt
graftc -c -s -ib Buttons.txt -iu Units.txt -ob Buttons.graft -ou Buttons.graft






---- Dat Req txt format ----


# comment -- can be on any line, anything after it is ignored

#DAT REQ:
1:  # ID 1
  0xFF02   # an opcode
  111      # a parameter
  65288    # can be hex or dec
  112      # other parameter
  0xFFFF   # end of list





---- Buttons.txt format ----


#BUTTONS:
1:  # buttonset ID -- can technically be anything, but should be something reasonable since button set array is contiguous (don't use huge numbers)
# position, icon ID, condition ID, action ID, condition parameter, action parameter, enable string, disable string
# should it be "pos, icon, disable str, enable str, condition ID, condition param, action ID, action param" ??
1, 130, 1, 2, 0, 0, 373, 0
2, 248, 3, 4, 0, 0, 372, 0

Unit 1:  # unit ID
# buttonset ID[, connected Unit = 65535[, stat func ID, disp func ID]]

Unit 1: 1              # connected unit defaults to 65535

Unit 2: 1, 65535

Unit 3: 1, 65535, 2, 1 # if stat info is omitted then it will not be updated

unit 4: -1             # sets button set to None


