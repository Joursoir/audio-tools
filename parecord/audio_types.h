#ifndef PAR_AUDIOTYPES_H
#define PAR_AUDIOTYPES_H

#include <stdint.h>
#include <sys/types.h>

#include "wav_header.h"

#define AUDIO_FORMAT_NONE -1
#define AUDIO_FORMAT_WAVE 0

struct audio_format {
	const char *name;
	off_t rsv_bytes;
};

struct wav_header *init_wav_header(struct wav_header *header,
	uint32_t size, uint32_t subchunk1Size, uint16_t audioFormat,
	uint16_t numChannels, uint32_t sampleRate, uint32_t bitsPerSample);
int checkAudioFormat(char *source);
off_t getOffset(int format);

#endif /* AT_AUDIOTYPES_H */
