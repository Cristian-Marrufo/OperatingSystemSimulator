// protect from multiple compiling
#ifndef SIMULATOR_H
#define SIMULATOR_H

// header files
#include "StringUtility.h"
#include "datatypes.h"
#include "simtimer.h"
#include "simDriver.h"
#include <pthread.h>

typedef struct PCBstruct
   {
      int processID;
      int time;
      ProcessState currentState;
      struct PCBstruct *nextNode;
   } PCB;

typedef struct MemoryManagerstruct
   {
      int usedMemory;
      int maxMemory;
   } MemoryManager;

typedef struct Interruptstruct 
{
    int processID;
    char interruptType[20];
    struct Interruptstruct *next;
} Interrupt;

typedef struct InterruptManagerstruct 
{
    Interrupt *head;
    Interrupt *tail;
    int size;
    pthread_mutex_t lock;
} InterruptManager;


// Function prototypes

/*
Name: runSim
Process: primary simulation driver
Function Input/Parameters: configuration data (ConfigDataType *),
                           metadata (OpCodeType *)
Function Output/Parameters: none
Function Output/Returned: none
Device Input/device: none
Device Output/device: none
Dependencies:
*/
void runSim(ConfigDataType *configPtr, OpCodeType *metaDataMstrPtr);
void checkDisplayOptions(ConfigDataType *configPtr, bool *monitor, bool *file);
void displayOutput(char *outputStr, bool monitorFlag, bool fileFlag, 
                                                                 FILE *filePtr);
PCB *generateProcessArray(OpCodeType *metaDataMstrPtr, bool monitorFlag, 
                                                  bool fileFlag, FILE *filePtr);
PCB *createProcess(PCB *processPtr, int processID);
void processOutput(int processID, char *strArg1, char *inOutArg, char *command, 
                                      char *time, char *state, char *outputStr);
void *waitForProcess(void *metaPtr);
void processThread(OpCodeType *metaPtr);
void clearPCBArray(PCB *head);
void scheduleProcesses(PCB **processArray, int schedCode);
void sortProcessArray(PCB **head);
bool allocateMemory(MemoryManager *memoryCapacity, int base, int offset);
void displayMemory(MemoryManager *memoryCapacity);

void processInterrupts(InterruptManager *interruptManager, PCB **processArray);
void addInterrupt(InterruptManager *interruptManager, int processID, 
                                                              const char *type);
Interrupt *fetchInterrupt(InterruptManager *interruptManager);
PCB *fetchNextProcess(PCB **processArray, int schedCode);
void roundRobinScheduling(PCB **processArray, int quantum);
void simulateExecution(PCB *currentProcess, ConfigDataType *configPtr, 
                                            InterruptManager *interruptManager);
PCB *findProcessByID(PCB *head, int processID);

#endif // SIMULATOR_H