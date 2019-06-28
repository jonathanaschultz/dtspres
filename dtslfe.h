#include <stdint.h>

int16_t to_int16(double value);
void init_channels(double gain);
void extract_channels(int size, int16_t* li, int16_t* ri, int16_t* ls, int16_t* rs, int16_t* lfe);
