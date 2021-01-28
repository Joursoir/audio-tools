#ifndef PAR_AUDIOTYPES_H
#define PAR_AUDIOTYPES_H

#include <stdint.h>
#include <sys/types.h>
#include <pulse/sample.h>

#include "wav_header.h"

#define MAX_AUDIO_TYPE_LEN 5
#define MAX_AUDIO_FORMAT_LEN 11

enum audio_types {
	AUDIO_TYPE_WAVE,
	AUDIO_TYPE_MAX,
	AUDIO_TYPE_NONE = -1
};

struct audio_types_info {
	const char name[MAX_AUDIO_TYPE_LEN];
	off_t rsv_bytes;
};

enum audio_formats {
	AUDIO_FORMAT_U8 = PA_SAMPLE_U8,
	AUDIO_FORMAT_ALAW = PA_SAMPLE_ALAW,
	AUDIO_FORMAT_ULAW = PA_SAMPLE_ULAW,
	AUDIO_FORMAT_S16LE = PA_SAMPLE_S16LE,
	AUDIO_FORMAT_S16BE = PA_SAMPLE_S16BE,
	AUDIO_FORMAT_FLOAT32LE = PA_SAMPLE_FLOAT32LE,
	AUDIO_FORMAT_FLOAT32BE = PA_SAMPLE_FLOAT32BE,
	AUDIO_FORMAT_S32LE = PA_SAMPLE_S32LE,
	AUDIO_FORMAT_S32BE = PA_SAMPLE_S32BE,
	AUDIO_FORMAT_S24LE = PA_SAMPLE_S24LE,
	AUDIO_FORMAT_S24BE = PA_SAMPLE_S24BE,
	AUDIO_FORMAT_S24_32LE = PA_SAMPLE_S24_32LE,
	AUDIO_FORMAT_S24_32BE = PA_SAMPLE_S24_32BE,
	AUDIO_FORMAT_MAX = 13,
	AUDIO_FORMAT_NONE = -1
};

struct wav_header *init_wav_header(struct wav_header *header,
	uint32_t size, uint16_t audioFormat, uint16_t numChannels,
	uint32_t sampleRate, uint32_t bitsPerSample);
int checkAudioType(char *source);
char *getAllAudioTypes();
off_t getOffset(int format);
int checkAudioFormat(char *source);
char *getAllAudioFormats();

#endif /* AT_AUDIOTYPES_H */
