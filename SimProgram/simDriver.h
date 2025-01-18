// protect from multiple compiling
#ifndef SIM_DRIVER_H
#define SIM_DRIVER_H

// header files
#include <stdio.h>
#include "datatypes.h"
#include "StringUtility.h"
#include "configops.h"
#include "metadataops.h"
#include "simulator.h"

// Program constants
typedef enum {MIN_NUM_ARGS = 3, LAST_FOUR_LETTERS = 4} PRGRM_CONSTANTS;

//Command line struct for storing command line switches settings
typedef struct CmdLineDataStruct
   {
      bool programRunFlag;
      bool configDisplayFlag;
      bool mdDisplayFlag;
      bool runSimFlag;

      char fileName[STD_STR_LEN];
   }  CmdLineData;

// Function prototypes

/*
Name: processCmdLine
Process: check for at least two arguments, 
         then sets Booleans depending on command line switches
         which can be in any order,
         also captures config file name which must be the last argument
Function Input/Parameters: number of arguments (int),
                           vector of arguments (char **)
Function Output/Parameters: pointer to command line structure (CmdLineData *)
                            with updated members,
                            set to default values if failure
                            to capture arguments
Function Output/Returned: Boolean result of argument capture,
                          true if at least one switch and config file name, 
                          false otherwise.
Device Input/device: none
Device Output/device: none
Dependencies: 
*/
bool processCmdLine(int numArgs, char **strVector, CmdLineData *clDataPtr);

/*
Name: showCommandLineFormat
Process: displays command line format as assistance to user
Function Input/Parameter: none
Function Output/Parameters: none
Function Output/Returned: none
Device Input/device: none
Device Ouput/monitor: data displayed as specified
Dependencies: printf
*/
void showCommandLineFormat();

/*
Name: clearCmdLineStruct
Process: sets command line structure data to defaults
         Boolean to false, fileName to empty string
Function Input/Parameters: pointer to command line structure (CmdLineData *)
Function Output/Parameters: pointer to command line structure (CmdLineData *)
                            with updated members
Function Output/Returned: none
Device Input/device: none
Device Output/device: none
Dependencies: none
*/
void clearCmdLineStruct(CmdLineData *clDataPtr);

#endif // SIM_DRIVER_H