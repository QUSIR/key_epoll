#ifndef HOTKEYSERVICE_H
#define HOTKEYSERVICE_H


#include <stdio.h>      /*标准输入输出定义*/
#include <stdlib.h>     /*标准函数库定义*/
#include <unistd.h>     /*Unix 标准函数定义*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>      /*文件控制定义*/
#include <termios.h>    /*POSIX 终端控制定义*/
#include <errno.h>      /*错误号定义*/
#include <string.h>
#include <sys/ioctl.h>

#define FALSE  -1
#define TRUE   0

static int speed_arr[] = { 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300 };
static int speed_name[] = { B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300, };


#include "ThreadDefine.h"
#include "Epoller.h"
#define DEVICE_LEN 2
class HotKeyService
{
public:
    HotKeyService();
    int start();
    void stop();
    THREAD_TYPE ThreadOfEpollGuard();


private:
    void set_speed(int fd, int baud);
    int set_Parity(int fd, int databits, int stopbits, int parity);
    int driver_fd[DEVICE_LEN];
    Epoller *epoller;
    THREAD_ID threadIdOfEpoll;
    bool is_key_press_;
    bool isThreadRunning;
    bool createThread();
    void destroyThread();

    bool openDevice(char *name,int *fd);
    void closeDevice(int *fd);
    void destroyKeyPressThread();

};

#endif // HOTKEYSERVICE_H
