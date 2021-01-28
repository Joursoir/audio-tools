#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio_types.h"

static const struct audio_types_info support_types[AUDIO_TYPE_MAX] = {
	{"wav", 44}
};

static const char
	audio_format_name[AUDIO_FORMAT_MAX][MAX_AUDIO_FORMAT_LEN] = {
	"U8",
	"A_LAW",
	"MU_LAW",
	"S16_LE",
	"S16_BE",
	"FLOAT32_LE",
	"FLOAT32_BE",
	"S32_LE",
	"S32_BE",
	"S24_LE",
	"S24_BE",
	"S24_32LE",
	"S24_32BE"
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
	for(i = 0; i < AUDIO_TYPE_MAX; i++) {
		if(strcmp(source, support_types[i].name) == 0) {
			return i;
		}
	}

	return AUDIO_TYPE_NONE;
}

char *getAllAudioTypes()
{
	int i;
	char *dest = malloc(sizeof(char) * MAX_AUDIO_TYPE_LEN *
		AUDIO_TYPE_MAX + sizeof(char));
	char *str = dest;
	for(i = 0; i < AUDIO_TYPE_MAX; i++) {
		str += sprintf(str, " %s", support_types[i].name);
	}

	return dest;
}

off_t getOffset(int format)
{
	return support_types[format].rsv_bytes;
}

int checkAudioFormat(char *source)
{
	int i;
	for(i = 0; i < AUDIO_FORMAT_MAX; i++) {
		if(strcmp(source, audio_format_name[i]) == 0) {
			return i;
		}
	}

	return PA_SAMPLE_INVALID;
}

char *getAllAudioFormats()
{
	int i;
	char *dest = malloc(sizeof(char) * MAX_AUDIO_FORMAT_LEN *
		AUDIO_FORMAT_MAX + sizeof(char));
	char *str = dest;
	for(i = 0; i < AUDIO_FORMAT_MAX; i++) {
		str += sprintf(str, " %s", audio_format_name[i]);
	}

	return dest;
}