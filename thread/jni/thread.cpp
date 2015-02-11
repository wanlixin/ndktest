#include <unistd.h>
#include <thread>

int main(int argc, char** argv) {

	std::thread threads[10];

	for (int i = 0; i < sizeof(threads) / sizeof(threads[0]); ++i) {
		threads[i] =
				std::thread(
						[i]() {
							printf("thread%d start pid: %d tid: %d pgid: %d ppid: %d\n", i, getpid(), gettid(), getpgid(getpid()), getppid());
							sleep(10);
							printf("thread%d end pid: %d tid: %d pgid: %d ppid: %d\n", i, getpid(), gettid(), getpgid(getpid()), getppid());
							return 0;
						});
	}

	printf("main pid: %d tid: %d pgid: %d ppid: %d\n", getpid(), gettid(),
			getpgid(getpid()), getppid());

	for (int i = 0; i < sizeof(threads) / sizeof(threads[0]); ++i) {
		threads[i].join();
	}

	return 0;
}
