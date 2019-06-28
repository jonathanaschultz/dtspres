#include <math.h>
#include <stdlib.h>
#include "libaue.h"
#include "dtslfe.h"

#define IIR_SIZE 6
#define PCM_SIZE (AUE_ENC_UNIT_SIZE / AUE_CHANNELS * 2)

static const double B_LF[IIR_SIZE] = { 5.902287323061e-12, 2.951143661531e-11, 5.902287323061e-11, 5.902287323061e-11, 2.951143661531e-11, 5.902287323061e-12 };
static const double A_LF[IIR_SIZE] = { 1, -4.963115133507, 9.853139865557, -9.780721075822, 4.854483143098, -0.963786799137 };

static const double B_HF[IIR_SIZE] = { 0.981726438035, -4.908632190175, 9.81726438035, -9.81726438035, 4.908632190175, -0.981726438035 };
static const double A_HF[IIR_SIZE] = { 1, -4.963115133507, 9.853139865557, -9.780721075822, 4.854483143098, -0.963786799137 };

static double L_LF_Xn[IIR_SIZE];
static double R_LF_Xn[IIR_SIZE];
static double L_LF_Yn[IIR_SIZE];
static double R_LF_Yn[IIR_SIZE];
static double L_HF_Xn[IIR_SIZE];
static double R_HF_Xn[IIR_SIZE];
static double L_HF_Yn[IIR_SIZE];
static double R_HF_Yn[IIR_SIZE];
static double L_I[PCM_SIZE];
static double R_I[PCM_SIZE];
static double L_LF[PCM_SIZE];
static double R_LF[PCM_SIZE];
static double L_HF[PCM_SIZE];
static double R_HF[PCM_SIZE];

static double gain = 1.0;

double dither() {
	double r = (double)rand() / (double)RAND_MAX - 0.5;
	return r;
}

void filter_channel(int size, const double* A, const double* B, double* Xn, double* Yn, double* X, double* Y) {
	for (int i = 0; i < size; i++) {
		for (int j = 1; j < IIR_SIZE; j++) {
			Xn[j - 1] = Xn[j];
			Yn[j - 1] = Yn[j];
		}
		Xn[IIR_SIZE - 1] = X[i];
		double sum = B[0] * Xn[IIR_SIZE - 1];
		if (sum != 0) {
			int zz = 5;
		}
		for (int j = 1; j < IIR_SIZE; j++) {
			sum += B[j] * Xn[IIR_SIZE - 1 - j] - A[j] * Yn[IIR_SIZE - 1 - j];
		}
		Yn[IIR_SIZE - 1] = sum;
		Y[i] = Yn[IIR_SIZE - 1];
	}
}

int16_t to_int16(double value) {
	double n = value + dither();
	if (n > 32767.0) {
		n = 32767.0;
	}
	else if (n < -32767.0) {
		n = -32767.0;
	}
	else {
		n = n + 0.5;
	}
	return (int16_t)n;
}

void init_channels(double _gain) {
	gain = _gain;
	for (int i = 0; i < IIR_SIZE; i++) {
		L_LF_Xn[i] = 0.0;
		R_LF_Xn[i] = 0.0;
		L_LF_Yn[i] = 0.0;
		R_LF_Yn[i] = 0.0;
		L_HF_Xn[i] = 0.0;
		R_HF_Xn[i] = 0.0;
		L_HF_Yn[i] = 0.0;
		R_HF_Yn[i] = 0.0;
	}
}

void extract_channels(int size, int16_t* li, int16_t* ri, int16_t* ls, int16_t* rs, int16_t* lfe) {
	for (int i = 0; i < size; i++) {
		L_I[i] = (double)li[i];
		R_I[i] = (double)ri[i];
	}
	filter_channel(size, A_LF, B_LF, L_LF_Xn, L_LF_Yn, L_I, L_LF);
	filter_channel(size, A_LF, B_LF, R_LF_Xn, R_LF_Yn, R_I, R_LF);
	filter_channel(size, A_HF, B_HF, L_HF_Xn, L_HF_Yn, L_I, L_HF);
	filter_channel(size, A_HF, B_HF, R_HF_Xn, R_HF_Yn, R_I, R_HF);
	for (int i = 0; i < size; i++) {
		ls[i] = to_int16(L_HF[i]);
		rs[i] = to_int16(R_HF[i]);
		lfe[i] = to_int16((L_LF[i] + R_LF[i]) / 2.0 * gain);
	}
}
