#ifndef PAR_AUDIOTYPES_H
#define PAR_AUDIOTYPES_H

#include <stdint.h>
#include <sys/types.h>

#include "wav_header.h"

enum audio_types {
	AUDIO_TYPE_WAVE,
	AUDIO_TYPE_NONE = -1
};

struct audio_types_info {
	const char *name;
	off_t rsv_bytes;
};

struct wav_header *init_wav_header(struct wav_header *header,
	uint32_t size, uint16_t audioFormat, uint16_t numChannels,
	uint32_t sampleRate, uint32_t bitsPerSample);
int checkAudioType(char *source);
off_t getOffset(int format);

#endif /* AT_AUDIOTYPES_H */
