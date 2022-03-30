// thermreg.c

#include "thermreg.h"
#include <stdlib.h>


void thermreg_init(thermreg_t* pr, float dt, float Pmax, float kP, float kI, int ebufl, float Tmin, float Tmax, float Tss, float Tso, float C, float R, int ncycl, int pbufl, float Pdnl, float Pdpl)
{
//	memset(pr, 0, sizeof(thermreg_t));
	pr->dt = dt;        // regulation period [s]
	pr->Pmin = 0;       // minimum output power [W]
	pr->Pmax = Pmax;    // maximum output power [W]
	pr->kP = kP;        // proportional constant
	pr->kI = kI;        // integration constant
	pr->ebufl = ebufl;  // length of error buffer
	pr->Ta = 293.15;    // ambient temperature [K]
	pr->Tmin = Tmin;    // temperature limit for mintemp error [K]
	pr->Tmax = Tmax;    // temperature limit for maxtemp error [K]
	pr->Tss = Tss;      // temperature limit for sensor short circuit error [K]
	pr->Tso = Tso;      // temperature limit for sensor out error [K]
	pr->C = C;          // thermal capacity of entire system [J/K]
	pr->R = R;          // thermal resistance between entire system and ambient []
	pr->ncycl = ncycl;  // number of regulator cycles per one error check cycle
	pr->pbufl = pbufl;  // length of power difference buffer
	pr->Pdnl = Pdnl;    // negative power difference limit [W]
	pr->Pdpl = Pdpl;    // positive power difference limit [W]
	thermreg_reset(pr);
	if (ebufl > 0)
		pr->ebuff = malloc(ebufl * sizeof(float)); // allocate error buffer
	if (pbufl > 0)
		pr->pbuff = malloc(pbufl * sizeof(float)); // allocate power difference buffer
}

void thermreg_done(thermreg_t* pr)
{
	if (pr->ebuff)
		free(pr->ebuff); // free error buffer
	if (pr->pbuff)
		free(pr->pbuff); // allocate power difference buffer
}

void thermreg_input(thermreg_t* pr, float Tc)
{
	pr->Tc = Tc; // update current temperature variable
	// check for sensor short circuit and sensor out error
	if (pr->Tss > pr->Tmax)
	{
		if (pr->Tc >= pr->Tss)
			pr->error = thermreg_error_SENSOR_SHC;
		if (pr->Tc <= pr->Tso)
			pr->error = thermreg_error_SENSOR_OUT;
	}
	else // (pr->Tss > pr->Tmax)
	{
		if (pr->Tc <= pr->Tss)
			pr->error = thermreg_error_SENSOR_SHC;
		if (pr->Tc >= pr->Tso)
			pr->error = thermreg_error_SENSOR_OUT;
	}
	// check for maxtemp and mintemp error
	if (pr->Tc > pr->Tmax)
		pr->error = thermreg_error_MAXTEMP;
	else if (pr->Tc < pr->Tmin)
		pr->error = thermreg_error_MINTEMP;
}

void thermreg_cycle(thermreg_t* pr)
{
	// calculate regulation
	float err = pr->Tt - pr->Tc; // regulation error
	// put error value into ring buffer and calculate sum of all values in buffer (ebufs)
	if (pr->ebufc < pr->ebufl) // error buffer is not full?
		pr->ebufc++;  // increment count
	else
		pr->ebufs -= pr->ebuff[pr->ebufi]; // subtract old value from error buffer sum
	pr->ebuff[pr->ebufi] = err; // put new value into buffer
	pr->ebufs += err; // add new value to error buffer sum
	pr->ebufi = (pr->ebufi + 1) % pr->ebufl; // increment index
	// calculate output power
	float out = err * pr->kP + pr->ebufs * pr->kI / pr->ebufl;
	// limit output power
	if (out < pr->Pmin) out = pr->Pmin;
	if (out > pr->Pmax) out = pr->Pmax;
	// set output power only in case of no error
	if (pr->error == thermreg_error_OK)
		pr->P = out; // set output power
	else
		pr->P = 0; // set output power to zero
}

void thermreg_check(thermreg_t* pr)
{
	if (++pr->cycl >= pr->ncycl)
	{
		if (pr->E == 0) // first pass - energy == 0 (cannot calculate energy increase)
			pr->E = pr->C * pr->Tc; // current energy [J]
		else
		{
			// calculate energy increase (dE [J]) from thermal capacity and current temperature
			float E = pr->C * pr->Tc; // current energy [J]
			float dE = E - pr->E; // energy increase
			pr->E = E; // update energy
			// calculate power from energy increase, temperature difference and thermal resistance
			pr->Pc = (dE / (pr->dt * pr->ncycl)) + ((pr->Tc - pr->Ta) / pr->R); // calculated power
			float Pd = pr->P - pr->Pc; // power difference between output power and calculated power
			// put average power difference value into ring buffer and calculate sum of all values in buffer (pbufs)
			if (pr->pbufc < pr->pbufl) // power difference buffer is not full?
				pr->pbufc++;  // increment count
			else
				pr->pbufs -= pr->pbuff[pr->pbufi]; // subtract old value from power difference buffer sum
			pr->pbuff[pr->pbufi] = Pd; // put new value into buffer
			pr->pbufs += Pd; // add new value to power difference buffer sum
			pr->pbufi = (pr->pbufi + 1) % pr->pbufl; // increment index
			pr->Pda = pr->pbufs / pr->pbufl; // average power difference [W]
			if (pr->Pda <= pr->Pdnl)
				pr->error = thermreg_error_PDNEGLIM;
			else if (pr->Pda >= pr->Pdpl)
				pr->error = thermreg_error_PDPOSLIM;
		}
		pr->cycl = 0; // reset counter
	}
}

void thermreg_reset(thermreg_t* pr)
{
	pr->Tt = 0;       // target temperature [K]
	// reset error buffer
	pr->ebufs = 0;    // sum of error buffer
	pr->ebufi = 0;    // index in error buffer
	pr->ebufc = 0;    // count of samples in error buffer
	// set output power to zero
	pr->P = 0;        // current output power [W]
	// set current thermal energy to zero
	pr->E = 0;        // current thermal energy of entire system [J]
	// reset power difference buffer
	pr->pbufs = 0;    // sum of power difference buffer
	pr->pbufi = 0;    // index in power difference buffer
	pr->pbufc = 0;    // count of samples in power difference buffer
	// reset error
	pr->error = thermreg_error_OK; // regulator error (thermreg_error_t)
}

const char* _error_text[] = {"SENSOR_SHC", "SENSOR_OUT", "MINTEMP", "MAXTEMP", "PDNEGLIM", "PDPOSLIM" };


const char* thermreg_error_text(thermreg_t* pr)
{
	if ((pr->error >= thermreg_error_OK) && (pr->error <= thermreg_error_PDPOSLIM))
		return _error_text[pr->error];
	else
		return "unknown";
}
