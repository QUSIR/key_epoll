#include "HotKeyService.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <stddef.h>
#include <linux/input.h>
#include <list>
#include <signal.h>
#include <string.h>

using std::list;


#define DEV_PATH1 "/dev/input/event4"
#define DEV_PATH2 "/dev/input/event5"

#define UART_DEVICE_NAME "/dev/ttyUSB1"




static THREAD_TYPE staticThreadOfEpollGuard(void* arg) {
	return ((HotKeyService*)arg)->ThreadOfEpollGuard();
}


HotKeyService::HotKeyService(){
    for(int i=0;i<DEVICE_LEN;i++){
        this->driver_fd[i]=-1;
    }

    this->epoller=new Epoller(1000,10);
}
void HotKeyService::set_speed(int fd, int baud){
	unsigned char i;
	int  status;
	struct termios Opt;

	for(i=0; i<sizeof(speed_arr)/sizeof(int); i++)
	{
		if(speed_name[i] == baud)
		{
			tcflush(fd, TCIOFLUSH);     //刷新输入输出数据，但是不读也不传送
			cfsetispeed(&Opt, speed_arr[i]);  	//设置输入波特率
			cfsetospeed(&Opt, speed_arr[i]); 	//设置输出波特率
			//用于设置终端的相关参数
			status = tcsetattr(fd, TCSANOW, &Opt);  //TCSANOW：不等数据传输完毕就立即改变属性
			if  (status != 0)
			{
				perror("tcsetattr fd");
				return;
			}
			tcflush(fd,TCIOFLUSH);
		}
	}

}
int HotKeyService::set_Parity(int fd, int databits, int stopbits, int parity){
        struct termios options;
        long vdisable;

        if(tcgetattr(fd,&options)!=0)
        {
            perror("SetupSerial 1");
            return(FALSE);
        }

        //设置数据位
        options.c_cflag &= ~CSIZE;
        switch(databits)
        {
            case 5:
                options.c_cflag |= CS5;
                break;
            case 6:
                options.c_cflag |= CS6;
                break;
            case 7:
                options.c_cflag |= CS7;
                break;
            case 8:
                options.c_cflag |= CS8;
                break;
            default:
                printf("Unsupported data size\n");
                return (FALSE);
        }

        //设置检验位
        switch (parity)
        {
            case 'n':
            case 'N':
            options.c_cflag &= ~PARENB;   /* Clear parity enable */
            options.c_iflag &= ~INPCK;     /* Disable parity checking */
            break;

            case 'o':
            case 'O':
            options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
            options.c_iflag |= INPCK;             /* Enable parity checking */
            break;

            case 'e':
            case 'E':
            options.c_cflag |= PARENB;     /* Enable parity */
            options.c_cflag &= ~PARODD;   /* 转换为偶效验*/
            options.c_iflag |= INPCK;       /* Enable parity checking */
            break;

            default:
            printf("Unsupported parity\n");
            return (FALSE);
        }

        //设置停止位
        switch (stopbits)
        {
            case 1:
            options.c_cflag &= ~CSTOPB;
            break;

            case 2:
            options.c_cflag |= CSTOPB;
            break;

            default:
            printf("Unsupported stop bits\n");
            return (FALSE);
        }
        tcflush(fd,TCIOFLUSH);

        //控制字符设置
        options.c_cc[VTIME]    = 50;    /* 设置超时5 seconds*/
        options.c_cc[VMIN]     = 0; /* 在计时器超时前，read返回[1,nbytes];若计时器超时，read返回0 */

        options.c_cc[VINTR]    = vdisable;   /**//* Ctrl-c */
        options.c_cc[VQUIT]    = vdisable;   /**//* Ctrl- */
        options.c_cc[VERASE]   = vdisable;   /**//* del */
        options.c_cc[VKILL]    = vdisable;   /**//* @ */
        options.c_cc[VEOF]     = vdisable;   /**//* Ctrl-d */
        options.c_cc[VSWTC]    = vdisable;   /**//* '' */
        options.c_cc[VSTART]   = vdisable;   /**//* Ctrl-q */
        options.c_cc[VSTOP]    = vdisable;   /**//* Ctrl-s */
        options.c_cc[VSUSP]    = vdisable;   /**//* Ctrl-z */
        options.c_cc[VEOL]     = vdisable;   /**//* '' */
        options.c_cc[VREPRINT] = vdisable;   /**//* Ctrl-r */
        options.c_cc[VDISCARD] = vdisable;   /**//* Ctrl-u */
        options.c_cc[VWERASE]  = vdisable;   /**//* Ctrl-w */
        options.c_cc[VLNEXT]   = vdisable;   /**//* Ctrl-v */
        options.c_cc[VEOL2]    = vdisable;   /**//* '' */

        /* 设置其他终端属性 */
#if 0
        options.c_cflag |= ( CLOCAL | CREAD);  //忽略DCD信号
        options.c_cflag &= ~CRTSCTS;          //关硬件流控制

        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //关回显,设置为noncanonical mode

        options.c_oflag &= ~OPOST;                         //行式输出

        options.c_iflag &= ~(IXON | IXOFF | IXANY);         //关软件流控制
        options.c_iflag |= IGNBRK;
#endif
        /*
         * Echo off, canonical mode off, extended input
         * processing off, signal chars off.
         */
        options.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

        /*
         * No SIGINT on BREAK, CR-to-NL off, input parity
         * check off, don't strip 8th bit on input, output
         * flow control off.
         */
        options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

        /*
         * Output processing off.
         */
        options.c_oflag &= ~(OPOST);


        if (tcsetattr(fd,TCSANOW,&options) != 0)
        {
            perror("SetupSerial 3");
            return (FALSE);
        }

        return (TRUE);

}
int HotKeyService::start(){

	if(!this->openDevice(DEV_PATH1,&this->driver_fd[0])){
		printf("HotKeyService::start - fail to open device\n");
		return -1;
	}
	this->epoller->add(this->driver_fd[0]);
    printf("DEV_PATH1 fd is %d\n",this->driver_fd[0]);
    /*
    if(!this->openDevice(DEV_PATH2,&this->driver_fd[1])){
		printf("HotKeyService::start - fail to open device\n");
		return -1;
	}
	this->epoller->add(this->driver_fd[1]);
    printf("DEV_PATH2 fd is %d\n",this->driver_fd[1]);
*/


    if(!this->openDevice(UART_DEVICE_NAME,&this->driver_fd[1])){
		printf("HotKeyService::start - fail to open device\n");
		return -1;
	}
	this->set_speed(this->driver_fd[1],9600); //设置串口波特率

	if (this->set_Parity(this->driver_fd[1],8,1,'n') == FALSE)  //设置8位数据位，1位停止位，无校验等其他设置。
	{
		printf("Set Parity Error\n");
	}

	this->epoller->add(this->driver_fd[1]);
    printf("DEV_PATH2 fd is %d\n",this->driver_fd[1]);



	if(!this->createThread()){
		printf("HotKeyService::start - fail to create thread\n");
		this->stop();
		return -3;
	}
	printf("HotKeyService::start - done\n");
	return 0;
}

void HotKeyService::stop(){
	this->destroyThread();
    for(int i=0;i<DEVICE_LEN;i++){
     	this->epoller->remove(this->driver_fd[i]);
	    this->closeDevice(&this->driver_fd[i]);
    }

	printf("HotKeyService::stop - done\n");
}

static bool set_non_blocking(int fd) {
	int opts;
	opts = fcntl(fd, F_GETFL);
	if (opts < 0)
		return false;
	opts = (opts | O_NONBLOCK);
	if (fcntl(fd, F_SETFL, opts) < 0)
		return false;
	return true;
}

bool HotKeyService::openDevice(char *name,int *fd){
	if(*fd>0)
		this->closeDevice(fd);
	*fd=open(name, O_RDWR);
	if(*fd<=0)
		return false;
	if(!set_non_blocking(*fd)){
		printf("HotKeyService::openDevice - fail to set non blocking mode\n");
		return false;
	}
	return true;
}

void HotKeyService::closeDevice(int *fd){
	if(*fd>0){
		close(*fd);
		*fd=-1;
	}
}

bool HotKeyService::createThread(){
	this->isThreadRunning=true;
	if(!CREATE_THREAD(&this->threadIdOfEpoll,staticThreadOfEpollGuard,this)){
		this->isThreadRunning=false;
		return false;
	}
	return true;
}

void HotKeyService::destroyThread(){
	this->isThreadRunning=false;
	pthread_join(this->threadIdOfEpoll,0);
}


void HotKeyService::destroyKeyPressThread(){
	this->is_key_press_=false;
}


THREAD_TYPE HotKeyService::ThreadOfEpollGuard(){
	struct input_event event;
	int sockfd;
    long long number=0;
    int n=0;
    list<int> sockfds;
	while(this->isThreadRunning){
    		this->epoller->getSockfds(sockfds);
            for(list<int>::iterator iter=sockfds.begin();iter!=sockfds.end();++iter){
                    if(*iter<=0) continue;
                    printf("read data fd is %d\n",*iter);
                    if(*iter==this->driver_fd[1]){
                        int nread;
                        char buff[512] = {0};
                        if((nread = read(*iter,buff,255))>0) //接收数据
                        {
                            printf("[RECEIVE] Len is %d,content is :\n",nread);
                            buff[nread]='\0';
                            printf("%s\n",buff);
                        }

                    }
                    else{

                        memset(&event,0,sizeof(event));
                        while(read(*iter, &event, sizeof(event))!=-1){
                            if(event.code!=0){
                                 printf("event.code is %d event.type %d\n",event.code,event.type);
                                 if (event.type == EV_KEY && event.value==1){

                                    if(event.code==28){
                                        n=(int)(number&0xffffff);
                                        printf("card=%d\n",n);
                                        number=0;
                                    }else if(event.code==11){
                                        number=number*10;
                                    }else{
                                        number=number*10+event.code-1;
                                    }
                                 }
                            }
                        }

                    }
            }

	}

	pthread_exit(0);
	return NULL;
}

