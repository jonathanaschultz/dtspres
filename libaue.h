#pragma once

#include <stdint.h>

#define AUE_FRAME_SIZE 735
#define AUE_DATA_START 13230000

#define AUE_CHANNELS 5
#define AUE_SAMPLERATE 44100
#define AUE_ENC_SAMPLE_SIZE (2 * AUE_CHANNELS)
#define AUE_ENC_BLOCK_SAMPLES 768
#define AUE_ENC_UNIT_SAMPLES (3 * AUE_ENC_BLOCK_SAMPLES)
#define AUE_ENC_BLOCK_SIZE (AUE_ENC_BLOCK_SAMPLES * AUE_ENC_SAMPLE_SIZE)
#define AUE_ENC_UNIT_SIZE (3 * AUE_ENC_BLOCK_SIZE)

extern uint8_t  xor_data_decryption_key_1;
extern uint8_t  xor_data_decryption_key_2;
extern uint16_t xor_session_key;

bool xor_decode_buffer(uint8_t* data, int size, int offset);
void xor_decode_buffer_key(int offset);
int  xor_decode_buffer_data(uint8_t* data, int offset, int size, int enc_start, int enc_end);

void xor_initialize_seed_byte_1_2_to_FF();
void xor_update_seed_byte_1_2(uint8_t* byte);
uint16_t xor_concatenate_seed_byte_1_2();
uint16_t xor_decode_word(uint16_t encoded_word);
