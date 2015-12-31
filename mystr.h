extern char between_character(char buffer[],char strout[], char strin[],char out[], int choose); //get the string between strbegin and strend character
extern void add_checksum(char buffer[] , int len); //the string's length is len,the last character is checksum 
extern int checksum_check(char buffer[], int len); //check the checksum 

extern int up(int fd); 			//make the camera up
extern int down(int fd);		//make the camera down
extern int left(int fd);		//make the camera left
extern int right(int fd);		//make the camera right
extern int zoomshort(int fd);	//make the camera zoomshort
extern int zoomlong(int fd);	//make the camera zoomlong
extern int focusclose(int fd);	//make the camera focusclose
extern int focusfar(int fd);	//make the camera focusfar
extern int stop(int fd);		//make the camera adjust stop

extern int remote(int fd);		//open the remote
extern int closeremote(int fd);	//close the remote

extern int heartbeatgpio(void); 	//the heart beat gpio
extern int ctrl_stream_gpio(int value); //start and stop stream gpio through the value
extern int openwatchdog(void); //opent the gpio129,the watchdog is work(EN)
extern int watchdoggpio(void); //feed the watchdog gpio128(WDI)