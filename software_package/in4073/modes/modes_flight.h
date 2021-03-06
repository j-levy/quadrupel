#define MAX_SPEED 800
#define MIN_SPEED 200


#define MIN(a,b) (a < b ? a : b)
#define MAX(a,b) (a > b ? a : b)

// this allows for better precision for controller tuning
#define P_SCALE (16)

// define easy-to-use macros for sensor - offset
#define __SAX    (sax - sax_offset)
#define __SAY    (say - say_offset)
#define __SAZ    (saz - saz_offset)
#define __SP     (sp - sp_offset)
#define __SQ     (sq - sq_offset)
#define __SR     (sr - sr_offset)
#define __PHI    (phi - phi_offset)
#define __THETA    (theta - theta_offset)
#define __PSI    (psi - psi_offset)
