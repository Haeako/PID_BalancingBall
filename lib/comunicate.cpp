#include "comunicate.h"

int main()
{
	int x, y ;
	Reader read;
	read.get_coor(&x, &y);
	std::cout << x << " " << y << std::endl;
	
	return 0;
	}
