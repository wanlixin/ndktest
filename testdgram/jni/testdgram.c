#include <sys/socket.h>
#include <sys/un.h>

#include <errno.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>

void logcat(const char* msg) {
	const char* name = "testdgram";

	size_t namelen = strlen(name);

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof addr);
	addr.sun_family = AF_LOCAL;
	addr.sun_path[0] = 0;
	memcpy(addr.sun_path + 1, name, namelen);

	socklen_t alen = namelen + offsetof(struct sockaddr_un, sun_path) + 1;

	int s = socket(AF_LOCAL, SOCK_DGRAM, 0);
	if (s < 0)
		return;

	int count = sendto(s, msg, strlen(msg), 0, (struct sockaddr *) &addr, alen);
	if (count < -1) {
		perror("sendto");
	}
	close(s);
}

int logserver() {
	const char* name = "testdgram";

	int s = socket(AF_LOCAL, SOCK_DGRAM, 0);
	if (s < 0)
		return -1;

	struct sockaddr_un addr;

	memset(&addr, 0, sizeof(addr));
	size_t namelen = strlen(name);

	addr.sun_path[0] = 0;
	memcpy(addr.sun_path + 1, name, namelen);
	addr.sun_family = AF_LOCAL;
	socklen_t alen = namelen + offsetof(struct sockaddr_un, sun_path) + 1;

	int n = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n));

	if (bind(s, (struct sockaddr *) &addr, alen) < 0) {
		perror("bind");
		close(s);
		return -1;
	}

	fcntl(s, F_SETFD, FD_CLOEXEC);

	for (;;) {
		char buffer[4096];

		int count = recv(s, buffer, sizeof(buffer), 0);

		if (count < 0) {
			perror("recv");
		} else {
			buffer[count] = 0;
			printf("%s\n", buffer);
		}
	}

	close(s);
	return 0;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("usages:\ttestdgram -server\n\ttestdgram message\n");
		return 0;
	}

	if (strcmp(argv[1], "-server") == 0) {
		logserver();
	} else {
		logcat(argv[1]);
	}

	return 0;
}
