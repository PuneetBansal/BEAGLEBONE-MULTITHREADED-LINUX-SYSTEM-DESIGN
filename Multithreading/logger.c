#include "logger.h"


void logToFile(logStruct dataToReceive)
{
    FILE *logging;
	//printf("value of timestamp in logtofile function is %s\n",printTimeStamp());
    if(strcmp(dataToReceive.source,"temperature")==0)
    {
        logging = fopen(LOGFILE_NAME,"a");
       if(dataToReceive.status==success)
       {            
            fprintf(logging,"%s %s value is %f\n",printTimeStamp(),dataToReceive.source,dataToReceive.value);
            
            if(strcmp(dataToReceive.unit,"Celcius")==0)
			{
                fprintf(logging, " Celsius\n");
            }
			else if(strcmp(dataToReceive.unit,"Fahrenheit")==0)
			{
                fprintf(logging, " Fahrenheit\n");
            }
            else if(strcmp(dataToReceive.unit,"Kelvin")==0)
			{
                fprintf(logging, " Kelvin\n");
            }
					
		}
       else
       {
           fprintf(logging, "%s (Failure message) - %s\n",printTimeStamp(),dataToReceive.message);
       }
    fclose(logging);
    }

    if(strcmp(dataToReceive.source,"light")==0)
    {
        logging = fopen(LOGFILE_NAME,"a");
       if(dataToReceive.status==success)
       {            
            fprintf(logging,"%s %s value is %f\n",printTimeStamp(),dataToReceive.source,dataToReceive.value);
        }
       else
       {
           fprintf(logging, "%s (Failure message) - %s\n",printTimeStamp(),dataToReceive.message);
       }
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
