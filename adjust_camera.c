#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

int up(int fd)
{
	int sockfd = fd;
	char command[] = {0xF2,0x01,0x05,0xF1,0xE9};//faild
	char command1[] = {0xF2,0x01,0x05,0x01,0xF9};//success
    int err;
    err = system(" /home/media/work/write1 up ");
    if(err == -1)
    {
		send(sockfd,command,sizeof(command),0);
        printf("system error\n");
    }
    else
    {
		send(sockfd,command1,sizeof(command1),0);
        printf("up ok\n");
    }
    return 0;
}


int down(int fd)
{
	int sockfd = fd;
	char command[] = {0xF2,0x01,0x05,0xF2,0xEA};//faild
	char command1[] = {0xF2,0x01,0x05,0x02,0xFA};//success
    int err;
    err = system(" /home/media/work/write1 down ");
    if(err == -1)
    {
		send(sockfd,command,sizeof(command),0);
        printf("system error\n");
    }
    else
    {
		send(sockfd,command1,sizeof(command1),0);
        printf("down ok\n");
    }
    return 0;
}

int left(int fd)
{
	int sockfd = fd;
	char command[] = {0xF2,0x01,0x05,0xF3,0xEB};
	char command1[] = {0xF2,0x01,0x05,0x03,0xFB};
	int err;
    err = system(" /home/media/work/write1 left ");
    if(err == -1)
    {
		send(sockfd,command,sizeof(command),0);
        printf("system error\n");
    }
    else
    {
		send(sockfd,command1,sizeof(command1),0);
        printf("left ok\n");
    }
    return 0;
}


int right(int fd)
{
	int sockfd = fd;
	char command[] = {0xF2,0x01,0x05,0xF4,0xEC};
	char command1[] = {0xF2,0x01,0x05,0x04,0xFC};
    int err;
    err = system(" /home/media/work/write1 right ");
    if(err == -1)
    {
		send(sockfd,command,sizeof(command),0);
        printf("system error\n");
    }
    else
    {
		send(sockfd,command1,sizeof(command1),0);
        printf("right ok\n");
    }
    return 0;
}


int zoomshort(int fd)
{
	int sockfd = fd;
	char command[] = {0xF2,0x02,0x05,0xF2,0xEB};//faild
	char command1[] = {0xF2,0x02,0x05,0x02,0xFB};//success
    int err;
    err = system(" /home/media/work/write1 zoomshort ");
    if(err == -1)
    {
		send(sockfd,command,sizeof(command),0);
        printf("system error\n");
    }
    else
    {
		send(sockfd,command1,sizeof(command1),0);
        printf("zoomshort ok\n");
    }
    return 0;
}


int zoomlong(int fd)
{
	int sockfd = fd;
	char command[] = {0xF2,0x02,0x05,0xF1,0xEA};//faild
	char command1[] = {0xF2,0x02,0x05,0x01,0xFA};//success
    int err;
    err = system(" /home/media/work/write1 zoomlong ");
    if(err == -1)
    {
		send(sockfd,command,sizeof(command),0);
        printf("system error\n");
    }
    else
    {
		send(sockfd,command1,sizeof(command1),0);
        printf("zoomlong ok\n");
    }
    return 0;
}


int focusclose(int fd)
{
	int sockfd = fd;
	char command[] = {0xF2,0x03,0x05,0xF1,0xEB};//faild
	char command1[] = {0xF2,0x03,0x05,0x01,0xFB};//success
    int err;
    err = system(" /home/media/work/write1 focusclose ");
    if(err == -1)
    {
		send(sockfd,command,sizeof(command),0);
        printf("system error\n");
    }
    else
    {
		send(sockfd,command1,sizeof(command1),0);
        printf("focusclose ok\n");
    }
    return 0;
}


int focusfar(int fd)
{
	int sockfd = fd;
	char command[] = {0xF2,0x03,0x05,0xF2,0xEC};//faild
	char command1[] = {0xF2,0x03,0x05,0x02,0xFC};//success	
	int err;
	err = system(" /home/media/work/write1 focusfar ");
	if(err == -1)
	{
		send(sockfd,command,sizeof(command),0);
		printf("system error\n");
	}
	else
	{
		send(sockfd,command1,sizeof(command1),0);
		printf("focusfar ok\n");
	}
	return 0;
}

int stop(int fd)
{
	int sockfd = fd;
	char command[] = {0xF2,0x05,0x05,0xF1,0xED};//faild
	char command1[] = {0xF2,0x05,0x05,0x01,0xFD};//success
	int err;
	err = system(" /home/media/work/write1 stop ");
	if(err == -1)
	{
		send(sockfd,command,sizeof(command),0);
		printf("system error\n");
	}
	else
	{
		send(sockfd,command1,sizeof(command1),0);
		printf("stop ok\n");
	}
	return 0;
}