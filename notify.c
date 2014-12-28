#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <poll.h>
#include <sys/inotify.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

pthread_mutex_t device_update;
bool need_update = false;
int mouse_fd;
bool isExit;

void* Processor(void *arg){

    int poll_ret = 0;
    int read_len = 0;
    int buf[64];
    struct pollfd device_read_fd;

    mouse_fd = open("/dev/input/mouse2",O_RDONLY);
    if(mouse_fd <= 0)perror("open device fail\n");
    
    device_read_fd.fd = mouse_fd;
    device_read_fd.events = POLLIN;

    while(!isExit){
	if(need_update == true){
        pthread_mutex_lock(&device_update);
	need_update = false;
	sleep(2);
	printf("new device is update\n");
        pthread_mutex_unlock(&device_update);
	close(mouse_fd);
	mouse_fd = open("/dev/input/mouse2",O_RDONLY);
	if(mouse_fd <= 0)printf("open device fail\n");
	}
	//sleep(1);

	poll_ret = poll(&device_read_fd,1,1);
	if(poll_ret <= 0)continue;
	else if(device_read_fd.revents = POLLIN){
	    read_len = read(mouse_fd, buf, 64);
	    if(read_len > 0)printf("read_len is %d\n",read_len);
	}
    }

    return;
}

void* Listener(void *arg){
    int notify_fd = 0;
    int poll_ret;
    int read_len;
    const char* pathname = "/dev/input/";
    struct pollfd add_device_pollfd;
    char buf[32];
    notify_fd = inotify_init();

    struct inotify_event add_device_inotify_event;

    inotify_add_watch(notify_fd, pathname, IN_CREATE);

    add_device_pollfd.fd = notify_fd; 
    add_device_pollfd.events = POLLIN;
    
    while(!isExit){

	poll_ret = poll(&add_device_pollfd,1,5);

	if(poll_ret < 0){
	    usleep(1000);
	    continue;
	    }
	else if(poll_ret = 0 )continue;
	else if(add_device_pollfd.revents == POLLIN){
		
		if( 0 < read(add_device_pollfd.fd,buf,32)){

			memcpy(add_device_inotify_event.name,
			buf+sizeof(add_device_inotify_event),
			*(buf+sizeof(add_device_inotify_event)+1));

		printf("event is %s\n",add_device_inotify_event.name);
		}

		pthread_mutex_lock(&device_update);
		need_update = true;
		pthread_mutex_unlock(&device_update);
    	}
    }
    return;
 
}


void myExit(){
    printf("SIGINT is catched\n");
    isExit = true;
}

int main(int argc,char** argv){

    pthread_t pth1,pth2;
    isExit = false;
    pthread_mutex_init(&device_update,NULL);
//    pthread_cond_init(&condition,NULL);

    signal(SIGINT,myExit);

    pthread_create(&pth2,NULL,Processor,NULL);
    pthread_create(&pth1,NULL,Listener,NULL);

    pthread_join(pth1,NULL);
    pthread_join(pth2,NULL);

    exit(0);
}
