#ifndef WAV_HEADER_H
#define WAV_HEADER_H

struct wav_header
{
    uint32_t chunkId;
    uint32_t chunkSize;
    uint32_t format;
    uint32_t subchunk1Id;
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    uint32_t subchunk2Id;
    uint32_t subchunk2Size;
};

#endif /* WAV_HEADER_H */
