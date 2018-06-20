#ifndef _ADL_ASM_COMMON_H_
#define _ADL_ASM_COMMON_H_

#define CORE_VCPU 0
#define CORE_IPPU 1

extern int core;
extern int init_core;

void vspa_set_default_section();
extern "C" int adl_size_lookup(unsigned);

#endif
