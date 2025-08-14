#ifndef __COMUNICATE_H__
#define __COMUNICATE_H__
#include <sys/man.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
    
class Reader {
    private:
        const char* name = "/my_shm";
        const int SIZE = 12;
        int shm_fd;
        void *ptr;
        int *nums
    public:
        Reader();
        void get_coor(int x, int y);
        ~Reader();
}
#endif

        
