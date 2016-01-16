#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
int main()
{	
	int fd_watchdog;
	fd_watchdog = open("/dev/watchdog", O_WRONLY);
	if(fd_watchdog == -1) {
		int err = errno;
		printf("\n!!! FAILED to open /dev/watchdog, errno: %d, %d\n", err, strerror(err));
	syslog(LOG_WARNING, "FAILED to open /dev/watchdog, errno: %d, %d", err, strerror(err));
	}

//feed the watchdog
//	if(fd_watchdog >= 0) {
	while(fd_watchdog){
		sleep(30);
		static unsigned char food = 0;
		ssize_t eaten = write(fd_watchdog, &food, 1);
		if(eaten != 1) 
		{
	    		printf("\n!!! FAILED feeding watchdog");
		}
    	}
//	close(fd_watchdog);
}
