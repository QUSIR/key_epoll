#ifndef KEY_H
#define KEY_H

#include <vector>

#include "Subject.h"
#include "ThreadDefine.h"
#include "Lock.h"

using std::vector;

class Key : public Subject{
public:
    Key(void);
    ~Key(void);
    bool init(char *Input_Id);
    void uninit();
    THREAD_TYPE ThreadOfReadDataProcess();
    THREAD_TYPE ThreadOfHandle();
private:
    bool createThread();
    void destroyThread();
    bool isVectorEmpty();
    void deleteVectorBegin();
    int keys_fd_;
    THREAD_ID thread_id_of_read_,thread_id_of_handle_;
    bool is_thread_running_;
    Lock lock_of_handle_,lock_of_vector_;
    vector<int> id_vector_;
};

#endif
