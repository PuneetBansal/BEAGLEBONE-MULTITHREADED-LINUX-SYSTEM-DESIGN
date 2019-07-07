# BEAGLEBONE MULTITHREADED LINUX SYSTEM DESIGN 
                                    Contributors  : Puneet Bansal and Nachiket Kelkar
                                    Professor     : Richard Hiedebercht
                                    

## OVERVIEW :
A multithreaded beaglebone console application to concurrently monitor 2 sensors (temperature and light) connected to the same I2C bus and log data along with exceptional conditions to a single file on the system.Additionally, an external interfacing task services requests from an off-system host inquiring the system’s status/logs. 

- Main Task 
  1. The mainTask is the parent thread which creates 4 children tasks : Temperature Sensor Task, Light Sensor Task, Logger Task & Socket          Server Task: .
  2. Checks for heartbeat messages from children task to ensure that all tasks are still alive and running on a regular interval.
  3. In the event of stopping the application, maintask cleans up all the initialisation and safely exits. 
  4. Performs Built In Self Test and logs error information to the console user.
  
- Temperature Sensor Task : 
  Responsible to communicate with TMP102 temperature sensor, to provide readings to the user and logging it in the log file.
  In case of sensor pull out, performs safe exit. 
  
- Light Sensor Task : Responsible for taking the LUX value from APDS 9301 Ambient Light Photo Intensity Sensor every 100 ms and log it to the log file. 

- Logger Task : Logs all the data to a log file 

- Socket Server Task : Acts as a TCP server and accepts connection from the client with various requests suchs as getting temperature and lux value from the sensor.

## SYSTEM ARCHITECTURE :
![SystemArchitecture](https://github.com/PuneetBansal/BEAGLEBONE-MULTITHREADED-LINUX-SYSTEM-DESIGN/blob/master/Images/System%20Architecture.jpg)

## FEATURES :
- Interface to 2 offboard sensors.
- Threads operating in event loop
- Synchronised buffer to collect logs from thread. 
- Built in Self Test and Unit Test implementation.
- Heartbeat mechanism to make sure all threads are running at regular interval
- Logger Task implementation with different Log Levels : DEBUG , ERROR, INFO
- All the tasks have their own message queues and structure. Any task who wants to communicate with the other task needs to populate the   other task’s structure and send it to it’s message queue.

## HARDWARE COMPONENTS: 
- BeagleBone Green 
- Temperature Sensor
- Luminous Intensity Sensor

## DOCUMENTATION :
- Detailed Project Report Can be found [here](https://github.com/PuneetBansal/BEAGLEBONE-MULTITHREADED-LINUX-SYSTEM-DESIGN/blob/master/Project%20Report/AESDProject1_Report.pdf)
- Each header(.h) file contains descriptive headers of each function.

## FOLDER STRUCTURE :
- Project Report Folder: Contains project report and code dump.
- Unit Testing Folder: Consists 3 unit tests that run on beaglebone.
- Socket Folder: Client file is present in this folder.
- Kernel Module : Contains linux loadable kernel module (extra credit)

## HOW TO RUN THE PROJECT : 
- Perform 'make maintask_cc' to generate the executable. 
- Transfer the generated executable via scp to BeagleBone Green and run it.




