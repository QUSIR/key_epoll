#ifndef HOTKEYSERVICE_H
#define HOTKEYSERVICE_H


#include <stdio.h>      /*��׼�����������*/
#include <stdlib.h>     /*��׼�����ⶨ��*/
#include <unistd.h>     /*Unix ��׼��������*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>      /*�ļ����ƶ���*/
#include <termios.h>    /*POSIX �ն˿��ƶ���*/
#include <errno.h>      /*����Ŷ���*/
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
