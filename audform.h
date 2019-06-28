#pragma once

#include <stdint.h>
#include <stdio.h>

#pragma pack(1)

typedef struct _aud_header_t {
	char     title[78];
	uint16_t reel;
	uint16_t serial;
	uint16_t tracks;
	uint8_t  start[4];
	uint8_t  end[4];
} aud_header_t;

#pragma pack()

class audform_t {
private:
	FILE* file;
	long data_offset;
	long data_length;
public:
	static void bcd_to_text(uint8_t bcd[4], char* text);
	static void text_to_bcd(char* text, uint8_t bcd[4]);
	aud_header_t aud_hdr;
	uint8_t aue_crypto_data[10];
	audform_t(FILE* file);
	long get_offset();
	void set_offset(long offset);
	long get_length();
	int  init(bool init_new, bool is_aue);
	void flush();
	int  sync();
	int  read(uint8_t* p_data, int size);
	int  write(uint8_t* p_data, int size);
};
