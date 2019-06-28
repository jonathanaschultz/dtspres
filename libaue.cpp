#include <stdio.h>
#include "libaue.h"

#define LSB16(w) (*((uint8_t*)&w + 0))
#define MSB16(w) (*((uint8_t*)&w + 1))

uint8_t xor_data_decryption_key_1;
uint8_t xor_data_decryption_key_2;

uint8_t xor_table_256_bytes_1[256] = { //NOTE: REMOVED FROM PUBLIC GITHUB REVISION, MAY BE COPYRIGHTED

};

uint8_t xor_table_256_bytes_2[256] = { //NOTE: REMOVED FROM PUBLIC GITHUB REVISION, MAY BE COPYRIGHTED

};

uint16_t xor_session_key;

bool xor_decode_buffer(uint8_t* data, int size, int offset) {
	bool decoded = false;
	if (AUE_DATA_START - size >= offset) {
		return false;
	}
	//printf("xor_decode_buffer(%08x, %d, %d)\n", data, size, offset);
	// loc_2DDE
	if (offset < AUE_DATA_START) {
		// l_adjust_frm_data_pointer
		data += AUE_DATA_START - offset;
		size -= AUE_DATA_START - offset;
		offset = AUE_DATA_START;
	}
	// l_begins_from_more_than_2_minutes
	offset -= AUE_DATA_START;
	// l_data_buffer_decode_cycle
	while (size) {
		int ofs = offset % (3 * 768 * 10);
		if (ofs <= (768 * 10)) {
			// l_do_block_1
			xor_decode_buffer_key(offset);
			xor_decode_buffer_data(data, offset, size, ofs, 768 * 10);
			decoded = true;
		}
		// l_do_block_2_or_3
		if (2304 * 10 - ofs > size) {
			// loc_2ED7
			size = 0;
		}
		else {
			// loc_2EE0
			offset += (3 * 768 * 10) - ofs;
			data += (3 * 768 * 10) - ofs;
			size -= (3 * 768 * 10) - ofs;
		}
	}
	return decoded;
}

void xor_decode_buffer_key(int offset) {
	uint16_t ki = offset / (AUE_ENC_UNIT_SIZE);
	xor_data_decryption_key_1 = ((ki + xor_table_256_bytes_1[(ki + LSB16(xor_session_key)) % 256]) % 256) % 53;
	xor_data_decryption_key_2 = (ki + xor_table_256_bytes_2[(ki + MSB16(xor_session_key)) % 256]) % 53;
}

int xor_decode_buffer_data(uint8_t* data, int offset, int size, int enc_start, int enc_end) {
	//printf("xor_decode_buffer_data(%08x, %d, %d, %d, %d)\n", data, offset, size, enc_start, enc_end);
	enc_end -= enc_start;
	if (enc_end < size) {
		size = enc_end;
	}
	if ((offset & 1) == 0) {
		data++;
		offset++;
		size--;
	}
	if (size == 0) {
		return 0;
	}
	// loc_2B02
	int sample_shift = offset % 10;
	int frame_counter = 0;
	int sample_index = offset / 10;
	int sample_start = enc_end; 
	switch (sample_index & 7) {
	// l_sample_index_eq_0_or_4
	case 0:
	case 4:
		sample_start = frame_counter;
		break;
	// l_sample_index_default
	default:
		sample_start = frame_counter;
		if (sample_shift != 1) {
			sample_start = 11 - sample_shift;
			sample_index++;
		}
		// loc_2B4C
		switch (sample_index & 7) {
		case 1:
		case 5:
			sample_start += 30;
			break;
		case 2:
		case 6:
			sample_start += 20;
			break;
		case 3:
		case 7:
			sample_start += 10;
			break;
		}
		break;
	}
	// l_sample_index_end_switch
	int data_offset = 0;
	if (size <= sample_start) {
		return 0;
	}
	// l_move_buffer_pointer_to_decode_position
	data += sample_start;
	offset += sample_start;
	size -= sample_start;
	if (size == 0) {
		return 0;
	}
	// l_decode_cycle
	int processed_samples = 0;
	while (size > 0) {
		uint8_t* curr;
		sample_index = offset / 10;
		int data_start = offset % 10;
		frame_counter = 2;
		if (size > 8) {
			// l_more_than_8_bytes_in_buffer_to_decode
			switch (sample_index & 7) {
			case 0:
				data_start--;
				if (data_start == 0) {
					if (size > 48) {
						// l_more_than_48_bytes
						curr = data;
						sample_index = size;
						if (sample_index > 48) {
							// l_more_than_48_samples
							int block_index = (sample_index + 31) / 80;
							// l_decode_with_key_1_key_2
							for (int i = 0; i < block_index; i++) {
								curr[ 0] += xor_data_decryption_key_1;
								curr[ 2] += xor_data_decryption_key_1;
								curr[ 4] += xor_data_decryption_key_1;
								curr[ 6] += xor_data_decryption_key_1;
								curr[ 8] += xor_data_decryption_key_1;
								curr[40] += xor_data_decryption_key_2;
								curr[42] += xor_data_decryption_key_2;
								curr[44] += xor_data_decryption_key_2;
								curr[46] += xor_data_decryption_key_2;
								curr[48] += xor_data_decryption_key_2;
								curr += 80;
								processed_samples += 80;
							}
						}
						// loc_2CC4
						curr = data;
						// loc_2D6A
					}
					else {
						// decode_with_key_1
						curr = data;
						curr[ 0] += xor_data_decryption_key_1;
						curr[ 2] += xor_data_decryption_key_1;
						curr[ 4] += xor_data_decryption_key_1;
						curr[ 6] += xor_data_decryption_key_1;
						curr[ 8] += xor_data_decryption_key_1;
						processed_samples = 40;
						// loc_2D6A
					}
				}
				else {
					// loc_2CFC
					curr = data;
					curr[ 0] += xor_data_decryption_key_1;
					processed_samples = frame_counter;
					// loc_2D6A
				}
				break;
			case 4:
				processed_samples = data_start;
				processed_samples--;
				if (processed_samples == 0) {
					curr = data;
					curr[ 0] += xor_data_decryption_key_2;
					curr[ 2] += xor_data_decryption_key_2;
					curr[ 4] += xor_data_decryption_key_2;
					curr[ 6] += xor_data_decryption_key_2;
					curr[ 8] += xor_data_decryption_key_2;
					processed_samples = 40;
					// loc_2D6A
				}
				else {
					curr = data;
					curr[ 0] += xor_data_decryption_key_2;
					processed_samples = frame_counter;
					// loc_2D6A
				}
				break;
			default:
				curr = data;
				processed_samples = frame_counter;
				// loc_2D6A
				break;
			}
			// loc_2D6A
			data_offset = 0;
			if (size < processed_samples) {
				return 0;
			}
		}
		else {
			// l_buffer_too_short
			switch (sample_index & 7) {
			case 0:
				curr = data;
				curr[ 2] += xor_data_decryption_key_1;
				break;
			case 4:
				curr = data;
				curr[ 2] += xor_data_decryption_key_2;
				break;
			default:
				curr = data;
				break;
			}
		}
		// l_advance_buffer_position
		curr += processed_samples;
		data += processed_samples;
		offset += processed_samples;
		size -= frame_counter;
	}
	return 0;
}

uint8_t xor_seed_byte_1;
uint8_t xor_seed_byte_2;

void xor_initialize_seed_byte_1_2_to_FF() {
	uint8_t byte = 0xff;
	xor_seed_byte_1 = byte;
	xor_seed_byte_2 = byte;
}

void xor_update_seed_byte_1_2(uint8_t* p_byte) {
	uint8_t byte1, byte2;
	byte1 = *p_byte;
	byte1 = byte1 ^ xor_seed_byte_1;
	byte2 = byte1;
	byte1 = xor_table_256_bytes_1[byte2];
	byte1 = byte1 ^ xor_seed_byte_2;
	xor_seed_byte_1 = byte1;
	byte1 = xor_table_256_bytes_2[byte2];
	xor_seed_byte_2 = byte1;
}

uint16_t xor_concatenate_seed_byte_1_2() {
	uint16_t concat_word;
	concat_word = (xor_seed_byte_2 << 8) | xor_seed_byte_1;
	return concat_word;
}

uint16_t xor_decode_word(uint16_t encoded_word) {
	uint8_t seed_byte;
	xor_initialize_seed_byte_1_2_to_FF();
	seed_byte = 0x54;
	xor_update_seed_byte_1_2(&seed_byte);
	seed_byte = (uint8_t)(encoded_word >> 8);
	xor_update_seed_byte_1_2(&seed_byte);
	seed_byte = (uint8_t)(encoded_word & 0x00ff);
	xor_update_seed_byte_1_2(&seed_byte);
	return xor_concatenate_seed_byte_1_2();
}
