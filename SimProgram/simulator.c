// header files
#include "simulator.h"

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
void runSim(ConfigDataType *configPtr, OpCodeType *metaDataMstrPtr)
   {
      OpCodeType *metaPtr = metaDataMstrPtr;
      PCB *processArray;
      MemoryManager memoryCapacity;
      memoryCapacity.usedMemory = 0;
      memoryCapacity.maxMemory = MAX_MEMORY;
      InterruptManager interruptManager = 
                                      { .head = NULL, .tail = NULL, .size = 0 };
      pthread_mutex_init(&interruptManager.lock, NULL);

      char timeStr[LARGE_STR_LEN];
      char outputStr[LARGE_STR_LEN];
      bool monitorFlag = false;
      bool fileFlag = false;

      FILE *filePtr = NULL;

      checkDisplayOptions(configPtr, &monitorFlag, &fileFlag);

      if( fileFlag )
         {
            filePtr = fopen(configPtr->logToFileName, "w");

            if( filePtr == NULL )
               {
                  printf("\nLog to file failed to open\n");
                  fileFlag = false;
               }
         }

      sprintf(outputStr, "\n\nSimulator Run\n=============\n\n");
      displayOutput(outputStr, monitorFlag, fileFlag, filePtr);

      if( configPtr->cpuSchedCode != CPU_SCHED_FCFS_N_CODE )
         {
            configPtr->cpuSchedCode = CPU_SCHED_FCFS_N_CODE;
         }
      
      accessTimer(ZERO_TIMER, timeStr);

      accessTimer(LAP_TIMER, timeStr);

      sprintf(outputStr, "%s, OS: Simulator start\n", timeStr);
      displayOutput(outputStr, monitorFlag, fileFlag, filePtr);

      processArray = generateProcessArray(metaDataMstrPtr, monitorFlag, 
                                                             fileFlag, filePtr);
      
      if(configPtr->cpuSchedCode == CPU_SCHED_SJF_N_CODE)
         {
            sortProcessArray(&processArray);
         }

      while( processArray != NULL || interruptManager.size > 0 )
         {
            processInterrupts(&interruptManager, &processArray);

            PCB *currentProcess = 
                       fetchNextProcess(&processArray, configPtr->cpuSchedCode);

            if (currentProcess == NULL) 
               {
                  sprintf(outputStr, "%s, OS: CPU idle\n", timeStr);
                  displayOutput(outputStr, monitorFlag, fileFlag, filePtr);
                  continue;
               }

            accessTimer(LAP_TIMER, timeStr);
            sprintf(outputStr, "%s, OS: Process %d selected with %d s" 
            " remaining\n", timeStr, processArray->processID, 
                                                            processArray->time);
            displayOutput(outputStr, monitorFlag, fileFlag, filePtr);

            sprintf(outputStr, "%s, OS: Process %d set to RUNNING\n", timeStr, 
                                                       processArray->processID);
            displayOutput(outputStr, monitorFlag, fileFlag, filePtr);

            simulateExecution(currentProcess, configPtr, &interruptManager);

            while( findSubString(metaPtr->strArg1, "end") == 
                                                           SUBSTRING_NOT_FOUND )
               {
                  
                  if(findSubString(metaPtr->command, "mem") != 
                                                            SUBSTRING_NOT_FOUND)
                     {
                        int base = metaPtr->intArg2;
                        int offset = metaPtr->intArg3;

                        if(!allocateMemory(&memoryCapacity, base, offset))
                           {
                              sprintf(outputStr, "%s, OS: Memmory allocation" 
                                            " failed for process %d\n", timeStr, 
                                                       processArray->processID);
                              displayOutput(outputStr, monitorFlag, fileFlag, 
                                                                       filePtr);
                           }
                        
                        if(configPtr->memDisplay)
                           {
                              displayMemory(&memoryCapacity);
                           }
                     }

                  processOutput(processArray->processID, metaPtr->strArg1, 
                              metaPtr->inOutArg, metaPtr->command, timeStr, 
                                                            "start", outputStr);
                  displayOutput(outputStr, monitorFlag, fileFlag, filePtr);
                  processThread(metaPtr);
                  accessTimer(LAP_TIMER, timeStr);
                  processOutput(processArray->processID, metaPtr->strArg1, 
                            metaPtr->inOutArg, metaPtr->command, timeStr, "end", 
                                                                     outputStr);
                  displayOutput(outputStr, monitorFlag, fileFlag, filePtr);
                  metaPtr = metaPtr->nextNode;
               }
            
            accessTimer(LAP_TIMER, timeStr);
            sprintf(outputStr, "%s, OS: Process %d end\n", timeStr, 
                                                       processArray->processID);
            displayOutput(outputStr, monitorFlag, fileFlag, filePtr);

            sprintf(outputStr, "%s, OS: Process %d set to EXIT\n", timeStr, 
                                                       processArray->processID);
            displayOutput(outputStr, monitorFlag, fileFlag, filePtr);

            processArray = processArray->nextNode;
            metaPtr = metaPtr->nextNode;
         }
      
      sprintf(outputStr, " %s, OS: System stop\n", timeStr);
      displayOutput(outputStr, monitorFlag, fileFlag, filePtr);

      accessTimer(LAP_TIMER, timeStr);
      sprintf(outputStr, " %s, OS: Simulation end\n", timeStr);
      displayOutput(outputStr, monitorFlag, fileFlag, filePtr);
      
      if(filePtr != NULL)
         {
            fclose(filePtr);
         }
      
      clearPCBArray(processArray);
   }

void checkDisplayOptions(ConfigDataType *configPtr, bool *monitorFlag, 
                                                                 bool *fileFlag)
   {
      if( configPtr->logToCode == LOGTO_MONITOR_CODE )
         {
            *monitorFlag = true;
            printf("\nDisplay to Monitor only\n");
         }
      
      else if( configPtr->logToCode == LOGTO_FILE_CODE )
         {
            *fileFlag = true;
            printf("\nDisplay to File only\n");

         }
      
      else if( configPtr->logToCode == LOGTO_BOTH_CODE )
         {
            *monitorFlag = true;
            *fileFlag = true;
            printf("\nDisplay to Monitor and File\n");
         }
      
      else
         {
            printf("\nDisplay option unspecified\n");
         }
   }

void displayOutput(char *outputStr, bool monitorFlag, bool fileFlag, 
                                                                  FILE *filePtr)
   {
      if( monitorFlag )
         {
            printf("%s", outputStr);
         }
      
      if( fileFlag && filePtr != NULL )
         {
            fprintf(filePtr, "%s", outputStr);
         }
      
      
   }

PCB *generateProcessArray(OpCodeType *metaDataMstrPtr, bool monitorFlag, 
                                                   bool fileFlag, FILE *filePtr)
   {
      char timeStr[LARGE_STR_LEN];
      char tempStr[LARGE_STR_LEN];
      OpCodeType *wkgPtr = metaDataMstrPtr;
      PCB *head = NULL;
      PCB *current = NULL;
      int processNumber = 0;

      accessTimer(LAP_TIMER, timeStr);

      while( wkgPtr != NULL )
         {
            if( current != NULL )
               {
                  if( wkgPtr->inOutArg[0] != NULL_CHAR )
                     {
                        current->time += (wkgPtr->intArg2 * 2);
                     }
                  
                  else
                     {
                        current->time += (wkgPtr->intArg2);
                     }
               }
            
            if( findSubString(wkgPtr->strArg1, "end") != SUBSTRING_NOT_FOUND &&
                findSubString(wkgPtr->command, "app") != SUBSTRING_NOT_FOUND )
                {
                  PCB *newProcess = createProcess(NULL, processNumber);

                  if( head == NULL )
                     {
                        head = newProcess;
                        current = head;
                     }
                  
                  else
                     {
                        current->nextNode = newProcess;
                        current = newProcess;
                     }
                  
                  sprintf(tempStr, "%s, OS: Process %d set to READY\n", 
                                                        timeStr, processNumber);
                  displayOutput(tempStr, monitorFlag, fileFlag, filePtr);

                  processNumber++;
                }
            wkgPtr = wkgPtr->nextNode;
         }
      
      return head;
   }

PCB *createProcess(PCB *processPtr, int processID)
   {
      if( processPtr == NULL )
         {
            processPtr = (PCB *)malloc(sizeof(PCB));
            processPtr->processID = processID;
            processPtr->currentState = NEW_STATE;
            processPtr->nextNode = NULL;
         }
      
      return processPtr;
   }

void processOutput(int processID, char *strArg1, char *inOutArg, char *command, 
                                       char *time, char *state, char *outputStr)
   {
      char tempStr[LARGE_STR_LEN];
      outputStr[0] = '\0';

      sprintf(tempStr, "%s, Process: %d, ", time, processID);

      if( findSubString(command, "cpu") != SUBSTRING_NOT_FOUND )
         {
            sprintf(tempStr + getStringLength(tempStr), "%s ", command);
         }
      
      sprintf(tempStr + getStringLength(tempStr), "%s ", strArg1);

      if( inOutArg[0] != NULL_CHAR )
         {
            if(findSubString(inOutArg, "in") != SUBSTRING_NOT_FOUND )
               {
                  sprintf(tempStr + getStringLength(tempStr), "input ");
               }
            
            else
               {
                  sprintf(tempStr + getStringLength(tempStr), "output ");
               }
         }
      
      sprintf(tempStr + getStringLength(tempStr), "operation %s\n", state);
      copyString(outputStr, tempStr);
   }

void *waitForProcess(void *metaPtr)
   {
      OpCodeType *opCode = (OpCodeType *)metaPtr;

      char timeStr[LARGE_STR_LEN];
      double timeStart, currentTime, timeEnd;

      accessTimer(LAP_TIMER, timeStr);
      timeStart = atof(timeStr);

      if( opCode->inOutArg[0] != NULL_CHAR )
         {
            opCode->intArg2 = (opCode->intArg2 * 2);
         }
      
      timeEnd = timeStart + ((double)opCode->intArg2 / 100.0);

      while( currentTime < timeEnd )
         {
            accessTimer(LAP_TIMER, timeStr);
            currentTime = atof(timeStr);
         }
      
      pthread_exit(NULL);
   }

void processThread(OpCodeType *metaPtr)
   {
      pthread_t threadID;

      pthread_create(&threadID, NULL, waitForProcess, (void *)metaPtr);

      pthread_join(threadID, NULL);
   }

void clearPCBArray(PCB *head)
   {
      PCB *temp;

      while(head != NULL)
         {
            temp = head->nextNode;
            free(head);
            head = temp;
         }
   }

void scheduleProcesses(PCB **processArray, int schedCode)
   {
      if(schedCode == CPU_SCHED_FCFS_N_CODE)
         {
            sortProcessArray(processArray);
         }
   }

void  sortProcessArray(PCB **head)
   {
      PCB *current, *index;

      int tempID, tempTime;
      ProcessState tempState;

      for(current = *head; current != NULL; current = current->nextNode)
         {
            for(index = current->nextNode; index != NULL; 
                                                        index = index->nextNode)
               {
                  if(index->time < current->time)
                     {
                        tempID = current->processID;
                        tempTime = current->time;
                        tempState = current->currentState;

                        current->processID = index->processID;
                        current->time = index->time;
                        current->currentState = index->currentState;

                        index->processID = tempID;
                        index->time = tempTime;
                        index->currentState = tempState;

                     }
               }
         }
   }

bool allocateMemory(MemoryManager *memoryCapacity, int base, int offset)
   {
      int requestedMemory = offset - base;

      if(memoryCapacity->usedMemory + requestedMemory > MAX_MEMORY)
         {
            printf("Memory Allocation Error: Not enough memory\n");
            return false;
         }
      
      memoryCapacity->usedMemory += requestedMemory;
      printf("Memory allocated from %d to %d\n", base, offset);
      return true;
   }

void displayMemory(MemoryManager *memoryCapacity)
   {
      printf("Current memory usage: %d KB\n", memoryCapacity->usedMemory);
   }


void addInterrupt(InterruptManager *interruptManager, int processID, 
                                                               const char *type) 
   {
      pthread_mutex_lock(&interruptManager->lock);
      Interrupt *newInterrupt = (Interrupt *)malloc(sizeof(Interrupt));
      if (newInterrupt == NULL) 
         {
            fprintf(stderr, 
                       "Error: Unable to allocate memory for new interrupt.\n");
            pthread_mutex_unlock(&interruptManager->lock);
            return;
         }

      newInterrupt->processID = processID;
      strcpy(newInterrupt->interruptType, type);
      newInterrupt->next = NULL;

    
      if (interruptManager->size == 0) 
         {
            interruptManager->head = newInterrupt;
            interruptManager->tail = newInterrupt;
         }

      else 
         {
            interruptManager->tail->next = newInterrupt;
            interruptManager->tail = newInterrupt;
         }

      interruptManager->size++;

      pthread_mutex_unlock(&interruptManager->lock);
   }

Interrupt *fetchInterrupt(InterruptManager *interruptManager) 
   {
      pthread_mutex_lock(&interruptManager->lock);

      if (interruptManager->size == 0) 
         {
            pthread_mutex_unlock(&interruptManager->lock);
            return NULL;
         }

    
      Interrupt *interrupt = interruptManager->head;
      interruptManager->head = interrupt->next;
      if (interruptManager->head == NULL) 
         {
            interruptManager->tail = NULL;
         }

      interruptManager->size--;

      pthread_mutex_unlock(&interruptManager->lock);

      return interrupt;
   }

void processInterrupts(InterruptManager *interruptManager, PCB **processArray) 
   {
      Interrupt *interrupt = fetchInterrupt(interruptManager);
      while (interrupt != NULL) 
         {
            printf("Processing interrupt: %s for Process %d\n", 
                                interrupt->interruptType, interrupt->processID);

            PCB *process = findProcessByID(*processArray, interrupt->processID);
            if (process != NULL) 
               {
                  process->currentState = READY_STATE;
               }

            free(interrupt);
            interrupt = fetchInterrupt(interruptManager);
         }
   }

PCB *fetchNextProcess(PCB **processArray, int schedCode) 
   {
      // Logic for SRTF-P and RR-P scheduling
      if (schedCode == CPU_SCHED_RR_P_CODE) 
         {
            PCB *process = *processArray;
            *processArray = process->nextNode;
            return process;
         }

      // Default FCFS
      return *processArray;
   }

void simulateExecution(PCB *currentProcess, ConfigDataType *configPtr, 
                                             InterruptManager *interruptManager) 
   {
      // Simulate one cycle of execution
      if (--currentProcess->time == 0) 
         {
            currentProcess->currentState = EXIT_STATE;
         }

    else 
      {
        addInterrupt(interruptManager, currentProcess->processID, 
                                                            "Quantum Complete");
      }
   }

PCB *findProcessByID(PCB *head, int processID) {
    PCB *current = head;
    while (current != NULL) {
        if (current->processID == processID) {
            return current;
        }
        current = current->nextNode;
    }
    return NULL; // Process not found
}