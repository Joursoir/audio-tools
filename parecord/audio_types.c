#include <string.h>

#include "audio_types.h"

static const struct audio_types_info support_types[] = {
	{"wav", 44}, // AUDIO_TYPE_WAVE
	{NULL, 0}
};

static const struct audio_formats_info support_formats[] = {
	{"U8", PA_SAMPLE_U8},
	{"A_LAW", PA_SAMPLE_ALAW},
	{"MU_LAW", PA_SAMPLE_ULAW},
	{"S16_LE", PA_SAMPLE_S16LE},
	{"S16_BE", PA_SAMPLE_S16BE},
	{"FLOAT32_LE", PA_SAMPLE_FLOAT32LE},
	{"FLOAT32_BE", PA_SAMPLE_FLOAT32BE},
	{"S32_LE", PA_SAMPLE_S32LE},
	{"S32_BE", PA_SAMPLE_S32BE},
	{"S24_LE", PA_SAMPLE_S24LE},
	{"S24_BE", PA_SAMPLE_S24BE},
	{"S24_32LE", PA_SAMPLE_S24_32LE},
	{"S24_32BE", PA_SAMPLE_S24_32BE},
	{NULL, 0}
};

struct wav_header *init_wav_header(struct wav_header *header,
	uint32_t size, uint16_t audioFormat, uint16_t numChannels,
	uint32_t sampleRate, uint32_t bitsPerSample)
{
	header->chunkId = 0x46464952; // = "RIFF"
	header->chunkSize = size - 8;
	header->format = 0x45564157; // = "WAVE"
	header->subchunk1Id = 0x20746d66; // = "fmt "
	header->subchunk1Size = 16;
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

int checkAudioType(char *source)
{
	int i;
	for(i = 0; support_types[i].name != NULL; i++) {
		if(strcmp(source, support_types[i].name) == 0) {
			return i;
		}
	}

	return AUDIO_TYPE_NONE;
}

off_t getOffset(int format)
{
	return support_types[format].rsv_bytes;
}

pa_sample_format_t checkAudioFormat(char *source)
{
	int i;
	for(i = 0; support_formats[i].name != NULL; i++) {
		if(strcmp(source, support_formats[i].name) == 0) {
			return support_formats[i].pa_format;
		}
	}

	return PA_SAMPLE_INVALID;
}