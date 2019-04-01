/************************************************************************************************
* File name   : client.c                                                                        *
* Authors     : Nachiket Kelkar and Puneet Bansal                                               *
* Description : The functions definition for client                                             *
* Tools used  : GNU make, gcc, arm-linux-gcc.                                                   *
************************************************************************************************/
https://stackoverflow.com/questions/18021189/how-to-connect-two-computers-over-internet-using-socket-programming-in-c 


#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>  
#include<unistd.h>
#include<string.h>
#include<stdlib.h>

#define SERVER_IP_ADDRESS "10.0.0.59"
//#define SERVER_IP_ADDRESS "128.138.189.162"

int main(void)
{
    int clientSocket; /* Socket Decriptor for Client */
    struct sockaddr_in serverAddr;
    struct hostent *ptr;
    int input;
    
    clientSocket=socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket<0)
    {
        perror("Client socket creation failed");
    }

    memset((char*)&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(10000);

    ptr=gethostbyname(SERVER_IP_ADDRESS);
    memcpy(&serverAddr.sin_addr,ptr->h_addr,ptr->h_length);

    if((connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)))==-1) 
    { printf("\nServer Not Ready !!\n"); exit(1); }

float val;
while(1)
{    
    printf("\nWelcome to the Client Socket. Enter the option you want to perform\n");
    printf("1.Get Temperature in Celsius\n2.Get Temperature in Kelvin\n3.Get Temperature in Fahrenheit\n4.Get Lux value\n->");
    scanf("%d",&input);
    send(clientSocket, (void*)&input, sizeof(input)+1,0);    
    
    if(read(clientSocket, &val, sizeof(val))==sizeof(val))
    {
	if(val!=-500)
	{	
		switch(input)
		{
			case 1:
			printf("Temperature value is %f Celsius\n",val);
			break;

			case 2:
			printf("Temperature value is %f Kelvin\n",val);
			break;
	
			case 3:
			printf("Temperature value is %f Fahrenheit\n",val);	
			break;

			case 4:
			printf("Light value is %f \n",val);
			if(val>50)
			printf("The vicinity of sensor has light \n");	
			else
			printf("The vicinity of sensor is dark \n");	
			break;
        
		}
    	}
	else
	{
	printf("Error reading from sensor\n");
	}
     }
}
return 0;
}

