#include <sys/types.h>  
#include <sys/socket.h>  
#include <stdio.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <unistd.h>  
#include <stdlib.h>  
#include <string.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/time.h>
#include <resolv.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <error.h> //
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "mystr.h"

#define MIN_COMMAND_LEN 5
#define MAX_COMMAND_LEN 50
#define BUFFER_SIZE 1024 //message mode
#define MAXBUF 100
#define WD_SERVER_PORT 9802

int check_flow = 0; //Define a global variable, used to check_flow 
int stop_stream_flag = 0;//if the stream is stop we make the flag to 0,,,at 1106
int watchdog = 0;//we use this global variable to make the watchdog function
int restart_flag = 0;//we use this flag,that we know the result we logging the server

int success_flag = 0; //if we sent the message success,turn the success_flag to 1
#if 0
char success_send[] = {0xF2,0x15,0x05,0x01,0x0D};
char faild_send[] = {0xF2,0x15,0x05,0xF1,0xFD};
#endif

char g_user[16] = {"board1"};
char g_passwd[16] = {"1"};

/*we send the flow to the server*/
int atcommand_to_server_pending = 0;
char atcommand_to_server[MAXBUF];

/*wen send the all the message to server*/
int atcommand_to_server_allmessage = 0;
char atcommand_to_servermesg[BUFFER_SIZE];

char return_command[2*MAX_COMMAND_LEN]; //the number that the server want to send.the length is 11
char return_detail[MAX_COMMAND_LEN] ; //the length is 5
int return_flag = 0;

int no_message_flag = 0; //stand no message
char no_message[MIN_COMMAND_LEN];

int getnum_flag = 0;
char phonenum[20];

int  g_pipefd[2];

int turn_on = 0, turn_off = 1;

int commanlen;

//by li
enum gst_wd_type{
GST_NEED_NONE=0,
GST_NEED_RESET,
};

enum hearbeat_wd_type{
	HEARTBEAT_NEED_NONE=0,
	HEARTBEAT_NEED_RESET,
};

int g_heartbeat_reset = HEARTBEAT_NEED_NONE;//we use this global variable to monitor hearbeat
int g_gst_reset = GST_NEED_NONE;//we use this global variable to monitor the gstreamer status.
//by li


enum
{
	START=0,
	LOGGING=1,
	PROCESSING=2,
	HEARTBEAT=3
};

struct circular_buffer
{
	char buf[MAXBUF*2];
	int read_pos;//used by reader
	int write_pos;//used by writer
	int empty_len;
}cbuffer;

int login_c(int fd);
int process_command(char buffer[],int sockfd);
int get_emptylen(struct circular_buffer* cubuffer);
int writecommand(char buffer[],struct circular_buffer *cbuffer,int len);
int getcommand(char buffer[],struct circular_buffer *cbuffer,int len);
int gst_start_stream(int fd);
int gst_stop_stream(int fd);


int main(int argc,char *argv[])  
{
	int sockfd = 0;
	int len;  
	struct sockaddr_in address;     
	int result;   
	int keepflag = 0;
	int choice;
	int ret;
	int flag = 1;
	int state;
	int state_logging_c,state_processing_c, state_connect_fail=0;
	char buffer[MAXBUF] = {0};
	char heartbeat[] = {0xF2,0x07,0x05,0x01,0xFF};//the command that the heartbeat
	char reset[] = {0xF2,0x13,0x05,0x01,0x0B};//reset that  logging
	char change[] = {0xF2,0x13,0x05,0xF1,0xFB};//change the state status

	char success_send[] = {0xF2,0x15,0x05,0x01,0x0D};
	char faild_send[] = {0xF2,0x15,0x05,0xF1,0xFD};
	pid_t pid_gst;
  
	fd_set rfds;
	struct timeval tv;
	int retval, maxfd = -1;
	
 
	if(argc != 3) 
	{  
		printf("Usage: fileclient <address> <port>/n");
		return 0;  
	}
  
  	if(pipe(g_pipefd) < 0) //create the pipe
  	{
  		printf("create pipe error\n");
  		return 0;
  	}

  	if((pid_gst=fork())<0)
  	{
  		printf("fork error\n");
  	}
  	else if(pid_gst == 0) // is child;
  	{
  		close(g_pipefd[1]);//close the child's write end
  		if(g_pipefd[0] != STDIN_FILENO)
  		{
  			if(dup2(g_pipefd[0],STDIN_FILENO) != STDIN_FILENO) //copy the stdin to the child's read 
  			{
  				printf("dup2 error to stdin\n");
  			}
  			close(g_pipefd[0]);//this time the new g_pipefd[0] is stdin,we close the old g_pipefd[0]	
  		}
  		execl("/home/media/work/videopush.out","videopush.out",g_user,(char*)0);//videopush.out will modify the udp port according to the username.
  	}
  //parent
  	close(g_pipefd[0]);//close the parents' read end

	sockfd = 0;
	state = START;
	cbuffer.read_pos=0;
	cbuffer.write_pos=0;
	cbuffer.empty_len=MAXBUF*2;
	
  	atcommand_process(); //send the message to 10010

  //	watchdog = 0;//crate the watchdog thread
  	watchdog_process();  //we process the watchdog
        
	while(1)
	{
		switch(state){
			case START:
				if(sockfd !=0)
				close(sockfd);
				if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1) 
				{  
					perror("sock");  
					state = START;
					break;
				} 
				bzero(&address,sizeof(address));  
				address.sin_family = AF_INET;  
				address.sin_port = htons(atoi(argv[2]));  
				inet_pton(AF_INET,argv[1],&address.sin_addr);  
				len = sizeof(address);  
   
				if((result = connect(sockfd, (struct sockaddr *)&address, len))==-1) 
				{  
					sleep(2);
					perror("connect"); 
					state_connect_fail ++ ;
					printf("connect's count is %d\n",state_connect_fail);
					if(state_connect_fail > 5)
					{
					//	watchdog = 1;
						g_heartbeat_reset=HEARTBEAT_NEED_RESET;
					}
					state = START;
					break;
				}
				state = LOGGING;
				state_logging_c=0;
				break;
	
		case LOGGING:
			ret = login_c(sockfd);//1 stands for success
			state_logging_c+=1;
			if(ret < 0)//failure
				{
					if(state_logging_c > 10)
						state = START;
					else
						state = LOGGING;
					break;
				}
			printf("LOGIN SUCESS!\n");
			if(restart_flag == 0)
			{
				printf("the network is wrong,reset\n");
				send(sockfd,reset,5,0);
			}
			else
			{
				printf("change the state logging\n");
				send(sockfd,change,5,0);
			}	
			keepflag = 0;
			state_connect_fail=0;

			state = PROCESSING;	
			state_processing_c = 0;

      		restart_flag = 0;
			break;
	
		case PROCESSING:
			state_processing_c++;
			FD_ZERO(&rfds);
			FD_SET(0, &rfds);
			maxfd = 0;

			FD_SET(sockfd, &rfds);
			if (sockfd > maxfd)
				maxfd = sockfd;

			tv.tv_sec = 4;//if server don't send any commnad to us within 5s(timeout is 4s,the gpio is 1s), then we send a hearbeat to the server
			tv.tv_usec = 0;

			retval = select(maxfd+1, &rfds, NULL, NULL, &tv);
			if(retval == -1)
			{
				printf("select is error\n");
				if(state_processing_c >10)
				{
					state = START;
					break;
				}
				else
				{
					state = PROCESSING;
					break;
				}
			}
			else if(retval ==0)
			{
				state = HEARTBEAT;
				break;
			}
			//now retval >0 ,we have command to dealwith
			if (FD_ISSET(sockfd, &rfds))
			{
				bzero(buffer, MAXBUF+1);
				len = recv(sockfd, buffer, sizeof(buffer),0);
        		printf("len = %d\n",len);


				if(len > 0){
					keepflag = 0;
					int len_command;
					if(len<=get_emptylen(&cbuffer)){
						//the the empty len is enough,so write the buffer to the circular buffer
						writecommand(buffer,&cbuffer,len);
					}
					//now get the command from the circular buffer 
					
					len_command=getcommand(buffer,&cbuffer,MAXBUF);	
					while(len_command >0){		
						process_command(buffer,sockfd);
						len_command=getcommand(buffer,&cbuffer,MAXBUF);
					}
					
				}
				else
				{
            		state = HEARTBEAT;
					printf("Failed to receive the message! \n");
					break;
				}
			}
		    
			state = state;//keep state on processing state	
			break;
	
		case HEARTBEAT:
			signal(SIGPIPE, SIG_IGN);
			
			if(atcommand_to_server_pending == 1)  //the lost flow
			{
				send(sockfd,atcommand_to_server,strlen(atcommand_to_server),0);//take the same effect as a heartbeat
				atcommand_to_server_pending=0;
				send(sockfd, heartbeat,5/*strlen(heartbeat)*/, 0);
			}

			else if(atcommand_to_server_allmessage == 1) //we receive the message
			{
				send(sockfd,atcommand_to_servermesg,strlen(atcommand_to_servermesg),0);
				atcommand_to_server_allmessage=0;
				send(sockfd,heartbeat,5/*strlen(heartbeat)*/,0);
			}

			else if(no_message_flag == 1) //we couldn't match the lost flow
			{
				send(sockfd,no_message,5,0);
				no_message_flag = 0;
				send(sockfd, heartbeat,5 /*strlen(heartbeat)*/, 0);
			}

			else if(getnum_flag == 1) //get the module's number
			{
				printf("OKOKOKOKOKOK\n");
				send(sockfd,phonenum,strlen(phonenum),0);
				getnum_flag = 0;
				send(sockfd, heartbeat,5 , 0);
			}

			else if(success_flag >0) //1 stand we send the message is success,2 stand faild
			{
				if(success_flag == 1)
				{
					send(sockfd,success_send,5,0);
					success_flag = 0;
					send(sockfd, heartbeat,5 , 0);
				}
				else if(success_flag == 2)
				{
					send(sockfd,faild_send,5,0);
					success_flag = 0;
					send(sockfd, heartbeat,5 , 0);
				}
			}

			else
			{
				len = send(sockfd, heartbeat,5 /*strlen(heartbeat)*/, 0);
				if (len <= 0)
				{
					printf("failed to send heartbeat !\n");//in this case, we also add keepflag by 1,to switch the state to START
					state =START;//it is recommended to set START state
					break;
				}
				else			
				{
					printf("keepflag = %d\n",keepflag);
					printf("heartbeat send!\n");
					keepflag += 1;
					if(keepflag > 20)
					{
						state = START;
					    if(keepflag >50)
					    {
                        //	watchdog = 1;
					    	g_heartbeat_reset=HEARTBEAT_NEED_RESET;
                        }    	
                        printf("we switch to START state because we sent 10 times heartbeat that but the server not sent us\n");
                        restart_flag = 1;
						break;
					}
				}
			}
			state = PROCESSING;
			state_processing_c =0;
			break;
					
		default:
			state = START;
			break;
		}//switch state machine
	}//while(1)
		
	exit(0);  
} 


//推视频流
void *operation(void *arg);
int start_stream(int fd)
{
   	pthread_t tid;
		int err;
		err = pthread_create(&tid,NULL,operation,(void *)fd);//at 10.12
    if(err != 0)
       printf("pthread creat error");	
}

void * operation(void * arg)
{
	int sockfd = (int)arg; //at 10.12
	char command[] = {0xF2,0x08,0x05,0xF1,0xF0};//faild
	char command1[] = {0xF2,0x08,0x05,0x01,0x00};//success
	char gst_start_command[]={114,0x0a,0x0d};//114:'r', stands for "run"
	stop_stream_flag = 1; //when we start the stream .we turn the flag to 1
	int status,ret;
	int val;
		
		printf("*****************sockfd:%d\n",sockfd);
		system(" /etc/init.d/rsyslog stop ");

		ret = write(g_pipefd[1],gst_start_command,3); //the parents' process through the g_pipifd[1] to write the command to the chiald's process

		if(ret>=0)
		{
			printf("start the stream successful\n");
			send(sockfd,command1,5,0);
//			startstreamgpio();
			ctrl_stream_gpio(turn_on);
		}
		else
		{
			printf("start the stream faild\n");
			send(sockfd,command,5,0);
		}

		pthread_exit((void *)1);
}

//停止推视频流
int stop_stream(int fd)
{
	int sockfd = fd;
	char command[] = {0xF2,0x09,0x05,0xF1,0xF1};//faild
	char command1[] = {0xF2,0x09,0x05,0x01,0x01};//success
	char gst_stop_command[]={115,0x0a,0x0d};//115:'s', stands for "stopping"
	int err;
	int ret;

	ret = write(g_pipefd[1],gst_stop_command,3);

	if(ret>=0)
	{
		printf("stop the stream successful\n");
		stop_stream_flag = 0; //at 1106,when we stop the stream , we make the flag to 0
		send(sockfd,command1,5/*sizeof(command1)*/,0);
	//	stopstreamgpio();
		ctrl_stream_gpio(turn_off);
	}
	else
	{
		printf("stop the stream is faild\n");
		send(sockfd,command,5,0);
	}
	return 0;
}


//check the keepalive
int keepalive()
{
	time_t timep;
    time (&timep);
    printf("%s", ctime(&timep));

	printf("HA HA HA HA the server is alive\n\n");
	heartbeatgpio();
}


//OMAP登录

//retuns:
//1: success
//-1 :failure
int login_c(int fd)
{
	int sockfd = fd;
	char command[40];

	char checksum = 0x00;
	int i; 

	sprintf(command, "%c%c%c%-16s%-16s",0xF2,0x06,0x24,g_user,g_passwd);

	for( i=0;i<35;i++ )
		checksum += command[i];
	command[35] = checksum;
//	command[35] = checksum + 0x01;
	command[36] = '\0';

	send(sockfd, command, strlen(command), 0);

	recv(sockfd, command, strlen(command), 0);
	if( command[3] == 0x01 )
		return 1;
	else if( command[3] == (char)0xFF )
		printf("Command ERROR!\n");
	else
		printf("The user or passwd is Wrong!\n");
	return -1;
}




/*
Name:process_command()
Description:
process various commands sent from the server
*/
int process_command(char buffer[],int sockfd)
{

int i; //show the receive command that from server
for(i=0;i<buffer[2];i++)
{
	printf("%4x",buffer[i]);
}
printf("\n");
                    printf("command is 0x0%x\n",buffer[1]); //show the command character
					if(buffer[1] == 0x01){
						switch(buffer[3]){
							case 0x01:
								up(sockfd);
								usleep(300000);
								stop(sockfd);
								break;
							case 0x02:
								down(sockfd);
								usleep(300000);
								stop(sockfd);
								break;
							case 0x03:
								left(sockfd);
								usleep(300000);
								stop(sockfd);	
								break;
							case 0x04:
								right(sockfd);
								usleep(300000);
								stop(sockfd);
								break;
						}
					}         //变倍长 变倍短
					else if(buffer[1] == 0x02){
						switch(buffer[3]){
							case 0x01:
								zoomlong(sockfd);//变倍长
								usleep(300000);
								stop(sockfd);
								break;
							case 0x02:
								zoomshort(sockfd);//变倍短
								usleep(300000);
								stop(sockfd);
								break;
						}
					}//处理聚焦近，远
					else if(buffer[1] == 0x03){
						switch(buffer[3]){
							case 0x01:
								focusclose(sockfd);
								usleep(300000);
								stop(sockfd);
								break;
							case 0x02:
								focusfar(sockfd);
								usleep(300000);
								stop(sockfd);
								break;
						}	
					}//stop the camera's up down left right and so on
					else if(buffer[1] == 0x05){
						stop(sockfd);
					}//add the keepalive
					else if(buffer[1] == 0x07){
						keepalive();//just print a message
					}
					else if (buffer[1] == 0x08){
						if(stop_stream_flag == 0)//have stopped
						{
							start_stream(sockfd);
						}
						else
						{
							printf("!!!the stream is pulling!!!\n");
						}
					}
					else if (buffer[1] == 0x09){
						stop_stream(sockfd);
					}
					else if(buffer[1] == 0x10){ //the command is to check the flow,then the globle is change,control
						check_flow = 1;
					}
                    else if(buffer[1] == 0x11){
                        remote(sockfd);
                    }
					else if(buffer[1] == 0x12){
						closeremote(sockfd);
					}	
					else if(buffer[1] == 0x15){
						return_flag = 1;
						memcpy(return_command,&buffer[3],11);
						memcpy(return_detail,&buffer[14],buffer[2]-15);
						commanlen = buffer[2]-15;

					}
					else if(buffer[1] == 0x16){
						printf("server receive the message that client send it!!\n");
					}
					else if(buffer[1] == 0x18){ //check the lost flow,app
						check_flow = 2;
					}
					return 0;
}

/*
   we can receive the message from the 10010
   when we receive the command ,we begin to check the flow
   when we couldn't receive and receive the command to check the flow,then we check the flow by ourselves
*/
//send the message
void *send_gsm(void *arg);
int atcommand_process(void)
{

        pthread_t tid1;
        int err;
        err = pthread_create(&tid1,NULL,send_gsm,(void *)NULL);//at 10.12
	if(err != 0)
		printf("error creat"); 
}

void * send_gsm(void * arg)
{

	enum{
		CONFIRM=0,
	    STYLE=1,
		SENDNUM=2,
		CHECK=3,
		PROCESS=4,
		DELETE=5,
		OPEN=6,
		WAIT=7,
		GETNUM=8
	};
		int stat;
    //    int sockfd = (int)arg;//at 10.12

        int portfd;
        int nwrite, nread;
        char buff[BUFFER_SIZE];
        char buffer1[20] = {'A','T','\r'};  //confirm the command
        char buffer2[20] = {'A','T','+','C','M','G','F','=','1','\r'}; //send the message's style,1->text,0->PDU
        char buffer3[20] = {'A','T','+','C','M','G','S','=','"','1','0','0','1','0','"','\r'};//send the message
        char buffer4[20] = {'C','X','L','L','\r',0x1A,'\r'};//send the message that check the stream
        char buffer5[20] = {'A','T','+','C','M','G','L','=','"','A','L','L','"','\r'}; //read the whole message
        char buffer6[20] = {'A','T','+','C','M','G','D','=','1',',','4','\r'}; //delete all the message
		char buffer7[20] = {'A','T','E','0','\r'};
		char buffer10[20] = {'A','T','+','C','N','U','M','\r'};

		int confirm_count = 0;
		portfd = 0;
		stat = OPEN;
	while(1)
	{
		switch(stat){
			case OPEN:    //open the seriport 
				if(portfd != 0)
					close(portfd);
				if((portfd = open("/dev/ttyUSB4",O_RDWR|O_NOCTTY|O_NDELAY)) < 0) //open the serial port
				{
					perror("open_port\n");
					stat = OPEN;
					break;
				}
				printf("open port ok!\n");
				write(portfd,buffer7,strlen(buffer7));
				printf("close ATE0\n");

				stat = CONFIRM;
				confirm_count++;
				break;

			case CONFIRM:  //AT confirm the seriport 
				nwrite = write(portfd,buffer1,strlen(buffer1));
					printf("write the confirm of message len is:%d\n",nwrite);
					printf("buffer1 is:%s\n",buffer1);
				sleep(2);
				nread = read(portfd, buff, BUFFER_SIZE);
					printf("receive the buff Len is:%d\n", nread);
					printf("confirm's buff is %s\n",buff);
				if(buff[2] == 'O' && buff[3] == 'K')
				{
					printf("confirm ok!!\n");
					stat = GETNUM;
					break;
				}
				else
				{
					printf("confirm is wrong!!\n");
					#if 0
					if(confirm_count > 80)
					{
						watchdog = 1;
					}
					#endif
					sleep(10);
					stat = OPEN;
					break;
				}

			case GETNUM:
				nwrite = write(portfd,buffer10,strlen(buffer10));
				sleep(2);
				char head[] = { 0xF2,0x17,0x12};
				bzero(phonenum,20);
				char strout[] = ",";
    			char strin[] = "\"";
    			char out[15];
    			int choose = 2;
				nread = read(portfd, buff, 100);
				between_character(buff,strout,strin,out,choose);
					printf("GET the number is:%s\n",out);
				memcpy(phonenum,head,3);
				memcpy(&phonenum[3],out,14);
				add_checksum(phonenum, 18);
				getnum_flag = 1;
					printf("!!!!getnum_flag is:%d\n,the phonenum is:%s\n",getnum_flag,phonenum );

				stat = STYLE;
				break;

			case STYLE:   //the message's mode 1->text,0->PDU
				confirm_count = 0;
				nwrite = write(portfd,buffer2,strlen(buffer2)); //send the mode of message
					printf("write the mode of message len is:%d\n",nwrite);
					printf("buffer2 is:%s\n",buffer2);
				sleep(2);
				nread = read(portfd, buff, BUFFER_SIZE);
					printf("receive the buff Len is:%d\n", nread);
				if(buff[2] == 'O' && buff[3] == 'K'){
					printf("mode ok!!\n");
					stat = CHECK;
					break;
				}
				else{
					printf("mode is wrong!!\n");
					stat = OPEN;
					break;
				}

			case CHECK:  //AT+CMGL="ALL",check the whether the SIM have message
				nwrite = write(portfd,buffer5,strlen(buffer5));  //check the SIM have or no message
					printf("write the index that you want look the message is:%d\n",nwrite);
					printf("buffer5 is:%s\n",buffer5);

				sleep(2);
				nread = read(portfd, buff, BUFFER_SIZE);
				if(buff[2] == 'O' && buff[3] == 'K'){   //the SIM have no message that we send 10010 to receive message
					printf("the mode is empty!!\n");
					stat = WAIT;
					break;
				}
				else{
					stat = PROCESS;
					break;
				}

			case SENDNUM:  //AT+CMGS="10010",send to 10010 and the server want to send the message to the number
				if(check_flow == 1 || check_flow == 2)
				{
					nwrite = write(portfd,buffer3,strlen(buffer3)); //send the number                                                                                                                       
						printf("!write the number len is:%d\n",nwrite);
						printf("buffer3 is:%s\n",buffer3);
					sleep(2);
					nwrite = write(portfd,buffer4,strlen(buffer4)); //send the command that check the stream
						printf("!write the number len is:%d\n",nwrite);
						printf("buffer4 is:%s\n",buffer4);

					sleep(60);
					stat = CHECK;
					break;
				}
				else if(return_flag == 1)
				{
					return_flag = 0;
					int num, content;
					char cmgs[] = {'A','T','+','C','M','G','S','=','"'};
					char buffer8[50];
					char buffer9[100];
					bzero(buffer8,50); //init the buffer8 to 0
					bzero(buffer9,100); //init the buffer9 to 0
					memcpy(buffer8,cmgs,9);
//					memcpy(&buffer8[9],return_command,11);
					memcpy(&buffer8[9],return_command,strlen(return_command));
//					buffer8[20] = '\"';
					buffer8[9+strlen(return_command)] = '\"';
//					buffer8[21] = '\r';
					buffer8[10+strlen(return_command)] = '\r';

		//			memcpy(buffer9,return_detail,strlen(return_detail));
		printf("the return_detail is:%s\n",return_detail);
		printf("the return_detail's len is:%d %d\n",strlen(return_detail),sizeof(return_detail));
		printf("return_detail[1] is:%c\n",return_detail[1]);
		printf("return_detail[3] is:%c\n",return_detail[3]);
		printf("return_detail[5] is:%c\n",return_detail[5]);
		int a,b=0;
		for(a=0;a<commanlen;a++)
		{
			if(a%2 == 0)
			{
				buffer9[b] = return_detail[a+1];
				b++;
			}
		}
		buffer9[commanlen/2] = '\r';
		buffer9[commanlen/2+1] = 0x1A;
		buffer9[commanlen/2+2] = '\r';
		#if 0
		buffer9[0] = return_detail[1];
		buffer9[1] = return_detail[3];
		buffer9[2] = return_detail[5];
		buffer9[3] = '\r';
		buffer9[4] = 0x1A;
		buffer9[5] = '\r';
		#endif
				//	buffer9[strlen(return_detail)] = '\r';
				//	buffer9[strlen(return_detail)+1] = 0x1A;
				//	buffer9[strlen(return_detail)+2] = '\r';
					num = write(portfd,buffer8,strlen(buffer8));
		printf("&&&&&&&num is:%d\n",num);
		printf("!&buffer8 is:%s\n",buffer8);
					sleep(2);
					content = write(portfd,buffer9,strlen(buffer9));
		printf("&&&&&&&&content is:%d\n",content);
		printf("buffer9 is:%s\n",buffer9);

					if(num > 0 && content >0)
					{
						success_flag = 1;
					}
					else
					{
						success_flag = 2;
					}
					sleep(60);
					stat = CHECK;
					break;
				}

					stat = CHECK;
					break;
	
			case PROCESS: //we process the string
				printf("receive the buff Len is:%d\n", nread);
				printf("buff is:%s\n",buff); //printf the message that receive GSM

			//this is we receive the commant to check flow
			//first when we receive the command to check flow the check_flow return 1
			if(check_flow == 1 || check_flow == 2)  //we only send the lost flow
			{
			//	check_flow = 0;
				char *string = NULL;
				char buffer[10];
				char comm[20];
				char checksum = 0x00;
				char *ptr;
				char *strbegin = "52694F596D4191CF0020",*strend = "004D0042000D000A0020672C6570";
				int beginindex,endindex,beginstrlength = strlen(strbegin);

				ptr = strstr(buff, strbegin);
					beginindex = ptr-buff-1;
					ptr = strstr(buff, strend);
					endindex = ptr-buff;
				int n = endindex - beginindex - beginstrlength;
				if(n>0)
				{
					string=(char*)malloc((n)*sizeof(char));
					strncpy(string, buff+beginindex+beginstrlength+1, n-1); 
					string[n-1]='\0';
					printf("string:%s\n", string); //打印剩余流量的字符串

                		int len = strlen(string); ///// 
					printf("%d\n",len); //打印剩余流量字符串长度
                		int i, j=0, k=0;
                		for(i=0;i<len;i++)
                		{ //处理剩余流量字符串string,string是收到的剩余流量的字符串,buffer是处理后的字符串
						if((i+1)%4 == 0)
						{ 
							if(string[i] != 'E' )
							{
								buffer[j]=string[i];
								j++;
							}   
							else
							{
								buffer[j] = '.';
								j++;
							}  
					   		buffer[len / 4] = '\0';	
						}   
					}   
          			printf("buffer is:%s\n",buffer);//打印处理后的字符串，显示剩余流量
          			free(string);
          			if(check_flow == 1)
          			{
          				check_flow = 0;
						sprintf(comm,"%c%c%c%-12s",0xF2,0x10,0x0F,buffer);//we make this command's length is 15,comm[14] is checksum
					}
					else if(check_flow == 2)
					{
						check_flow = 0;
						sprintf(comm,"%c%c%c%-12s",0xF2,0x18,0x0F,buffer);//we make this command's length is 15,comm[14] is checksum,app
					}

					add_checksum(comm,15);

					signal(SIGPIPE, SIG_IGN);  //ignore the sigpipe,because the sigpipe will kill the whole process by default
					if(strlen(comm) != 0)
					{
						memcpy(atcommand_to_server,comm,strlen(comm));
						atcommand_to_server_pending=1;
						sleep(3);					
						stat = DELETE;
						break;
					}
					else
					{
						stat = CHECK;
						break;
					}
				}//n >0
				else
				{
				//	char nomessage[] = {0xF2,0x10,0x05,0xF1,0xF8}; //stand we couldn't match the lost flow,control
				//	char nomessage1[] = {0xF2,0x18,0x05,0xF1,0x00};//faild mathch message app
					if(check_flow == 1)
					{
						check_flow = 0;
						char nomessage[] = {0xF2,0x10,0x05,0xF1,0xF8}; //stand we couldn't match the lost flow,control
						memcpy(no_message,nomessage,5);
						no_message_flag = 1;
					}
					else if(check_flow == 2)
					{
						check_flow = 0;
						char nomessage[] = {0xF2,0x18,0x05,0xF1,0x00};//faild mathch message app
						memcpy(no_message,nomessage,5);
						no_message_flag = 1;
					}
					sleep(3);
					stat = DELETE;
					break;
				}//n<=0
		    }//if(check_flow == 1)

		    else  //this is we receive the message that send by another number //if(check_flow != 1),we send the whole message
		    {
		    //	char out[50] ; //the function return the character(stand the number)
    		//	bzero(out,50);
		    	char out[11];
		    	memset(out,'0',11);

    			char thestring[80]; //we get the forward 100 character that from the GSM module 
		    	bzero(thestring,80);
		    	memcpy(thestring,buff,80);
				char strout[] = ",";
    			char strin[] = "\"";
    			int choose = 3;
    			between_character(thestring, strout, strin , out,choose);
    			printf("THE number is %s\n",out);  //printf the number
    			printf("the nread is:%d\n",nread);

    			/**[0] = F2
    			*  [1] = 0x16
    			*  [2] = len
    			*  [3] = the NO. of the message 
    			*  [4]-[14] the number that we receive the message
				**/
    			if(nread <= 239) //241 stand:1 B stand 8 b = 255,the head's length is 3,the number's length is 11;255-3-11=241
    			{
					char out1[255];
		    		char whole_mesg[] = {0xF2,0x16};
		    		bzero(atcommand_to_servermesg,BUFFER_SIZE);
		    		memcpy(atcommand_to_servermesg,whole_mesg,2);
		    	//	atcommand_to_servermesg[2] = nread+16;	//the command's length
		    		atcommand_to_servermesg[3] = 1;
		    		memcpy(&atcommand_to_servermesg[4],out ,11);//the number that send the client's message
			
		    		char strout[] = "\r";
    				char strin[] = "\n";
    				int choose = 2;
    				between_character(buff,strout,strin,out1,choose);
    				atcommand_to_servermesg[2] = strlen(out1)+16;
					strcat(atcommand_to_servermesg,/*buff*/out1);

					atcommand_to_server_allmessage = 1;

					add_checksum(atcommand_to_servermesg, strlen(out1)+16);
				}
				else
				{
					stat = DELETE;
					break;
				}

				sleep(3);
				stat = DELETE;
				break;
		    }
		    	
		    stat = CHECK;
		    break;



		case DELETE: //AT+CMGD=1,4 ,delete all message
			nwrite = write(portfd,buffer6,strlen(buffer6));  //delete all the message
				printf("write the delete all the message's length is:%d\n",nwrite);
				printf("buffer6 is:%s\n",buffer6);
			int k = 0;
            sleep(2);
            nread = read(portfd, buff, BUFFER_SIZE);
				printf("receive the buff Len is:%d\n", nread);
            if(buff[2] == 'O' && buff[3] == 'K')
             	{
                printf("delete ALL the message is ok!!\n");
                stat = CHECK;
                break;
			}	
            else
            {
                printf("delete message is wrong!!\n");
                stat = DELETE;
				break;
            }

		case WAIT: //we check the message mode ,if the mode is empty ,jump it
				if(check_flow > 0 || return_flag == 1)
				{  
				//	check_flow = 0;
					stat = SENDNUM;
					break;
                }
                else
                {
                    sleep(30);
                 	stat = CHECK;
		    		sleep(2);
                  	break;
                }

		default:
			stat = OPEN;
			break;
		}//switch the machine
	}//while(1)
	exit(0);
}

		
int get_emptylen(struct circular_buffer* cbuffer)
{
	return cbuffer->empty_len;
}

int writecommand(char buffer[],struct circular_buffer *cbuffer,int len)
{
	int i;
	if(len>cbuffer->empty_len){
		printf("not enough space for writing command\n");
		return -1;
	}
	
	for(i=0;i<len;i++){
		cbuffer->buf[cbuffer->write_pos]=buffer[i];
		cbuffer->write_pos++;
		if(cbuffer->write_pos>=2*MAXBUF)
			cbuffer->write_pos=0;
	}
	cbuffer->empty_len-=len;
	return 0;
}

int getcommand( char buffer[], struct circular_buffer *cbuffer, int len)
{
	int read_pos_backup = 0;
	
	int i;
	int command_len;
	char ch;
	int cbuflen = 2*MAXBUF - cbuffer->empty_len;//the effective character on the circular buffer
	if(cbuflen <= 0)				//没有有效数据；
	return -1;

	//searching the sync word
	for(i=0;i<cbuflen;i++) {
		ch = cbuffer->buf[cbuffer->read_pos];
		if(ch == 0xF1)	
			break;
		else {
			printf("discard this ch:%02x\n",ch);
			cbuffer->read_pos++;
			if(cbuffer->read_pos >= 2*MAXBUF)
				cbuffer->read_pos = 0;
			cbuffer->empty_len++;   
			if(cbuffer->empty_len == 2*MAXBUF)
				return -1;
        	}
	}
    
	cbuflen = 2*MAXBUF - cbuffer->empty_len;//the effective character on the circular buffer
	command_len = cbuffer->buf[(cbuffer->read_pos+2)%(MAXBUF*2)];//the length of this command
	if(cbuflen < MIN_COMMAND_LEN)				//uncomplete command remains on the circular buffer
		return -1;

	if(command_len > MAX_COMMAND_LEN || command_len < MIN_COMMAND_LEN) {
		printf("the command_len is invailed!\n");
		printf("discard this char(command_len):%02x\n",cbuffer->buf[cbuffer->read_pos]);
		cbuffer->read_pos++;
		if(cbuffer->read_pos >= 2*MAXBUF)
			cbuffer->read_pos = 0;
		
		cbuffer->empty_len++;
		return 1;
	}
	
	if(command_len > cbuflen) {
		printf("uncomplete command remains on the circular buffer\n");
		return -1;
    }
    else {
		read_pos_backup = cbuffer->read_pos + 1;				//备份当前读指针+1；
		if(read_pos_backup >= 2*MAXBUF)
			read_pos_backup = 0;
		
		for(i=0;i<command_len;i++){				//copy the command;
			buffer[i] = cbuffer->buf[cbuffer->read_pos];
//	printf("%3x",buffer[i]);
			
			cbuffer->read_pos++;
			if(cbuffer->read_pos >= 2*MAXBUF)
				cbuffer->read_pos = 0;
		}
	printf("\n");
		
		if(checksum_check(buffer, command_len) == 0) {			//校验和正确
			cbuffer->empty_len += command_len;

			return command_len;
		}
		else {													//校验和错误，恢复读指针；
			cbuffer->read_pos = read_pos_backup;
			cbuffer->empty_len++;
			return 1;
		}
	}
}


//the watchdog function
void *watchdog_function(void *arg);

//void *gst_monitor_function(void *arg);//0107
watchdog_process(void)
{
	  	pthread_t tid3 /*,tid4*/;
        int err;
        err = pthread_create(&tid3,NULL,watchdog_function,(void*)NULL);//at 11.16
		if(err != 0)
			printf("error creat"); 
#if 0 
		err = pthread_create(&tid4,NULL,gst_monitor_function,(void*)NULL); //0107
		if(err !=0)
			printf("error create thread of gst_monitor_function\n");
#endif
}
#if 0
//the thread:gst_monitor_function is used to receive the report/cmd from gstreamer
void *gst_monitor_function(void *arg)
{
	struct sockaddr_in servaddr; 
	char buffer[20];
  int addr_len=sizeof(struct sockaddr_in);
  int len;
  int SO_REUSEADDR_on = 1;
  int wd_sock;
  
  /* always init first */
  wd_sock = socket(AF_INET,SOCK_DGRAM,0);
  
  g_gst_reset =GST_NEED_NONE;
  
  setsockopt(wd_sock, SOL_SOCKET, SO_REUSEADDR, &SO_REUSEADDR_on, sizeof(SO_REUSEADDR_on));

  bzero(&servaddr,sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(WD_SERVER_PORT);
  if(bind(wd_sock,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
    printf("bind udp port:%d error\n",WD_SERVER_PORT);
    return NULL;
  }

  while(1){
  	bzero(buffer,sizeof(buffer));
  	len = recvfrom(wd_sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&servaddr ,&addr_len);
  	if(strncmp(buffer,"reset",5)==0){
  		//gstream requires to reboot the system
  		g_gst_reset=GST_NEED_RESET;
  	}
	}
  
}
#endif

void *watchdog_function(void *arg)
{
	enum
	{
		CIRCLE=0,
		SENDGPIO=1
	};
	openwatchdog(); //make the watchdog's EN work
	int stat;
     stat = CIRCLE;
     while(1)
     {
     	switch(stat)
     	{
     		case CIRCLE:
     		//	if(watchdog == 0)
     			if(g_heartbeat_reset == HEARTBEAT_NEED_NONE && g_gst_reset==GST_NEED_NONE)
     			{
   					watchdoggpio();						
    			}//if(watchdog == 0)
    			else
    			{
    				stat = SENDGPIO;
    				break;
    			}
    			stat = CIRCLE;
    			break ;
    		case SENDGPIO:
    			sleep(20); //we couldn't send the signal to the gpio,then the watchadog is resert
    			stat = CIRCLE;
    			break;
     	}//switch(stat)
     }//while(1)
}//watchdog_function()
