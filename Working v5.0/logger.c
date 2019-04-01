/************************************************************************************************
* File name   : logger.c                                                                        *
* Authors     : Puneet Bansal and Nachiket Kelkar                                               *
* Description : The function definition used for communication by logger task.                  *
* Tools used  : GNU make, gcc, arm-linux-gcc                                                    *
************************************************************************************************/
#include "logger.h"
extern int count;


void logToFile(char *fileName, logStruct dataToReceive)
{
   
    //printf("value of count is %d",count);
    FILE *logging;
    char level[20];
    //char state[20];
    

    if(dataToReceive.logLevel==alert)
    {
        strcpy(level,"[ALERT]");
    }
    else if(dataToReceive.logLevel==info)
    {
        strcpy(level,"[INFO]");   
    }
    else
    {
        strcpy(level,"[DEBUG]");
    }

  
    //mainInfoToOthers fileInfo;
    //printf("file name in logtofile function is %s\n",fileName);
	//printf("debug level is %s\n",level);
    //printf("value of timestamp in logtofile function is %s\n",printTimeStamp());
    if(strcmp(dataToReceive.source,"temperature")==0)
    {
       //printf("Recevied unit is %s\n",dataToReceive.unit);
        if(count ==1)
        {
            logging = fopen(fileName,"w");
            count=0;
        }
        else
        {
            logging = fopen(fileName,"a");
        }
        
               
       if(dataToReceive.status==success)
       {            
            fprintf(logging,"%s %s [%s] : %f %s\n",printTimeStamp(),level,dataToReceive.source,dataToReceive.value,dataToReceive.unit);
           
        }
       else
       {
           fprintf(logging, "%s %s [%s] (Failure message) - %s\n",printTimeStamp(),level,dataToReceive.source,dataToReceive.message);
       }
    fclose(logging);
    }

    if(strcmp(dataToReceive.source,"light")==0)
    {
       if(count ==1)
        {
            logging = fopen(fileName,"w");
            count=0;
        }
        else
        {
            logging = fopen(fileName,"a");
        }

    if(dataToReceive.status==success)
       {            
            fprintf(logging,"%s %s [%s] : %f\n",printTimeStamp(),level,dataToReceive.source,dataToReceive.value);
        }
       else
       {
           fprintf(logging, "%s %s [%s] %s\n",printTimeStamp(),level,dataToReceive.source,dataToReceive.message);
       }
    
    fclose(logging);
    
    }

    if(strcmp(dataToReceive.source,"socket")==0)
    {
       if(count ==1)
        {
            logging = fopen(fileName,"w");
            count=0;
        }
        else
        {
            logging = fopen(fileName,"a");
        }
    
        fprintf(logging, "%s %s [%s] %s\n",printTimeStamp(),level,dataToReceive.source,dataToReceive.message);
       
    
    fclose(logging);
    
    }

    if(strcmp(dataToReceive.source,"main")==0)
    {
       if(count ==1)
        {
            logging = fopen(fileName,"w");
            count=0;
        }
        else
        {
            logging = fopen(fileName,"a");
        }
    
        fprintf(logging, "%s %s [%s] %s\n",printTimeStamp(),level,dataToReceive.source,dataToReceive.message);
       
    
    fclose(logging);
    
    }
}

char* printTimeStamp()
{
    char* time_stamp=malloc(40);
    struct timespec thTimeSpec;
    clock_gettime(CLOCK_REALTIME, &thTimeSpec);
    sprintf(time_stamp,"[s: %ld, ns: %ld]",thTimeSpec.tv_sec,thTimeSpec.tv_nsec);
    //printf("Value of time_stamp is %s",time_stamp);
    return time_stamp;
}
