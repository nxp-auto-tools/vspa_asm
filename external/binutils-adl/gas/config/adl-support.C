//
// Support code for the ADL assembler.
//

#ifndef _MSC_VER
extern "C" {
# include "as.h"
}
#endif

#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <string>

extern "C" {
# include "as.h"
# include "elf/ppc.h"
# include "safe-ctype.h"
#include "dw2gencfi.h"
}

#include "adl-asm-impl.h"

using namespace std;
using namespace adl;

/* Whether to target elf64 or elf32.  For legacy purposes, I'm going to keep it
   at 32 for now. */
static unsigned int adl_obj64 = 0;

// Default option parsing.  Calls acb_parse_option as an optional
// extension-library hook.  Also calls acb_march_parse_option for optional
// micro-architecture option processing (-m option stuff).
int adl_parse_option (int c,char *arg)
{
  switch (c) {
  case OPTION_IGNORE_LOWBITS:
    adl_set_check_low_bits(false);
    break;
  case 'a':
    if (strcmp (arg, "64") == 0) {
      adl_obj64 = 1;
    }
    else if (strcmp (arg, "32") == 0) {
      adl_obj64 = 0;
    } else {
      return 0;
    }
    break;
  case 'u':
    /* -u means that any undefined symbols should be treated as
       external, which is the default for gas anyhow.  */
    break;
  case 'm':
    if (strcmp (arg, "little") == 0 || strcmp (arg, "little-endian") == 0) {
      target_big_endian = 0;
    }
    else if (strcmp (arg, "big") == 0 || strcmp (arg, "big-endian") == 0) {
      target_big_endian = 1;
    }
    else if (strcmp (arg, "no-rules") == 0) {
      adl_set_check_asm_rules(0);
    }
    else if (acb_march_parse_option(arg)) {
      // Return an error if the optional hook returns an error.
      return 1;
    }
    break;
  case 'l':
    /* Solaris as takes -le (presumably for little endian).  For completeness
       sake, recognize -be also.  */
    if (strcmp (arg, "e") == 0)
      {
        target_big_endian = 0;
      }
    else
      return 0;
    break;
  case 'b':
    if (strcmp (arg, "e") == 0)
      {
        target_big_endian = 1;
      }
    else
      return 0;
    break;

  default:
    return acb_parse_option(c,arg);
  }

  return 1;
}

// Do any processing immediately after handling the options.
void adl_after_parse_args ()
{
  acb_after_parse_args();
}

extern "C" const bfd_arch_info_type bfd_adl_arch;

// We grab the machine from the architecture definition we have in the created
// disassembler.
unsigned long default_adl_mach()
{
  return bfd_adl_arch.mach;
}

extern char *default_adl_target_format ()
{
  extern const char *adl_default_targ64;
  extern const char *adl_default_targ64le;

  extern const char *adl_default_targ32;
  extern const char *adl_default_targ32le;  
  
  return (char*)(target_big_endian
	  ? (adl_obj64 ? adl_default_targ64   : adl_default_targ32)
	  : (adl_obj64 ? adl_default_targ64le : adl_default_targ32le));
}

void acb_show_usage(FILE *);

extern string adl_asm_version;

void adl_show_usage (FILE *stream ATTRIBUTE_UNUSED)
{
  fprintf (stream, "\
--ignore-low-bits		Ignore low bits in operands even if they should be zero.\n\
-a32			Generate ELF32\n\
-a64			Generate ELF64\n\
-mno-rules		Turn off assembly rule checking\n\
-mlittle, -mlittle-endian, -le\n\
			Generate code for a little endian machine\n\
-mbig, -mbig-endian, -be\n\
			Generate code for a big endian machine\n");

  // Add on any extension library option help.
  acb_show_usage(stream);
  if (!adl_asm_version.empty()) {
    fprintf(stream,"\nADL generated assembler version %s\n",adl_asm_version.c_str());
  }
}


// Just call to an externally supplied handler.
long adl_relax_frag (segT segment, fragS *fragP, long stretch)
{
  return acb_relax_frag(segment,fragP,stretch);
}

void adl_cfi_frame_initial_instructions (void)
{
  cfi_add_CFA_def_cfa (1, 0);
}
