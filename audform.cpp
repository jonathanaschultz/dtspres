#include <memory.h>
#include "audform.h"

#if !defined min
	#define min(a, b) ((a < b) ? a : b)
#endif

#if !defined max
	#define max(a, b) ((a > b) ? a : b)
#endif

audform_t::audform_t(FILE* file) {
	this->file = file;
	data_offset = -1;
	data_length = -1;
}

long audform_t::get_offset() {
	return ftell(file) - data_offset;
}

void audform_t::set_offset(long offset) {
	fseek(file, offset + data_offset, SEEK_SET);
}

long audform_t::get_length() {
	return data_length;
}

void audform_t::bcd_to_text(uint8_t bcd[4], char* text) {
	text[ 0] = ((bcd[3] >> 4) & 0x0f) + '0';
	text[ 1] = ((bcd[3] >> 0) & 0x0f) + '0';
	text[ 2] = ':';
	text[ 3] = ((bcd[2] >> 4) & 0x0f) + '0';
	text[ 4] = ((bcd[2] >> 0) & 0x0f) + '0';
	text[ 5] = ':';
	text[ 6] = ((bcd[1] >> 4) & 0x0f) + '0';
	text[ 7] = ((bcd[1] >> 0) & 0x0f) + '0';
	text[ 8] = '.';
	text[ 9] = ((bcd[0] >> 4) & 0x0f) + '0';
	text[10] = ((bcd[0] >> 0) & 0x0f) + '0';
	text[11] = '\0';
}

void audform_t::text_to_bcd(char* text, uint8_t bcd[4]) {
	int hh = 0, mi = 0, ss = 0, mm = 0;
	sscanf(text, "%2d:%2d:%2d.%2d", &hh, &mi, &ss, &mm);
	bcd[3] = ((hh / 10) << 4) || (hh % 10);
	bcd[2] = ((mi / 10) << 4) || (mi % 10);
	bcd[1] = ((ss / 10) << 4) || (ss % 10);
	bcd[0] = ((mm / 10) << 4) || (mm % 10);
}

int audform_t::init(bool init_new, bool is_aue) {
	if (fseek(file, 0, SEEK_SET))
		return -1;
	if (!init_new) {
		if (fread(&aud_hdr, sizeof(aud_hdr), 1, file) != 1)
			return -2;
		fpos_t pos;
		if (fgetpos(file, &pos))
			return -3;
		data_offset = (int)pos;
		if (is_aue) 
			if (fread(aue_crypto_data, sizeof(aue_crypto_data), 1, file) != 1)
				return -4;
		if (fseek(file, 0, SEEK_END))
			return -5;
		if (fgetpos(file, &pos))
			return -6;
		data_length = (int)pos - data_offset;
		if (fseek(file, data_offset, SEEK_SET))
			return -7;
	}
	else {
		memset(&aud_hdr, 0, sizeof(aud_hdr));
	}
	return 0;
}

void audform_t::flush() {
	fflush(file);
}

int audform_t::sync() {
	return 0;
}

int audform_t::read(uint8_t* p_data, int size) {
	return fread(p_data, 1, size, file);
}

int audform_t::write(uint8_t* p_data, int size) {
	return fwrite(p_data, 1, size, file);
}
