#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <pulse/simple.h>

#define BUFSIZE 1024
volatile sig_atomic_t flag_do = 1;

void handler(int s)
{
	signal(SIGINT, handler);
	flag_do = 0;
}

int loop_write(int fd, const void *data, size_t size)
{
	int ret = 0;

	while(size > 0) {
		int r;

		if((r = write(fd, data, size)) < 0)
			return r;

		if(r == 0)
			break;

		ret += r;
		data = (const uint8_t *) data + r;
		size -= (size_t) r;
	}

	return ret;
}

int main(int argc, char *argv[])
{
	signal(SIGINT, handler);

	pa_simple *connection;
	pa_sample_spec specification;
	int fd_output = STDOUT_FILENO;

	if(argv[1] != NULL) {
		mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		fd_output = open(argv[1], O_WRONLY | O_CREAT| O_TRUNC, mode);
		if(fd_output == -1) {
			perror("[Error] open");
			exit(1);
		}
	}

	specification.format = PA_SAMPLE_S16LE;
	specification.channels = 2;
	specification.rate = 44100;

	connection = pa_simple_new(NULL,
					argv[0],
					PA_STREAM_RECORD,
					NULL,
					"record",
					&specification,
					NULL,
					NULL,
					NULL);
	if(!connection) {
		fprintf(stderr, "[Error] pa_simple_new() failed\n");
		return 1;
	}

	fprintf(stderr, "[Message] Use \"Ctrl + C\" for exit\n");
	while(flag_do) {
		uint8_t buf[BUFSIZE];

		// Record some data
		if(pa_simple_read(connection, buf, sizeof(buf), NULL) < 0) {
			fprintf(stderr, "[Error] pa_simple_read() failed\n");
			return 1;
		}

		// And write it to fd_output
		if(loop_write(fd_output, buf, sizeof(buf)) != sizeof(buf)) {
			fprintf(stderr, "[Error] write() failed\n");
			return 1;
		}
	}

	if(connection)
		pa_simple_free(connection);

	return 0;
}