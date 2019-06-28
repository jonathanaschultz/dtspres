#include <stdint.h>
#include <windows.h>

#pragma once

#define APTX_MODE_DECODE 0
#define APTX_MODE_ENCODE 1

// size = 0x0014 (20)
typedef struct _aptxQuantizationTable_t {
	/* 0x00 */ int encStates; // must be power of 2
	/* 0x04 */ int m_04;
	/* 0x08 */ int m_08;
	/* 0x0c */ int* table1;
	/* 0x10 */ int* table2;
} aptxQuantizationTable_t;

// size = 0x00e4 (228)
typedef struct _aptxQuantizer_t {
	/* 0x0000 */ int m_00[2];
	/* 0x0008 */ int16_t m_08[4];
	/* 0x0010 */ int m_10[0x1e];
	/* 0x0088 */ uint8_t m_88[0x4c];
	/* 0x00d4 */ int state[2];
	/* 0x00dc */ int m_dc[2];
} aptxQuantizer_t;

// size = 0x05d0 (1488)
typedef struct _aptxChannel_t {
	/* 0x0000 */ int qmf34idx;
	/* 0x0004 */ int qmf32idx;
	/* 0x0008 */ float qmf34A[34];
	/* 0x0090 */ float qmf34B[34];
	/* 0x0118 */ float qmf32A[32];
	/* 0x0198 */ float qmf32B[32];
	/* 0x0218 */ aptxQuantizer_t quantizer[4];
	/* 0x05a8 */ int mode; // can be 1 - don't process or 2
	/* 0x05ac */ int m_05ac;
	/* 0x05b0 */ int m_05b0;
	/* 0x05b4 */ int m_05b4;
	/* 0x05b8 */ int m_05b8;
	/* 0x05bc */ int m_05bc;
	/* 0x05c0 */ int m_05c0;
	/* 0x05c4 */ int m_05c4;
	/* 0x05c8 */ int m_05c8;
	/* 0x05cc */ int m_05cc;
} aptxChannel_t;

// size = 0x0bb8 (3000)
typedef struct _aptxCtx_t {
	/* 0x0000 */ int mode; // bit 32 - is MMX supported
	/* 0x0004 */ int p3;
	/* 0x0008 */ int m_08; // 0xfffffffd
	/* 0x000c */ int m_0c;
	/* 0x0010 */ int channels;
	/* 0x0014 */ uint8_t* aptx_data; // size = 0x114 (276)
	/* 0x0018 */ aptxChannel_t aptxChannel[2]; // size = 1488 for channels
} aptxCtx_t;

extern float FLT_COEF_1[68];
extern float FLT_COEF_2[68];
extern float DEC_FLT_0[32];
extern uint32_t dword_1000F4EC[32];
extern uint32_t dword_1000F56C[39];
extern uint32_t dword_1000F608[50];
extern uint32_t stub_dword_1000F608[0x8000];
extern uint32_t dword_1000F6D0[16];
extern uint32_t dword_1000F710[88];
extern uint32_t dword_1000F8F0[28];
extern uint32_t dword_1000F960[32];
extern uint32_t dword_1000F9E0[4];
extern double dbl_1000FA80;
extern double dbl_1000FA88;
extern double dbl_1000FA90;
extern double dbl_1000FA98;

void aptx_initialize(aptxCtx_t* aptxCtx, uint32_t aptxMode, int needDataBuf, int p4, int channels);
// if needDataBuf >= 0 allocate aptx_data
aptxCtx_t* aptxCreate(int aptxMode, int needDataBuf, int p3, int channels);
void aptxDelete(aptxCtx_t* aptxCtx);
void aptxDecInit(aptxCtx_t* aptxCtx, int channels);
int aptxDecode(aptxCtx_t* aptxCtx, int samples, int mode_bit_1, int16_t* pcmBuf, int mode_bit_2, uint16_t* aptxBuf);
int aptxDec(aptxCtx_t* aptxCtx, int samples, int16_t* pcmBuf, uint16_t* aptxBuf, uint32_t* channelStatus);

void dec_100013D1(double* data);
void dec_10001637(double* data);
void dec_100019F8(double* p1, int p2, int p3, int p4);
void aptxQMF34(double out[2], float* coef, float* data);
double aptxQMF32(float data1[32], float data2[32]);
int aptxDoubleToIntStd(double value);
int aptxDoubleToIntSym(double value);
int dec_100028F1(int state[2], aptxQuantizationTable_t* qtzTable, int encBits, int a4, char a5);
void dec_10002A1F(int* data, aptxQuantizer_t* aptxQuantizer, int p3);
void dec_10002C26(int value, int* data, aptxQuantizer_t* aptxQuantizer, int p4);
void mmx_aptxChannelDecode(aptxChannel_t* aptxChannel, int* data, uint16_t encWord, int skip1, int skip2);
void dec_10003691(int p1, int p2, int p3);
void std_aptxChannelDecode(aptxChannel_t* aptxChannel, int* data, uint16_t encWord, int skip1, int skip2);
void aptxQMF(aptxChannel_t* aptxChannel, int* data);
int aptxQuantizeBank(aptxQuantizer_t* aptxQuantizer, uint32_t encBits, int encSize, int p4, int p5, int p6);
uint32_t dec_10004026(aptxCtx_t* aptxCtx, int p2, uint16_t* aptxBuf);
void dec_aptx_data0_lt_0(aptxCtx_t* aptxCtx, uint16_t* aptxBuf);
void dec_aptx_data0_ge_0(aptxCtx_t* aptxCtx, uint16_t* aptxBuf);

extern void (*p_fn_dec_100013D1)(double* data);
extern void (*p_fn_dec_10001637)(double* data);
extern void (*p_fn_dec_100019F8)(double* p1, int p2, int p3, int p4);
extern void (*p_fn_quad_filter_100020D2)(float* p_out, float* p_coef, float* p_data);
extern float (*p_fn_dec_fp_conv)(float* coef, float* data);
extern int (*p_fn_conv_float_to_int_1)(float p1);
extern int (*p_fn_conv_float_to_int_2)(float p1);
extern void (*p_fn_dec_100028F1)(int p1, int p, int p3, int p4, int p5);
extern void (*p_fn_dec_10002A1F)(int p1, int p, int p3);
extern void (*p_fn_dec_10002C26)(int p1, int p2, int p3, int p4);
extern void (*p_fn_dec_10003629)(int p1, int p2, int p3, int p4, int p5);
extern void (*p_fn_dec_10003691)(int p1, int p2, int p3);
extern void (*p_fn_dec_100039D8)(int p1, int p2, int p3, int p4, int p5);
extern void (*p_fn_dec_10003AB3)(int p1, int p2);
extern void (*p_fn_dec_10003F7A)(int p1, int p2, int p3, int p4, int p5, int p6);
extern void (*p_fn_dec_10004026)(int p1, int p2, int p3);
extern void (*p_fn_dec_10004371)(int p1, int p2, int p3);
extern void (*p_fn_dec_10004772)(int p1, int p);

void stub_dec_100013D1(double* data);
void stub_dec_10001637(double* data);
void stub_dec_100019F8(double* p1, int p2, int p3, int p4);
void stub_quad_filter_100020D2(float* p_out, float* p_coef, float* p_data);
float stub_dec_fp_conv(float* coef, float* data);
int stub_conv_float_to_int_1(float p1);
int stub_conv_float_to_int_2(float p1);
void stub_dec_100028F1(int p1, int p, int p3, int p4, int p5);
void stub_dec_10002A1F(int p1, int p, int p3);
void stub_dec_10002C26(int p1, int p2, int p3, int p4);
void stub_dec_10003629(int p1, int p2, int p3, int p4, int p5);
void stub_dec_10003691(int p1, int p2, int p3);
void stub_dec_100039D8(int p1, int p2, int p3, int p4, int p5);
void stub_dec_10003AB3(int p1, int p2);
void stub_dec_10003F7A(int p1, int p2, int p3, int p4, int p5, int p6);
void stub_dec_10004026(int p1, int p2, int p3);
void stub_dec_10004371(int p1, int p2, int p3);
void stub_dec_10004772(int p1, int p2);

void assign_stub_addresses(HMODULE aptxLib);