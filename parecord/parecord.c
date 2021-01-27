#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>
#include <errno.h>
#include <sys/stat.h>
#include <pulse/simple.h>

#include "audio_types.h"

#define BUFSIZE 1024
volatile static sig_atomic_t flag_do = 1;

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
	specification.format = PA_SAMPLE_S16LE;
	specification.channels = 2;
	specification.rate = 44100;

	int fd_output = STDOUT_FILENO;
	int file_type = AUDIO_TYPE_NONE, result;
	off_t offset;
	const struct option long_options[] = {
		{"help", no_argument, NULL, 'h'},
		{"file-type", required_argument, NULL, 't'},
		{"file-types", no_argument, NULL, 'T'},
		{NULL, 0, NULL, 0}
	};

	while((result = getopt_long(argc, argv, "ht:T", long_options, NULL)) != -1) {
		switch(result) {
			case 'h': { 
				// print help
				return 0;
			}
			case 't': {
				file_type = checkAudioType(optarg);
				if(file_type != AUDIO_TYPE_NONE) break;
				// else print formats (below)
			}
			case 'T': {
				// print formats
				return 0;
			}
			default: break;
		}
	}

	if(argv[optind] != NULL) {
		mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		fd_output = open(argv[optind], O_WRONLY | O_CREAT| O_TRUNC, mode);
		if(fd_output == -1) {
			perror("[Error] open()");
			return 1;
		}
	}

	// reserve bytes for audio header:
	if(file_type != AUDIO_TYPE_NONE) {
		offset = getOffset(file_type);
		if(lseek(fd_output, offset, SEEK_SET) == -1) {
			perror("[Error] lseek() form header");
			return 1;
		}
	}

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

	if(file_type != AUDIO_TYPE_NONE) {
		if(lseek(fd_output, 0, SEEK_SET) == -1) {
			perror("[Error] lseek()");
			return 1;
		}
	}

	switch(file_type)
	{
	case AUDIO_TYPE_NONE: break;
	case AUDIO_TYPE_WAVE: {
		struct stat s;
		if(fstat(fd_output, &s) == -1) {
			perror("[Error] fstat");
			return 1;
		}

		struct wav_header *header = malloc(offset);
		init_wav_header(header, s.st_size, 1,
			specification.channels, specification.rate, 16);

		#ifdef DEBUG
			fprintf(stderr, "chunkId: %#x\n", header->chunkId);
			fprintf(stderr, "chunkSize: %#x\n", header->chunkSize);
			fprintf(stderr, "format: %#x\n", header->format);
			fprintf(stderr, "subchunk1Id: %#x\n", header->subchunk1Id);
			fprintf(stderr, "subchunk1Size: %#x\n", header->subchunk1Size);
			fprintf(stderr, "audioFormat: %#x\n", header->audioFormat);
			fprintf(stderr, "numChannels: %#x\n", header->numChannels);
			fprintf(stderr, "sampleRate: %#x\n", header->sampleRate);
			fprintf(stderr, "byteRate: %#x\n", header->byteRate);
			fprintf(stderr, "blockAlign: %#x\n", header->blockAlign);
			fprintf(stderr, "bitsPerSample: %#x\n", header->bitsPerSample);
			fprintf(stderr, "subchunk2Id: %#x\n", header->subchunk2Id);
			fprintf(stderr, "subchunk2Size: %#x\n", header->subchunk2Size);
		#endif

		if(write(fd_output, header, offset) < 0) {
			perror("[Error] write()");
			return 1;
		}
	}
	default: break;
	}

	if(fd_output != STDOUT_FILENO) {
		close(fd_output);
	}

	return 0;
}
