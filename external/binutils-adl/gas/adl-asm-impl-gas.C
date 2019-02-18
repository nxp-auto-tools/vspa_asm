//
// Machine independent functions for use with the gas assembler.
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <inttypes.h>
#include <algorithm>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <sstream>

extern "C" {
# include "xregex.h"
# include "as.h"
# include "safe-ctype.h"
# include "struc-symbol.h"
# include "dwarf2dbg.h"
# include "write.h"
# include "subsegs.h"
}

#include "adl-asm-impl.h"
#include "adl-asm-info.h"
#include "adl-asm-common.h"
#include "mw-info.h"
#include "target_ISA.h"

#ifndef CHAR_BIT
# define CHAR_BIT 8
#endif

using namespace std;
using namespace adl;

#if (defined(_MSC_VER) && (_MSC_VER == 1500)) || defined(__CYGWIN__)

# if !defined(__CYGWIN__)
using namespace std::tr1;
# endif

#include <sstream>
namespace std
{
	string to_string(long long value)
	{
		stringstream ss("");
		ss << value;
		return ss.str();
	}
}
#endif

bool dmem_coff = false;
bool dmem_mvip = false;

# define VSPA_CORE 3

#define VSPA_ABI 0

#define ELF32_EF_VSPA_FLAGS(c, r, a, p) \
	(((c) & 0xffu)\
	| (((r) & 0xffu) << 8)\
	| (((a) & 0xffu) << 16)\
	| (((p) & 0xffu) << 24))

extern bool ignore_case;
extern int num_floats;

//  Class that holds the ".debug_line" and ".debug_info" start and end symbols
// for an object or function symbol.
class debug_info
{
private:
	// The object/function symbol.
	symbolS *symbol_;
	// Symbol that points to the start of the corresponding contents in the
	// ".debug_line" section.
	symbolS *debug_line_start_;
	// Symbol that points to the end of the corresponding contents in the
	// ".debug_line" section.
	symbolS *debug_line_end_;
	// Symbol that points to the start of the corresponding contents in the
	// ".debug_info" section.
	symbolS *debug_info_start_;
	// Symbol that points to the end of the corresponding contents in the
	// ".debug_info" section.
	symbolS *debug_info_end_;

public:
	//  Constructor.
	debug_info(symbolS *symbol) : symbol_(symbol), debug_line_start_(NULL),
		debug_line_end_(NULL), debug_info_start_(NULL), debug_info_end_(NULL)
	{
	}

	//  Get the symbol.
	symbolS *get_symbol() const
	{
		return symbol_;
	}

	//  Get the ".debug_line" start symbol.
	symbolS *get_debug_line_start() const
	{
		return debug_line_start_;
	}

	//  Get for the ".debug_line" end symbol.
	symbolS *get_debug_line_end() const
	{
		return debug_line_end_;
	}

	//  Get the ".debug_info" start symbol.
	symbolS *get_debug_info_start() const
	{
		return debug_info_start_;
	}

	//  Get the ".debug_info" end symbol.
	symbolS *get_debug_info_end() const
	{
		return debug_info_end_;
	}

	//  Check if the ".debug_line" start symbol is set.
	bool has_debug_line_start() const
	{
		return debug_line_start_ != NULL;
	}

	//  Check if the ".debug_line" end symbol is set.
	bool has_debug_line_end() const
	{
		return debug_line_end_ != NULL;
	}

	//  Check if the ".debug_info" symbols are set.
	bool has_debug_info() const
	{
		return debug_info_start_ != NULL && debug_info_end_ != NULL;
	}

	//  Set the ".debug_line" start symbol.
	void set_debug_line_start(symbolS *sym)
	{
		debug_line_start_ = sym;
	}

	//  Set the ".debug_line" end symbol.
	void set_debug_line_end(symbolS *sym)
	{
		debug_line_end_ = sym;
	}

	//  Set the ".debug_info" symbols.
	void set_debug_info(symbolS *start, symbolS *end)
	{
		debug_info_start_ = start;
		debug_info_end_ = end;
	}
};

//  Class that handles the debug information for function/object symbols.
class debug_info_handler
{
private:
	//  Class that implements a hashing operation for symbols.
	class hasher
	{
	public:
		//  Hashing operator.
		size_t operator() (symbolS * const &key) const
		{
			valueT v1 = reinterpret_cast<valueT>(symbol_get_frag(key));
			valueT v2 = S_GET_VALUE(key);

			// Cantor pairing function.
			return (v1 + v2) * (v1 + v2 + 1) / 2 + v2;
		}
	};

	//  Class that implements an equal operation for symbols.
	class equal
	{
	public:
		//  Equal operator.
		bool operator()(symbolS * const &s1, symbolS * const &s2) const
		{
			return (symbol_get_frag(s1) == symbol_get_frag(s2))
				&& (S_GET_VALUE(s1) == S_GET_VALUE(s2));
		}
	};

	typedef unordered_multimap<symbolS *, debug_info *, hasher, equal>
		debug_map_t;
	// Map containing debug information indexed by start symbols.
	debug_map_t starts_;
	// Map containing debug information indexed by end symbols (for functions
	// only).
	debug_map_t ends_;

	//  Generate ".MW_INFO" remove_block information for a function/object.
	inline void generate_remove_block(symbolS *stripped, symbolS *start,
		symbolS *end)
	{
		expressionS *expr = static_cast<expressionS *>(malloc(
			sizeof(expressionS)));
		expr->X_op = O_subtract;
		expr->X_add_symbol = end;
		expr->X_op_symbol = start;
		mwinfo_save_remove_block(start, stripped, expr);
	}

public:
	//  Register function.
	void add_function(symbolS *start, symbolS *end)
	{
		debug_info *di = new debug_info(start);
		starts_.insert(make_pair(start, di));
		ends_.insert(make_pair(end, di));
	}

	//  Check if a symbol is a function start. If check_handled is true, only
	// function start symbols that don't have a .debug_line start symbol are
	// considered.
	bool is_func_start(symbolS *sym, bool check_handled = false)
	{
		const auto range = starts_.equal_range(sym);
		for (auto itd = range.first, itd_end = range.second; itd != itd_end;
			++itd)
		{
			const debug_info *di = itd->second;
			if (!check_handled || !di->has_debug_line_start())
			{
				return true;
			}
		}
		return false;
	}

	//  Check if a symbol is a function end. If check_handled is true, only
	// function start symbols that don't have a .debug_line start symbol are
	// considered.
	bool is_func_end(symbolS *sym, bool check_handled = false)
	{
		const auto range = ends_.equal_range(sym);
		for (auto itd = range.first, itd_end = range.second; itd != itd_end;
			++itd)
		{
			const debug_info *di = itd->second;
			if (!check_handled || !di->has_debug_line_end())
			{
				return true;
			}
		}
		return false;
	}

	//  Add .debug_line start symbol for a function that doesn't have a start
	// symbol already.
	void add_debug_line_start(symbolS *sym, symbolS *start)
	{
		const auto range = starts_.equal_range(sym);
		bool done = false;
		for (auto itd = range.first, itd_end = range.second; itd != itd_end;
			++itd)
		{
			debug_info *di = itd->second;
			if (!di->has_debug_line_start())
			{
				di->set_debug_line_start(start);
				done = true;
			}
		}
		assert(done);
	}

	// Add .debug_line end symbol for a function that doesn't have an end
	// symbol already.
	void add_debug_line_end(symbolS *sym, symbolS *end)
	{
		const auto range = ends_.equal_range(sym);
		bool done = false;
		for (auto itd = range.first, itd_end = range.second; itd != itd_end;
			++itd)
		{
			debug_info *di = itd->second;
			if (!di->has_debug_line_end())
			{
				di->set_debug_line_end(end);
				done = true;
			}
		}
		assert(done);
	}

	//  Add .debug_info start and end symbols for a function or object.
	void add_debug_info(symbolS *sym, symbolS *start, symbol *end)
	{
		const auto range = starts_.equal_range(sym);
		debug_info *di = NULL;
		for (auto itd = range.first, itd_end = range.second; itd != itd_end;
			++itd)
		{
			debug_info *cdi = itd->second;
			// Search for a specific symbol by comparing the pointers.
			if (sym == cdi->get_symbol())
			{
				di = cdi;
				break;
			}
		}

		if (S_IS_FUNCTION(sym))
		{
			assert(di);
			di->set_debug_info(start, end);
		}
		else
		{
			assert(!di);
			di = new debug_info(sym);
			di->set_debug_info(start, end);
			starts_.insert(make_pair(sym, di));
		}
	}

	//  Generate ".MW_INFO" remove block information for all functions and
	// objects.
	void generate_remove_block()
	{
		for (debug_map_t::iterator itd = starts_.begin(),
			itd_end = starts_.end(); itd != itd_end; ++itd)
		{
			debug_info *di = itd->second;
			if (di->has_debug_line_start() && di->has_debug_line_end())
			{
				generate_remove_block(di->get_symbol(),
					di->get_debug_line_start(), di->get_debug_line_end());
			}
			if (di->has_debug_info())
			{
				generate_remove_block(di->get_symbol(),
					di->get_debug_info_start(), di->get_debug_info_end());
			}
		}
	}

	//returns the end symbol for a function
	void check_func_size()
	{
		for (debug_map_t::iterator itd = starts_.begin(),
			itd_end = starts_.end(); itd != itd_end; ++itd)
		{
			symbolS *start_sym = itd->first;

			if (S_IS_FUNCTION(start_sym))
			{
				//check only for functions
				debug_info *dis = itd->second;
				//get all end_of_function symbols associated to this function
				for (debug_map_t::iterator ited = ends_.begin(),
					ited_end = ends_.end(); ited != ited_end; ++ited)
				{
					debug_info *die = ited->second;
					if (die == dis)
					{
						symbolS *end_sym = ited->first;
						struct elf_obj_sy *sy_start_obj;
						offsetT sym_size = 0;
						offsetT sym_diff = 0; 
						expressionS expr;
						bool undef = false;

						if (bfd_is_und_section(start_sym->bsym->section))
						{
							as_bad(_("Function start symbol %s is undefined"), S_GET_NAME(start_sym));
							undef = true;
						}
						
						if (bfd_is_und_section(end_sym->bsym->section))
						{
							as_bad(_("Function end symbol %s is undefined"), S_GET_NAME(end_sym));
							undef = true;
						}

						// Using an expression is a workaround. 
						// In this assember processing momment, the sym_size expression cannot be 
						// resolved correctly all times (in case sym_size = PC - start_symbol)
						// This happens because when computing expressions values, the assembler
						// doesn't take in consideration the fragment offset of the symbols in 
						// the expressions. 
						// Using an expression instead of 
						//		S_GET_VALUE(end_sym) - S_GET_VALUE(start_sym) 
						// forces the assembler to use same mechanism.
						if (!undef)
						{
							memset(&expr, 0, sizeof(expressionS));
							expr.X_add_symbol = end_sym;
							expr.X_op_symbol = start_sym;
							expr.X_op = O_subtract;
							resolve_expression(&expr);
							assert(expr.X_op == O_constant);
							sym_diff = expr.X_add_number;
						}


						sy_start_obj = symbol_get_obj(start_sym);
						if (sy_start_obj->size
							&& resolve_expression(sy_start_obj->size)
							&& sy_start_obj->size->X_op == O_constant)
						{
							sym_size = sy_start_obj->size->X_add_number;
						}
						else if (sy_start_obj->size)
						{
							as_bad(_(".size expression for %s "
								"does not evaluate to a constant"), S_GET_NAME(start_sym));
						}
						else
						{
							sym_size = S_GET_SIZE(start_sym);
						}


						if ((sym_size != 0)
							&& (sym_diff != sym_size))
						{
							TC_SYMFIELD_TYPE *sym_decl = symbol_get_tc(start_sym);
							as_bad_where(sym_decl->decl_file, sym_decl->decl_line,
								_("Size of function %s 0x(%llx) does not evaluate to difference between \n"
									"symbol end %s and start symbol %s"),
								S_GET_NAME(start_sym),
								sym_size,
								S_GET_NAME(end_sym),
								S_GET_NAME(start_sym));
						}
					}
				}
			}
		}
	}

	//  Destructor.
	~debug_info_handler()
	{
		for (debug_map_t::iterator itd = starts_.begin(),
			itd_end = starts_.end(); itd != itd_end; ++itd)
		{
			delete itd->second;
		}
	}
};

// list of pairs for variable symbols and their corresponding debug location
// symbols; used to fill the beginning and ending addresses for symbols'
// locations
static vector<pair<symbolS *, symbolS *> > debug_loc_symbols;

/* flag for half floating point precision */
bool enable_hprec = false;
/* flag for double floating point precision */
bool enable_dprec = false;

// Number of AUs.
size_t AU_NUM = 64;
bool has_nau_option = false;
// Core precision.
enum core_type_t core_type = core_type_sp;
int isa_id = -1;
char* isa_name = NULL;

// Debug information handler.
static debug_info_handler dih;

/* VCPU common section */
asection bfd_vcom_section = BFD_FAKE_SECTION(bfd_vcom_section,
	SEC_IS_COMMON, NULL, "*VCOM*", 0);
/* IPPU common section */
asection bfd_icom_section = BFD_FAKE_SECTION(bfd_icom_section,
	SEC_IS_COMMON, NULL, "*ICOM*", 0);

extern "C" {
	extern const char FLT_CHARS[];
}

struct section_entry
{
	segT sec;
	const char *name;
	int core;
	flagword flags;
};

#define VCPU_BSS_SECTION 0
#define IPPU_BSS_SECTION 1
#define IPPU_TEXT_SECTION 2

struct section_entry vspa_sections[] =
{
	{ NULL, ".vbss", CORE_VCPU, SEC_ALLOC },
	{ NULL, ".ibss", CORE_IPPU,  SEC_ALLOC },
	{ NULL, ".itext", CORE_IPPU, SEC_ALLOC | SEC_LOAD | SEC_RELOC
	| SEC_READONLY | SEC_CODE | SEC_HAS_CONTENTS },
};

// Symbols used to implement complex relocations
static unordered_set<symbolS *> complex_reloc_symbols;

enum reloc_operator
{
	operator_noop = 0,
	operator_neg = 1,
	operator_bitnot = 2,
	operator_boolnot = 3,
	operator_mul = 4,
	operator_div = 5,
	operator_rem = 6,
	operator_add = 7,
	operator_sub = 8,
	operator_lshl = 9,
	operator_lshr = 10,
	operator_ashl = 11,
	operator_ashr = 12,
	operator_ls = 13,
	operator_le = 14,
	operator_gt = 15,
	operator_ge = 16,
	operator_eq = 17,
	operator_neq = 18,
	operator_bitand = 19,
	operator_bitor = 20,
	operator_bitxor = 21,
	operator_booland = 22,
	operator_boolor = 23
};

static reloc_howto_type r_vspa_push_pc = HOWTO(252, 0, 0, 32, 0, 0,
	complain_overflow_dont, bfd_elf_generic_reloc, "R_VSPA_PUSH_PC",
	0, 0, 0, 0);

// For gas, we just call the gas messaging routines.
// Since C's stdargs kinda suck in many ways, we have to duplicate this code.

void adl_tsktsk (const char *format, ...)
{
  va_list args;

  va_start (args, format);
  as_vtsktsk(format,args);
  va_end (args);
}

void adl_warn (const char *format, ...)
{
  va_list args;

  va_start (args, format);
  as_vwarn(format,args);
  va_end (args);
}

void adl_warn_where (char *file, unsigned int line, const char *format, ...)
{
  va_list args;

  va_start (args, format);
  as_vwarn_where(file,line,format,args);
  va_end (args);
}

void adl_bad (const char *format, ...)
{
  va_list args;

  va_start (args, format);
  as_vbad(format,args);
  va_end(args);
}

void adl_bad_where (char *file, unsigned int line, const char *format, ...)
{
  va_list args;

  va_start (args, format);
  as_vbad_where(file,line,format,args);
  va_end (args);
}

void adl_fatal (const char *format, ...)
{
  va_list args;

  va_start (args, format);
  as_vfatal(format,args);
  va_end (args);
}

/*
*  Parse VSPA 16-bit fixed float constants and generate the corresponding
* encoding
*/
void gen_fixed_float16(char *arg, LITTLENUM_TYPE *words) {
	union {
		unsigned int uint;
		float flt;
	} val, abs, int_val, round;
	bool neg, nan;
	float diff;

	/* parse the value */
	val.flt = atof(arg);

	neg = val.uint & 0x80000000;
	nan = false;
	/* check if the value is not a number */
	if ((val.uint & 0x7F800000) == 0x7F800000) {
		if (neg) {
			nan = val.uint > 0x7F800000;
		}
		else {
			nan = val.uint > 0xFF800000;
		}
	}

	/* get the absolute value */
	abs.flt = neg ? -val.flt : val.flt;
	if (abs.flt > 1.0 || nan) {
		abs.flt = 1.0;
	}

	/* get the integer value */
	int_val.flt = abs.flt * 32768.0;

	/* round the value to the nearest integer */
	round.flt = 0.0f;
	if (int_val.flt >= 0.5) {
		round.flt = int_val.flt + 0.5;
	}
	round.uint = (unsigned int)round.flt;
	diff = int_val.flt - ((float)((unsigned int)int_val.flt));
	/* if we are the half of two integers we round to the smaller one */
	if (diff == 0.5) {
		round.uint &= 0xFFFE;
	}

	/* check for the maximum value */
	if (round.uint > 0x7FFF) {
		round.uint = 0x7FFF;
	}

	/* add the sign bit */
	if (neg) {
		round.uint |= 0x8000;
	}

	memcpy(words, &round.uint, 2);
}

//  Check if the operand of an instruction is a floating point constant, in
// which case the corresponding encoding is generated.
static void handle_fp_const(expressionS *ex, char *arg, bool &hprec_err,
	bool &dprec_err)
{
	LITTLENUM_TYPE words[MAX_LITTLENUMS];

	if (arg[0] != '0')
	{
		as_bad("number too big");
		return;
	}

	// Check the precision of the constant.
	switch (arg[1])
	{
	case 'h':
	case 'H':
		if (enable_hprec)
		{
			gen_to_words(words, 1, 5);
			ex->X_add_number = words[0];
			ex->X_md = 16;
		}
		else
		{
			hprec_err = true;
		}
		break;
	case 'r':
	case 'R':
		gen_fixed_float16(arg + 2, words);
		ex->X_add_number = words[0];
		ex->X_md = 16;
		break;
	case 'f':
	case 'F':
	case 's':
	case 'S':
		gen_to_words(words, 2, 8);
		ex->X_add_number = (words[0] << 16) | words[1];
		ex->X_md = 32;
		break;
	case 'd':
	case 'D':
		if (!enable_dprec)
		{
			dprec_err = true;
		}
		ex->X_md = 64;
		break;
	default:
		as_bad("number too big");
		return;
	}

	ex->X_op = O_constant;
}


//  Check if we have a double precision floating point macro, in which case the
// expression takes the value of the corresponding part of the floating point
// constant.
bool dp_macro(char *arg, expressionS *ex, bool &dprec_err, bool &macro_err)
{
	int(*compare)(const char *, const char *, size_t) = ignore_case
		? strncasecmp : strncmp;
	bool low = !compare(arg, "__DP_LOW(", 9);
	bool high = !compare(arg, "__DP_HIGH(", 10);

	// Check if we have a floating point double precision macro.
	char *p = strchr(arg, ')');
	if ((!low && !high) || !p)
	{
		return false;
	}

	// Check if the constant is enclosed between parantheses and if it is a
	// floating point constant.
	unsigned start = low ? 8 : 9;
	if (strlen(arg) < start + 3
		|| arg[start + 1] != '0'
		|| !strchr(FLT_CHARS, arg[start + 2]))
	{
		macro_err = true;
		ex->X_op = O_constant;
		ex->X_md = 32;
		return true;
	}

	if (!enable_dprec)
	{
		dprec_err = true;
	}

	p[0] = 0;
	// Get the encoding of the floating point value.
	LITTLENUM_TYPE words[MAX_LITTLENUMS];
	atof_ieee(arg + start + 3, 'd', words);
	p[0] = ')';

	// Store the corresponding part of the constant.
	if (low)
	{
		ex->X_add_number = (words[2] << 16) | words[3];
	}
	else
	{
		ex->X_add_number = (words[0] << 16) | words[1];
	}

	ex->X_op = O_constant;
	ex->X_md = 32;

	return true;
}

//  Check if an expression contains a symbol that was not yet resolved.
bool has_unresolved_symbol(expressionS *ex)
{
	switch (ex->X_op)
	{
	case O_symbol:
	case O_symbol_rva:
	case O_uminus:
	case O_bit_not:
	case O_logical_not:
		return ((S_GET_SEGMENT(ex->X_add_symbol) == expr_section)
			? has_unresolved_symbol(&ex->X_add_symbol->sy_value)
			: ((S_GET_SEGMENT(ex->X_add_symbol) == absolute_section)
				? false
				: !ex->X_add_symbol->sy_flags.sy_resolved));
	case O_multiply:
	case O_divide:
	case O_modulus:
	case O_left_shift:
	case O_right_shift:
	case O_bit_inclusive_or:
	case O_bit_or_not:
	case O_bit_exclusive_or:
	case O_bit_and:
	case O_add:
	case O_subtract:
	case O_eq:
	case O_ne:
	case O_lt:
	case O_le:
	case O_ge:
	case O_gt:
	case O_logical_and:
	case O_logical_or:
	case O_index:
	case O_md1:
	case O_md2:
		return ((S_GET_SEGMENT(ex->X_add_symbol) == expr_section)
			? has_unresolved_symbol(&ex->X_add_symbol->sy_value)
			: ((S_GET_SEGMENT(ex->X_add_symbol) == absolute_section)
				? false
				: !ex->X_add_symbol->sy_flags.sy_resolved))
			|| ((S_GET_SEGMENT(ex->X_op_symbol) == expr_section)
				? has_unresolved_symbol(&ex->X_op_symbol->sy_value)
				: ((S_GET_SEGMENT(ex->X_op_symbol) == absolute_section)
					? false
					: !ex->X_op_symbol->sy_flags.sy_resolved));
	default:
		return false;
	}
}

// Wrapper to expression() method
segT parse_expr(expressionS *ex,
	char *arg,
	int len,
	bool &unresolved,
	bool &hprec_err,
	bool &dprec_err,
	bool &dp_macro_err)
{
   // Null-terminate input argument and pointer input_line_pointer at it.
  char *tmp = input_line_pointer;
  char backup = arg[len];
  arg[len] = 0;
  input_line_pointer = arg;
  segT e;

  // Check if the operand is a floating point macro.
  if (dp_macro(arg, ex, dprec_err, dp_macro_err))
  {
	  e = absolute_section;
	  goto parse_expr_end;
  }

  // Parse the expression (which takes input from input_line_pointer).
  e = expression(ex);

  // The expression was not parsed completely.
  if (strlen(input_line_pointer))
  {
	  e = NULL;
	  goto parse_expr_end;
  }

  // Check if the parsed value is a floating point constant.
  if (ex->X_op == O_big && ex->X_add_number <= 0)
  {
	  handle_fp_const(ex, arg, hprec_err, dprec_err);
  }

  if (ex->X_op == O_symbol || ex->X_add_symbol != NULL) {
    symbolS* symbol = symbol_find(arg);
    unresolved = ((symbol == NULL) || (symbol->sy_flags.sy_resolved == 0));
  }

  // Now restore input_line_pointer.
parse_expr_end:
  arg[len] = backup;
  input_line_pointer = tmp;
  return e;
}

// Handle multichar line_comments here;
// Need to use a hook, because otherwise this lines considered as error in read.c
// Reset line label
// TBD: to void
extern "C" bfd_boolean adl_start_line_hook(void) {

  if (num_line_comment_strs == 0) {
    return FALSE;
  }
  int i;
  for (i = 0; i != num_line_comment_strs; ++i) {
    const char *str = line_comment_strs[i];
    if (!strncmp(input_line_pointer,str,strlen(str))) {
      ignore_rest_of_line(); 
      return FALSE;
    }
  }
  return FALSE;
}


void handle_itable_change(const char* instr_table_name, int instr_table) 
{
  if (instr_table != current_table) {
    // A table-change has occurred.  We need to add a mapping
    // symbol to record this point.
    current_table = instr_table;

    {  // TBD 
      const int BufSize = 256;
      char buf[BufSize];
      sprintf(buf,"$$%s",instr_table_name);
      symbolS * symbolP = symbol_new (buf, now_seg, (valueT) frag_now_fix (), frag_now);
      symbol_table_insert (symbolP);
      symbol_get_bfdsym (symbolP)->flags |= BSF_FUNCTION | BSF_LOCAL;
    }
  }
}


void write_instr(char *f, unsigned *instr,int instr_sz,fragS *frag_n, uint32_t addr_mask) 
{
  int addr_mod;

  addr_mod = frag_now_fix () & addr_mask;
  if (frag_n->has_code && frag_n->insn_addr != addr_mod) {
    as_bad (_("instruction address could not be aligned"));
  }
  frag_n->insn_addr = addr_mod;
  // FIXME: figure out what's the problem with "has_code"
  //frag_n->has_code = 1;
  md_big_number_to_chars (f, instr, instr_sz);
}

// Note: This only grows the fragment in order to allocate space, but we don't
// truly allocate the space so that we can handle relaxed branches that are
// actually handled by external, user-supplied code.  Instead, we allocate space
// in handle_fixups, since that's only called once we know we don't have a
// special relocation to handle.
//
// The hack is that we do grow the frag for parallel architectures, due to the
// weird way in which we handle multiple groups.  So, this means that we can't
// properly handle externa relocs, like branch relaxation, and a parallel
// architecture.
//
// Of course, the whole parallel-architecture aspect is a gigantic hack, so we
// probably just need to completely rethink and rewrite this code anyway.
void alloc_instr_buf(InstrInfo &info)
{
  info.f = frag_more(info.opcode->size);
  info.frag = frag_now;
}


void save_instr(InstrBundles &instrInfos,char *s, adl_opcode *opcode, adl_opcode *src_opcode, expressionS *operand_values,
                const Args &args,const char *file_name, int line_number, unsigned pc, int maxfields, int instr_table,
                const char *instr_table_name,const string &eoi_str) 
{
  handle_itable_change(instr_table_name,instr_table);

  InstrInfo &info = instrInfos.add();
  info.init(s,opcode,src_opcode,operand_values,args,file_name,line_number,pc,maxfields,instr_table,instr_table_name);

  instrInfos.add_separator(eoi_str);
}

//  Align the size of a section. The alignment of the section doesn't change
// its size.
valueT md_section_align (asection *seg,valueT addr)
{
	return addr;
}

//  Generate a relocation that pushes the program counter on the stack.
static arelent *create_push_pc_reloc(bfd_size_type address)
{
	arelent *reloc = static_cast<arelent *>(xmalloc(sizeof(arelent)));

	reloc->sym_ptr_ptr = static_cast<asymbol **>(xmalloc(sizeof(asymbol *)));
	*reloc->sym_ptr_ptr = symbol_get_bfdsym(&abs_symbol);
	reloc->howto = &r_vspa_push_pc;
	reloc->address = address;
	reloc->addend = 0;

	return reloc;
}

//  Generate a relocation that pushes the operand of an expression on the
// stack; if the symbol of the expression is temporary, a relocation that
// pushes the program counter is generated.
static arelent *create_push_reloc(symbolS *sym,
	bfd_size_type address,
	offsetT addend)
{
	static reloc_howto_type r_vspa_push = HOWTO(253, 0, 0, 32, 0, 0,
		complain_overflow_dont, bfd_elf_generic_reloc, "R_VSPA_PUSH",
		0, 0, 0, 0);
	arelent *reloc = static_cast<arelent *>(xmalloc(sizeof(arelent)));

	reloc->sym_ptr_ptr = static_cast<asymbol **>(xmalloc(sizeof(asymbol *)));
	if (sym == NULL)
	{
		*reloc->sym_ptr_ptr = symbol_get_bfdsym(&abs_symbol);
		reloc->howto = &r_vspa_push;
	}
	else if (symbol_get_tc(sym)->is_dot)
	{
		*reloc->sym_ptr_ptr = symbol_get_bfdsym(&abs_symbol);
		reloc->howto = &r_vspa_push_pc;
	}
	else
	{
		*reloc->sym_ptr_ptr = symbol_get_bfdsym(sym);
		reloc->howto = &r_vspa_push;
	}
	reloc->address = address;
	reloc->addend = addend;

	return reloc;
}

//  Generate a relocation representing an arithmetic, logic or comparison
// operator.
static arelent *create_oper_reloc(bfd_size_type address, offsetT addend)
{
	arelent *reloc = static_cast<arelent *>(xmalloc(sizeof(arelent)));
	static reloc_howto_type r_vspa_oper = HOWTO(254, 0, 0, 32, 0, 0,
		complain_overflow_dont, bfd_elf_generic_reloc, "R_VSPA_OPER",
		0, 0, 0, 0);

	reloc->sym_ptr_ptr = static_cast<asymbol **>(xmalloc(sizeof(asymbol *)));
	*reloc->sym_ptr_ptr = symbol_get_bfdsym(&abs_symbol);
	reloc->howto = &r_vspa_oper;
	reloc->address = address;
	reloc->addend = addend;

	return reloc;
}

//  Generate a relocation for a stack pop operation.
static arelent *create_pop_reloc(bfd_size_type address, offsetT addend)
{
	static reloc_howto_type r_vspa_pop = HOWTO(255, 0, 0, 32, 0, 0,
		complain_overflow_dont, bfd_elf_generic_reloc, "R_VSPA_POP",
		0, 0, 0, 0);
	arelent *reloc = static_cast<arelent *>(xmalloc(sizeof(arelent)));

	reloc->sym_ptr_ptr = static_cast<asymbol **>(xmalloc(sizeof(asymbol *)));
	*reloc->sym_ptr_ptr = symbol_get_bfdsym(&abs_symbol);
	reloc->howto = &r_vspa_pop;
	reloc->address = address;
	reloc->addend = addend;

	return reloc;
}

//  Generate the corresponding list of relocations for expressions involving
// the difference of two symbols and a signed addend.
static void handle_subtraction_reloc(fixS *fixp,
	arelent **relocs,
	reloc_howto_type *howto,
	unsigned rightshift)
{
	// Create the corresponding relocations.
	offsetT where = fixp->fx_frag->fr_address + fixp->fx_where;
	arelent *push_op1 = create_push_reloc(fixp->fx_addsy,
	where, fixp->fx_addnumber << rightshift);
	arelent *push_op2 = create_push_reloc(fixp->fx_subsy, where, 0);
	arelent *operation = create_oper_reloc(where, operator_sub);
	arelent *pop_rel = create_pop_reloc(where, howto->type);

	// Set the relocations.
	relocs[0] = push_op1;
	relocs[1] = push_op2;
	relocs[2] = operation;
	relocs[3] = pop_rel;
	relocs[4] = NULL;
}

//  Generate the corresponding list of relocations for a relocation against
// a temporary dot symbol.
static void handle_temp_symbol_reloc(fixS *fixp,
	arelent **relocs,
	reloc_howto_type *howto,
	unsigned rightshift)
{
	// Create the corresponding relocations.
	offsetT where = fixp->fx_frag->fr_address + fixp->fx_where;
	arelent *push_op = create_push_reloc(fixp->fx_addsy,
	where, fixp->fx_addnumber << rightshift);
	arelent *pop_rel = create_pop_reloc(where, howto->type);

	// Set the relocations.
	relocs[0] = push_op;
	relocs[1] = pop_rel;
	relocs[2] = NULL;
}

//  Generate the corresponding relocations for a complex expression
static bool handle_expression(string &expr,
	offsetT where,
	arelent **relocs,
	size_t &index,
	reloc_operator expr_oper,
	unsigned right_shift)
{
	string::size_type token_end = expr.find(':');
	string token;
	if (token_end == string::npos)
	{
		token = expr;
		expr = "";
	}
	else
	{
		token = expr.substr(0, token_end);
		expr = expr.substr(token_end + 1);
	}

	if (!token.compare("."))
	{
		if (index >= MAX_RELOC_EXPANSION)
		{
			return false;
		}

		relocs[index++] = create_push_pc_reloc(where);
		return true;
	}
	else if (token[0] == 's' || token[0] == 'S')
	{
		if (index >= MAX_RELOC_EXPANSION)
		{
			return false;
		}

		// Get symbol length.
		unsigned long len = strtol(token.substr(1).c_str(), NULL, 10);

		// Get symbol name.
		string symbol_name = expr.substr(0, len);
		if (expr.size() == len)
		{
			expr = "";
		}
		else
		{
			expr = expr.substr(len + 1);
		}

		symbolS *symbol = symbol_find(symbol_name.c_str());
		assert(symbol != NULL);

		relocs[index++] = create_push_reloc(symbol, where, 0);
		return true;
	}
	else if (token[0] == '#')
	{
		if (index >= MAX_RELOC_EXPANSION)
		{
			return false;
		}

		// Get immediate value.
		long value = static_cast<long>(strtoul(token.substr(1).c_str(),
			NULL, 16));
		if (expr_oper == operator_add || expr_oper == operator_sub)
		{
			value <<= right_shift;
		}
		relocs[index++] = create_push_reloc(NULL, where, value);

		return true;
	}

	reloc_operator oper = operator_noop;
	bool unary = false;
	if (!token.compare("0-"))
	{
		oper = operator_neg;
		unary = true;
	}
	else if (!token.compare("~"))
	{
		oper = operator_bitnot;
		unary = true;
	}
	else if (!token.compare("!"))
	{
		oper = operator_boolnot;
		unary = true;
	}
	else if (!token.compare("*"))
	{
		oper = operator_mul;
	}
	else if (!token.compare("/"))
	{
		oper = operator_div;
	}
	else if (!token.compare("%"))
	{
		oper = operator_rem;
	}
	else if (!token.compare("+"))
	{
		oper = operator_add;
	}
	else if (!token.compare("-"))
	{
		oper = operator_sub;
	}
	else if (!token.compare("<<<"))
	{
		oper = operator_lshl;
	}
	else if (!token.compare(">>>"))
	{
		oper = operator_lshr;
	}
	else if (!token.compare("<<"))
	{
		oper = operator_ashl;
	}
	else if (!token.compare(">>"))
	{
		oper = operator_ashr;
	}
	else if (!token.compare("<"))
	{
		oper = operator_ls;
	}
	else if (!token.compare("<="))
	{
		oper = operator_le;
	}
	else if (!token.compare(">"))
	{
		oper = operator_gt;
	}
	else if (!token.compare(">="))
	{
		oper = operator_ge;
	}
	else if (!token.compare("=="))
	{
		oper = operator_eq;
	}
	else if (!token.compare("!="))
	{
		oper = operator_neq;
	}
	else if (!token.compare("&"))
	{
		oper = operator_bitand;
	}
	else if (!token.compare("^"))
	{
		oper = operator_bitxor;
	}
	else if (!token.compare("|"))
	{
		oper = operator_bitor;
	}
	else if (!token.compare("&&"))
	{
		oper = operator_booland;
	}
	else if (!token.compare("||"))
	{
		oper = operator_boolor;
	}

	if (oper == operator_noop)
	{
		as_bad("Unrecognized operand in complex relocation: '%s'", token.c_str());
		return true;
	}

	bool ok = true;
	ok &= handle_expression(expr, where, relocs, index, oper, right_shift);
	if (!unary)
	{
		ok &= handle_expression(expr, where, relocs, index, oper, right_shift);
	}

	ok &= (index < MAX_RELOC_EXPANSION);
	if (ok)
	{
		relocs[index++] = create_oper_reloc(where, oper);
	}

	return ok;
}

//  Handle a fix-up that contains a complex expression.
void handle_complex_relocation(fixS *fixp,
	arelent **relocs,
	reloc_howto_type *howto,
	unsigned rightshift)
{
	string expr(S_GET_NAME(fixp->fx_addsy));
	size_t index = 0;
	offsetT where = fixp->fx_frag->fr_address + fixp->fx_where;
	bool ok = handle_expression(expr, where, relocs, index, operator_noop,
		rightshift);

	if (ok && (fixp->fx_subsy || fixp->fx_addnumber))
	{
		ok &= (index + 2 <= MAX_RELOC_EXPANSION);
		if (ok)
		{
			relocs[index++] = create_push_reloc(fixp->fx_subsy, where,
				fixp->fx_addnumber << rightshift);
			relocs[index++] = create_oper_reloc(where, operator_sub);
		}
	}

	ok &= (index < MAX_RELOC_EXPANSION);
	if (ok)
	{
		relocs[index++] = create_pop_reloc(where, howto->type);
		relocs[index] = NULL;
	}
	else
	{
		AS_BAD_WHERE(fixp->fx_file,
			fixp->fx_line,
			"Expression too complex for a relocation.");
		relocs[0] = NULL;
	}
}

/* Generate a reloc for a fixup.  */
arelent **tc_gen_reloc (asection *seg ATTRIBUTE_UNUSED,fixS *fixp)
{
	static arelent *relocs[MAX_RELOC_EXPANSION + 1];
	reloc_howto_type *howto = adl_reloc_type_lookup(stdoutput,
		fixp->fx_r_type);

	if (howto == (reloc_howto_type *)NULL)
	{
		AS_BAD_WHERE(fixp->fx_file,
			fixp->fx_line,



			_("reloc %d not supported by object file format"),
			(int)fixp->fx_r_type);
		relocs[0] = NULL;
		return relocs;
	}

	// WA CMVSPA-2280: for relocation R_VSPA_DMEM_20 and R_VSPA_DMEM_21, the addend shouldn't be shifted
	unsigned rightshift = (fixp->fx_r_type == 26 || fixp->fx_r_type == 27)? 0: howto->rightshift;

	// Create expression relocations.
	if (fixp->fx_addsy)
	{
		unordered_set<symbolS *>::iterator sym_it =
			complex_reloc_symbols.find(fixp->fx_addsy);

		// Handle complex relocation.
		if (sym_it != complex_reloc_symbols.end())
		{
			handle_complex_relocation(fixp, relocs, howto, rightshift);
			return relocs;
		}

		// Handle simple symbol subtraction expression relocation.
		if (fixp->fx_subsy)
		{
			handle_subtraction_reloc(fixp, relocs, howto, rightshift);
			return relocs;
		}
		// Handle symbol subtraction expression relocation including a dot
		// symbol.
		else if (symbol_get_tc(fixp->fx_addsy)->is_dot)
		{
			handle_temp_symbol_reloc(fixp, relocs, howto, rightshift);
			return relocs;
		}
	}

	// Create regular relocation.
	arelent *reloc = static_cast<arelent *>(xmalloc(sizeof(arelent)));
	reloc->sym_ptr_ptr = static_cast<asymbol **>(xmalloc(sizeof(asymbol *)));
	*reloc->sym_ptr_ptr = fixp->fx_addsy
		? symbol_get_bfdsym(fixp->fx_addsy) : NULL;
	reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
	reloc->howto = howto;
	reloc->addend = fixp->fx_addnumber << rightshift;
	relocs[0] = reloc;
	relocs[1] = NULL;

	return relocs;
}


/* The location from which a PC relative jump should be calculated,
   given a PC relative reloc.  */
long md_pcrel_from_section (fixS *fixp ATTRIBUTE_UNUSED, segT sec ATTRIBUTE_UNUSED)
{
  return fixp->fx_frag->fr_address + fixp->fx_where;
}


ATTRIBUTE_UNUSED void handle_fixups(char *f,fragS *curr_frag,struct adl_fixup *fixups,int num_fixups, int instr_sz)
{
  int i;
  for (i = 0; i != num_fixups; ++i) {
    struct adl_fixup *fixup = &fixups[i];
    fixS *fix = 0;
    if (fixup->reloc != BFD_RELOC_UNUSED) {
      // We have to deal with a relocation.
      int size;

      reloc_howto_type *reloc_howto = adl_reloc_type_lookup (stdoutput, fixup->reloc);

      if (!reloc_howto) {
        as_fatal (_("Unknown relocation %d"),fixup->reloc);
      }

      size = bfd_get_reloc_size (reloc_howto);

	  int offset = adl_reloc_offset(fixup->reloc, fixup->operand, instr_sz);

      if (size < 1 || size > 16) {
        as_fatal (_("Bad relocation size %d"),size);
      }

      fix = fix_new_exp (curr_frag,
                         f - curr_frag->fr_literal + offset,
                         size,
                         &fixup->exp,
                         reloc_howto->pc_relative,
                         fixup->reloc);

	  fix->tc_fix_data.right_shift = reloc_howto->rightshift;
      fix->tc_fix_data.bit_mask    = ~(~0 << reloc_howto->bitsize);

      if ((size + offset) > instr_sz) {
        as_bad (_("Relocation extends past end of instruction.  Too large of a relocation used.")); 
      }

    } else {
      // Bit of a hack here- only relative, local symbols will be converted into
      // simple fixups.  If we have a modifier, we want it to be handled in that
      // way, since we're using a function to adjust it.  So, set the flag to be
      // relative if we have a modifier.
      bool pcrel = (fixup->is_relative || (fixup->operand && fixup->operand->modifier));
      
      // Default case- not a relocation, just a fixup.
      fix = fix_new_exp (curr_frag,
                         f - curr_frag->fr_literal,
                         instr_sz,
                         &fixup->exp,
                         pcrel,
                         BFD_RELOC_UNUSED);
    }
    fix->fx_line                    = fixup->line_number;
    fix->tc_fix_data.operand        = fixup->operand;
    fix->tc_fix_data.opcode         = fixup->opcode;
    fix->tc_fix_data.operand_values = fixup->operand_values;
    fix->tc_fix_data.operand_arg    = fixup->operand_arg;
    fix->tc_fix_data.operand_macro  = fixup->operand_macro;
    fix->tc_fix_data.is_macro       = fixup->is_macro;
    fix->tc_fix_data.instr_size     = fixup->instr_size;
  }
}


//  Handle fixup expressions that don't generate relocations for VSPA.
void adl_vspa_fix_expressions(fixS *fixP, valueT value)
{
	char *where = fixP->fx_frag->fr_literal + fixP->fx_where;

	// Ignore expressions that result in relocations.
	if (fixP->fx_addsy || fixP->fx_subsy)
	{
		return;
	}

	// The fixup won't generate any relocation.
	fixP->fx_done = 1;

	if (fixP->fx_r_type == adl_get_reloc_type_by_name("_1byte"))
	{
		md_number_to_chars(where, value, 1);
	}
	else if (fixP->fx_r_type == adl_get_reloc_type_by_name("_2byte"))
	{
		md_number_to_chars(where, value, 2);
	}
	else if (fixP->fx_r_type == adl_get_reloc_type_by_name("_4byte")
		|| fixP->fx_r_type == adl_get_reloc_type_by_name("_word"))
	{
		md_number_to_chars(where, value, 4);
	}
}

//  Dump complex relocation expression.
static string dump_expression(string &expr)
{
	string::size_type token_end = expr.find(':');
	string token;
	if (token_end == string::npos)
	{
		token = expr;
		expr = "";
	}
	else
	{
		token = expr.substr(0, token_end);
		expr = expr.substr(token_end + 1);
	}

	if (!token.compare("."))
	{
		return string(".");
	}
	else if (token[0] == 's' || token[0] == 'S')
	{
		// Get symbol length.
		unsigned long len = strtol(token.substr(1).c_str(), NULL, 10);

		// Get symbol name.
		string symbol_name = expr.substr(0, len);
		if (expr.size() == len)
		{
			expr = "";
		}
		else
		{
			expr = expr.substr(len + 1);
		}

		return symbol_name;
	}
	else if (token[0] == '#')
	{
		// Get symbol immediate value.
		long value = static_cast<long>(strtoul(token.substr(1).c_str(),
			NULL, 16));
		return std::to_string(value);
	}

	bool unary = false;
	string oper;
	if (!token.compare("0-"))
	{
		oper = "-";
		unary = true;
	}
	else if (!token.compare("~")
		|| !token.compare("!"))
	{
		unary = true;
		oper = token;
	}
	else
	{
		oper = token;
	}

	string result = "(";
	if (unary)
	{
		result.append(oper);
	}
	else
	{
		result.append(dump_expression(expr));
		result.append(" ");
		result.append(oper);
		result.append(" ");
	}

	result.append(dump_expression(expr));
	result.append(")");

	return result;
}

//  Dump fixup expression.
static string dump_expression(fixS *fixP)
{
	string result;
	symbolS *add_sym = fixP->fx_addsy;
	if (add_sym)
	{
		if (add_sym->bsym->flags & (BSF_RELC | BSF_SRELC))
		{
			string expr = S_GET_NAME(add_sym);
			result = dump_expression(expr);
		}
		else if (S_GET_SEGMENT(add_sym) == absolute_section)
		{
			result = S_GET_VALUE(add_sym);
		}
		else
		{
			result = S_GET_NAME(add_sym);
		}
	}

	symbolS *sub_sym = fixP->fx_subsy;
	if (sub_sym)
	{
		if (add_sym)
		{
			result.append(" - ");
		}
		else
		{
			result.append("-");
		}

		if (sub_sym->bsym->flags & (BSF_RELC | BSF_SRELC))
		{
			string expr = S_GET_NAME(sub_sym);
			result.append(dump_expression(expr));
		}
		else if (S_GET_SEGMENT(sub_sym) == absolute_section)
		{
			result = S_GET_VALUE(sub_sym);
		}
		else
		{
			result.append(S_GET_NAME(sub_sym));
		}
	}

	long long offset = static_cast<long long>(fixP->fx_offset);
	if (offset > 0)
	{
		result.append(" + ");
		result.append(std::to_string(offset));
	}
	else if (offset < 0)
	{
		result.append(" - ");
		result.append(std::to_string(-offset));
	}

	return result;
}

void adl_apply_fix (fixS *fixP ,valueT *valP ,segT seg ATTRIBUTE_UNUSED)
{
  valueT value = * valP;

  // Store original relocation data, since we might clear it out for a local
  // label.
  bfd_reloc_code_real_type orig_reloc = fixP->fx_r_type;
  int was_reloc = (fixP->fx_r_type != BFD_RELOC_UNUSED);

  if (fixP->fx_addsy != NULL || fixP->fx_subsy != NULL) {
    /* Hack around bfd_install_relocation brain damage.  */
    if (fixP->fx_pcrel) {
      value += fixP->fx_frag->fr_address + fixP->fx_where;
    }
  } else {
    fixP->fx_done = 1;
    // Local symbol, so we don't use a relocation.
    fixP->fx_r_type = BFD_RELOC_UNUSED;
  }

  // Original offset- we stick this into the addend.
  valueT origvalue = value;
  
  int is_reloc = (fixP->fx_r_type != BFD_RELOC_UNUSED);

  // We need to update the value for fixups and for relocations which are not
  // pc-relative.

  // Handle using our generated code if we have installed relocations or this is
  // just an internal fixup.
  if (!is_reloc || adl_have_custom_relocs()) {
    // Just a fixup- we lookup the symbol and insert the location.
    const struct adl_fixup_data *fix_data = (const struct adl_fixup_data *)&(fixP->tc_fix_data);
    const struct adl_operand *operand = fix_data->operand;
    const struct adl_opcode  *opcode  = fix_data->opcode;   // Mutable for macro instruction
    shared_expr *operand_values       = fix_data->operand_values;
    int operand_arg                   = fix_data->operand_arg;                              
    int line_number                   = fixP->fx_line;
    
    adl_operand_macro *operand_macro  = fix_data->operand_macro;
    bool is_macro                     = fix_data->is_macro;
    unsigned instr_size               = fix_data->instr_size;

    unsigned char *where;
    unsigned instr[MAX_INSTR_WORDS];
    
    // Fetch the instruction, insert the fully resolved operand
    // value, and stuff the instruction back again.
    where = (unsigned char*)fixP->fx_frag->fr_literal + fixP->fx_where;

    // Fixups require an offset, but our read/write logic requires the original
    // address of the instruction.  So, here we have to undo the offset in order
    // to read the correct data.  We have to do this even for a local label.  We
    // also apply any actions associated with the relocation, for consistency.
    if (was_reloc) {
      reloc_setter  rs = 0;
      reloc_action  ra = 0;
      int size = 0, inner_size = 0, offset = -1;
      adl_get_reloc_funcs(orig_reloc,&rs,&ra,&size,&inner_size,&offset);
      // If the offset wasn't set, then align with operand bit position.
      if (offset < 0) {
        offset = (operand) ? operand->bitpos/8 : 0;
      }
	  //wa cmpvspa-2087, works in conjuction with wa for cmpvspa-2030
	  //for little endian, offset should be 0
	  if (target_big_endian)
	  {
		  where -= offset;
	  }

      if (ra) {
        // Invoke the action.  If we're no longer a relocation, then we consider
        // this a linker pass, since we won't be emitting a relocation, and thus
        // this won't be processed again.
        value = ra(value,(!is_reloc));
      }
    }

    if (is_reloc) {
      // Stick in the value, based upon what we know, in order to allow this
      // code to run w/o linking.  It's not perfect, but should cover most
      // cases.
      value = (value >> fix_data->right_shift);
      if (fix_data->bit_mask) {
        value &= fix_data->bit_mask;
      }
    }

    // If little-endian, then left-justify to word boundary.
    if (!target_big_endian && ((instr_size % 4) != 0)) {
      where -= instr_size % 4;
    }
    
    // Make sure that our range is OK, e.g. we want to make sure we detect if
    // somebody is trying to branch to a label that's too far away.  Don't do
    // this if we have a modifier (function)- we assume that the user is smart
    // enough to do their own range checking in that case.
    if (!is_reloc && operand && !operand->modifier && !adl_check_range(value,operand)) {
      AS_BAD_WHERE (fixP->fx_file, fixP->fx_line, _("target is out of the operand's range"));
    }
	if (fixP->fx_r_type == BFD_RELOC_UNUSED
		&& operand_arg == 2 /*check set.loop instruction only when processing last operand*/
		&& (check_set_loop_labels(fix_data->opcode)
		|| (fix_data->is_macro && (fix_data->operand_macro->operand->flags & ADL_FROM_LOOP_INST))))
	{
		unsigned int max_val = 1 << operand->ptr->width;
		int val = value;
		if (operand->modifier)
		{
			assert(operand_arg >= 0);
			expressionS *ex = operand_values[0].get();
			ex += operand_arg;
			ex->X_add_number = value;
			ex -= operand_arg;
			val = operand->modifier(ex, value);
		}
		if (val > max_val)
		{
			as_bad_where(fixP->fx_file, fixP->fx_line,
				"number of instructions in loop %d is too big. Maximum allowed %d",
				val, max_val);
		}
	}
	
    int k;
    //FIXME: simplify this code
	unsigned limit = min((unsigned)MAX_INSTR_WORDS, (unsigned)(fixP->fx_frag->fr_fix - fixP->fx_where));
    if (target_big_endian) {
      for (k = 0; k != limit ; ++k) {
        instr[k] = (bfd_getb32 ((unsigned char *) where + k*4));
      }
    } else {
      for (k = 0; k != limit ; ++k) {
        instr[k] = (bfd_getl32 ((unsigned char *) where + k*4));
      }
    }

    // Insert the fixed-up value.
    // insert_value (instr, ((uint64_t)value),operand);
    if (operand) {
      if (is_macro && (operand_macro->width < operand_macro->instr_width)) {    // Short-circuit evaluation
        insert_value (instr, ((uint64_t)value), operand, operand_macro); //
      } else {
        insert_value (instr, ((uint64_t)value), operand);
      }
	}
	else {
		if (fixP->fx_size > 4) {
			if (target_big_endian) {
				instr[0] = (value >> 32);
				instr[1] = (value & 0xffffffff);
			}
			else {
				instr[1] = (value >> 32);
				instr[0] = (value & 0xffffffff);
			}
		}
		else if (fixP->fx_size == 4) {
			*instr = value;
		}
		else if (fixP->fx_size == 2) {
			// If big-endian, then we'll be in the upper half of the word.
			if (target_big_endian) {
				*instr = (value << 16) | (value & 0xffff);
			}
			else {
				*instr = (*instr & 0xffff0000) | (value & 0xffff);
			}
		}
		else if (fixP->fx_size == 1) {
			// If big-endian, then we'll be in the upper byte of the word.
			if (target_big_endian) {
				*instr = (value << 24) | (value & 0xff);
			}
			else {
				*instr = (*instr & 0xffffff00) | (value & 0xff);
			}

		}
		else {
			as_bad(_("Cannot handle fixup of size %d"), fixP->fx_size);
		}
	}

    // Do we have modifiers?  If so, then update all of them.
    if (operand_values) {
      // Stuff the new value into the operand array, so that modifier functions
      // see the new value.
      assert(operand_arg >= 0);
	  expressionS *ex = operand_values[0].get();
	  ex += operand_arg;
      ex->X_add_number = value;
      int i;
      for (i = 0; i != opcode->num_operands; ++i) {
        const struct adl_operand *op = &opcode->operands[i];
		expressionS *ex = NULL;
		if (op->arg >= 0)
		{ 
			ex = operand_values[0].get() + op->arg;
		}
        if (op->modifier && (!ex || ex->X_op != O_symbol)) {
          // Modifier function exists- use it.
          // insert_modifier(instr,op,operand_values,line_number);
            if (is_macro) { 
                insert_modifier(instr,op,operand_values->get(),line_number, operand_macro); //
            } else {
                insert_modifier(instr,op,operand_values->get(),line_number);
            }          
        }
      }
    }

    // Now write the value back.
    if (target_big_endian) {
      for (k = 0; k != limit ; ++k) {
        bfd_putb32 ((bfd_vma) instr[k], (unsigned char *) where + 4*k);
      }
    } else {
      for (k = 0; k != limit ; ++k) {
        bfd_putl32 ((bfd_vma) instr[k], (unsigned char *) where + 4*k);
      }
    }

    bool done = false;
    if (is_reloc || fixP->fx_done) {
      if (is_reloc) {
        *valP = value;
        fixP->fx_addnumber = origvalue;
      }
      done = true;
    }

    if (!done) {
      acb_fixup_operand(fixP,operand);
    }
    
    // Then free this operand-values memory. 
	//Calling release decrements the shared pointer count and releases memory
	//if the count reaches 0
	if (operand_values)
	{
		operand_values->release();
	}
      
    // Freeing opcode also free operand (fixups[fc].operand)
    if (is_macro) {                 // opcode points to the "dynamic" copy, so free it
        assert(opcode);             // For fixup or macro, we assert opcode.
        assert(opcode->operands);   // For fixup, we assert opcode->operands.
        free(opcode->operands);
        free((void*)opcode);  
    }
      
    free(operand_macro);    // It is OK to free a null pointer
    

  } else {
    // Called when we don't have a fixup for just an operand.  If we have
    // installed relocations, then skip this.
    if (!adl_have_custom_relocs()) {
      acb_fixup_instr(fixP,valP,seg);
    }
  }

  *valP = value;
  fixP->fx_addnumber = origvalue;
}

extern "C" void free_buffer(unsigned x) { x = 0 ; }

void adl_error(const char *msg,int pos,int group) 
{
  char *file;
  unsigned int line;

  as_where(&file,&line);
  as_bad_where(file,line,msg);
  num_rule_errors++;
}

void adl_info(const char *msg,int pos,int group) 
{
  char *file;
  unsigned int line;
  as_where(&file,&line);
  InstrBundles &infos = instrInfos;
  if (!infos.empty(group)) {
    InstrInfo &info = infos.get_instr(group,pos);
    line = info.line_number;
  }

  as_warn_where(file,line,msg);
}

// Hook to manipulate multiple packets.  This is called immediately before
// post_packet_asm, so it can be used to combine packets, if necessary.
void __attribute__((weak)) acb_process_packets(InstrBundles &instrs,int curgrp,int lastgrp)
{
  // By default, do nothing.
}


// This must be overridden in a user supplied library, if relocations require
// extra handling.
void __attribute__((weak)) acb_handle_reloc_extra(unsigned *instr,unsigned instr_sz,
                                                  bfd_reloc_code_real_type reloc,const adl_opcode *operand,
                                                  const adl_operand *opcode,expressionS *exp)
{
  as_bad(_("Relocation requiring machine-dependent extra handling encountered, but no handler exists.")); 
}

void __attribute__((weak)) acb_handle_convert_frag(bfd *abfd,asection *sec,fragS *fragp)
{
  as_bad(_("No handler specified for frag conversion."));
}

int __attribute__((weak)) acb_estimate_size_before_relax (fragS *fragp,asection *seg)
{
  as_bad(_("No handler specified for relax size estimation."));
  return 0;
}

long __attribute__((weak)) acb_relax_frag (segT segment, fragS *fragP, long stretch)
{
  as_bad(_("No handler specified for frag relaxing."));
  return 0;
}

bool __attribute__((weak)) acb_apply_fix (fixS *fixP ,valueT *valP ,segT seg ATTRIBUTE_UNUSED)
{
  // Use ADL's handler by default.
  return false;
}

void __attribute__((weak)) acb_setup_finish(struct hash_control *instr_hash,struct adl_field *all_fields,
                                            int num_fields,const reloc_howto_type *relocs,int num_relocs)
{
  // By default, do nothing.
}

// By default, do pre-allocation so that we can handle packets of instructions.
bool __attribute__((weak)) acb_prealloc_instr_bufs()
{
  return true;
}

// By default, do nothing- we preallocate buffer space.
void __attribute__((weak)) acb_alloc_instr_buf(adl::InstrInfo & ATTRIBUTE_UNUSED)
{
}

// By default, use the normal expression parsing capability.
segT __attribute__((weak)) acb_parse_expr(expressionS *ex, const char *arg)
{
	// Parse the expression (which takes input from input_line_pointer).  Doesn't
	// currently handle 64-bit literals, which we might need to fix in the future.
	return expr(1, ex, expr_evaluate);
}

// By default, does nothing.
void __attribute__((weak)) acb_pop_insert()
{
}

// Any additional processing immediately after the options have been handled.
void __attribute__((weak)) acb_after_parse_args()
{
}

// Display any additional help information.
void __attribute__((weak)) acb_show_usage(FILE *)
{
}

//  Create a fix-up for a symbol used as a parameter of an assembler directive.
void adl_cons_fix_new(fragS *frag,
	int where,
	unsigned int nbytes,
	expressionS *exp,
	bfd_reloc_code_real_type r ATTRIBUTE_UNUSED)
{
	bfd_reloc_code_real_type reloc_type;


	switch (nbytes)
	{
	case 1:
		reloc_type = adl_get_reloc_type_by_name("_1byte");
		break;
	case 2:
		reloc_type = adl_get_reloc_type_by_name("_2byte");
		break;
	case 4:
		// Debug sections generate different relocation type for 4 bytes.
		if (now_seg->flags & SEC_ALLOC)
		{
			reloc_type = adl_get_reloc_type_by_name("_word");
		}
		else
		{
			reloc_type = adl_get_reloc_type_by_name("_4byte");
		}
		break;
	default:
		as_bad(_("unsupported BFD relocation size %u"), nbytes);
		reloc_type = adl_get_reloc_type_by_name("_4byte");
	}

	fix_new_exp(frag, where, static_cast<int>(nbytes), exp, 0, reloc_type);
}

/*
*  Do not make the relocation against the section's symbol because of linker
* optimization (stripping)
*/
bfd_boolean adl_fix_adjustable(fixS *fixP)
{
	return FALSE;
}

/*
*  This function is called when an assembler directive is encountered
*/
void adl_pseudo_op()
{
	adl_set_pseudo_op();
}

/*
*  Handles floating point definition assembler directives
*/
void adl_float_cons(int float_type)
{
	char type = TOLOWER(float_type);
	num_floats = 0;

	if (type == 'd') {
		if (enable_dprec) {
			float_cons(float_type);
		}
		else {
			as_bad("double precision float not supported");
			ignore_rest_of_line();
			return;
		}
	}
	else if (type == 'h') {
		if (enable_hprec) {
			float_cons(float_type);
		}
		else {
			as_bad("half precision float not supported");
			ignore_rest_of_line();
			return;
		}
	}
	else {
		float_cons(float_type);
	}

	if ((type == 'r' || type == 'h') && num_floats % 2) {
		as_bad("odd number of float constants");
	}
}

/*
*  Similar to "do_align" from "read.c"
*/
static void do_align(int n, char *fill, int len, int max)
{
	if (now_seg == absolute_section) {
		if (fill != NULL) {
			while (len-- > 0) {
				if (*fill++ != '\0') {
					as_warn(_("ignoring fill value in absolute section"));
					break;
				}
			}
		}

		fill = NULL;
		len = 0;
	}

#ifdef md_flush_pending_output
	md_flush_pending_output();
#endif

	adl_do_align(n, fill, len, max);

	record_alignment(now_seg, n - OCTETS_PER_BYTE_POWER);
}

/*
*  Handles the ".align" directive; it is the same as "s_align", except that it
* treats the alignment boundary as 4-byte words
*/
static void adl_s_align(int arg, int bytes_p)
{
	unsigned int align_limit = TC_ALIGN_LIMIT;
	unsigned int align;
	offsetT fill = 0;
	int max;
	int fill_p;

	if (is_end_of_line[(unsigned char)*input_line_pointer]) {
		align = arg; /* default value from pseudo-op table */
	}
	else {
		align = get_absolute_expression();
		SKIP_WHITESPACE();
	}

	/* align to 4-byte word boundary */
	align <<= bytes_p;

	/* convert to a power of 2 */
	if (align != 0) {
		unsigned int i;

		for (i = 0; (align & 1) == 0; align >>= 1, ++i) {
		}

		if (align != 1) {
			as_bad(_("alignment not a power of 2"));
		}

		align = i;
	}

	if (align > align_limit) {
		align = align_limit;
		as_warn(_("alignment too large: %u assumed"), align);
	}

	if (*input_line_pointer != ',') {
		fill_p = 0;
		max = 0;
	}
	else {
		++input_line_pointer;

		if (*input_line_pointer == ',') {
			fill_p = 0;
		}
		else {
			fill = get_absolute_expression();
			SKIP_WHITESPACE();
			fill_p = 1;
		}

		if (*input_line_pointer != ',') {
			max = 0;
		}
		else {
			++input_line_pointer;
			max = get_absolute_expression();
		}
	}

	if (!fill_p) {
		if (arg < 0) {
			as_warn(_("expected fill pattern missing"));
		}

		do_align(align, (char *)NULL, 0, max);
	}
	else {
		int fill_len;

		if (arg >= 0) {
			fill_len = 1;
		}
		else {
			fill_len = -arg;
		}

		if (fill_len <= 1) {
			char fill_char;

			fill_char = fill;
			do_align(align, &fill_char, fill_len, max);
		}
		else {
			char ab[16];

			if ((size_t)fill_len > sizeof ab) {
				abort();
			}

			md_number_to_chars(ab, fill, fill_len);
			do_align(align, ab, fill_len, max);
		}
	}

	demand_empty_rest_of_line();
}

/*
*  Machine dependent alignment, which creates symbols for the beginning and
* the end of the padding zone (used to dump the information in the "MW_INFO"
* section )
*/
void adl_do_align(int n, char *fill, int len, int max)
{
	/* create symbol before doing the alignment */
	symbolS *sym_begin = create_align_symbol(false);
	struct align_symbols *symbols;

	/* do the alignment */
	if (n != 0 && !need_pass_2) {
		if (fill == NULL) {
			if (subseg_text_p(now_seg)) {
				frag_align_code(n, max);
			}
			else {
				frag_align(n, 0, max);
			}
		}
		else if (len <= 1) {
			frag_align(n, *fill, max);
		}
		else {
			frag_align_pattern(n, fill, len, max);
		}
	}

	/* create symbol after doing the alignment */
	symbolS *sym_end = create_align_symbol(true);

	/* save the pair of alignment symbols */
	mwinfo_save_align_syms(sym_begin, sym_end);
}

/*
*  Handles the ".align" assembler directive
*/
void adl_align_word(int arg)
{
	// On VSPA3 there should be no shift
	adl_s_align(arg, 0);
}

/*
*  Sets the alignment information for a symbol
*/
void adl_sym_align(int ignore ATTRIBUTE_UNUSED)
{
	char *name = input_line_pointer;
	char c = get_symbol_end();
	char *p = input_line_pointer;
	expressionS exp;
	symbolS *symbolP;
	*p = c;

	if (name == p) {
		as_bad(_("expected symbol name"));
		ignore_rest_of_line();
		return;
	}

	SKIP_WHITESPACE();
	if (*input_line_pointer == ',')
		input_line_pointer++;

	expression_and_evaluate(&exp);
	if (exp.X_op != O_constant) {
		if (exp.X_op != O_absent) {
			as_bad(_("bad or irreducible absolute expression"));
		}
		else {
			as_bad(_("missing alignment expression"));
			ignore_rest_of_line();
			return;
		}

		exp.X_add_number = 0;
	}

	/* retrieve the symbol or create it */
	*p = 0;
	symbolP = symbol_find_or_make(name);
	*p = c;

	/* set the alignment in bytes */
	symbol_get_tc(symbolP)->alignment = exp.X_add_number << 2;

	demand_empty_rest_of_line();
}

/*
*  Handles the ".ref_by_addr" assembler directive, which specifies that a
* symbol is referenced by address
*/
void adl_ref_by_addr(int ignore ATTRIBUTE_UNUSED)
{
	char *name = input_line_pointer;
	char c = get_symbol_end();
	char *p = input_line_pointer;
	symbolS *symbolP;

	if (name == p) {
		*p = c;
		as_bad(_("expected symbol name"));
		ignore_rest_of_line();
		return;
	}

	/* retrieve the symbol or create it */
	symbolP = symbol_find_or_make(name);
	*p = c;

	/* set the ref_by_addr flag */
	symbol_get_tc(symbolP)->ref_by_addr = 1;

	demand_empty_rest_of_line();
}

/*
*  Handles the ".opt_mw_info" assembler directive, used to specify if the
* "MW_INFO" information regarding references by address is enabled or not
*/
void adl_opt_mw_info(int arg)
{
	expressionS exp;

	/* parse the value of the option */
	SKIP_WHITESPACE();
	expression_and_evaluate(&exp);
	if (exp.X_op != O_constant) {
		if (exp.X_op != O_absent) {
			as_bad(_("the only valid values for the MW_INFO option are 0 and 1"));
		}
		else {
			as_bad(_("missing value for the MW_INFO option"));
		}

		ignore_rest_of_line();
		return;
	}
	else if (exp.X_add_number < 0 || exp.X_add_number > 1) {
		as_bad(_("the only valid values for the MW_INFO option are 0 and 1"));
		ignore_rest_of_line();
		return;
	}

	/* set the "MW_INFO" option value */
	mwinfo_set_option(exp.X_add_number);

	demand_empty_rest_of_line();
}

/*
*  Handles the ".remove_block" assembler directive, which links a symbol that
* can be stripped by the linker with his corresponding block in the debug
* information section
*/
void adl_remove_block(int ignore ATTRIBUTE_UNUSED)
{
	char *name = input_line_pointer;
	char c = get_symbol_end();
	char *p = input_line_pointer;
	expressionS *exp;
	symbolS *symbol, *stripped_symbol;
	*p = c;

	if (name == p) {
		as_bad(_("expected block beginning symbol name"));
		ignore_rest_of_line();
		return;
	}

	SKIP_WHITESPACE();
	if (*input_line_pointer == ',')
		input_line_pointer++;

	exp = (expressionS *)malloc(sizeof(expressionS));
	expression_and_evaluate(exp);
	if (exp->X_op != O_constant && exp->X_op != O_subtract) {
		as_bad(_("only constant or difference of two symbols allowed"));
		ignore_rest_of_line();
		free(exp);
		return;
	}

	SKIP_WHITESPACE();
	if (*input_line_pointer == ',') {
		input_line_pointer++;
	}

	char *stripped_name = input_line_pointer;
	char stripped_c = get_symbol_end();
	char *stripped_p = input_line_pointer;
	*stripped_p = stripped_c;
	if (stripped_name == stripped_p) {
		as_bad(_("expected name for the symbol to be stripped"));
		ignore_rest_of_line();
		free(exp);
		return;
	}

	/* retrieve or create the block symbol */
	*p = 0;
	symbol = symbol_find_or_make(name);
	*p = c;

	/* retrieve or create the symbol that can be stripped */
	*stripped_p = 0;
	stripped_symbol = symbol_find_or_make(stripped_name);
	*stripped_p = stripped_c;

	/* save the information in order to be dumped in the "MW_INFO" section */
	mwinfo_save_remove_block(symbol, stripped_symbol, exp);

	demand_empty_rest_of_line();
}

/*
*  Handles the ".stack_effect" assembler directive, which specifies the
* stack effect of a function
*/
void adl_stack_effect(int ignore ATTRIBUTE_UNUSED)
{
	char *name = input_line_pointer;
	char c = get_symbol_end();
	char *p = input_line_pointer;
	expressionS exp;
	symbolS *symbolP;
	*p = c;

	if (name == p) {
		as_bad(_("expected symbol name"));
		ignore_rest_of_line();
		return;
	}

	SKIP_WHITESPACE();
	if (*input_line_pointer == ',')
		input_line_pointer++;

	expression_and_evaluate(&exp);
	if (exp.X_op != O_constant) {
		if (exp.X_op != O_absent) {
			as_bad(_("bad or irreducible absolute expression"));
		}
		else {
			as_bad(_("missing stack effect expression"));
			ignore_rest_of_line();
			return;
		}

		exp.X_add_number = 0;
	}

	/* retrieve the symbol or create it */
	*p = 0;
	symbolP = symbol_find_or_make(name);
	*p = c;

	/* set the stack_effect */
	symbol_get_tc(symbolP)->stack_effect = exp.X_add_number;

	demand_empty_rest_of_line();
}

//  Hook called after the symbol table is processed.
void adl_frob_file()
{
	mwinfo_dump_type_0();
	mwinfo_dump_type_6();
	mwinfo_dump_type_8();
	mwinfo_dump_type_10();
	mwinfo_dump_type_11();
	mwinfo_dump_type_13();
}

//  Update the location beginning and ending addresses for local typed symbols,
// which are computed using the virtual address and the size of the symbol's
// section.
void debug_loc_final_processing(bfd *abfd)
{
	// Check if there are any values to fill.
	if (!debug_loc_symbols.size())
	{
		return;
	}

	asection *loc_section = bfd_get_section_by_name(abfd, ".debug_loc");
	assert(loc_section);
	bfd_size_type size = bfd_get_section_size(loc_section);
	bfd_byte *contents = new bfd_byte[size];
	bfd_get_section_contents(abfd, loc_section, contents, 0, size);

	for (vector<pair<symbolS *, symbolS *> >::iterator syms
		= debug_loc_symbols.begin(); syms != debug_loc_symbols.end(); ++syms)
	{
		segT symbol_section = S_GET_SEGMENT(syms->first);
		struct bfd_elf_section_data *esd = elf_section_data(symbol_section);
		valueT beg_addr, end_addr, offset;

		// Beginning address.
		beg_addr = esd->this_hdr.sh_addr;
		// Ending address.
		end_addr = beg_addr + esd->this_hdr.sh_size;
		// Offset in section.
		offset = S_GET_VALUE(syms->second);

		// Write the values in the contents of the section.
		int entry_size = DWARF2_ADDR_SIZE(abfd);
		md_number_to_chars(reinterpret_cast<char *>(contents + offset),
			beg_addr, entry_size);
		md_number_to_chars(reinterpret_cast<char *>(contents + offset
			+ entry_size), end_addr, entry_size);
	}

	// Update the contents of the section.
	bfd_set_section_contents(abfd, loc_section, contents, 0, size);

	// Clear the list of symbol pairs since it is not needed anymore.
	debug_loc_symbols.clear();

	// Free the buffer used to store the contents of the section.
	delete[] contents;
}

/*
*  Hook called after the processing of the section headers is done
*/
void adl_final_write_processing(bfd *abfd, bfd_boolean linker)
{
	/* process the "MW_INFO" section content and write it */
	mwinfo_final_processing(abfd);
}

/*
*  Additionally processing the ELF section headers before writing them out
*/
bfd_boolean adl_section_processing(bfd *abfd, Elf_Internal_Shdr *shdr)
{
	/* only the ".MW_INFO" section header is processed */
	if (!shdr->bfd_section
		|| strcmp(bfd_get_section_name(abfd, shdr->bfd_section),
			MW_INFO_SECTION_NAME)) {
		return TRUE;
	}

	/* set section type */
	mwinfo_set_section_type(shdr);

	return TRUE;
}

//  Create a new unique temporary lable name.
static char *get_temp_label_name()
{
	// Counter for temporary symbol names.
	static unsigned long temp_label_count = 0;
	const char *prefix = "__temp_label_";
	size_t prefix_len = strlen(prefix);
	size_t len, index_len;
	char index[32];

	sprintf(index, "%ld", temp_label_count++);
	index_len = strlen(index);
	len = prefix_len + index_len + 1;

	// Create label name.
	char *name = static_cast<char *>(malloc(len));
	assert(name);
	memcpy(name, prefix, prefix_len);
	memcpy(name + prefix_len, index, index_len);
	name[len - 1] = 0;

	return name;
}

/*
*  Save a symbol name on a permanent obstack, and convert it according to the
* object file format
*/
static char *save_symbol_name(const char *name)
{
	unsigned int name_length;
	char *ret;

	name_length = strlen(name) + 1;	/* +1 for \0.  */
	obstack_grow(&notes, name, name_length);
	ret = (char *)obstack_finish(&notes);

#ifdef tc_canonicalize_symbol_name
	ret = tc_canonicalize_symbol_name(ret);
#endif

	if (!symbols_case_sensitive) {
		char *s;
		for (s = ret; *s != '\0'; s++) {
			*s = TOUPPER(*s);
		}
	}

	return ret;
}

/*
*  Adjust temporary symbols by replacing their names with new unique names
*/
static void adjust_temp_symbols()
{
	static const char local_label_char = '\002';

	for (symbolS *symp = symbol_rootP; symp; symp = symbol_next(symp))
	{
		if (strcmp(S_GET_NAME(symp), FAKE_LABEL_NAME)
			&& !strchr(S_GET_NAME(symp), local_label_char))
		{
			continue;
		}

		/* remove temporary dot symbols used in relocations; they are replaced by a
		* stack relocation which pushes the PC
		*/
		if (symbol_get_tc(symp)->is_dot && symbol_used_in_reloc_p(symp))
		{
			symbol_remove(symp, &symbol_rootP, &symbol_lastP);
			continue;
		}

		char *name = get_temp_label_name();
		char *preserved_name = save_symbol_name(name);
		S_SET_NAME(symp, preserved_name);
		free(name);
	}
}

//  Remove symbols corresponding to complex relocations from the symbol table.
static void remove_complex_reloc_symbols()
{
	for (symbolS *symp = symbol_rootP; symp; symp = symbol_next(symp))
	{
		if (!(symp->bsym->flags & (BSF_RELC | BSF_SRELC)))
		{
			continue;
		}

		complex_reloc_symbols.insert(symp);
		symbol_remove(symp, &symbol_rootP, &symbol_lastP);
	}

}

//  Final adjustments of the symbol table.
void adl_adjust_symtab()
{
	dih.generate_remove_block();

	remove_complex_reloc_symbols();
	mwinfo_dump_type_3();
	adjust_temp_symbols();
}

/*
*  Sets the file name and the line number where a certain symbol was defined
*/
void symbol_set_debug_info(symbolS *symbol)
{
	/* set the information only if the debug info generation is enabled */
	if (!dwarf2_loc_directive_seen && debug_type != DEBUG_DWARF2)
	{
		return;
	}

	char *file_name;
	unsigned int line_number;
	struct adl_sy *tc_sym = symbol_get_tc(symbol);

	/* get the filename and the line number where the symbol is declared */
	as_where(&file_name, &line_number);

	tc_sym->decl_file = file_name;
	tc_sym->decl_line = line_number;
}


//  Handle the assembler directives used to define common symbols; it sets
// the debug info for the defined symbol (only for VCPU).
void adl_common(int is_common)
{
	if (flag_mri && is_common)
	{
		s_mri_common(0);
		as_bad("Unsupported MRI mode for common symbol definition directive");
		ignore_rest_of_line();
	}
	else
	{
		symbolS *symbolP = s_comm_internal(0, elf_common_parse);
		if (core == CORE_VCPU)
		{
			symbol_set_debug_info(symbolP);
		}
	}
}

static segT vspa_get_section(int i)
{
	struct section_entry *seg = vspa_sections + i;

	if (seg->sec == NULL)
	{
		seg->sec = subseg_new(seg->name, 0);
		bfd_set_section_flags(stdoutput, seg->sec, seg->flags);
		if ((seg->flags & SEC_LOAD) == 0)
		{
			seg_info(seg->sec)->bss = 1;
		}

		if (seg->core == CORE_IPPU
			&& (bfd_get_section_flags(stdoutput, seg->sec) & SEC_CODE))
		{
			bfd_vma section_flags = elf_section_flags(seg->sec);
			elf_section_flags(seg->sec) = section_flags | SHF_IPPU;
		}
	}

	return seg->sec;
}

/*
*  Allocate space for a core specific common symbol in the corresponding
* bss section
*/
void vspa_bss_alloc(int core, symbolS *symbolP, addressT size, int align)
{
	char *pfrag;
	segT current_seg = now_seg;
	subsegT current_subseg = now_subseg;
	segT bss_seg;

	if (core == CORE_VCPU)
	{
		bss_seg = vspa_get_section(VCPU_BSS_SECTION);
	}
	else if (core == CORE_IPPU)
	{
		bss_seg = vspa_get_section(IPPU_BSS_SECTION);
	}
	else
	{
		bss_seg = bss_section;
	}

	subseg_set(bss_seg, 1);

	if (align)
	{
		record_alignment(bss_seg, align);
		frag_align(align, 0, 0);
	}

	if (S_GET_SEGMENT(symbolP) == bss_seg)
	{
		symbol_get_frag(symbolP)->fr_symbol = NULL;
	}

	symbol_set_frag(symbolP, frag_now);
	pfrag = frag_var(rs_org, 1, 1, 0, symbolP, size, NULL);
	*pfrag = 0;

#ifdef S_SET_SIZE
	S_SET_SIZE(symbolP, size);
#endif
	S_SET_SEGMENT(symbolP, bss_seg);

	subseg_set(current_seg, current_subseg);
}


/*
*  Parse the parameters for a core specific common symbol directive
*/
symbolS *vspa_common_parse(int core, symbolS *symbolP, addressT size)
{
	addressT align = 0;
	int is_local = symbol_get_obj(symbolP)->local;

	if (*input_line_pointer == ',')
	{
		char *save = input_line_pointer;

		input_line_pointer++;
		SKIP_WHITESPACE();

		if (*input_line_pointer == '"')
		{
			/* for sparc. accept .common symbol, length, "bss" */
			input_line_pointer++;
			/* some use the dot, some don't */
			if (*input_line_pointer == '.')
			{
				input_line_pointer++;
			}
			/* some say data, some say bss */
			if (strncmp(input_line_pointer, "bss\"", 4) == 0)
			{
				input_line_pointer += 4;
			}
			else if (strncmp(input_line_pointer, "data\"", 5) == 0)
			{
				input_line_pointer += 5;
			}
			else
			{
				char *p = input_line_pointer;
				char c;

				while (*--p != '"')
				{
				}

				while (!is_end_of_line[(unsigned char)*input_line_pointer])
				{
					if (*input_line_pointer++ == '"')
					{
						break;
					}
				}

				c = *input_line_pointer;
				*input_line_pointer = '\0';
				as_bad(_("bad .common segment %s"), p);
				*input_line_pointer = c;
				ignore_rest_of_line();
				return NULL;
			}

			is_local = 0;
		}
		else
		{
			input_line_pointer = save;
			align = parse_align(is_local);
			if (align == (addressT)-1)
			{
				return NULL;
			}
		}
	}

	if (is_local)
	{
		vspa_bss_alloc(core, symbolP, size, align);
		S_CLEAR_EXTERNAL(symbolP);
	}
	else
	{
		S_SET_VALUE(symbolP, size);
		S_SET_ALIGN(symbolP, align);
		S_SET_EXTERNAL(symbolP);
		if (core == CORE_VCPU)
		{
			S_SET_SEGMENT(symbolP, &bfd_vcom_section);
		}
		else if (core == CORE_IPPU)
		{
			S_SET_SEGMENT(symbolP, &bfd_icom_section);
		}

	}

	symbol_get_bfdsym(symbolP)->flags |= BSF_OBJECT;

	return symbolP;
}

//  Handle the assembler directives used to define common symbols for a
// specific core; it sets the debug info for the defined symbol (only for VCPU).
void adl_def_common(int core)
{
	symbolS *symbolP = s_comm_internal(core, vspa_common_parse);
	if (core == CORE_VCPU)
	{
		symbol_set_debug_info(symbolP);
	}
}



//  Handle the assembler directive used to define local common symbols; it
// sets the debug info for the defined symbol (only for VCPU).
void adl_local_common(int ignore ATTRIBUTE_UNUSED)
{
	symbolS *symbolP = s_comm_internal(0, s_lcomm_internal);

	if (symbolP)
	{
		symbol_get_bfdsym(symbolP)->flags |= BSF_OBJECT;
		if (core == CORE_VCPU)
		{
			symbol_set_debug_info(symbolP);
		}
	}
}

/*
*  Set the current core for, which the instructions are assembled
*/
void adl_set_core(int core_id)
{
	core = core_id;

	/* write all the remaining instructions form the queue */
	adl_cleanup();

	/* set the default text section */
	vspa_set_default_section();
}

/*
*  Stores a pair consisting of a typed symbol (variable) and its debug
* location information beginning symbol used to fill the beginning and ending
* address for the location of the typed symbol
*/
void add_debug_location_pair(symbolS *var, symbolS *loc)
{
	debug_loc_symbols.push_back(make_pair(var, loc));
}


/*
*  Hook called after the contents of the section are written
*/
bfd_boolean adl_after_write_object_contents(bfd *abfd)
{
	/* fill location beginning and ending addresses for local symbols */
	debug_loc_final_processing(abfd);

	return TRUE;
}

//  Set the file header flags to include the processor version, the AU
// configuration and the ABI version.
static void set_header_flags(bfd *abfd)
{
	unsigned au_flag;

	if (AU_NUM >= 2 && AU_NUM <= 64)
	{
		au_flag = bfd_log2(AU_NUM);
	}
	else
	{
		au_flag = 0;
	}

	if (isa_id == -1)
	{
		isa_id = get_default_ISA_id();
		/* disable warning when no ISA is set in command line, since no ISA option is a valid behavior (and will be further used for backward compatibility) */
//		as_warn("Command line option -isa <value> was not used. Will use default isa %s", get_default_ISA_name());
	}

	elf_elfheader(abfd)->e_flags =
		ELF32_EF_VSPA_FLAGS(VSPA_CORE, au_flag, isa_id, core_type);
}

/*
*  Create object file specific data; set the hook that is called after
* the contents of the section are written
*/
bfd_boolean adl_elf_make_object(bfd *abfd)
{
	bfd_boolean ret = bfd_elf_make_object(abfd);
	if (!ret)
	{
		return ret;
	}

	/* set the hook called after the contents of the sections are written */
	//elf_tdata(abfd)->after_write_object_contents =
	//	adl_after_write_object_contents;

	set_header_flags(abfd);

	return ret;
}

int vspa_march_parse_option(char *arg)
{
	if (!strncmp(arg, "prec", 4))
	{
		char *option = arg + 4;
		if (!strlen(option))
		{
			as_bad("option '-mprec' requires an argument");
			return 1;
		}
		else if (*option == '=')
		{
			option++;
			if (!strlen(option))
			{
				as_bad("option '-mprec' requires an argument");
				return 1;
			}

			string value(option);
			std::transform(value.begin(), value.end(), value.begin(),
				::tolower);

			if (!value.compare("h"))
			{
				enable_hprec = true;
			}
			else if (!value.compare("d"))
			{
				enable_dprec = true;
			}
			else if (!value.compare("dh") || !value.compare("hd"))
			{
				enable_hprec = enable_dprec = true;
			}
			else
			{
				as_bad("invalid '-mprec' option argument: \'%s\'", option);
			}
			return 1;
		}
	}
	else if (!strcmp(arg, "vcpu"))
	{
		core = CORE_VCPU;
		return 1;
	}
	else if (!strcmp(arg, "ippu"))
	{
		core = CORE_IPPU;
		return 1;
	}
	else if (!strncmp(arg, "au", 2))
	{
		int au = atoi(arg + 2);

		if (au < 2 || au > 64 || ((au - 1) & au))
		{
			as_bad("invalid value for the \'-mau\' option; the allowed "
				"values are 2|4|8|16|32|64");
		}
		else
		{
			AU_NUM = au;
		}
		has_nau_option = true;

		return 1;
	}
}

//  Check for VSPA specific assembler options.
int vspa_parse_option(int c, char *arg)
{
	if (c == OPTION_CORE_TYPE)
	{
		if (!strcmp(arg, "sp"))
		{
			core_type = core_type_sp;
		}
		else if (!strcmp(arg, "dp"))
		{
			core_type = core_type_dp;
		}
		else if (!strcmp(arg, "spl"))
		{
			core_type = core_type_spl;
		}
		else if (!strcmp(arg, "dpl"))
		{
			core_type = core_type_dpl;
		}
		else
		{
			as_fatal("invalid '-core_type' argument: \'%s\'", arg);
		}
		return 1;
	}
	else if (c == OPTION_ISA)
	{
		if (!is_valid_ISA(arg))
		{
			const char *tmp_name = get_default_ISA_name();
			as_warn("invalid isa option: \'%s\'. Will use default isa %s", arg, tmp_name);
			isa_id = get_ISA_id(arg);
			isa_name = (char*)malloc(strlen(tmp_name));
			strcpy(isa_name, tmp_name);
		}
		else
		{
			isa_id = get_ISA_id(arg);
			isa_name = (char*)malloc(strlen(arg));
			strcpy(isa_name, arg);
		}
		if (!has_nau_option)
		{
			AU_NUM = get_default_au_count_for_ISA_id(isa_id);
		}
		return 1;
	}
	
	return 0;
}

//  Display VSPA specific usage options.
void vspa_show_usage(FILE *stream)
{
	fprintf(stream, "-mprec=PREC\t\tEnable floating point precision PREC,"
		" where PREC is one of: D, H or both D and H.\n");
	fprintf(stream, "-mvcpu\t\tAssemble code for VCPU core (default)\n");
	fprintf(stream, "-mippu\t\tAssemble code for IPPU core\n");
	fprintf(stream, "-mau[2|4|8|16|32|64]\t\tProcessor AU version\n");
	fprintf(stream, "-core_type=TYPE\t\tCore precision type, where TYPE is "
		"one of: sp, dp, spl, dpl.\n");
	fprintf(stream, "-isa=TYPE\t\tISA type\n");
}

//  Transform a "const - symbol" expression into a "-symbol + const"
// expression; this is done to prevent generating a wrong addend for a
// potential relocation, given that for sections other than those used for
// debugging purposes a symbol's addend is specified in 4-byte words or 2-byte
// half-words (VSPA1/VSPA2). Also, constant expressions using logical shift
// operators like "const <<< const" or "const >>> const" are checked and
// resolved.
bfd_boolean adl_optimize_expression(expressionS *resultP,
	operatorT op_left,
	expressionS *right)
{
	if (op_left == O_subtract
		&& resultP->X_op == O_constant
		&& right->X_op == O_symbol)
	{
		resultP->X_op = O_uminus;
		resultP->X_add_symbol = right->X_add_symbol;
		resultP->X_add_number += right->X_add_number;
		return TRUE;
	}
	else if (((op_left == O_md1) || (op_left == O_md2))
		&& resultP->X_op == O_constant
		&& right->X_op == O_constant)
	{
		resultP->X_op = O_constant;
		valueT shift = static_cast<valueT>(right->X_add_number);

		if (shift >= sizeof(valueT) * CHAR_BIT)
		{
			as_warn_value_out_of_range(_("shift count"), shift, 0,
				sizeof(valueT) * CHAR_BIT - 1, NULL, 0);
			resultP->X_add_number = 0;
		}
		else
		{
			if (op_left == O_md1)
			{
				resultP->X_add_number = static_cast<offsetT>(
					static_cast<valueT>(resultP->X_add_number) << shift);
			}
			else
			{
				resultP->X_add_number = static_cast<offsetT>(
					static_cast<valueT>(resultP->X_add_number) >> shift);
			}
		}

		return TRUE;
	}

	return FALSE;
}

/*
*  Mark the symbol as being a dot symbol
*/
void adl_new_dot_label(symbolS *symbolP)
{
	symbol_get_tc(symbolP)->is_dot = 1;
}

/*
*  Get the section index for core specific common sections
*/
bfd_boolean adl_backend_section_from_bfd_section(bfd *abfd,
	asection *sec, int *retval)
{
	if (sec == &bfd_vcom_section)
	{
		*retval = SHN_LOPROC;
	}
	else if (sec == &bfd_icom_section)
	{
		*retval = SHN_LOPROC + 1;
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

/*
*  Sets the default section at the beginning of the process or whenever the
* user switches the core
*/
void vspa_set_default_section()
{
	segT sec;
	if (core == CORE_IPPU)
	{
		sec = vspa_get_section(IPPU_TEXT_SECTION);
	}
	else
	{
		sec = text_section;
	}

	subseg_set(sec, 0);
}

/*
*  Mark the section with a processor specific flag if it is a text section
* belonging to the IPPU core
*/
void adl_elf_section_change()
{
	if (core == CORE_IPPU
		&& (bfd_get_section_flags(stdoutput, now_seg) & SEC_CODE))
	{
		bfd_vma section_flags = elf_section_flags(now_seg);
		elf_section_flags(now_seg) = section_flags | SHF_IPPU;
	}
}

//  Parse a symbol name as a parameter of a directive and retrieve the symbol
// from the symbol table if existing or create it if not.
static symbolS *get_sym_from_input_line_and_check(void)
{
	char *name;
	char c;
	symbolS *sym;

	name = input_line_pointer;
	c = get_symbol_end();
	sym = symbol_find_or_make(name);
	*input_line_pointer = c;
	SKIP_WHITESPACE();

	// There is no symbol name if input_line_pointer has not moved.
	if (name == input_line_pointer)
	{
		as_bad(_("Missing symbol name in directive"));
	}
	return sym;
}

/*
*  Mark symbol as allowing multiple definitions
*/
void adl_elf_multidef(int ignore ATTRIBUTE_UNUSED)
{
	int c;
	symbolS *symbolP;

	do
	{
		symbolP = get_sym_from_input_line_and_check();
		c = *input_line_pointer;
		// First we set the symbol weak in order to do conversion from local if
		// needed.
		if (S_IS_WEAK(symbolP))
		{
			char *file;
			unsigned line;
			as_where(&file, &line);
			as_warn_where(file, line, _("can't  make weak symbol multidef"));
		}
		else
		{
			S_SET_WEAK(symbolP);
			symbolP->bsym->flags &= ~(BSF_WEAK);
			symbolP->bsym->flags |= BSF_MULTIDEF;
		}

		if (c == ',')
		{
			input_line_pointer++;
			SKIP_WHITESPACE();
			if (*input_line_pointer == '\n')
			{
				c = '\n';
			}
		}
	} while (c == ',');

	demand_empty_rest_of_line();
}

//  Handle cusotom operators. It is used to accommodate the logical shift
// operators "<<<" and ">>>".
operatorT vspa_operator(char *name, int mode, char *end)
{
	// No custom unary operators.
	if (mode == 1)
	{
		return O_absent;
	}

	if (name == NULL && end == NULL)
	{
		return O_illegal;
	}

	*input_line_pointer = *end;
	operatorT op = O_illegal;
	size_t num_chars = 0;
	assert(name);
	if (!strncmp(name, "<<<", 3))
	{
		// Logical shift left.
		num_chars = 3;
		op = O_md1;
	}
	else if (!strncmp(name, ">>>", 3))
	{
		// Logical shift right.
		num_chars = 3;
		op = O_md2;
	}
	else if (!strncmp(name, "<<", 2))
	{
		num_chars = 2;
		op = O_left_shift;
	}
	else if (!strncmp(name, ">>", 2))
	{
		num_chars = 2;
		op = O_right_shift;
	}
	else if (!strncmp(name, "<=", 2))
	{
		num_chars = 2;
		op = O_le;
	}
	else if (!strncmp(name, ">=", 2))
	{
		num_chars = 2;
		op = O_ge;
	}
	else if (name[0] == '<')
	{
		num_chars = 1;
		op = O_lt;
	}
	else if (name[0] == '>')
	{
		num_chars = 1;
		op = O_gt;
	}

	// Move the input line pointer after the end of the operator.
	if (op != O_illegal)
	{
		input_line_pointer = name + num_chars;
		*end = *input_line_pointer;
	}

	return op;
}

//  Collect function start and end symbols and register functions with the
// debug information handler.
void collect_func_info()
{
	if (!dwarf2_loc_directive_seen && debug_type != DEBUG_DWARF2)
	{
		return;
	}

	for (symbolS *sym = symbol_rootP; sym; sym = symbol_next(sym))
	{
		if (!S_IS_FUNCTION(sym))
		{
			continue;
		}

		// Search for F<func_name>_end.
		stringstream ss;
		const char *sym_name = S_GET_NAME(sym);
		ss << "F" << sym_name << "_end";
		symbolS *end_sym = symbol_find(ss.str().c_str());
		if (end_sym
			&& (!S_IS_EXTERNAL(end_sym) || S_IS_EXTERNAL(sym)))
		{
			dih.add_function(sym, end_sym);
			continue;
		}

		// Search for FuncEnd<func_name>.
		ss.str("");
		ss << "FuncEnd" << sym_name;
		end_sym = symbol_find(ss.str().c_str());
		if (end_sym)
		{
			dih.add_function(sym, end_sym);
			continue;
		}
		assert(end_sym == NULL);

		offsetT sym_size = 0;
		elf_obj_sy *sy_obj = symbol_get_obj(sym);
		if (sy_obj->size)
		{
			if (resolve_expression(sy_obj->size)
				&& sy_obj->size->X_op == O_constant)
			{
				sym_size = sy_obj->size->X_add_number;
			}
			else
			{
				as_bad(_(".size expression for %s does not evaluate to a "
					"constant."), sym_name);
			}
		}
		else
		{
			sym_size = S_GET_SIZE(sym);
		}

		if (!sym_size)
		{
			TC_SYMFIELD_TYPE *sym_decl = symbol_get_tc(sym);
			as_warn_where(sym_decl->decl_file, sym_decl->decl_line, _("Missing"
				" definition of end symbol \'F%s_end\' or \'FuncEnd%s\' for "
				"function %s. The assembler was not able to generate the end "
				"symbol automatically because the size of the function's "
				"symbol is either undefined or zero."), sym_name, sym_name,
				sym_name);
			continue;
		}

		// Automatically generate end symbol for function.
		end_sym = symbol_new(ss.str().c_str(), S_GET_SEGMENT(sym),
			S_GET_VALUE(sym) + sym_size, sym->sy_frag);
		dih.add_function(sym, end_sym);
	}
}

//  Force alignment of code sections to the size of the program memory word.
static void adjust_sec_alignment(bfd *abfd, asection *sec,
	void *arg ATTRIBUTE_UNUSED)
{
	if (!(bfd_get_section_flags(stdoutput, sec) & SEC_CODE))
	{
		return;
	}

	// The code sections should be aligned to program memory word size, which
	// is 64 bits (2^3 bytes) for VCPU and 32 (2^2 bytes) bits for IPPU.
	size_t min_align = (elf_section_flags(sec) & SHF_IPPU) ? 2 : 3;

	if (bfd_get_section_alignment(abfd, sec) < min_align)
	{
		bfd_set_section_alignment(abfd, sec, min_align);
	}
}

//  Hook called after relocation generation.
void vspa_frob_file_after_relocs()
{
	// Adjust alignment of code sections.
	bfd_map_over_sections(stdoutput, adjust_sec_alignment, NULL);
}

extern "C" {
	//  Register ".debug_line" start.
	void register_debug_line_start(symbolS *sym, symbolS *start)
	{
		dih.add_debug_line_start(sym, start);
	}

	//  Register ".debug_line" end.
	void register_debug_line_end(symbolS *sym, symbolS *end)
	{
		dih.add_debug_line_end(sym, end);
	}

	//  Register ".debug_info" start and end.
	void register_debug_info(symbolS *sym, symbolS *start, symbolS *end)
	{
		dih.add_debug_info(sym, start, end);
	}

	// Check if a symbol corresponds to a function end that doesn't have a
	// ".debug_line" end symbol set.
	bool is_func_end(symbolS *sym)
	{
		return dih.is_func_end(sym, true);
	}

	// Check if a symbol corresponds to a function start that doesn't have a
	// ".debug_line" start symbol set.
	bool is_func_start(symbolS *sym)
	{
		return dih.is_func_start(sym, true);
	}

	// Check if a symbol corresponds to a function start/
	bool is_func_start_no_check(symbolS *sym)
	{
		return dih.is_func_start(sym);
	}
}

//  Hook called after relocation generation.
void adl_check_debug_info()
{
	// Check function size == (func_end(f) - f);
	dih.check_func_size();
}
