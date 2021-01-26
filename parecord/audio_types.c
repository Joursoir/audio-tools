#include <string.h>

#include "audio_types.h"

static const struct audio_format support_formats[] = {
	{"wav", 44}, // AUDIO_FORMAT_WAVE = 0
	{NULL, 0}
};

struct wav_header *init_wav_header(struct wav_header *header,
	uint32_t size, uint32_t subchunk1Size, uint16_t audioFormat,
	uint16_t numChannels, uint32_t sampleRate, uint32_t bitsPerSample)
{
	header->chunkId = 0x46464952; // = "RIFF"
	header->chunkSize = size - 8;
	header->format = 0x45564157; // = "WAVE"
	header->subchunk1Id = 0x20746d66; // = "fmt "
	header->subchunk1Size = subchunk1Size;
	header->audioFormat = audioFormat;
	header->numChannels = numChannels;
	header->sampleRate = sampleRate;
	header->byteRate = sampleRate * numChannels * (bitsPerSample/8);
	header->blockAlign = numChannels * (bitsPerSample/8);
	header->bitsPerSample = bitsPerSample;
	header->subchunk2Id = 0x61746164; // = "data"
	header->subchunk2Size = size - 44;

	return header;
}

int checkAudioFormat(char *source)
{
	int i;
	for(i = 0; support_formats[i].name != NULL; i++) {
		if(strcmp(source, support_formats[i].name) == 0) {
			return i;
		}
	}

	return AUDIO_FORMAT_NONE;
}

off_t getOffset(int format)
{
	return support_formats[format].rsv_bytes;
}