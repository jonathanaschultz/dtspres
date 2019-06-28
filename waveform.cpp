#include <memory.h>
#include <stdlib.h>
#include "waveform.h"

#if !defined min
	#define min(a, b) ((a < b) ? a : b)
#endif

#if !defined max
	#define max(a, b) ((a > b) ? a : b)
#endif

waveform_t::waveform_t(FILE* file) {
	this->file = file;
	data_offset = -1;
	data_length = -1;
}

int waveform_t::init() {
	if (fseek(file, 0, SEEK_SET))
		return -1;
	if (fread(&wav_riff, sizeof(WAV_RIFF), 1, file) != 1)
		return -2;
	if (memcmp(wav_riff.signature, "RIFF", sizeof(wav_riff.signature)))
		return -2;
	if (memcmp(wav_riff.type, "WAVE", sizeof(wav_riff.type)))
		return -2;
	if (fread(&wav_fmt.signature, 8, 1, file) != 1)
		return -3;
	if (memcmp(wav_fmt.signature, "fmt ", sizeof(wav_fmt.signature)))
		return -3;
	if (wav_fmt.length <= sizeof(WAV_FMT)) {
		if (fread(&wav_fmt.fmt, wav_fmt.length, 1, file) != 1)
			return -3;
	}
	else {
		if (fread(&wav_fmt.fmt, sizeof(WAV_FMT) - 8, 1, file) != 1)
			return -3;
	}
	if (fseek(file, sizeof(WAV_RIFF) + 8 + wav_fmt.length, SEEK_SET))
		return -4;
	if (fread(&wav_data, sizeof(WAV_DATA), 1, file) != 1)
		return -4;
	if (memcmp(wav_data.signature, "data", sizeof(wav_data.signature)))
		return -4;
	data_offset = sizeof(WAV_RIFF) + 8 + wav_fmt.length + sizeof(WAV_DATA);
	data_length = wav_data.length;
	switch (wav_fmt.fmt.pcm.wf.wFormatTag) {
	case WAVE_FORMAT_PCM:
		alignment = wav_fmt.fmt.pcm.wf.nBlockAlign;
		bits = wav_fmt.fmt.pcm.wBitsPerSample;
		channels = wav_fmt.fmt.pcm.wf.nChannels;
		samplerate = wav_fmt.fmt.pcm.wf.nSamplesPerSec;
		break;
	case WAVE_FORMAT_EXTENSIBLE:
		alignment = wav_fmt.fmt.ext.Format.nBlockAlign;
		bits = wav_fmt.fmt.ext.Format.wBitsPerSample;
		channels = wav_fmt.fmt.ext.Format.nChannels;
		samplerate = wav_fmt.fmt.ext.Format.nSamplesPerSec;
		break;
	}
	samplesize = alignment / channels;
	if (samplesize > sizeof(uint32_t) || samplesize * channels != alignment)
		return -5;
	return 0;
}

int waveform_t::init(int channels, int samplerate, int bits, uint32_t channelMask, bool oldFormat) {
	if (fseek(file, 0, SEEK_SET))
		return -1;
	memcpy(wav_riff.signature, "RIFF", sizeof(wav_riff.signature));
	wav_riff.length = 0;
	memcpy(wav_riff.type, "WAVE", sizeof(wav_riff.type));
	if (fwrite(&wav_riff, sizeof(wav_riff), 1, file) != 1)
		return -2;
	if (!oldFormat) {
		memcpy(wav_fmt.signature, "fmt ", sizeof(wav_fmt.signature));
		wav_fmt.length = sizeof(WAVEFORMATEXTENSIBLE);
		wav_fmt.fmt.ext.Format.wFormatTag      = WAVE_FORMAT_EXTENSIBLE;
		wav_fmt.fmt.ext.Format.nChannels       = (uint16_t)channels;
		wav_fmt.fmt.ext.Format.nSamplesPerSec  = (uint32_t)samplerate;
		wav_fmt.fmt.ext.Format.wBitsPerSample  = (uint16_t)bits;
		wav_fmt.fmt.ext.Format.nBlockAlign     = wav_fmt.fmt.ext.Format.nChannels * (wav_fmt.fmt.ext.Format.wBitsPerSample / 8 + (wav_fmt.fmt.ext.Format.wBitsPerSample % 8 ? 1 : 0));
		wav_fmt.fmt.ext.Format.nAvgBytesPerSec = wav_fmt.fmt.ext.Format.nSamplesPerSec * wav_fmt.fmt.ext.Format.nBlockAlign;
		wav_fmt.fmt.ext.Format.cbSize          = 0x16;
		wav_fmt.fmt.ext.Samples.wValidBitsPerSample = (uint16_t)bits;
		wav_fmt.fmt.ext.dwChannelMask = channelMask;
		// Set GUID KSDATAFORMAT_SUBTYPE_PCM
		wav_fmt.fmt.ext.SubFormat.Data1    = 0x00000001;
		wav_fmt.fmt.ext.SubFormat.Data2    = 0x0000;
		wav_fmt.fmt.ext.SubFormat.Data3    = 0x0010;
		wav_fmt.fmt.ext.SubFormat.Data4[0] = 0x80;
		wav_fmt.fmt.ext.SubFormat.Data4[1] = 0x00;
		wav_fmt.fmt.ext.SubFormat.Data4[2] = 0x00;
		wav_fmt.fmt.ext.SubFormat.Data4[3] = 0xaa;
		wav_fmt.fmt.ext.SubFormat.Data4[4] = 0x00;
		wav_fmt.fmt.ext.SubFormat.Data4[5] = 0x38;
		wav_fmt.fmt.ext.SubFormat.Data4[6] = 0x9b;
		wav_fmt.fmt.ext.SubFormat.Data4[7] = 0x71;
		if (fwrite(&wav_fmt, wav_fmt.length + 8, 1, file) != 1)
			return -3;
		this->alignment = wav_fmt.fmt.ext.Format.nBlockAlign;
		this->bits = wav_fmt.fmt.ext.Format.wBitsPerSample;
		this->channels = wav_fmt.fmt.ext.Format.nChannels;
		this->samplerate = wav_fmt.fmt.ext.Format.nSamplesPerSec;
	}
	else {
		memcpy(wav_fmt.signature, "fmt ", sizeof(wav_fmt.signature));
		wav_fmt.length = sizeof(PCMWAVEFORMAT);
		wav_fmt.fmt.pcm.wf.wFormatTag      = WAVE_FORMAT_PCM;
		wav_fmt.fmt.pcm.wf.nChannels       = (uint16_t)channels;
		wav_fmt.fmt.pcm.wf.nSamplesPerSec  = (uint32_t)samplerate;
		wav_fmt.fmt.pcm.wBitsPerSample     = (uint16_t)bits;
		wav_fmt.fmt.pcm.wf.nBlockAlign     = wav_fmt.fmt.pcm.wf.nChannels * (wav_fmt.fmt.pcm.wBitsPerSample / 8 + (wav_fmt.fmt.pcm.wBitsPerSample % 8 ? 1 : 0));
		wav_fmt.fmt.pcm.wf.nAvgBytesPerSec = wav_fmt.fmt.pcm.wf.nSamplesPerSec * wav_fmt.fmt.pcm.wf.nBlockAlign;
		if (fwrite(&wav_fmt, wav_fmt.length + 8, 1, file) != 1)
			return -3;
		this->alignment = wav_fmt.fmt.pcm.wf.nBlockAlign;
		this->bits = wav_fmt.fmt.pcm.wBitsPerSample;
		this->channels = wav_fmt.fmt.pcm.wf.nChannels;
		this->samplerate = wav_fmt.fmt.pcm.wf.nSamplesPerSec;
	}
	samplesize = alignment / channels;
	memcpy(wav_data.signature, "data", sizeof(wav_data.signature));
	wav_data.length = 0;
	if (fwrite(&wav_data, sizeof(wav_data), 1, file) != 1)
		return -4;
	data_offset = ftell(file);
	data_length = 0;
	fflush(file);
	return 0;
}

int waveform_t::clone(waveform_t& output_wav) {
	int err = 0;
	long file_pos = ftell(file);
	uint8_t* p_buffer = new uint8_t[data_offset];
	if (!p_buffer) {
		err = -1;
		goto end_clone;
	}
	if (fseek(file, 0, SEEK_SET)) {
		err = -2;
		goto end_clone;
	}
	if (fseek(output_wav.file, 0, SEEK_SET)) {
		err = -3;
		goto end_clone;
	}
	if (fread(p_buffer, data_offset, 1, file) != 1) {
		err = -4;
		goto end_clone;
	}
	if (fwrite(p_buffer, data_offset, 1, output_wav.file) != 1) {
		err = -5;
		goto end_clone;
	}
	if (output_wav.init() < 0) {
		err = -6;
		goto end_clone;
	}
	if (fseek(output_wav.file, data_offset + data_length - 1, SEEK_SET)) {
		err = -7;
		goto end_clone;
	}
	char c = 0;
	if (fwrite(&c, 1, 1, output_wav.file) != 1) {
		err = -8;
		goto end_clone;
	}
	output_wav.sync();
	if (fseek(output_wav.file, data_offset, SEEK_SET)) {
		err = -9;
		goto end_clone;
	}
end_clone:
	if (p_buffer)
		delete[] p_buffer;
	fseek(file, file_pos, SEEK_SET);
	fseek(file, file_pos, SEEK_SET);
	output_wav.data_length = 0;
	return err;
}

void waveform_t::flush() {
	fflush(file);
}

int waveform_t::sync() {
	if (data_offset < 0)
		return -1;
	long curr_pos = ftell(file);
	fseek(file, 0, SEEK_END);
	long feof_pos = ftell(file);
	fseek(file, 4, SEEK_SET);
	data_length = feof_pos - 8;
	fwrite(&data_length, 4, 1, file);
	fseek(file, data_offset - 4, SEEK_SET);
	data_length = feof_pos - data_offset;
	fwrite(&data_length, 4, 1, file);
	fseek(file, curr_pos, SEEK_SET);
	return 0;
}

int waveform_t::read(int channel, int16_t* p_buffer, int index, int offset, int samples) {
	int samples_to_read = min(data_length / alignment - index, samples);
	if (samples_to_read <= 0)
		return 0;
	if (fseek(file, data_offset + index * alignment, SEEK_SET))
		return -1;
	for (int i = 0; i < samples_to_read; i++) {
		for (int ch = 0; ch < channels; ch++) {
			int32_t sample = 0;
			if (fread(&sample, samplesize, 1, file) != 1)
				return i;
			sample <<= 8 * (4 - samplesize);
			if (ch == channel)
				p_buffer[offset + i] = (int16_t)(sample >> 16);
		}
	}
	return samples_to_read;
}

int waveform_t::write(int channel, int16_t* p_buffer, int index, int offset, int samples) {
	if (fseek(file, data_offset + index * alignment, SEEK_SET))
		return -1;
	for (int i = 0; i < samples; i++) {
		for (int ch = 0; ch < channels; ch++) {
			if (ch == channel) {
				int32_t sample = p_buffer[offset + i] << 16;
				if (fwrite(&sample, samplesize, 1, file) != 1)
					return i;
			}
			else
				if (fseek(file, samplesize, SEEK_CUR))
					return -1;
		}
	}
	return samples;
}

int waveform_t::read(int16_t* p_buffer, int samples) {
	int samples_to_read = min(data_length / alignment, samples);
	if (samples_to_read <= 0)
		return 0;
	for (int i = 0; i < samples_to_read; i++) {
		for (int ch = 0; ch < channels; ch++) {
			int32_t sample = 0;
			if (fread(&sample, samplesize, 1, file) != 1)
				return i;
			sample <<= 8 * (4 - samplesize);
			p_buffer[i * channels + ch] = (int16_t)(sample >> 16);
		}
	}
	return samples_to_read;
}

int waveform_t::write(int16_t* p_buffer, int samples) {
	return fwrite(p_buffer, channels * samplesize, samples, file);
}
