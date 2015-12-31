#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fcntl.h>  //add the gpio,1026
#include <malloc.h>

#define DATA_LEN 2
#define GPIO_NUM_LEN 4
#define file_len 126
#define GPIO_CLASS_NAME "/sys/class/gpio/export" //1026

/*this is use to add the gpio*/
char  bu[DATA_LEN];
int fd_class,fd_direction,fd_value,fd_num;
char *gpio_dir[2]={"out","in"};
int gpio_num1 = 34; 
int gpio_num2 = 37;
int gpio_num3 = 128;
int gpio_num4 = 129;
char gpio_direction_name1[file_len];
char gpio_direction_name2[file_len];
char gpio_value_name1[file_len];
char gpio_value_name2[file_len];
char *gpio_name1 = "34"; //heartbeat
char *gpio_name2 = "37";//start stop stream
char *gpio_name3 = "128";//we use this gpio to the watchdog
char *gpio_name4 = "129"; //open the watchdog 

//when we reach a heartbeat,the gpio is light 1 times
int heartbeatgpio()
{
	sprintf(gpio_direction_name1,"/sys/class/gpio/gpio%s/direction",gpio_name1); //heartbeat
	sprintf(gpio_value_name1,"/sys/class/gpio/gpio%s/value",gpio_name1);         //heartbeat

    int a=0,b=1;
    if((access(gpio_direction_name1,F_OK)==0)||(access(gpio_value_name1,F_OK)==0))
        goto run;
    if((fd_class=open(GPIO_CLASS_NAME,O_WRONLY))<0){
        perror("failed open gpio class\n");
        goto err_open_class;
    }
    if(write(fd_class,gpio_name1,strlen(gpio_name1))<0){
        printf("falied write gpio name (%s)\n",gpio_name1);
        goto err_write;
    }
    if((fd_direction=open(gpio_direction_name1,O_WRONLY))<0){
        perror("failed open gpio direction\n");
        goto err_open_direction;
    }   
    if(write(fd_direction,gpio_dir[0],strlen(gpio_dir[0]))<0){
        printf("falied write gpio (%s) direction \n",gpio_name1);
        goto err_write;
    }
run:
    if((fd_value=open(gpio_value_name1,O_RDWR))<0){
        perror("falied open gpio value\n");
        goto err_open_value;
    }
    sprintf(bu,"%d",a);
    write(fd_value,bu,1);
    sleep(1);
    sprintf(bu,"%d",b);
    write(fd_value,bu,1);

    err_open_class:
    err_open_direction:
    err_open_value:
    err_write:
    if(fd_class)
        close(fd_class);
    if(fd_direction)
        close(fd_direction);
    if(fd_value)
        close(fd_value);
    if(gpio_name1)
    	return 0;
}

int ctrl_stream_gpio(int value)
{
	sprintf(gpio_direction_name2,"/sys/class/gpio/gpio%s/direction",gpio_name2);  //start stop stream
	sprintf(gpio_value_name2,"/sys/class/gpio/gpio%s/value",gpio_name2);         //start stop stream

 //   int a=0;
    if((access(gpio_direction_name2,F_OK)==0)||(access(gpio_value_name2,F_OK)==0))
        goto run; 
    if((fd_class=open(GPIO_CLASS_NAME,O_WRONLY))<0){
        perror("failed open gpio class\n");
        goto err_open_class;
    }    
    if(write(fd_class,gpio_name2,strlen(gpio_name2))<0){
        printf("falied write gpio name (%s)\n",gpio_name2);
        goto err_write;
    }    
    if((fd_direction=open(gpio_direction_name2,O_WRONLY))<0){
        perror("failed open gpio direction\n");
        goto err_open_direction;
    }    
    if(write(fd_direction,gpio_dir[0],strlen(gpio_dir[0]))<0){
        printf("falied write gpio (%s) direction \n",gpio_name2);
        goto err_write;
    }    
run:
    if((fd_value=open(gpio_value_name2,O_RDWR))<0){
        perror("falied open gpio value\n");
        goto err_open_value;
    }    
    sprintf(bu,"%d",value);
    write(fd_value,bu,1);

    err_open_class:
    err_open_direction:
    err_open_value:
    err_write:
    if(fd_class)
        close(fd_class);
    if(fd_direction)
        close(fd_direction);
    if(fd_value)
        close(fd_value);
    if(gpio_name2)
    return 0;
}

int watchdoggpio()
{
	sprintf(gpio_direction_name1,"/sys/class/gpio/gpio%s/direction",gpio_name3); //heartbeat
	sprintf(gpio_value_name1,"/sys/class/gpio/gpio%s/value",gpio_name3);         //heartbeat

    int a=0,b=1;
    if((access(gpio_direction_name1,F_OK)==0)||(access(gpio_value_name1,F_OK)==0))
        goto run;
    if((fd_class=open(GPIO_CLASS_NAME,O_WRONLY))<0){
        perror("failed open gpio class\n");
        goto err_open_class;
    }
    if(write(fd_class,gpio_name3,strlen(gpio_name3))<0){
        printf("falied write gpio name (%s)\n",gpio_name3);
        goto err_write;
    }
    if((fd_direction=open(gpio_direction_name1,O_WRONLY))<0){
        perror("failed open gpio direction\n");
        goto err_open_direction;
    }   
    if(write(fd_direction,gpio_dir[0],strlen(gpio_dir[0]))<0){
        printf("falied write gpio (%s) direction \n",gpio_name3);
        goto err_write;
    }
run:
    if((fd_value=open(gpio_value_name1,O_RDWR))<0){
        perror("falied open gpio value\n");
        goto err_open_value;
    }
    sprintf(bu,"%d",a);
    write(fd_value,bu,1);
    usleep(5000);
    sprintf(bu,"%d",b);
    write(fd_value,bu,1);
    usleep(5000);
    sprintf(bu,"%d",a);
    write(fd_value,bu,1);
    usleep(5000);
    sprintf(bu,"%d",b);
    write(fd_value,bu,1);

    err_open_class:
    err_open_direction:
    err_open_value:
    err_write:
    if(fd_class)
        close(fd_class);
    if(fd_direction)
        close(fd_direction);
    if(fd_value)
        close(fd_value);
    if(gpio_name3)
    	return 0;
}

int openwatchdog()
{
	sprintf(gpio_direction_name1,"/sys/class/gpio/gpio%s/direction",gpio_name4); //heartbeat
	sprintf(gpio_value_name1,"/sys/class/gpio/gpio%s/value",gpio_name4);         //heartbeat

    int a=0,b=1;
    if((access(gpio_direction_name1,F_OK)==0)||(access(gpio_value_name1,F_OK)==0))
        goto run;
    if((fd_class=open(GPIO_CLASS_NAME,O_WRONLY))<0){
        perror("failed open gpio class\n");
        goto err_open_class;
    }
    if(write(fd_class,gpio_name4,strlen(gpio_name4))<0){
        printf("falied write gpio name (%s)\n",gpio_name4);
        goto err_write;
    }
    if((fd_direction=open(gpio_direction_name1,O_WRONLY))<0){
        perror("failed open gpio direction\n");
        goto err_open_direction;
    }   
    if(write(fd_direction,gpio_dir[0],strlen(gpio_dir[0]))<0){
        printf("falied write gpio (%s) direction \n",gpio_name4);
        goto err_write;
    }
run:
    if((fd_value=open(gpio_value_name1,O_RDWR))<0){
        perror("falied open gpio value\n");
        goto err_open_value;
    }
    sprintf(bu,"%d",a);
    write(fd_value,bu,1);

    err_open_class:
    err_open_direction:
    err_open_value:
    err_write:
    if(fd_class)
        close(fd_class);
    if(fd_direction)
        close(fd_direction);
    if(fd_value)
        close(fd_value);
    if(gpio_name4)
    	return 0;
}