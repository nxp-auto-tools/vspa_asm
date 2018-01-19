#include <stdio.h>

#include "adl-asm-common.h"

extern "C" {
# include "as.h"
}

enum bfd_architecture vcpu_ppc_arch();
char *vcpu_ppc_target_format();
unsigned long vcpu_ppc_mach();

extern "C" void vcpu_md_apply_fix(fixS *fixP, valueT *valP, segT seg);
extern "C" void ippu_md_apply_fix(fixS *fixP, valueT *valP, segT seg);

extern "C" void vcpu_md_assemble(char *str, unsigned line_number);
extern "C" void ippu_md_assemble(char *str, unsigned line_number);

extern "C" int vcpu_md_parse_option(int c, char *arg);
extern "C" int ippu_md_parse_option(int c, char *arg);
int ppc_parse_opt(int c, char *arg);

extern "C" void vcpu_md_show_usage(FILE *stream ATTRIBUTE_UNUSED);
extern "C" void ippu_md_show_usage(FILE *stream ATTRIBUTE_UNUSED);
void ppc_show_usg(FILE *stream ATTRIBUTE_UNUSED);

void vcpu_md_begin();
void ippu_md_begin();

// The default core is VCPU.
int core = CORE_VCPU;
int init_core;

//  Do initializations.
static void do_initializations()
{
	// Add '>' and '<' characters as name beginners in order to accommodate
	// custom logical shift operators "<<<" and  ">>>".
	lex_type[(unsigned char) '<'] = LEX_BEGIN_NAME;
	lex_type[(unsigned char) '>'] = LEX_BEGIN_NAME;

	// Set precedence for "<<<".
	expr_set_rank(O_md1, 8);
	// Set precedence for ">>>".
	expr_set_rank(O_md2, 8);
}

void md_begin()
{
	do_initializations();

	init_core = CORE_IPPU;
	ippu_md_begin();
	init_core = CORE_VCPU;
	vcpu_md_begin();

	vspa_set_default_section();
}

int md_parse_option(int c, char *arg)
{
	bool vcpu_opt = vcpu_md_parse_option(c, arg);
	bool ippu_opt = ippu_md_parse_option(c, arg);


	if (vcpu_opt || ippu_opt)
	{
		return 1;
	}

	return ppc_parse_opt(c, arg);
}

void md_assemble(char *str, unsigned line_number)
{
	if (core == CORE_VCPU)
	{
		vcpu_md_assemble(str, line_number);
	}
	else
	{
		ippu_md_assemble(str, line_number);
	}
}

void md_show_usage(FILE *stream ATTRIBUTE_UNUSED)
{
	vcpu_md_show_usage(stream);
	ippu_md_show_usage(stream);
	ppc_show_usg(stream);
}

void md_apply_fix(fixS *fixP, valueT *valP, segT seg)
{
	if (core == CORE_VCPU)
	{
		vcpu_md_apply_fix(fixP, valP, seg);
	}
	else
	{
		ippu_md_apply_fix(fixP, valP, seg);
	}
}

enum bfd_architecture ppc_arch()
{
	return vcpu_ppc_arch();
}

char *ppc_target_format()
{
	return vcpu_ppc_target_format();
}

unsigned long ppc_mach()
{
	return vcpu_ppc_mach();
}
