#include "version.h"
#define BFD_VERSION_DATE 20110627
#define BFD_VERSION 221010000
#ifdef _VSPA2_
#define BFD_VERSION_STRING  "(FSL VSPA2 ADL " VSPA_ADL_VERSION ") " "2.21.1"
#else
#define BFD_VERSION_STRING  "(FSL VSPA ADL " VSPA_ADL_VERSION ") " "2.21.1"
#endif
#define REPORT_BUGS_TO "<http://www.sourceware.org/bugzilla/>"
