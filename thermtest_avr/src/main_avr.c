
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
int16_t ebuff[22];

#define _0C 273.15F


#define SIM_DT  0.01
#define SIM_MUL 4


int main(int argc, char**argv)
{
	reg.kP = 130;         // proportional constant
	reg.kIneg = 88;       // integration constant
	reg.Tc = 20 * THERMREG_AVR_TMUL;     // current temperature [C] * THERMREG_AVR_TMUL
//	reg.Tt = 0;           // target temperature [C]
	reg.ebuff = ebuff;    // error buffer
//	reg.ebufs = 0;        // sum of error buffer
//	reg.ebufi = 0;        // index in error buffer
//	reg.ebufc = 0;        // count of samples in error buffer
	reg.ebufl = sizeof(ebuff)/sizeof(ebuff[0]);  // length of error buffer
//	reg.P = 0;            // current output power (0-255 = 0-100% of maximum power)
	reg.shre = 2;         // right shift of err
	reg.sh_i = 2;         // right shift of ebufs * kI
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
	float dt = SIM_DT; // delta t
	float temp = 250;
	reg.Tt = (int16_t)(temp * 16);
	int c = SIM_MUL;
	while (t < 500)
	{
		sim_nozzle_cycle(&sim, dt);
		if (++c >= SIM_MUL)
		{
			thermreg_avr_input_float(&reg, sim.Ts - _0C);
			thermreg_avr_cycle(&reg);
			c = 0;
		}
		sim.P = reg.P * 38.0F / 255;
		// t Tc P
//		printf("%.2f\t%.2f\t%.2f\n", (double)t, (double)sim.Ts - _0C, (double)reg.P * 38 / 255);
		//float Pp = (reg.Tt - reg.Tc) * reg.kP / (1 << reg.shro);
		float Pp = reg.out_p;
		Pp = 38.0F * Pp / 255;
		//float Pi = -reg.kIneg * reg.ebufs / ((1 << reg.shre) * (1 << reg.shro));
		//float Pi = -reg.kIneg * reg.ebufs / ((1 << reg.shro));
		//float Pi = -reg.kIneg * reg.ebufs;
		float Pi = reg.out_i;
		//if (reg.sh_i < 0)
		//	Pi = Pi * (1 << -reg.sh_i) / (1 << reg.shro);
		//else if (reg.sh_i > 0)
		//	Pi = Pi / (1 << reg.sh_i) * (1 << reg.shro);
		Pi = 38.0F * Pi / 255;
		// t Tc P Pp Pi
		printf("%.2f\t%.2f\t%.2f\t%.0f\t%.0f\n", (double)t, (double)sim.Ts - _0C, (double)reg.P * 38 / 255, Pp, Pi);
		t += dt;
	}
}

