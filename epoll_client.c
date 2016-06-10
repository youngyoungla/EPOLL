

#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>

	
void usage(char* proc)
{
  printf("Usage:%s[ip][port]\n",proc);
}
int main(int argc,char* argv[])
{
	if(argc!=3)
	{
		usage(argv[0]);
		exit(1);
	}
char* ip=argv[1];
int port=atoi(argv[2]);

	//socket
	int sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock<0)
	{
		perror("sock");
		exit(2);
	}
	struct sockaddr_in remote;
	remote.sin_family=AF_INET;
	remote.sin_port=htons(port);
	remote.sin_addr.s_addr=inet_addr(ip);

     int ret=connect(sock,(struct sockaddr*)&remote,sizeof(remote));
	 if(ret<0)
	 {
       perror("coneect");
	 }

	 char buf[1024];
	 while(1)
	 {
		 memset(buf,'\0',sizeof(buf));
		 read(0,buf,sizeof(buf)-1);
		 write(sock,buf,sizeof(buf));
		 sleep(1);
		 ssize_t _s=read(sock,buf,sizeof(buf)-1);
		 if(_s>0)
		 {
			 buf[_s]='\0';
			 printf("echo:%s",buf);
			 fflush(stdout);
		 }
	 }
	 return 0;
}
