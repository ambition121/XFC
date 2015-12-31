all:
	gcc -g -o watchdogpipe3 watchdogpipe3.c mystr.c adjust_camera.c remote.c ctrl_gpio.c -lpthread
