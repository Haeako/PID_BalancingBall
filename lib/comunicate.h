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
        Reader()
        {
            self.shm_fd = shm_open(name, O_RONLY,0666);
            if (self.shm_fd == -1){
                std::cerr << "[E] Failed to open shared memory";
                return 1;
                }
            ptr = mmap(0, SIZE, PROT_RED, MAP_SHARED, shm_fd, 0);
            if(ptr == MAP_FAILED){
                std::cerr << "[E] Failed to map " << std::endl;
                }
        }
        void get_coor(int x, int y)
        {
            // cast nums ptr to extrac data
            nums = (int*)ptr;
            x = nums[0];
            y = nums[1];
            return;
        }
        ~Reader()
        {
            munmap(ptr, SIZE);
            close (shm_fd);
        }
}
        
