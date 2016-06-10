#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<fcntl.h>
#include<assert.h>
#include<unistd.h>
#include<errno.h>

#define MAX_NUM 10
#define BUF_SIZE 10240
typedef struct _data_buf  //?
{
int fd;
char buf[BUF_SIZE];
}databuf_t,*databuf_p;
void usage(char* proc)
{
	printf("%s:[ip][port]\n",proc);
	exit(-1);
}
int start_up(char* ip,int port)
{
  assert(ip);
  //socket
  int sock=socket(AF_INET,SOCK_STREAM,0);
  if(sock<0)
  {
	  perror("sock");
	  exit(1);
  }
  struct sockaddr_in local;
  local.sin_addr.s_addr=inet_addr(ip);
  local.sin_port=htons(port);
  local.sin_family=AF_INET;
  
  //bind
  if(bind(sock,(struct sockaddr*)&local,sizeof(local))<0)
  {
	  perror("bind");
	  exit(1);
  }
  //listen
  if(listen(sock,5)<0)
  {
	  perror("listen");
	  exit(2);
  }
  return sock;
}
void server(int listen_sock)
{
  int epoll_fd=epoll_create(256);
  if(epoll_fd<0)
  {
	  perror("epoll_create");
	  exit(3);
  }
  int done=0;
  int new_sock=-1;
  int num=0;
  int i=0;
  int max=MAX_NUM;

  struct epoll_event ev;
  ev.data.fd=listen_sock;
  ev.events=EPOLLIN|EPOLLET;
 if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,listen_sock,&ev)<0)
 {
	 perror("epoll_ctl");
	 exit(1);
 }
  struct epoll_event ev_out[MAX_NUM];//why?
while(!done)
 {
  switch(num=epoll_wait(epoll_fd,ev_out,MAX_NUM,5000))
  {
	  case 0:
		  printf("timeout\n");
		  break;
	case -1:
		  perror("epoll_wait");
		  break;
	default:
		  {
	        for(i=0;i<num;i++)
			{
				if(ev_out[i].data.fd==listen_sock&&(ev_out[i].events\
							&EPOLLIN))
				{
				   struct sockaddr_in client;
				   socklen_t len=sizeof(client);
				   int new_sock=accept(listen_sock,\
						   (struct sockaddr*)&client,&len);
				   if(new_sock<0)
				   {
					   perror("new_sock");
					   exit(2);
				   }
                   ev.data.fd=new_sock;
                   ev.events=EPOLLIN|EPOLLET;
				   printf("get connection\n");
				   
                  if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,new_sock,&ev)<0)
                  {
	                perror("epoll_ctl");
	                exit(1);
                  }
				  
				  

				}
			   else 
			   {
				  if(ev_out[i].events&EPOLLIN)
				  {
				
				   int fd=ev_out[i].data.fd;//why not using buf?
                   databuf_p mem=(databuf_p)malloc(BUF_SIZE);
				   mem->fd=fd;
				   ssize_t _s=read(mem->fd,mem->buf,sizeof(mem->buf)-1);
				   if(_s>0)
				   {
                    mem->buf[_s]='\0';
					 printf("%s",mem->buf);
					 fflush(stdout);
					 ev.events=EPOLLOUT;
					 ev.data.ptr=mem;
					 epoll_ctl(epoll_fd,EPOLL_CTL_MOD,fd,&ev);

				   }
				   else if(_s==0)
				   {
					   printf("client closed\n");
					   epoll_ctl(epoll_fd,EPOLL_CTL_DEL,fd,NULL);
					   free(mem);
					   exit(1);
				   }
				   else
				   {
					   perror("read");
					   exit(1);
				   }
				   }
				 else if(ev_out[i].events&EPOLLOUT)
				 {
			        databuf_p mem=(databuf_p)ev_out[i].data.ptr;	
					int fd=mem->fd;
					write(mem->fd,mem->buf,strlen(mem->buf));
                    ev.events=EPOLLIN;
					ev.data.fd=fd;
					epoll_ctl(epoll_fd,EPOLL_CTL_MOD,fd,&ev);
					free(mem);
				 }
				 else
				 {}
			   }
			}
				
		  }
  }
 }
}
 

int main(int argc,char* argv[])
{
	if(argc!=3)
	{
		usage(argv[0]);
		exit(1);
	}
    int port=atoi(argv[2]);
	char* ip=argv[1];
    int sock=start_up(ip,port);
	server(sock);
	return 0;
}
