#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

using namespace std;

int main() {
	int rs = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (rs < 0) {
		cout << errno << "\n";
		return 1;
	}
	cout << "rs: " << rs << "\n";
}
