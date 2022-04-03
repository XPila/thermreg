// thermreg_avr.h

#ifndef _THERMREG_AVR_H
#define _THERMREG_AVR_H

#include <inttypes.h>

// temperature multiplier - 16 means resolution 1/16 [C or K]
#define THERMREG_AVR_TMUL 16


// regulator errors
typedef enum
{
	thermreg_avr_error_OK = 0,         // no error
	thermreg_avr_error_SENSOR_SHC = 1, // sensor short circuit
	thermreg_avr_error_SENSOR_OUT = 2, // sensor out
	thermreg_avr_error_MINTEMP = 3,    // temperature <Tmin
	thermreg_avr_error_MAXTEMP = 4,    // temperature >Tmax
	thermreg_avr_error_PDNEGLIM = 5,   // power difference negative limit
	thermreg_avr_error_PDPOSLIM = 6,   // power difference positive limit
} thermreg_avr_error_t;


// regulator structure - 18 bytes
typedef struct
{
	// regulation
	uint8_t kP;      // proportional constant
	uint8_t kIneg;   // negative integration constant
	int16_t Tc;      // current temperature [C] * THERMREG_AVR_TMUL
	int16_t Tt;      // target temperature [C] * THERMREG_AVR_TMUL
	int16_t* ebuff;  // error buffer
	int32_t ebufs;   // sum of error buffer
	uint8_t ebufi;   // index in error buffer
	uint8_t ebufc;   // count of samples in error buffer
	uint8_t ebufl;   // length of error buffer
	uint8_t P;       // current output power (0-255 = 0-100% of maximum power)
	int8_t error;    // regulator error (thermreg_avr_error_t)
	uint8_t shre:4;  // right shift of ebufs * kI
	uint8_t shro:4;  // right shift of output
} thermreg_avr_t;


// set input temperature as float [C]
// this function should be called before each call of thermreg_avr_cycle with fresh temperature value
extern void thermreg_avr_input_float(thermreg_avr_t* pr, float Tc);

// do regulation cycle
// this function must be called periodically in intervals equal to 'dt' (delta-t)
// when "error" member variable is set, this function does nothing
extern void thermreg_avr_cycle(thermreg_avr_t* pr);

// reset internal control variables, empty buffers, regulation starts from beginning
// this function must be called to clear "error" member variable
extern void thermreg_avr_reset(thermreg_avr_t* pr);


#endif // _THERMREG_AVR_H
