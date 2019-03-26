#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mq.h"

#define LOGFILE_NAME "mylogfile.txt"

void logToFile(logStruct dataToReceive);
char *printTimeStamp();