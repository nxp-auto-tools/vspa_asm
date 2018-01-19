//
// Header file for support routines for ADL assemblers.
//

#ifndef _ADL_ASM_IMPL_H_
#define _ADL_ASM_IMPL_H_

#include <inttypes.h>

extern "C"
{
# include "as.h"
}

enum core_type_t
{
	core_type_sp,
	core_type_dp,
#if defined(_VSPA2_)
	core_type_spl,
	core_type_dpl,
#endif
};

extern enum core_type_t core_type;
extern size_t AU_NUM;
extern bool ignore_case;

#if defined(_VSPA2_)
extern bool vspa1_on_vspa2;
#define VSPA_MODE vspa1_on_vspa2
#endif

#ifdef __cplusplus
# define EXTERN_C extern "C"
# define EXT_C_BEGIN extern "C" {
#else
# define EXTERN_C
# define EXT_C_END }
#endif  

#include "ansidecl.h"

// Various flag definitions.
#define ADL_RELATIVE 1
#define ADL_ABSOLUTE 2
#define ADL_SIGNED   4
#define ADL_REGISTER 8
#define ADL_EXT_SIGNED 16
#define ADL_FROM_LOOP_INST 32
#define MAX_INSTR_WORDS 8
#define MAX_BLOCKS 10
#define INSTR_BUFFER_SIZE 10
#define MAX_INSTR_FIELDS 32
#define MAXINT32 2147483647ll
#define MININT32 -2147483648ll

#ifndef __WORDSIZE
# if defined(_WIN32)
#  define __WORDSIZE 32
# elif defined(_WIN64)
#  define __WORDSIZE 64
# endif
#endif

#if __WORDSIZE == 64
#define ptr_int_t int64_t
#define ptr_uint_t uint64_t
#else
#define ptr_int_t int32_t
#define ptr_uint_t uint32_t
#endif

// Define to true if C99-style array declarations are allowed (size can be set
// via variables at declaration time).
#ifdef _MSC_VER
# define DeclArray(type,name,size) type *name = (type*)alloca(sizeof(type)*(size))
#else
# define DeclArray(type,name,size) type name[size]
#endif

#if defined(_WIN32) || defined(_WIN64)
# define strcasecmp _stricmp
# define strncasecmp _strnicmp
#elif defined(__CYGWIN32__)
# define strcasecmp stricmp
# define strncasecmp strnicmp
#endif

#ifdef _VSPA2_
# define OPTION_VSPA1_ON_VSPA2 33333
# define OPTION_CORE_TYPE 33334
#else
# define OPTION_ENGR297095_ON 33333
# define OPTION_ENGR297095_OFF 33334
# define OPTION_CMPVSPA784_ON 33335
# define OPTION_CMPVSPA784_OFF 33336
# define OPTION_CORE_TYPE 33337
#endif

// The following structures cannot be in the adl namespace because we must
// support forward references within C headers.

// Structure of storing enumerated field data.
struct enum_field
{
  // The option string.  Null means last entry in the list.
  const char *name;

  // The option's value.  This is *not* aligned with the instruction word; it's
  // just the raw value.
  unsigned    value;
};

// Outer structure for enumerated fields:  Stores a sorted list of enums, plus the number of elements.
struct enum_fields
{
  enum_field *enums;
  unsigned    num;

  bool valid() const { return enums != 0; };
};


// Structure for describing instruction fields.
struct adl_field
{
  // Instruction field name.
  const char *name;

  // Global id, set via the front-end.
  int id;

  int iface_id;

  // Width of fields.
  int width;

  // The inserter function- returns word with the value inserted and 0s
  // everywhere else.
  void (*inserter)(unsigned* instr,bfd_uint64_t val);
  void (*clearer)(unsigned* instr);
  int default_value;
  // These are relocation related information
  // instr_width - width of the instruction the relocation was defined.
  //               the assembler may fix the relocation offset information in case
  //               the relocation is used in an instruction with a different width
  
  int reloc_type;
  int instr_width;
  int assembler;
  enum_fields *enums; 	// Field's enumeration - in use only for setting assembler fields

  // Sets the prefix portion of a field, if exists
  void (*pfx_setter)(bfd_uint64_t val,int index,int group);
};

// Structure describing prefix field
struct adl_prefix_field 
{
  int value;
  int default_value;
  int width;
  unsigned field_width;
  unsigned mask;
};

using modifier_t = bfd_uint64_t(*)(const expressionS *operands,
	unsigned __cia);

// Modifier function information.
struct modifier_info
{
	// Pointer to the modifier function.
	modifier_t modifier;
	// Number of the operand values used.
	size_t op_num;
	// List of indexes corresponding to the operands.
	const size_t *operands;
};

struct adl_prefix_fields
{
  struct adl_prefix_field **fields;
};

// Structure for describing an instruction's operand, which is a field
// with some per-instruction modifiers.
struct adl_operand {
  // Index of field in operands array.  Will be -1 for a pseudo field.
  int                     index;

  // Index of argument in list of arguments on an assembler line.
  int                     arg;

  // Various flags.
  int                     flags;

  // An optional left-shift value.
  int                     leftshift;

  // Starting bit position of field in instruction (bits from the lhs, or
  // low-memory).
  int                     bitpos;

  // Lower/upper bounds and low-bit mask.
  bfd_int64_t             lower;
  bfd_int64_t             upper;
  bfd_uint64_t            mask;

  // For shorthand instructions which use an expression to set a value,
  // this function encodes the value.
  bfd_uint64_t (*modifier)(const expressionS *operands, unsigned PC);

  // Pointer to the adl_field operand object.
  const struct adl_field *ptr;
  
  // For operands that have validity checks.
  bfd_uint64_t (*checker)(bfd_uint64_t,bool);

  // For operands that have input validity checks.
  bool  (*validator)(const expressionS *operands);

  // If prefix field: number of entry in pfx_fields array, -1 otherwise.
  int prefix_field_id;
  // If prefix field and field is of type 'X': number of 'X' entry in pfx_fields array, -1 otherwise.
  int prefix_field_type_id;
};

// Used to for assembler action code
struct adl_operand_val
{
  unsigned      val;
  symbolS      *symbol;
};

namespace adl {
  struct InstrInfo;
}

// Stores information about an instruction attribute's value.  We can handle
// integer or string values.  If 'str' is 0, then 'num' is valid, else 'str' is
// valid.  A value of 0 for 'attr' indicates the last item.
struct adl_instr_attr_val {
  uint64_t    attr;   // The attribute index.
  unsigned    num;    // Numerical value, if it has one.
  const char *str;    // String value, if relevant.
};

// Stores information about instruction attributes.  If values exist, stores
// either an integer or string value for that attribute.
struct adl_instr_attrs {
  uint64_t            attrs;  // Stores all attributes.
  adl_instr_attr_val *vals;   // If any values exist, stores them here.
};

// Structure for describing register information or other stuff requiring a name
// -> index mapping.
struct adl_name_pair
{
  // Register name.
  const char *name;

  // Index value.
  unsigned index;
};

// Structure for pairs of integers for various items.
struct adl_int_pair
{
  int key;
  int value;
};

struct adl_opcode;

// The structure which describes each instruction.
struct adl_opcode
{
  // The ADL name for the instruction (not what's necessarily used for assembly).
  const char *name;

  // Size in bytes.  Will be 0 for a shorthand.
  unsigned char size;
  
  // The width in bits.
  unsigned width;  
  
  // The maximum width in bits
  unsigned max_width;
  
  // Alignment mask.
  unsigned long addr_mask;

  // Opcode itself. It is an array of 32bit long words
  // FIXME : define MAX_INSTR_WORDS according to core.instr_width()
  unsigned opcode[MAX_INSTR_WORDS];

  // Information about enumerated fields.
  enum_fields **enums;

  // Regular expression used for parsing operands.
  const char *regex;

  // Pointer to the compiled regular expression.
  void *regcomp;

  // Number of operands for this instruction.
  int num_operands;

  // Number of operaand terms for this instruction, may be larger than
  // num_operands for instruction fields with syntax
  int num_exprs;

  // Number of prefix-field operands (ones which come before the instruction name).
  unsigned int num_pfx_operands;

  // Number of perm-field operands (where order doesn't matter).
  unsigned int num_prm_operands;

  // Array of operand information.
  adl_operand *operands;

  // For shorthand instructions, this represents target instructions.  Operands
  // are taken from the original operand array parsed for the shorthand and then
  // placed into the resulting target instructions.
  adl_opcode *alias_targets;
  int num_alias_targets;

  // In syntax: specifies where sequence of %p-fields's ends 
  // (we assume that it can only start at 0)
  int p_end;

  void (*action)(struct adl_operand_val *op_vals,unsigned position);
 
  // Stores instruction attributes, if any exist.  Null pointer if none exist,
  // otherwise points to an adl_instr_attr structure.
  adl_instr_attrs *attrs;

  // The block to which instructions belongs. Really only used by StarCore.
  unsigned num_blks;
  char blks[MAX_BLOCKS];

  // Given a field id, returns relevant counter value.  If not relevant, then
  // simply returns index, which is the current index in the packet.
  int (*prefix_counter)(int id,int index);

  // Modified any relevant prefix counters.
  void (*mod_prefix_counters)();

  // Auto-generated cross-operand checks.
  bool (*argchecker) (const expressionS *operand_values);
  
  // User defined rules.
  void  (*checker) (struct adl::InstrInfo &,int position);
  void* (*dests)(void);
  int   longest; // index of opcode with longest immediate corresponding to the same fields

  bool is_shorthand() const { return size == 0; };
};

// The structure which stores information about instructions of the same name.
struct adl_instr
{
  // Instruction name.
  const char *name;

  // Number of possible opcodes.
  int long num_opcodes;

  // Array of possible instructions.
  struct adl_opcode *opcodes;
};


struct adl_operand_macro    // For macro instruction fix-up
{
  unsigned width;           // The width of the micro instruction corresponding to the macro instruction field

  unsigned max_width;       // The maximum width of the instruction

  unsigned instr_width;     // The width of the macro instruction 

  unsigned shift;           // The shift of the micro-instruction field with respect to the end of the macro instruction
                            // The shift is used for inserting value by insert_value_simple()

  unsigned arg_base;        // The index of the first operand value for the microinstruction in the array of operand
                            // values of the macroinstruction

  adl_operand * operand;    // The operand of the "static" macro instruction corresponding to the micro instruction
                            // For macro instruction, insert value in 2 passes, thus save this operand for 2nd insertion
                            // If the operand = 0, then use insert_value_simple() instead insert_value()
};

//  Information specific to jmp/jsr instructions.
struct jmp_jsr_info
{
	// Instruction has "jmp" mnemonic.
	bool jmp_mnemonic;
	// Instruction is conditional.
	bool cond;
};

// Current line label symbol
extern symbolS *curr_label;

EXTERN_C bfd_boolean adl_start_line_hook(void);

extern void adl_cleanup(void);

void flush_instruction_queue();

extern void adl_set_curr_label(symbolS *);

// Assembler only instruction action code.
typedef void(*adl_asm_action_t)(int);
struct adl_asm_instr {
  const char *name;
  adl_asm_action_t action;
};


// Assembles an instruction- called by md_assemble.
EXTERN_C void adl_assemble (char *str,struct hash_control *op_hash, struct hash_control *asm_op_hash, 
                            struct hash_control *reg_hash,struct hash_control *instr_pfx_fields_hash,
                            int maxfields,int instr_table,const char *instr_table_name,unsigned line_number);
char *adl_assemble_instr(char *str,
	struct hash_control *op_hash, 
	struct hash_control *asm_op_hash ATTRIBUTE_UNUSED,
	struct hash_control *reg_hash,
	struct hash_control *instr_pfx_fields_hash,
	int maxfields,
	uint32_t line_number,
	int instr_table,
	const char *instr_table_name);

// Handles fixups- called by md_apply_fix.
void adl_apply_fix (fixS *fixP ,valueT *valP ,segT seg);


// Helper functions called from adl_post_asm and adl_post_packet_asm ADL hooks.
void  adl_set_field(int field_id,int iface_id,int val,int pos,int group);
void  adl_set_field(int field_id,int iface_id,int val,adl::InstrInfo &info);
void  adl_set_enum_field(int field_id,int iface_id,const char *val,int pos,int group);
void  adl_set_enum_field(int field_id,int iface_id,const char *val,adl::InstrInfo &info);
// Uses to enforce encoding selection
bool adl_pre_set_field(int field_id,int iface_id,int val);
bool adl_pre_set_enum_field(int field_id,int iface_id,const char* val);
// Returns FALSE if field not defined for instruction
unsigned adl_get_field(int field_id,int iface_id,int pos,int group);
unsigned adl_get_field(int field_id,int iface_id,const adl::InstrInfo &info);
// Returns true if the given field exists for this instruction.
bool  adl_query_field(int field_id,int iface_id,const adl::InstrInfo &info);

void*  adl_get_dest(int pos,int group);
int adl_get_instr_position(const adl::InstrInfo &,int group);
int  adl_get_current_group(void);
int  adl_group_size(int group);
// Eror logging used by the rule checkers. Errors are flushed at cleanup 
void adl_error(const char* err,int pos,int group);
// Info logging used by the rule checkers. Errors are flushed at cleanup 
void adl_info(const char* info,int pos,int group);

void adl_set_check_asm_rules(int i);


// Helper function to set a prefix field slice
void adl_set_prefix_field_slice(struct adl_prefix_field *pfield,int i,int value);

// Helper function to test instruction's attribute existence
bool adl_has_instr_attr(unsigned attr, int group);

// Leftover for use with the DevTech compiler.
struct adl_reloc {
  int   type;
  int   offset;
};

// Get all instruction/operand names - required for DevTech integration
extern "C" void adl_get_instr_names(const char*** names, int *num_names);
extern "C" void adl_get_instr_ops(const struct adl_name_pair **, int *num_names);

// Bit-mask manipulation functions in syntax expressions.
unsigned count_leading_zeros(offsetT n,unsigned s);
unsigned count_trailing_zeros(offsetT n);
unsigned count_ones(offsetT n);

bool opcode_has_attr(const struct adl_opcode *opcode, unsigned attr);
bool opcode_get_attr_val(struct adl_opcode *opcode, unsigned attr,
	std::string &val);

void adl_set_pseudo_op();
void gen_fixed_float16(char *, LITTLENUM_TYPE *);
void adl_float_cons(int float_type);
void adl_align_word(int arg);
void adl_sym_align(int arg);
void adl_remove_block(int arg);
void adl_ref_by_addr(int arg);
void adl_opt_mw_info(int arg);
void adl_common(int arg);
void adl_def_common(int arg);
void adl_local_common(int arg);
void adl_set_core(int arg);
void adl_elf_multidef(int ignore);
void adl_stack_effect(int arg);

void symbol_set_debug_info(symbolS *symbolP);

int vspa_parse_option(int c, char *arg);
void vspa_show_usage(FILE *stream);

//
// Callbacks from machine-independent code to hand-written processor-dependent code.
// These functions start with "acb" to denote an ADL callback.
//

// Hooks for fixup/relocation support.
EXTERN_C void acb_fixup_operand(fixS *fixP,const struct adl_operand *operand);
EXTERN_C void acb_fixup_instr(fixS *fixP,valueT *valP,segT seg);
  
// Called during instruction assembly- allows constants to contain relocation information.
EXTERN_C void acb_const_hook(char *str,expressionS *ex,const struct adl_operand *operand);
// Called during instruction assembly to handle a relocation.
EXTERN_C bfd_reloc_code_real_type acb_reloc_hook(char *str,expressionS *ex,const struct adl_operand *operand);
  
// Called to process a .switch option.  This is a generated function.
EXTERN_C void acb_parse_option(char *s,int n);



#endif
