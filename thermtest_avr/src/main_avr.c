
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <conio.h>
#include "thermreg_avr.h"
#include "sim_nozzle.h"



void test0(void); // 0 - target temperature = 250C, OK


sim_nozzle_t sim;

thermreg_avr_t reg;
int16_t ebuff[90];

#define _0C 273.15F



int main(int argc, char**argv)
{
	reg.kP = 148;         // proportional constant
	reg.kIneg = 96;         // integration constant
	reg.Tc = 20 * THERMREG_AVR_TMUL;     // current temperature [C] * THERMREG_AVR_TMUL
//	reg.Tt = 0;           // target temperature [C]
	reg.ebuff = ebuff;    // error buffer
//	reg.ebufs = 0;        // sum of error buffer
//	reg.ebufi = 0;        // index in error buffer
//	reg.ebufc = 0;        // count of samples in error buffer
	reg.ebufl = sizeof(ebuff)/sizeof(ebuff[0]);  // length of error buffer
//	reg.P = 0;            // current output power (0-255 = 0-100% of maximum power)
	reg.shre = 6;         // right shift of ebufs * kI
	reg.shro = 3;         // right shift of output
	thermreg_avr_reset(&reg);

	sim_nozzle_init(&sim);

	test0();

	getch();
	return 0;
}



void test0(void)
{
	float t = 0; // time
	float dt = 0.01; // delta t
	float temp = 250;
	reg.Tt = (int16_t)(temp * 16);
	while (t < 500)
	{
		sim_nozzle_cycle(&sim, dt);
		thermreg_avr_input_float(&reg, sim.Ts - _0C);
		thermreg_avr_cycle(&reg);
		sim.P = reg.P * 38.0F / 255;
		// t Tc P
//		printf("%.2f\t%.2f\t%.2f\n", (double)t, (double)sim.Ts - _0C, (double)reg.P * 38 / 255);
		float Pp = (reg.Tt - reg.Tc) * reg.kP / 8;
		Pp = 38.0F * Pp / 255;
		float Pi = -reg.kIneg * reg.ebufs / (64 * 8);
		Pi = 38.0F * Pi / 255;
		// t Tc P Pp Pi
		printf("%.2f\t%.2f\t%.2f\t%.0f\t%.0f\n", (double)t, (double)sim.Ts - _0C, (double)reg.P * 38 / 255, Pp, Pi);
		t += dt;
	}
}

