#include <conio.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "libaptx.h"
#include "libaue.h"
#include "audform.h"
#include "waveform.h"
#include "dtslfe.h"

enum file_ext_t {EXT_UNK, EXT_AUD, EXT_AUE, EXT_WAV};

void copyright() {
	printf("DTS AUD/E <=> WAVe File Converter. Version 0.1.2\n\n");
}

void usage() {
	printf("Use: audconv -i <inp-file> -o <out-file> [-key <num>] [-lfe <dB>] [-title <str>] [-reel <num>] [-serial <num>] [-tracks <num>] [-start <str>] [-end <str>]\n");
	printf("Where:\n");
	printf("\t-i <inp-file> : Input WAVe, AUD or AUE file name\n");
	printf("\t-o <out-file> : Output WAVe or AUD file name\n");
	printf("\t-key <num>    : Override AUE decryption key\n");
	printf("\t-lfe <dB>     : Add scaled LFE channel +-dB\n");
	printf("\t-title <str>  : AUD file title string\n");
	printf("\t-reel <num>   : AUD file reel number\n");
	printf("\t-serial <num> : AUD file serial number\n");
	printf("\t-tracks <num> : AUD file number of tracks\n");
	printf("\t-start <str>  : AUD file start time\n");
	printf("\t-end <str>    : AUD file end time\n");
	printf("\n");
}

file_ext_t get_file_ext(char* file_name) {
	file_ext_t file_ext = EXT_UNK;
	if (strlen(file_name) > 3) {
		char* ext = &file_name[strlen(file_name) - 4];
		if (_stricmp(ext, ".AUD") == 0) {
			file_ext = EXT_AUD;
		}
		if (_stricmp(ext, ".AUE") == 0) {
			file_ext = EXT_AUE;
		}
		if (_stricmp(ext, ".WAV") == 0) {
			file_ext = EXT_WAV;
		}
	}
	return file_ext;
}

bool is_interrupted = false;

void signal_ctrl_c (int sig) {
	is_interrupted = true;
}

FILE* inpFile = NULL;
FILE* outFile = NULL;
uint16_t session_key = 0;
double gain = 1.0;

int ch_map[AUE_CHANNELS] = {0, 3, 2, 4, 1};

uint16_t aptxData[AUE_ENC_UNIT_SIZE];
uint16_t aptxChData[AUE_CHANNELS][AUE_ENC_UNIT_SIZE / AUE_CHANNELS];
int16_t pcmData[AUE_ENC_UNIT_SIZE * 2 * 6 / 5];
int16_t pcmChData[AUE_CHANNELS][AUE_ENC_UNIT_SIZE / AUE_CHANNELS * 2];
int16_t pcmChLs[AUE_ENC_UNIT_SIZE / AUE_CHANNELS * 2];
int16_t pcmChRs[AUE_ENC_UNIT_SIZE / AUE_CHANNELS * 2];
int16_t pcmChLFE[AUE_ENC_UNIT_SIZE / AUE_CHANNELS * 2];

void close_files() {
	if (inpFile) {
		fclose(inpFile);
		inpFile = NULL;
	}
	if (outFile) {
		fclose(outFile);
		outFile = NULL;
	}
}

int main(int argc, char* argv[]) {
	copyright();
	int i = 1;
	char* szInpFile = NULL;
	char* szOutFile = NULL;
	char* szKey = NULL;
	char* szVol = NULL;
	char* szTitle = NULL;
	char* szReel = NULL;
	char* szSerial = NULL;
	char* szTracks = NULL;
	char* szStart = NULL;
	char* szEnd = NULL;

	bool do_decrypt = false;
	bool do_decode = false;
	bool do_encode = false;
	bool do_lfe = false;
	while (i < argc) {
		if (!strcmp(argv[i], "-i"     )) szInpFile = argv[++i];
		if (!strcmp(argv[i], "-o"     )) szOutFile = argv[++i];
		if (!strcmp(argv[i], "-key"   )) szKey     = argv[++i];
		if (!strcmp(argv[i], "-lfe"   )) szVol     = argv[++i];
		if (!strcmp(argv[i], "-title" )) szTitle   = argv[++i];
		if (!strcmp(argv[i], "-reel"  )) szReel    = argv[++i];
		if (!strcmp(argv[i], "-serial")) szSerial  = argv[++i];
		if (!strcmp(argv[i], "-tracks")) szTracks  = argv[++i];
		if (!strcmp(argv[i], "-start" )) szStart   = argv[++i];
		if (!strcmp(argv[i], "-end"   )) szEnd     = argv[++i];
		i++;
	}
	if (!szInpFile || !szOutFile) {
		usage();
		return 1;
	}
	inpFile = fopen(szInpFile, "rb");
	if (!inpFile) {
		printf("Error: Cannot open %s file\n", szInpFile);
		close_files();
		return -1;
	}
	outFile = fopen(szOutFile, "wb");
	if (!outFile) {
		printf("Error: Cannot open %s file\n", szOutFile);
		close_files();
		return -2;
	}
	file_ext_t inp_ext = get_file_ext(szInpFile);
	file_ext_t out_ext = get_file_ext(szOutFile);
	switch (inp_ext) {
	case EXT_AUD:
		switch (out_ext) {
		case EXT_WAV:
			do_decrypt = false;
			do_decode = true;
			do_encode = false;
			break;
		}
		break;
	case EXT_AUE:
		switch (out_ext) {
		case EXT_AUD:
			do_decrypt = true;
			do_decode = false;
			do_encode = false;
			break;
		case EXT_WAV:
			do_decrypt = true;
			do_decode = true;
			do_encode = false;
			break;
		}
		break;
	case EXT_WAV:
		switch (out_ext) {
		case EXT_AUD:
			do_decrypt = false;
			do_decode = false;
			do_encode = true;
			break;
		}
		break;
	}
	if (szKey) {
		int skey;
		sscanf(szKey, "%i", &skey);
		session_key = (uint16_t)skey;
	}
	if (szVol) {
		double dB = 0.0;
		sscanf(szVol, "%lg", &dB);
		gain = pow(10.0, dB / 20.0);
		do_lfe = true;
	}
	if (!do_decrypt && !do_decode && !do_encode) {
		printf("Error: Unsupported conversion from %s to %s\n", szInpFile, szOutFile);
		close_files();
		return -3;
	}
	if (!libaptx_load()) {
		printf("Error: Cannot load aptx100.dll\n");
		close_files();
		return -4;
	}
	audform_t* aue = NULL;
	audform_t* aud = NULL;
	waveform_t* wav = NULL;

	if (do_decrypt || do_decode) {
		aue = new audform_t(inpFile);
		aue->init(false, do_decrypt);
		char start[12];
		char end[12];
		aue->bcd_to_text(aue->aud_hdr.start, start);
		aue->bcd_to_text(aue->aud_hdr.end, end);
		printf("title:  %s\n", aue->aud_hdr.title); 
		printf("reel:   %u\n", aue->aud_hdr.reel);
		printf("serial: %u\n", aue->aud_hdr.serial);
		printf("tracks: %u\n", aue->aud_hdr.tracks);
		printf("start:  %s\n", start);
		printf("end:    %s\n", end);
		if (do_decrypt) {
			uint16_t aue_file_key = (aue->aue_crypto_data[2] << 8) | aue->aue_crypto_data[1];
			for (uint32_t candidate_key = 0; candidate_key < 65536; candidate_key++) {
				uint16_t decrypted_key = xor_decode_word((uint16_t)candidate_key);
				if (aue->aue_crypto_data[0] == 1 && decrypted_key == aue_file_key) {
					printf("AUEkey: 0x%04x\n", (uint16_t)candidate_key);
					if (szKey == NULL) {
						session_key = (uint16_t)candidate_key;
					}
				}
			}
			aue->set_offset(0);
		}
		if (!do_decode) {
			aud = new audform_t(outFile);
			aud->init(true, false);
			aud->write((uint8_t*)&aue->aud_hdr, sizeof(aue->aud_hdr));
		}
		else {
			wav = new waveform_t(outFile);
			uint32_t channel_mask;
			int channels;
			if (do_lfe) {
				channel_mask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
				channels = AUE_CHANNELS + 1;
				init_channels(gain);
			}
			else {
				channel_mask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
				channels = AUE_CHANNELS;
			}
			wav->init(channels, AUE_SAMPLERATE, 16, channel_mask, false);
		}
		if (!do_decode) {
			printf("\nDecrypting...\n");
			while (!is_interrupted) {
				int offset = aue->get_offset();
				if (offset >= aue->get_length()) {
					break;
				}
				int frame_bytes = AUE_ENC_UNIT_SIZE;
				if (offset > AUE_DATA_START - AUE_ENC_UNIT_SIZE && offset < AUE_DATA_START) {
					frame_bytes = AUE_DATA_START - offset;
				}
				memset(aptxData, 0, frame_bytes);
				int read_bytes = aue->read((uint8_t*)aptxData, frame_bytes);
				if (read_bytes <= 0) {
					break;
				}
				if (do_decrypt && offset == 0) {
					aptxData[0] = 0;
					aptxData[1] = 0;
				}
				if (do_decrypt && offset >= AUE_DATA_START) {
					xor_session_key = session_key;
					xor_decode_buffer((uint8_t*)aptxData, read_bytes, offset);
				}
				aud->write((uint8_t*)aptxData, read_bytes);
			}
		}
		else {
			printf("\nDecoding...\n");
			aptxCtx_t* aptxCtxs[AUE_CHANNELS];
			for (int ch = 0; ch < AUE_CHANNELS; ch++) {
				aptxCtxs[ch] = fn_aptxCreate(APTX_MODE_DECODE | 4, -1, 0, 1);
				//aptxCtxs[ch]->mode = aptxCtxs[ch]->mode & 0x7fffffff; // don't use MMX
			}
			while (!is_interrupted) {
				int rc;
				int offset = aue->get_offset();
				if (offset >= aue->get_length()) {
					break;
				}
				int frame_bytes = AUE_ENC_UNIT_SIZE;
				if (offset > AUE_DATA_START - AUE_ENC_UNIT_SIZE && offset < AUE_DATA_START) {
					frame_bytes = AUE_DATA_START - offset;
				}
				int frame_samples = frame_bytes / AUE_CHANNELS * 2;
				memset(aptxData, 0, frame_bytes);
				int read_bytes = aue->read((uint8_t*)aptxData, frame_bytes);
				if (read_bytes <= 0) {
					break;
				}
				if (do_decrypt && offset == 0) {
					aptxData[0] = 0;
					aptxData[1] = 0;
				}
				if (do_decrypt && offset >= AUE_DATA_START) {
					xor_session_key = session_key;
					xor_decode_buffer((uint8_t*)aptxData, read_bytes, offset);
				}
				int read_samples = read_bytes / AUE_CHANNELS * 2;
				for (int ch = 0; ch < AUE_CHANNELS; ch++) {
					for (int i = 0; i < read_bytes / AUE_CHANNELS; i++) {
						aptxChData[ch][i] = aptxData[ch + i * AUE_CHANNELS];
					}
					rc = fn_aptxDec(aptxCtxs[ch], read_samples, pcmChData[ch], aptxChData[ch], 0);
					if (rc) {
						printf("Error: APT-x100 decoding error: %d\n", rc);
						break;
					}
				}
				if (do_lfe) {
					extract_channels(read_samples, pcmChData[1], pcmChData[3], pcmChLs, pcmChRs, pcmChLFE);
					for (int i = 0; i < read_samples; i++) {
						pcmData[0 + i * (AUE_CHANNELS + 1)] = pcmChData[0][i];
						pcmData[4 + i * (AUE_CHANNELS + 1)] = pcmChLs[i];
						pcmData[2 + i * (AUE_CHANNELS + 1)] = pcmChData[2][i];
						pcmData[5 + i * (AUE_CHANNELS + 1)] = pcmChRs[i];
						pcmData[1 + i * (AUE_CHANNELS + 1)] = pcmChData[4][i];
						pcmData[3 + i * (AUE_CHANNELS + 1)] = pcmChLFE[i];
					}
				}
				else {
					for (int ch = 0; ch < AUE_CHANNELS; ch++) {
						for (int i = 0; i < read_samples; i++) {
							pcmData[ch_map[ch] + i * AUE_CHANNELS] = pcmChData[ch][i];
						}
					}
				}
				rc = wav->write(pcmData, read_samples);
			}
			if (wav) {
				wav->sync();
			}
			for (int ch = 0; ch < AUE_CHANNELS; ch++) {
				if (aptxCtxs[ch]) {
					fn_aptxDelete(aptxCtxs[ch]);
				}
			}
		}
	}

	if (do_encode) {
		printf("Encoding...\n");
		wav = new waveform_t(inpFile);
		wav->init();
		if (!((wav->get_channels() == 5 || wav->get_channels() == 6) && wav->get_samplerate() == 44100)) {
			printf("Error: Unsupported WAVe file format.\n");
			close_files();
			return -5;
		}
		aud = new audform_t(outFile);
		aud->init(true, false);
		strncpy(aud->aud_hdr.title, szTitle ? szTitle : "Untitled", sizeof(audform_t::aud_hdr.title));
		int num;
		szReel ? sscanf(szReel, "%i", &num) : 1;
		aud->aud_hdr.reel = (uint16_t)num;
		szSerial ? sscanf(szSerial, "%i", &num) : 1234;
		aud->aud_hdr.serial = (uint16_t)num;
		szTracks ? sscanf(szTracks, "%i", &num) : 1;
		aud->aud_hdr.tracks = (uint16_t)num;
		char* szTime0 = "00:00:00.00";
		audform_t::text_to_bcd(szStart ? szStart : szTime0, aud->aud_hdr.start);
		audform_t::text_to_bcd(szEnd ? szEnd : szTime0, aud->aud_hdr.start);
		aud->write((uint8_t*)&aud->aud_hdr, sizeof(aud->aud_hdr));
		aud->flush();
		aptxCtx_t* aptxCtxs[AUE_CHANNELS];
		for (int ch = 0; ch < AUE_CHANNELS; ch++) {
			aptxCtxs[ch] = fn_aptxCreate(APTX_MODE_ENCODE | 4, -1, 0, 1);
			//aptxCtxs[ch]->mode = aptxCtxs[ch]->mode & 0x7fffffff; // don't use MMX
		}
		while (!is_interrupted) {
			int rc;
			int frame_samples = AUE_ENC_UNIT_SIZE / AUE_CHANNELS * 2;
			int read_samples = wav->read(pcmData, frame_samples);
			if (read_samples <= 0) {
				break;
			}
			if (wav->get_channels() == 5) {
				for (int ch = 0; ch < AUE_CHANNELS; ch++) {
					for (int i = 0; i < read_samples; i++) {
						pcmChData[ch][i] = pcmData[ch_map[ch] + i * AUE_CHANNELS];
					}
				}
			}
			if (wav->get_channels() == 6) {
				for (int i = 0; i < read_samples; i++) {
					pcmChData[0][i] = pcmData[0 + i * (AUE_CHANNELS + 1)];
					pcmChData[2][i] = pcmData[2 + i * (AUE_CHANNELS + 1)];
					pcmChData[4][i] = pcmData[1 + i * (AUE_CHANNELS + 1)];
					if (do_lfe) {
						int16_t LFE = pcmData[3 + i * (AUE_CHANNELS + 1)];
						pcmChData[1][i] = to_int16(pcmData[4 + i * (AUE_CHANNELS + 1)] + gain * LFE);
						pcmChData[3][i] = to_int16(pcmData[5 + i * (AUE_CHANNELS + 1)] + gain * LFE);
					}
				}
			}
			for (int ch = 0; ch < AUE_CHANNELS; ch++) {
				rc = fn_aptxEnc(aptxCtxs[ch], read_samples, pcmChData[ch], aptxChData[ch], 0);
				if (rc) {
					printf("Error: APT-x100 encoding error: %d\n", rc);
					break;
				}
				for (int i = 0; i < read_samples / 2; i++) {
					aptxData[ch + i * AUE_CHANNELS] = aptxChData[ch][i];
				}
			}
			rc = aud->write((uint8_t*)aptxData, read_samples / 2 * AUE_CHANNELS);
		}
		aud->sync();
		for (int ch = 0; ch < AUE_CHANNELS; ch++) {
			if (aptxCtxs[ch]) {
				fn_aptxDelete(aptxCtxs[ch]);
			}
		}
	}

	printf("Done!\n");
	close_files();
	libaptx_free();
}
