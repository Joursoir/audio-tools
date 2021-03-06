#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>
#include <errno.h>
#include <sys/stat.h>
#include <pulse/error.h>
#include <pulse/simple.h>

#include "audio_types.h"
#include "device.h"

#define PAR_RATE_MIN 2000U
#define PAR_CHANNELS_MIN 1U

#define BUFSIZE 1024
#define STD_REC_FORMAT AUDIO_FORMAT_S16LE
#define STD_REC_RATE 44100U
#define STD_REC_CHANNELS PAR_CHANNELS_MIN

volatile static sig_atomic_t flag_do = 1;

void signal_handler(int s)
{
	signal(SIGINT, signal_handler);
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
	signal(SIGINT, signal_handler);

	pa_simple *connection;
	pa_sample_spec specification;

	int fd_output = STDOUT_FILENO, result, error;
	int file_type = AUDIO_TYPE_NONE;
	int record_format = STD_REC_FORMAT;
	uint32_t record_rate = STD_REC_RATE;
	uint8_t record_channels = STD_REC_CHANNELS;
	char *device = NULL;
	off_t offset;
	const struct option long_options[] = {
		{"help", no_argument, NULL, 'h'},
		{"device", required_argument, NULL, 'd'},
		{"list-devices", no_argument, NULL, 'D'},
		{"file-type", required_argument, NULL, 't'},
		{"file-types", no_argument, NULL, 'T'},
		{"format", required_argument, NULL, 'f'},
		{"formats", no_argument, NULL, 'F'},
		{"channels", required_argument, NULL, 'c'},
		{"rate", required_argument, NULL, 'r'},
		{NULL, 0, NULL, 0}
	};

	while((result = getopt_long(argc, argv, "hd:Dt:Tf:Fc:r:",
		long_options, NULL)) != -1) {
		switch(result) {
		case 'h': {
			printf("Synopsis:\n"
					"\tparecord [OPTION] ... [FILE]\n"
					"Option:\n"
					"\t-h, --help\n"
					"\t\tPrint this text.\n"
					"\t-d, --device\n"
					"\t\tName of the audio devices to record stream.\n"
					"\t-D, --list-devices\n"
					"\t\tPrint all recorder audio devices\n"
					"\t-t, --file-type\n"
					"\t\tFile type (pcm, wav). PCM is used by default\n"
					"\t-T, --file-types\n"
					"\t\tPrint valid file types\n"
					"\t-f, --format\n"
					"\t\tSample format. S16_LE is used by default\n"
					"\t-F, --formats\n"
					"\t\tPrint valid sample formats\n"
					"\t-c, --channels\n"
					"\t\tThe number of channels. One channel is used by default.\n"
					"\t-r, --rate\n"
					"\t\tSample rate in Hz. %d Hz is used by default.\n",
					STD_REC_RATE);
			return 0;
		}
		case 'd': {
			device = malloc((strlen(optarg)+1) * sizeof(char));
			strcpy(device, optarg);
			break;
		}
		case 'D': {
			int i, list_devices_len;
			struct list_devices *input_devices
				= getInputDeviceList(&list_devices_len);

			if(!input_devices)
				return 1;

			printf("**** Input devices ****\n");
			for(i = 0; i < list_devices_len; i++) {
				printf("device %d: %s\n"
						"\t%s\n",
						i+1, input_devices[i].name, input_devices[i].description);
			}

			freeDeviceList(input_devices, list_devices_len);
			return 0;
		}
		case 't': {
			file_type = checkAudioType(optarg);
			if(file_type != AUDIO_TYPE_NONE) break;
			// else print file types (below)
		}
		case 'T': {
			char *types = getAllAudioTypes();
			printf("Valid file types are:%s\n", types);
			free(types);
			return 0;
		}
		case 'f': {
			record_format = checkAudioFormat(optarg);
			if(record_format != PA_SAMPLE_INVALID) break;
			// else print formats (below)
		}
		case 'F': {
			char *formats = getAllAudioFormats();
			printf("Valid sample formats are:%s\n", formats);
			free(formats);
			return 0;
		}
		case 'c': {
			record_channels = atoi(optarg);

			if(record_channels < PAR_CHANNELS_MIN || record_channels > PA_CHANNELS_MAX) {
				fprintf(stderr, "Valid channels values are %d - %d\n",
					PAR_CHANNELS_MIN, PA_CHANNELS_MAX);
				return 1;
			}
			break;
		}
		case 'r': {
			record_rate = atoi(optarg);

			if(record_rate < PAR_RATE_MIN || record_rate > PA_RATE_MAX) {
				fprintf(stderr, "Valid rate values are %d - %d Hz\n",
					PAR_RATE_MIN, PA_RATE_MAX);
				return 1;
			}
			break;
		}
		default: break;
		}
	}

	specification.format = record_format;
	specification.channels = record_channels;
	specification.rate = record_rate;

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
					device,
					"record",
					&specification,
					NULL,
					NULL,
					&error);
	if(!connection) {
		fprintf(stderr, "[Error] pa_simple_new(): %s\n", pa_strerror(error));
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
			fprintf(stderr, "WAV HEADER\n"
				"chunkId: %#x\n"
				"chunkSize: %#x\n"
				"format: %#x\n"
				"subchunk1Id: %#x\n"
				"subchunk1Size: %#x\n"
				"audioFormat: %#x\n"
				"numChannels: %#x\n"
				"sampleRate: %#x\n"
				"byteRate: %#x\n"
				"blockAlign: %#x\n"
				"bitsPerSample: %#x\n"
				"subchunk2Id: %#x\n"
				"subchunk2Size: %#x\n",
				header->chunkId, header->chunkSize, header->format,
				header->subchunk1Id, header->subchunk1Size, header->audioFormat,
				header->numChannels, header->sampleRate, header->byteRate,
				header->blockAlign, header->bitsPerSample, header->subchunk2Id,
				header->subchunk2Size);
		#endif

		if(write(fd_output, header, offset) < 0) {
			perror("[Error] write()");
			return 1;
		}
	}
	default: break;
	}

	// clear:

	if(fd_output != STDOUT_FILENO) {
		close(fd_output);
	}

	if(!device)
		free(device);

	return 0;
}
