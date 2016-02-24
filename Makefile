all:
	gcc  watchdogpipe3.c adjust_camera.c ctrl_gpio.c mystr.c remote.c `pkg-config --cflags --libs gstreamer-0.10` -o watchdogipe3 -lpthread
	gcc  -g gst-start-stream1.c `pkg-config --cflags --libs gstreamer-0.10` -o videopush.out -lpthread
	gcc write1.c -o write1

