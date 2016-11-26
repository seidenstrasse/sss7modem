#include <stdio.h>
#include <stdint.h>

#include "libsss7.h"

int main(int argc, char const *argv[]) {
	libsss7_start("/tmp/ttyVA");

	libsss7_stop();
	return 0;
}
