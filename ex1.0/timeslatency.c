#include <sys/times.h>

int main(void) {
	struct tms t;

	for(int i = 0; i < 10*1000*1000;i++)
	{
		times(&t);
	}

	return 0;
}
