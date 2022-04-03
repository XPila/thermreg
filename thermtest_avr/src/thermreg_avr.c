// thermreg.c

#include "thermreg_avr.h"
#include <stdlib.h>


void thermreg_avr_input_float(thermreg_avr_t* pr, float Tc)
{
	pr->Tc = (int16_t)(Tc * THERMREG_AVR_TMUL + 0.5); // update current temperature variable
}

void thermreg_avr_cycle(thermreg_avr_t* pr)
{
	// calculate regulation
	int16_t err = pr->Tt - pr->Tc; // regulation error
	int32_t out = err * pr->kP; // calculate output power (proportional part)
	// put error value into ring buffer and calculate sum of all values in buffer (ebufs)
	if (pr->ebufc < pr->ebufl) // error buffer is not full?
		pr->ebufc++;  // increment count
	else
		pr->ebufs -= pr->ebuff[pr->ebufi]; // subtract old value from error buffer sum
	pr->ebuff[pr->ebufi] = err; // put new value into buffer
	pr->ebufs += err; // add new value to error buffer sum
	pr->ebufi = (pr->ebufi + 1) % pr->ebufl; // increment index
	int32_t out_i = pr->ebufs * -pr->kIneg; // calculate output power (integration part)
	if (out_i >= 0) // is positive?
		out_i >>= pr->shre; // do right shift
	else
		out_i = ~(~out_i >> pr->shre); // complement - right shift - complement
	out += out_i; // add to output power
	if (out < 0) out = 0; // limit negative output power
	out >>= pr->shro; // do right shift
	if (out > 255) out = 255; // limit maximum output power
	// set output power only in case of no error
	if (pr->error == thermreg_avr_error_OK)
		pr->P = out; // set output power
	else
		pr->P = 0; // set output power to zero
}

void thermreg_avr_reset(thermreg_avr_t* pr)
{
	pr->Tt = 0;       // target temperature [C] * THERMREG_AVR_TMUL
	// reset error buffer
	pr->ebufs = 0;    // sum of error buffer
	pr->ebufi = 0;    // index in error buffer
	pr->ebufc = 0;    // count of samples in error buffer
	// set output power to zero
	pr->P = 0;        // current output power [0-255]
	// reset error
	pr->error = thermreg_avr_error_OK;
}
