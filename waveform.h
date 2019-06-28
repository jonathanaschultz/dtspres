#pragma once

#include <stdint.h>
#include <stdio.h>

#ifdef WIN32

#include <windows.h>
#include <ks.h>
#include <ksmedia.h>
#include <mmsystem.h>

#endif

#pragma pack(1)

#ifndef WIN32

#define WAVE_FORMAT_PCM             1
#define WAVE_FORMAT_EXTENSIBLE 0xfffe

typedef struct _GUID {
	uint32_t Data1;
	uint16_t Data2;
	uint16_t Data3;
	uint8_t  Data4[8];
} GUID;

typedef struct {
	uint16_t wFormatTag;
	uint16_t nChannels;
	uint32_t nSamplesPerSec;
	uint32_t nAvgBytesPerSec;
	uint16_t nBlockAlign;
} WAVEFORMAT;

typedef struct {
	WAVEFORMAT wf;
	uint16_t   wBitsPerSample;
} PCMWAVEFORMAT;

typedef struct {
	uint16_t wFormatTag;
	uint16_t nChannels;
	uint32_t nSamplesPerSec;
	uint32_t nAvgBytesPerSec;
	uint16_t nBlockAlign;
	uint16_t wBitsPerSample;
	uint16_t cbSize;
} WAVEFORMATEX; 

typedef struct {
	WAVEFORMATEX Format;
	union {
		uint16_t wValidBitsPerSample;
		uint16_t wSamplesPerBlock;
		uint16_t wReserved;
  } Samples;
	uint32_t dwChannelMask;
	GUID     SubFormat;
} WAVEFORMATEXTENSIBLE;

#endif

typedef struct {
	uint8_t  signature[4];
	uint32_t length;
	uint8_t  type[4];
} WAV_RIFF;

typedef struct {
	uint8_t  signature[4];
	uint32_t length;
	union {
		PCMWAVEFORMAT        pcm;
		WAVEFORMATEXTENSIBLE ext;
	} fmt;
} WAV_FMT;

typedef struct {
	uint8_t  signature[4];
	uint32_t length;
} WAV_DATA;

#pragma pack()

class waveform_t {
private:
	FILE* file;
	WAV_RIFF wav_riff;
	WAV_FMT  wav_fmt;
	WAV_DATA wav_data;
	long data_offset;
	long data_length;
	int alignment;
	int bits;
	int channels;
	int samplerate;
	int samplesize;
public:
	waveform_t(FILE* file);
	int  init();
	int  init(int channels, int samplerate, int bits, uint32_t channelMask, bool oldFormat);
	void flush();
	int  clone(waveform_t& output_wav);
	int  sync();
	int get_bits() {
		return bits;
	};
	int get_channels() {
		return channels;
	};
	int get_samplerate() {
		return samplerate;
	};
	int get_samplesize() {
		return samplesize;
	};
	int get_samples() {
		return data_length / alignment;
	};
	int read(int channel, int16_t* p_buffer, int index, int offset, int samples);
	int write(int channel, int16_t* p_buffer, int index, int offset, int samples);
	int read(int16_t* p_buffer, int samples);
	int write(int16_t* p_buffer, int samples);
};
