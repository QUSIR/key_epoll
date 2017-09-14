#include"key.h"

#include<stdio.h>
#include <unistd.h>
#include<linux/input.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <time.h>

#include "SignalType.h"
#include "Logger.h"

//#define Input_Id "/dev/input/event2"

static THREAD_TYPE KeyThreadOfReadDataMainProcess(void* arg) {
    return ((Key*)arg)->ThreadOfReadDataProcess();
}

static THREAD_TYPE staticThreadOfHandle(void* arg) {
    return ((Key*)arg)->ThreadOfHandle();
}

THREAD_TYPE Key::ThreadOfReadDataProcess() {
    struct input_event data;
    long long number=0;
    int n=0;
    while(this->is_thread_running_){
        if (read (keys_fd_, &data, sizeof (data)) == sizeof (data)){
            if (data.type == EV_KEY && data.value==1){
//                printf("Key::ThreadOfReadDataProcess - data.code:%d\n",data.code);
                if(data.code==28){
                    n=(int)(number&0xffffff);
//                    printf("card id:%d time:%d\n",n,(int)time(NULL));
                    this->lock_of_vector_.lock();
                    this->id_vector_.push_back(n);
                    this->lock_of_vector_.unlock();
                    this->lock_of_handle_.unlock();
//                    this->sendSignal(SignalType::InputCard,(void*)n);
                    number=0;
                }else if(data.code==11){
                    number=number*10;
//                    printf("number=%lld\n",number);
                }else{
                    number=number*10+data.code-1;
//                    printf("number=%lld\n",number);
                }
            }
        }

    }
    return NULL;
}

THREAD_TYPE Key::ThreadOfHandle(){
    while(this->is_thread_running_){
        if(this->isVectorEmpty()){
            this->lock_of_handle_.lock();
            continue;
        }
        this->lock_of_vector_.lock();
        vector<int>::iterator iter=this->id_vector_.begin();
        this->lock_of_vector_.unlock();
//        printf("Key::ThreadOfHandle - input card:%d time:%d\n",(*iter),(int)time(NULL));
        this->sendSignal(SignalType::InputCard,(void*)(*iter));
        this->deleteVectorBegin();
    }
    return NULL;
}

Key::Key(void){
    this->thread_id_of_read_=0;
    this->thread_id_of_handle_=0;
    this->keys_fd_=0;
    this->id_vector_.clear();
}
Key::~ Key(void){
    this->uninit();
}
bool Key::init(char *Input_Id){
    this->keys_fd_ = open (Input_Id, O_RDONLY);
    if (this->keys_fd_ < 0){
        log_error ("open %s error!\n",Input_Id);
        return false;
    }

    if(!this->createThread()){
        log_error("Key::init - fail to create thread\n");
        return false;
    }
    return true;
}

void Key::uninit(){
    this->destroyThread();
    if(this->keys_fd_!=0){
        close (this->keys_fd_);
        this->keys_fd_=0;
    }
}

bool Key::createThread(){
    this->is_thread_running_=true;
    if(!CREATE_THREAD(&this->thread_id_of_read_,KeyThreadOfReadDataMainProcess,this)){
        log_error("Key::createThread - fail to create read thread\n");
        return false;
    }
    if(!CREATE_THREAD(&this->thread_id_of_handle_,staticThreadOfHandle,this)){
        log_error("Key::createThread - fail to create handle thread\n");
        return false;
    }
    return true;
}

void Key::destroyThread(){
    this->is_thread_running_=false;
    this->lock_of_handle_.unlock();
    if(this->thread_id_of_read_!=0)
        pthread_join(this->thread_id_of_read_,0);
    this->thread_id_of_read_=0;
    if(this->thread_id_of_handle_!=0)
        pthread_join(this->thread_id_of_handle_,0);
    this->thread_id_of_handle_=0;
}

bool Key::isVectorEmpty(){
    this->lock_of_vector_.lock();
    bool is_empty=this->id_vector_.empty();
    this->lock_of_vector_.unlock();
    return is_empty;
}

void Key::deleteVectorBegin(){
    this->lock_of_vector_.lock();
    vector<int>::iterator iter=this->id_vector_.begin();
    this->id_vector_.erase(iter);
    this->lock_of_vector_.unlock();
}
