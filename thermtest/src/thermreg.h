// thermreg.h

#ifndef _THERMREG_H
#define _THERMREG_H

// regulator errors
typedef enum
{
	thermreg_error_OK = 0,         // no error
	thermreg_error_SENSOR_SHC = 1, // sensor short circuit
	thermreg_error_SENSOR_OUT = 2, // sensor out
	thermreg_error_MINTEMP = 3,    // temperature <Tmin
	thermreg_error_MAXTEMP = 4,    // temperature >Tmax
	thermreg_error_PDNEGLIM = 5,   // power difference negative limit
	thermreg_error_PDPOSLIM = 6,   // power difference positive limit
} thermreg_error_t;

// regulator structure
typedef struct
{
	// regulation
	float dt;      // regulation period [s]
	float Pmin;    // minimum output power [W] (this member should be 0 in most cases)
	float Pmax;    // maximum output power [W]
	float kP;      // proportional constant
	float kI;      // integration constant
	float Tc;      // current temperature [K]
	float Tt;      // target temperature [K]
	float* ebuff;  // error buffer
	float ebufs;   // sum of error buffer
	int ebufi;     // index in error buffer
	int ebufc;     // count of samples in error buffer
	int ebufl;     // length of error buffer
	float P;       // current output power [W]
	// error checking
	float Ta;      // ambient temperature [K]
	float Tmin;    // temperature limit for mintemp error [K]
	float Tmax;    // temperature limit for maxtemp error [K]
	float Tss;     // temperature limit for sensor short circuit error [K]
	float Tso;     // temperature limit for sensor out error [K]
	float C;       // thermal capacity of entire system [J/K]
	float R;       // thermal resistance between entire system and ambient [K/W]
	float E;       // current thermal energy of entire system [J]
	float Pc;      // calculated output power [W]
	int ncycl;     // number of regulator cycles per one error check cycle
	int cycl;      // error check cycle counter
	float* pbuff;  // power difference buffer
	float pbufs;   // sum of power difference buffer
	int pbufi;     // index in power difference buffer
	int pbufc;     // count of samples in power difference buffer
	int pbufl;     // length of power difference buffer
	float Pdnl;    // negative power difference limit [W]
	float Pdpl;    // positive power difference limit [W]
	float Pda;     // average power difference [W]
	int error;     // regulator error (thermreg_error_t)
} thermreg_t;


// initialize all member variables and do thermreg_reset
// all function parameters corresponds to members in thermreg_t structure, Pmin is set to zero (can be changed after init)
// buffers ebuff and pbuff are allocated using malloc
extern void thermreg_init(thermreg_t* pr, float dt, float Pmax, float kP, float kI, int ebufl, float Tmin, float Tmax, float Tss, float Tso, float C, float R, int ncycl, int pbufl, float Pdnl, float Pdpl);

// free allocated buffers
extern void thermreg_done(thermreg_t* pr);

// set input temperature and do limit check, set "error" member variable in case when value Tc excess any limit (Tmin, Tmax, Tss, Tso)
// this function should be called before each call of thermreg_cycle with fresh temperature value
extern void thermreg_input(thermreg_t* pr, float Tc);

// do regulation cycle
// this function must be called periodically in intervals equal to 'dt' (delta-t)
// when "error" member variable is set, this function does nothing
extern void thermreg_cycle(thermreg_t* pr);

// check output power vs temperature change, set "error" member variable in case when power difference excess positive or negative limit (Pdn, Pdp)
// this function must be called after each call of thermreg_cycle
extern void thermreg_check(thermreg_t* pr);

// reset internal control variables, empty buffers, regulation and output power checking starts from beginning
// this function must be called to clear "error" member variable
extern void thermreg_reset(thermreg_t* pr);


extern const char* thermreg_error_text(thermreg_t* pr);


#endif // _THERMREG_H
