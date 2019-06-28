#include "libaptx.h"

static HMODULE aptxLib = NULL;

aptxAutoAux_t* fn_aptxAutoAux;
aptxCreate_t*  fn_aptxCreate;
aptxDelete_t*  fn_aptxDelete;
aptxDecInit_t* fn_aptxDecInit;
aptxDec_t*     fn_aptxDec;
aptxDecode_t*  fn_aptxDecode;
aptxEncInit_t* fn_aptxEncInit;
aptxEnc_t*     fn_aptxEnc;
aptxEncode_t*  fn_aptxEncode;

bool libaptx_load() {
	aptxLib = LoadLibrary("aptx100.dll");
	if (!aptxLib) {
		return false;
	}
	fn_aptxAutoAux = (aptxAutoAux_t*)GetProcAddress(aptxLib, "aptxAutoAux");
	fn_aptxCreate  = (aptxCreate_t*)GetProcAddress(aptxLib, "aptxCreate");
	fn_aptxDelete  = (aptxDelete_t*)GetProcAddress(aptxLib, "aptxDelete");
	fn_aptxDecInit = (aptxDecInit_t*)GetProcAddress(aptxLib, "aptxDecInit");
	fn_aptxDec     = (aptxDec_t*)GetProcAddress(aptxLib, "aptxDec");
	fn_aptxDecode  = (aptxDecode_t*)GetProcAddress(aptxLib, "aptxDecode");
	fn_aptxEncInit = (aptxEncInit_t*)GetProcAddress(aptxLib, "aptxEncInit");
	fn_aptxEnc     = (aptxEnc_t*)GetProcAddress(aptxLib, "aptxEnc");
	fn_aptxEncode  = (aptxEncode_t*)GetProcAddress(aptxLib, "aptxEncode");
	return true;
}

bool libaptx_free() {
	if (aptxLib) {
		FreeLibrary(aptxLib);
		aptxLib = NULL;
		return true;
	}
	return false;
}
