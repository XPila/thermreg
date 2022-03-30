
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <conio.h>
#include "thermreg.h"
#include "sim_nozzle.h"




unsigned long time_ms(void);

void test0(void); // 0 - target temperature = 250C, OK
void test1(void); // 1 - target temperature = 250C, utrzene topeni v dobe nahrivani (30s po startu)
void test2(void); // 2 - target temperature = 250C, utrzene topeni v dobe stabilni teploty (100s po startu)
void test3(void); // 3 - target temperature = 250C, porucha termistoru v dobe stabilni teploty (100s po startu), ukazuje 260C
void test4(void); // 4 - target temperature = 250C, porucha termistoru v dobe stabilni teploty (100s po startu), ukazuje 240C


sim_nozzle_t sim;

thermreg_t reg;


#define _0C 273.15F


int main(int argc, char**argv)
{
	thermreg_init(&reg,
		0.01,     // regulation period [s]
		38,       // maximum output power [W]
		44,       // proportional constant
		-40,      // integration constant
		90,       // length of error buffer
		5+_0C,    // temperature limit for mintemp error [K]
		295+_0C,  // temperature limit for maxtemp error [K]
		0+_0C,    // temperature limit for sensor short circuit error [K]
		300+_0C,  // temperature limit for sensor out error [K]
		9,        // thermal capacity of entire system [J/K]
		24.5,     // thermal resistance between entire system and ambient [K/W]
		10,       // number of regulator cycles per one error check cycle
		200,      // length of power difference buffer
		-15,      // negative power difference limit [W]
		15        // positive power difference limit [W]
	);

	sim_nozzle_init(&sim);

	test4();

	getch();
	return 0;
}



void test0(void)
{
	float t = 0; // time
	float dt = 0.01; // delta t
	float temp = _0C + 250;
	reg.Tt = temp;
	while (t < 500)
	{
		sim_nozzle_cycle(&sim, dt);
		thermreg_input(&reg, sim.Ts);
		thermreg_cycle(&reg);
		thermreg_check(&reg);
		sim.P = reg.P;
		printf("%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%d\n", t, reg.Tc - _0C, reg.P, reg.Pc, reg.P - reg.Pc, reg.Pda, reg.error?1:0);
		if (reg.error != 0)
		{
		}
		t += dt;
	}
}

void test1(void)
{
	float t = 0; // time
	float dt = 0.01; // delta t
	float temp = _0C + 250;
	reg.Tt = temp;
	while (t < 500)
	{
		sim_nozzle_cycle(&sim, dt);
		thermreg_input(&reg, sim.Ts);
		thermreg_cycle(&reg);
		thermreg_check(&reg);
		if (t >= 30)
			sim.P = 0;
		else
			sim.P = reg.P;
		printf("%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%d\n", t, reg.Tc - _0C, reg.P, reg.Pc, reg.P - reg.Pc, reg.Pda, reg.error?1:0);
		if (reg.error != 0)
		{
		}
		t += dt;
	}
}

void test2(void)
{
	float t = 0; // time
	float dt = 0.01; // delta t
	float temp = _0C + 250;
	reg.Tt = temp;
	while (t < 500)
	{
		sim_nozzle_cycle(&sim, dt);
		thermreg_input(&reg, sim.Ts);
		thermreg_cycle(&reg);
		thermreg_check(&reg);
		if (t >= 100)
			sim.P = 0;
		else
			sim.P = reg.P;
		printf("%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%d\n", t, reg.Tc - _0C, reg.P, reg.Pc, reg.P - reg.Pc, reg.Pda, reg.error?1:0);
		if (reg.error != 0)
		{
		}
		t += dt;
	}
}

void test3(void)
{
	float t = 0; // time
	float dt = 0.01; // delta t
	float temp = _0C + 250;
	reg.Tt = temp;
	while (t < 500)
	{
		sim_nozzle_cycle(&sim, dt);
		if (t >= 100)
			thermreg_input(&reg, _0C + 240);
		else
			thermreg_input(&reg, sim.Ts);
		thermreg_cycle(&reg);
		thermreg_check(&reg);
			sim.P = reg.P;
		printf("%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%d\n", t, reg.Tc - _0C, reg.P, reg.Pc, reg.P - reg.Pc, reg.Pda, reg.error?1:0);
		if (reg.error != 0)
		{
		}
		t += dt;
	}
}

void test4(void)
{
	float t = 0; // time
	float dt = 0.01; // delta t
	float temp = _0C + 250;
	reg.Tt = temp;
	while (t < 500)
	{
		sim_nozzle_cycle(&sim, dt);
		if (t >= 100)
			thermreg_input(&reg, _0C + 260);
		else
			thermreg_input(&reg, sim.Ts);
		thermreg_cycle(&reg);
		thermreg_check(&reg);
			sim.P = reg.P;
		printf("%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%d\n", t, reg.Tc - _0C, reg.P, reg.Pc, reg.P - reg.Pc, reg.Pda, reg.error?1:0);
		if (reg.error != 0)
		{
		}
		t += dt;
	}
}


unsigned long time_ms(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


