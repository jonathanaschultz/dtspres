#pragma once

#include <windows.h>
#include "lib/aptx100.h"

typedef BOOL aptxAutoAux_t(aptxCtx_t* aptxCtx, BOOL p2);
typedef aptxCtx_t* aptxCreate_t(int aptxMode, int p2, int p3, int channels);
typedef void aptxDelete_t(aptxCtx_t* aptxCtx);

typedef void aptxDecInit_t(aptxCtx_t* aptxCtx, int channels);
typedef int  aptxDec_t(aptxCtx_t* aptxCtx, int samples, int16_t* pcmBuf, uint16_t* aptxBuf, int* channel_status);
typedef int  aptxDecode_t(aptxCtx_t* aptxCtx, int samples, int mode_bit_1, int16_t* pcmBuf, int mode_bit_2, uint16_t* aptxBuf);

typedef void aptxEncInit_t(aptxCtx_t* aptxCtx, int channels);
typedef int  aptxEnc_t(aptxCtx_t* aptxCtx, int samples, int16_t* pcmBuf, uint16_t* aptxBuf, int* channel_status);
typedef int  aptxEncode_t(aptxCtx_t* aptxCtx, int samples, int mode_bit_1, int16_t* pcmBuf, int mode_bit_2, uint16_t* aptxBuf);

extern aptxAutoAux_t* fn_aptxAutoAux;
extern aptxCreate_t*  fn_aptxCreate;
extern aptxDelete_t*  fn_aptxDelete;
extern aptxDecInit_t* fn_aptxDecInit;
extern aptxDec_t*     fn_aptxDec;
extern aptxDecode_t*  fn_aptxDecode;
extern aptxEncInit_t* fn_aptxEncInit;
extern aptxEnc_t*     fn_aptxEnc;
extern aptxEncode_t*  fn_aptxEncode;

bool libaptx_load();
bool libaptx_free();
