#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

//Can be remotely updated
int remote(int fd)
{
        int sockfd = fd;
        char command[] = {0xF2,0x11,0x05,0xF1,0xF9};//faild
        char command1[] = {0xF2,0x11,0x05,0x01,0x09};//success
        int err;
        err = system("./remote.sh ");
        if((err == -1)){
            send(sockfd,command,sizeof(command),0);
            printf("system remote error\n");
        }
        else{
            if(WIFEXITED(err)){
                if(0 == WEXITSTATUS(err)){
						printf("RUN the remote successful!\n");
						send(sockfd,command1,sizeof(command1),0);
                }
                else{
						printf("RUN the remote faild! code is:\n",WEXITSTATUS(err));
						send(sockfd,command,sizeof(command),0);
                }
            }
            else{
                    printf("exit status = [%d]\n", WEXITSTATUS(err));
                    send(sockfd,command,sizeof(command),0);
                }
        }
        return 0;
}
//close the remote
int closeremote(int fd)
{
        int sockfd = fd;
        char command[] = {0xF2,0x12,0x05,0xF1,0xFA};//faild
        char command1[] = {0xF2,0x12,0x05,0x01,0x0A};//success
        int err;
        err = system(" /home/media/work/closeremote.sh ");
        if((err == -1)){
                send(sockfd,command,sizeof(command),0);
                printf("system closeremote error\n");
        }
        else{
                if(WIFEXITED(err)){
                    if(0 == WEXITSTATUS(err)){
                            printf("RUN the closeremote successful!\n");
                            send(sockfd,command1,sizeof(command1),0);
                    }
                    else{
                            printf("RUN the closeremote faild! code is:\n",WEXITSTATUS(err));
                            send(sockfd,command,sizeof(command),0);
                    }
                }
                else{
                        printf("exit status = [%d]\n", WEXITSTATUS(err));
                        send(sockfd,command,sizeof(command),0);
                }
        }
        return 0;
}