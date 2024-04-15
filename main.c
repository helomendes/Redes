#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

int main() {
	int socket_handle = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	return 0;
}
