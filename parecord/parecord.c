#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>
#include <errno.h>
#include <pulse/simple.h>

#define BUFSIZE 1024
volatile sig_atomic_t flag_do = 1;

enum audio_format {
	none, // = 0
	wav // = 1
};

static const char *correct_formats[] = {
	"wav", NULL
};

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
	int result, i;
	enum audio_format file_format = none;
	const struct option long_options[] = {
		{"help", no_argument, NULL, 'h'},
		{"format", required_argument, NULL, 'f'},
		{"formats", no_argument, NULL, 'F'},
		{NULL, 0, NULL, 0}
	};

	while((result = getopt_long(argc, argv, "hf:F", long_options, NULL)) != -1) {
		switch(result) {
			case 'h': { 
				// print help
				break;
			}
			case 'f': {
				for(i = 0; correct_formats[i] != NULL; i++) {
					if(strcmp(optarg, correct_formats[i]) == 0) {
						file_format = i+1;
						break;
					}
				}

				if(file_format != none) break;
				// else print formats (below)
			}
			case 'F': {
				// print formats
				break;
			}
			default: break;
		}
	}
	
	if(argv[optind] != NULL) {
		mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		fd_output = open(argv[optind], O_WRONLY | O_CREAT| O_TRUNC, mode);
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
