/*

Header file for PowerPC for use by an ADL-generated assembler.

*/

#include "adl-asm-setup.h"


#define md_start_line_hook()  (adl_start_line_hook ()); 
#define md_cleanup()  (adl_cleanup ()); 
#define tc_frob_label(S) (adl_set_curr_label (S));

extern bfd_boolean adl_start_line_hook PARAMS ((void));
extern void adl_cleanup PARAMS ((void));
extern void adl_set_curr_label PARAMS ((symbolS *));

/* The target BFD architecture.  */
#define TARGET_ARCH (ppc_arch ())
#define TARGET_MACH (ppc_mach ())
extern enum bfd_architecture ppc_arch PARAMS ((void));
extern unsigned long ppc_mach PARAMS ((void));

/* The target BFD format.  */
#define TARGET_FORMAT (ppc_target_format ())
extern char *ppc_target_format PARAMS ((void));

#define ADL_TARGET_ARCH bfd_arch_powerpc

/* Default target machine type, set by command-line option. */
extern unsigned long default_adl_mach PARAMS ((void));

/* Default target format type, set by command-line option. */
extern char *default_adl_target_format PARAMS ((void));

