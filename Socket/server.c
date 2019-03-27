#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<netdb.h>


int main(void) 
{
int serverSocket,client_connected,len;
struct sockaddr_in client_addr,server_addr;
struct hostent *ptrh;
int n=0; 
char message[100],received[100];

serverSocket=socket(AF_INET, SOCK_STREAM, 0);

memset((char*)&server_addr,0,sizeof(server_addr));

server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(10000);

server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

if(bind(serverSocket,
(struct sockaddr*)&server_addr,sizeof(server_addr)) == -1)
printf("Bind Failure\n");
else
printf("Bind Success:<%u>\n", serverSocket);

listen(serverSocket,5);
len=sizeof(struct sockaddr_in);

client_connected=accept(serverSocket,(struct sockaddr*)&client_addr,&len);
if (-1 != client_connected)
  printf("Connection accepted:<%u>\n", client_connected);
int y=0;
int i=0;
int input;
const float val=29.98;

while(1)
{
    int rb=read(client_connected,&input, sizeof(input));
    if(rb==sizeof(input))
    {
    	printf("Message received from client is %d\n",input);
	if(input==1)
        {    	
        int rw=send(client_connected,(void*)&val,sizeof(val)+1,0);
        }
    }	 
}

close(serverSocket);
 printf("\nServer Socket Closed !!\n");
return 0;
}
