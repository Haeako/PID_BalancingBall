#include "point.h"

Reader::Reader() {
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
void Reader::get_coor(int x, int y) {
 // cast nums ptr to extrac data
  nums = (int*)ptr;
  x = nums[0];
  y = nums[1];
  return;
}
~Reader::Reader()
{
  munmap(ptr, SIZE);
  close(shm_fd);
}
        
