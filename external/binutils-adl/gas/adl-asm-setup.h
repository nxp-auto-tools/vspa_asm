//
// Various setup defines.  This should be included by the processor
// configuration file (tc-<arch>.h).
//

#ifndef _ADL_ASM_SETUP_H_
#define _ADL_ASM_SETUP_H_

// We'll do the to-lower operation outself b/c we have the complexity of
// multiple instructions in a packet.
#define TC_CASE_SENSITIVE

// Allow the generation of more than one relocation for a single fix-up
#define RELOC_EXPANSION_POSSIBLE

// Allow the generation of maximum 32 relocations for a single fix-up
#define MAX_RELOC_EXPANSION 32

#ifdef LABELS_WITHOUT_COLONS
# undef LABELS_WITHOUT_COLONS
#endif
#define LABELS_WITHOUT_COLONS 1

// For bool type support.
#include <stdbool.h>

// Fixups get this additional item.
struct adl_fixup_data {
	// The operand needing the fixup.
	const struct adl_operand *operand;
	// Data about the entire instruction. Mutable for macro instruction.
	struct adl_opcode *opcode;
	// Operand values, if required to setup dependent operands (may be 0).
	expressionS *operand_values;
	// The PC, essentially.
	int line_number;
	// The related operand in the macro instruction.
	struct adl_operand_macro *operand_macro;
	// The fix-up corresponds to a macroinstruction.
	bool is_macro;
};

#define TC_FIX_TYPE struct adl_fixup_data

// Allow complex relocations.
#define OBJ_COMPLEX_RELC

// Additional symbol information.
struct adl_sy
{
	// The alignment of the symbol.
	int alignment;
	// The symbol is reference by address.
	char ref_by_addr;
	// The symbol is created from the dot symbol.
	char is_dot;
	// The symbol is used to detect BigFoot issues.
	char is_bf;
	// File in which the symbol was defined.
	char *decl_file;
	// Line at which the symbol was defined.
	int decl_line;
	// Stack effect of that symbol/function
	unsigned int stack_effect;
};

#define TC_SYMFIELD_TYPE struct adl_sy

// Called by the fixup function for initialization of user data.
#define TC_INIT_FIX_DATA(fixP) { memset(&fixP->tc_fix_data, 0, sizeof(struct adl_fixup_data)); }

#ifndef TC_ALIGN_LIMIT
# define TC_ALIGN_LIMIT (stdoutput->arch_info->bits_per_address - 1)
#endif

/* Set the endianness we are using.  Default to big endian.  */
#ifndef TARGET_BYTES_BIG_ENDIAN
# define TARGET_BYTES_BIG_ENDIAN 1
#endif

/* $ is used to refer to the current location.  */
/*#define DOLLAR_DOT*/
/* allows use of $ instead of "0x"*/

#define LITERAL_PREFIXDOLLAR_HEX

#define DIFF_EXPR_OK		/* foo-. gets turned into PC relative relocs */

/* We don't need to handle .word strangely.  */
#define WORKING_DOT_WORD

/* Question marks are permitted in symbol names.  */
#define LEX_QM 1

/* Square and curly brackets are permitted in symbol names.  */
#define LEX_BR 3

#define DWARF2_LINE_MIN_INSN_LENGTH 4
/* Use 32-bit address to represent a symbol address */
#define DWARF2_ADDR_SIZE(bfd) 4

#define MAX_LITTLENUMS 4
#define SHF_IPPU (1 << 29)

#define tc_comment_chars comment_chars
extern const char comment_chars[];

#define tc_symbol_chars adl_symbol_chars
extern const char adl_symbol_chars[];

#define tc_parallel_separator_chars adl_parallel_separator_chars
extern const char adl_parallel_separator_chars[];

extern void md_cleanup PARAMS((void));

/* call md_pcrel_from_section, not md_pcrel_from */
#define MD_PCREL_FROM_SECTION(FIX, SEC) md_pcrel_from_section(FIX, SEC)
extern long md_pcrel_from_section(struct fix *, segT);

#define md_operand(x)

#define md_operator vspa_operator
extern operatorT vspa_operator(char *, int, char *);

/* Values passed to md_apply_fix don't include symbol values.  */
#define MD_APPLY_SYM_VALUE(FIX) 0

#define DWARF2_FORMAT(SEC) dwarf2_format_32bit

#define TC_CONS_FIX_NEW adl_cons_fix_new
extern void adl_cons_fix_new(fragS *, int, unsigned int, expressionS *);

#define tc_fix_adjustable adl_fix_adjustable
extern bfd_boolean adl_fix_adjustable(struct fix *);

#define md_pseudo_op adl_pseudo_op
extern void adl_pseudo_op();

#define tc_frob_file adl_frob_file
extern void adl_frob_file();

#define tc_frob_file_after_relocs vspa_frob_file_after_relocs
extern void vspa_frob_file_after_relocs();

#define tc_adjust_symtab adl_adjust_symtab
extern void adl_adjust_symtab();

#define md_post_relax_hook adl_check_debug_info()
extern void adl_check_debug_info();

extern void adl_final_write_processing(bfd *abfd, bfd_boolean linker);

extern bfd_boolean adl_section_processing(bfd *abfd, Elf_Internal_Shdr *shdr);

#define md_do_align(n,f,l,m,j)\
	adl_do_align (n,f,l,m);\
	goto j;

void adl_do_align(int n, char *fill, int len, int max);

void add_debug_location_pair(symbolS *var, symbolS *loc);

bfd_boolean adl_elf_make_object(bfd *abfd);

// Prevent the evaluation of expressions involving symbols when the value
// of the symbols is known.
#define md_allow_local_subtract(resultP, right, rightseg) 0

// Prevent the relocations for expressions involving subtraction of symbols to
// be computed.
#define TC_FORCE_RELOCATION_SUB_SAME(FIX, SEG)  1
#define TC_FORCE_RELOCATION_SUB_LOCAL(FIX, SEG) 1
#define TC_FORCE_RELOCATION_SUB_ABS(FIX, SEG) 1

// Validate a fixup which involves the subtraction of two symbols
#define TC_VALIDATE_FIX_SUB(FIX, SEG) 1

#define md_optimize_expr adl_optimize_expression
bfd_boolean adl_optimize_expression(expressionS *, operatorT, expressionS *);

#define tc_new_dot_label adl_new_dot_label
void adl_new_dot_label(symbolS *symbolP);

extern bfd_boolean adl_backend_section_from_bfd_section(bfd *abfd,
	asection *sec,
	int *retval);

#define md_elf_section_change_hook adl_elf_section_change
extern void adl_elf_section_change();

#define md_flush_pending_output adl_flush_pending_output
extern void adl_flush_pending_output();

#define md_end adl_end
extern void adl_end();

#endif
