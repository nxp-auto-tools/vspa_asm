//
// Machine independent functions
//


#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <cctype>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

extern "C"
{
# include "xregex.h"
# include "as.h"
# include "safe-ctype.h"
# include "dwarf2dbg.h"
}

#include "adl-asm-impl.h"
#include "adl-asm-info.h"
#include "adl-asm-common.h"
#include "mw-info.h"

#if defined(_MSC_VER) && (_MSC_VER == 1500)
using namespace std::tr1;
#endif

#if defined(_VSPA2_)
# include "asm-translate.h"
#endif

#define NUM_CORES 2

using namespace std;
using namespace adl;

extern bool dmem_coff;
extern bool dmem_mvip;

#if defined(_VSPA2_)
# define JMP_JSR_INST "jmp/jsr/loop_break/swi"
extern bool vspa1_on_vspa2;
#else
# define JMP_JSR_INST "jmp/jsr"
#endif

static void init_line_assemble(uint32_t &line_number)
{
  char *file_name;
  as_where(&file_name, &line_number);
};

#ifndef LONG_MIN
# define LONG_MIN (-2147483647L - 1)
#endif
#ifndef LONG_MAX
# define LONG_MAX 2147483647L
#endif

extern "C" {

	/* Characters which are used to indicate an exponent in a floating
	point number.  */
	extern const char EXP_CHARS[] = "eE";

	/* Characters which mean that a number is a floating point constant,
	as in 0d1.0.  */
	extern const char FLT_CHARS[] = "hHrRsSfFdD";

	/* We define a '-m' option for instruction table selection. */
	const char *md_shortopts = "l:b:m:d";

	const char *EMPTY_STRING = "";

	extern const pseudo_typeS md_pseudo_table[] =
	{
		/* define words with size of 4 octets (instead of 2 octets) */
		{"word", cons, 4},
		{"hfloat", adl_float_cons, 'h'},
		{"hfixed", adl_float_cons, 'r'},
		{"single", adl_float_cons, 's'},
		{"float", adl_float_cons, 'f'},
		{"double", adl_float_cons, 'd'},
		{"align", adl_align_word, 2},
		{"symbol_align", adl_sym_align, 0},
		{"remove_block", adl_remove_block, 0},
		{"ref_by_addr", adl_ref_by_addr, 0},
		{"opt_mw_info", adl_opt_mw_info, 0},
		{"comm", adl_common, 0},
		{"common", adl_common, 1},
		{"common.s", adl_common, 1},
		{"vcomm", adl_def_common, CORE_VCPU},
		{"icomm", adl_def_common, CORE_IPPU},
		{"lcomm", adl_local_common, 1},
		{"vcpu", adl_set_core, CORE_VCPU},
		{"ippu", adl_set_core, CORE_IPPU},
		{"multidef", adl_elf_multidef, 0},
		{"stack_effect", adl_stack_effect, 0 },
		{ NULL, NULL, 0 }
	};

	/* The target specific pseudo-ops which we support.  */

	extern const struct option md_longopts[] = {
#ifdef _VSPA2_
		{"vspa1-on-vspa2", no_argument, NULL, OPTION_VSPA1_ON_VSPA2},
#else
		{"engr297095-on", no_argument, NULL, OPTION_ENGR297095_ON},
		{"engr297095-off", no_argument, NULL, OPTION_ENGR297095_OFF},
		{"cmpvspa741-on", no_argument, NULL, OPTION_CMPVSPA784_ON},
		{"cmpvspa741-off", no_argument, NULL, OPTION_CMPVSPA784_OFF },
#endif
		{"core_type", required_argument, NULL, OPTION_CORE_TYPE},
		
		{NULL, no_argument, NULL, 0}
	};
	extern const size_t md_longopts_size = sizeof(md_longopts);

	extern symbolS *section_symbol(segT);
}

//  Structure that keeps the information specific to an address corresponding to
// beginning of a macroinstruction.
struct address_info
{
	segT seg;
	fragS *frag;
	valueT where;

	//  Constructor.
	address_info(segT _seg, fragS *_frag, valueT _where) :
		seg(_seg),
		frag(_frag),
		where(_where)
	{
	}
};

//  Loop information value.
class loop_info_val
{
public:
	//  Default constructor.
	loop_info_val()
	{
	}

	//  The loop information value is unknown.
	virtual bool is_unknown() const
	{
		return false;
	}

	//  The loop information value is kept in a register.
	virtual bool is_reg() const
	{
		return false;
	}

	//  The loop information value is constant.
	virtual bool is_const() const
	{
		return false;
	}

	//  The loop information value is expressed as a difference of symbols.
	virtual bool is_diff() const
	{
		return false;
	}

	//  Get the numeric value of the loop iteration count.
	virtual size_t get_value() const
	{
		return 0;
	}

	//  Check if the difference of symbols is equal to zero.
	virtual bool diff_is_zero() const
	{
		return false;
	}

	//  Clone method.
	virtual loop_info_val *clone() const = 0;

	//  Destructor.
	virtual ~loop_info_val()
	{
	}
};

//  Unknown loop information value.
class loop_info_val_unkn : public loop_info_val
{
public:
	//  The loop information value is unknown.
	bool is_unknown() const
	{
		return true;
	}

	//  Clone method.
	loop_info_val_unkn *clone() const
	{
		return new loop_info_val_unkn();
	}
};

//  Register loop information value.
class loop_info_val_reg : public loop_info_val
{
public:
	//  The loop information value is kept in a register.
	bool is_reg() const
	{
		return true;
	}

	//  Clone method.
	loop_info_val_reg *clone() const
	{
		return new loop_info_val_reg();
	}
};

//  Constant loop information value.
class loop_info_val_const : public loop_info_val
{
private:
	size_t value_;
public:
	//  Constructor.
	loop_info_val_const(size_t value) :
		value_(value)
	{
	}

	//  Copy constructor.
	loop_info_val_const(const loop_info_val_const &livc) : loop_info_val(livc),
		value_(livc.value_)
	{
	}

	//  Clone method.
	loop_info_val_const *clone() const
	{
		return new loop_info_val_const(*this);
	}

	//  The loop information value is constant.
	bool is_const() const
	{
		return true;
	}

	//  Get the numeric value of the loop information.
	size_t get_value() const
	{
		return value_;
	}
};

//  Loop information value expressed as a difference of symbols.
class loop_info_val_diff : public loop_info_val
{
private:
	// Symbols corresponding to the beginning and the end of the hardware loop
	// body.
	symbolS *begin_, *end_;
public:
	//  Constructor.
	loop_info_val_diff(symbolS *begin, symbolS *end) :
		begin_(begin),
		end_(end)
	{
	}

	//  Copy constructor.
	loop_info_val_diff(const loop_info_val_diff &livd) : loop_info_val(livd),
		begin_(livd.begin_),
		end_(livd.end_)
	{
	}

	// Clone method.
	loop_info_val_diff *clone() const
	{
		return new loop_info_val_diff(*this);
	}

	//  The loop information value is expressed as a difference of symbols.
	bool is_diff() const
	{
		return true;
	}

	//  Check if the difference of symbols is equal to zero.
	bool diff_is_zero() const
	{
		return S_GET_VALUE(begin_) == S_GET_VALUE(end_);
	}
};

//  Loop iteration count and size information.
struct loop_info
{
private:
	//  Free dynamically allocated memory.
	void destroy()
	{
		delete iter_cnt;
		delete size;
	}
public:
	// Iteration count and size values.
	loop_info_val *iter_cnt, *size;

	//  Constructor.
	loop_info() :
		iter_cnt(new loop_info_val_unkn()),
		size(new loop_info_val_unkn())
	{
	}

	// Operator=.
	const loop_info &operator=(const loop_info &li)
	{
		if (&li != this)
		{
			destroy();
			iter_cnt = li.iter_cnt->clone();
			size = li.size->clone();
		}

		return *this;
	}

	//  Copy constructor.
	loop_info(const loop_info &li) :
		iter_cnt(li.iter_cnt->clone()),
		size(li.size->clone())
	{
	}

	//  Clone method.
	loop_info *clone() const
	{
		return new loop_info(*this);
	}

	//  Destructor.
	~loop_info()
	{
		destroy();
	}
};

//  Structure that keeps change-of-flow specific information.
struct coff_info
{
	enum cof_type type;
	symbolS *target;
	offsetT imm;
	fragS *frag;
	valueT where;

	//  Constructor.
	coff_info() :
		type(cof_no_type),
		target(NULL),
		imm(0),
		frag(NULL),
		where(0)
	{
	}
};

//  Structure that keeps section specific change-of-flow and DMEM access
// information.
struct section_flags
{
	// Flag for a DMEM access microinstruction contained in the previous bundle
	// or the bundle before the previous.
	bool prev_dmem;
	bool prev2_dmem;
	// Flag for a "rts" microinstruction contained in the previous bundle or
	// the bundle before the previous.
	bool prev_rts;
	bool prev2_rts;
	// Flag for a "jmp" or "jsr" microinstruction contained in the previous
	// bundle or the bundle before the previous.
	bool prev_jmp_jsr;
	bool prev2_jmp_jsr;
	// Conditional/unconditional "jmp"/"jsr" macroinstruction contained
	// in the previous bundle or the bundle before the previous.
	struct jmp_jsr_info prev_jj_info;
	struct jmp_jsr_info prev2_jj_info;
	
	// "loop_begin" instruction included in the previous bundle.
	bool prev_loop_begin;
	// "loop_end" instruction included in the previous bundle.
	bool prev_loop_end;

	// Loop information applying to the previous bundle.
	const loop_info *prev_loop_info;

	// Counts the number of macroinstructions between the last control
	// instruction (jmp, jsr, rts) and the previous instruction bundle.
	size_t prev_control_counter;
	// Counts the number of macroinstructions between the last control
	// instruction (jmp, jsr, rts) and the current instruction bundle
	size_t control_counter;

	// Information corresponding to a "jmp", "jsr" or "rts" microinstruction
	// contained in the previous bundle or the bundle before the previous.
	struct coff_info prev_coff_info;
	struct coff_info prev2_coff_info;

	//  Constructor.
	section_flags() :
		prev2_jmp_jsr(false),
		prev_jmp_jsr(false),
		prev2_jj_info({ false, false }),
		prev_jj_info({ false, false }),
		prev2_rts(false),
		prev_rts(false),
		prev2_dmem(false),
		prev_dmem(false),
		prev_loop_begin(false),
		prev_loop_end(false),
		prev_loop_info(new loop_info()),
		prev_control_counter(2),
		control_counter(3),
		prev2_coff_info(coff_info()),
		prev_coff_info(coff_info())
	{
	}

	//  Copy constructor.
	section_flags(const section_flags &sf) :
		prev2_jmp_jsr(sf.prev2_jmp_jsr),
		prev_jmp_jsr(sf.prev_jmp_jsr),
		prev2_jj_info(sf.prev2_jj_info),
		prev_jj_info(sf.prev_jj_info),
		prev2_rts(sf.prev2_rts),
		prev_rts(sf.prev_rts),
		prev2_dmem(sf.prev2_dmem),
		prev_dmem(sf.prev_dmem),
		prev_loop_begin(sf.prev_loop_begin),
		prev_loop_end(sf.prev_loop_end),
		prev_loop_info(new loop_info(*sf.prev_loop_info)),
		prev_control_counter(sf.prev_control_counter),
		control_counter(sf.control_counter),
		prev2_coff_info(sf.prev2_coff_info),
		prev_coff_info(sf.prev_coff_info)
	{
	}

	//  Increment control counter after propagating the current value.
	void inc_control_counter(bool reset)
	{
		prev_control_counter = control_counter;
		if (reset)
		{
			control_counter = 0;
		}
		else
		{
			control_counter++;
		}
	}

	//  Decrement control counter without propagating the current value.
	void dec_control_counter()
	{
		control_counter--;
	}
};

//  Mapping between sections and their corresponding instruction type flags.
class section_state
{
private:
	typedef unordered_map<segT, struct section_flags *> sf_map;
	//  Map that links sections with their coressponding flags.
	sf_map *sec_flags;

	//  Free dynamically allocated memory.
	void destroy()
	{
		for (sf_map::iterator sf_it = sec_flags->begin();
			sf_it != sec_flags->end(); ++sf_it)
		{
			struct section_flags *sf = sf_it->second;
			if (sf->prev_loop_info)
			{
				delete sf->prev_loop_info;
			}
			delete sf;
		}

		delete sec_flags;
	}

public:
	//  Constructor.
	section_state()
	{
		sec_flags = new sf_map();
	}

	//  Copy constructor.
	section_state(const section_state &ss)
	{
		sec_flags = new sf_map();
		for (sf_map::iterator sf_it = ss.sec_flags->begin();
			sf_it != ss.sec_flags->end(); ++sf_it)
		{
			(*sec_flags)[sf_it->first] = new section_flags(*sf_it->second);
		}
	}

	//  Operator=.
	const section_state &operator=(const section_state &ss)
	{
		if (&ss != this)
		{
			destroy();
			sec_flags = new sf_map();
			for (sf_map::iterator sf_it = ss.sec_flags->begin();
				sf_it != ss.sec_flags->end(); ++sf_it)
			{
				(*sec_flags)[sf_it->first] = new section_flags(*sf_it->second);
			}
		}

		return *this;
	}

	//  Destructor.
	~section_state()
	{
		destroy();
	}

	//  Retrieve the flags corresponding to a certain section. If the section
	// doesn't have any flags associated yet, a new entry is created for it.
	struct section_flags *get_section_flags(segT sec)
	{
		sf_map::iterator sf_it = sec_flags->find(sec);
		
		if (sf_it == sec_flags->end())
		{
			struct section_flags *sf = new struct section_flags();
			(*sec_flags)[sec] = sf;
			return sf;
		}
		else
		{
			return sf_it->second;
		}
	}

	//  Update the flags corresponding to a specific section.
	void update_section_flags(segT sec, const bool &rts, const bool &jmp_jsr,
							  const struct jmp_jsr_info &jj_info,
							  const bool &dmem, const struct coff_info &info,
							  const bool &loop_begin, const bool &loop_end,
							  const struct loop_info *li)
	{
		struct section_flags *sf = get_section_flags(sec);

		sf->prev2_rts = sf->prev_rts;
		sf->prev2_jmp_jsr = sf->prev_jmp_jsr;
		sf->prev2_jj_info = sf->prev_jj_info;
		sf->prev2_dmem = sf->prev_dmem;
		sf->prev2_coff_info = sf->prev_coff_info;

		sf->prev_rts = rts;
		sf->prev_jmp_jsr = jmp_jsr;
		sf->prev_jj_info = jj_info;
		sf->prev_dmem = dmem;
		sf->prev_coff_info = info;
		sf->prev_loop_begin = loop_begin;
		sf->prev_loop_end = loop_end;
		if (sf->prev_loop_info)
		{
			delete sf->prev_loop_info;
		}
		sf->prev_loop_info = li;
	}
};

//  Pool of dynamically allocated strings.
class string_pool
{
private:
	//  Custom hash function for C strings.
	struct hash_cstr
	{
		size_t operator() (const char *str) const
		{
			return hash<string>()(str);
		}
	};

	//  Custom equal key function for C strings.
	struct key_equal_cstr
	{
		bool operator()(const char *str1, const char *str2) const
		{
			return !strcmp(str1, str2);
		}
	};

	// Set of dynamically allocated strings.
	unordered_set<const char *, struct hash_cstr, struct key_equal_cstr> pool_;

public:
	//  Add lowercase version of string if not present.
	const char *add_lowercase(const char *str)
	{
		string new_str(str);
		std::transform(new_str.begin(), new_str.end(), new_str.begin(),
			::tolower);
		unordered_set<const char *, struct hash_cstr,
			struct key_equal_cstr>::iterator str_it 
				= pool_.find(new_str.c_str());
		if (str_it == pool_.end())
		{
			string::size_type len = new_str.size() + 1;
			char *st = new char[len];
			memcpy(st, new_str.c_str(), len);
			pool_.insert(st);
			return st;
		}

		return *str_it;
	}

	//  Destructor.
	~string_pool()
	{
		for (unordered_set<const char *, struct hash_cstr,
			struct key_equal_cstr>::iterator string_it = pool_.begin();
			string_it != pool_.end(); ++string_it)
		{
			delete[] *string_it;
		}
	}
};

//  Stack containing loop specific information.
class loop_info_val_stack
{
private:
	// Internal stack of loop iteration information.
	vector<struct loop_info *> *stack;

	//  Free dynamically allocated memory.
	void destroy()
	{
		for (vector<struct loop_info *>::iterator stack_it = stack->begin();
			stack_it != stack->end(); ++stack_it)
		{
			delete *stack_it;
		}
		delete stack;
	}
public:
	//  Constructor.
	loop_info_val_stack()
	{
		stack = new vector<struct loop_info *>();
		push();
	}

	//  Copy constructor.
	loop_info_val_stack(const loop_info_val_stack &livs)
	{
		stack = new vector<struct loop_info *>();
		for (vector<struct loop_info *>::iterator stack_it
			= livs.stack->begin(); stack_it != livs.stack->end(); ++stack_it)
		{
			stack->push_back(new struct loop_info(**stack_it));
		}
	}

	//  Operator=.
	const loop_info_val_stack &operator=(const loop_info_val_stack &livs)
	{
		if (&livs != this)
		{
			destroy();
			stack = new vector<struct loop_info *>();
			for (vector<struct loop_info *>::iterator stack_it
				= livs.stack->begin(); stack_it != livs.stack->end();
				++stack_it)
			{
				stack->push_back(new struct loop_info(**stack_it));
			}
		}
		return *this;
	}

	//  Destructor.
	~loop_info_val_stack()
	{
		destroy();
	}

	//  Push a default entry on the stack.
	void push()
	{
		stack->push_back(new loop_info());
	}

	//  Pop the entry from the top of the stack.
	void pop()
	{
		if (!stack->empty())
		{
			struct loop_info *li = stack->back();
			stack->pop_back();
			delete li;
		}
	}

	//  Get the entry from the top of stack.
	struct loop_info *top()
	{
		if (stack->empty())
		{
			return new loop_info();
		}
		else
		{
			return stack->back();
		}
	}

	//  Update the iteration count of the entry from the top of the stack.
	void update_iter(struct loop_info_val *liv)
	{
		if(!stack->empty())
		{
			delete (*stack)[stack->size() - 1]->iter_cnt;
			(*stack)[stack->size() - 1]->iter_cnt = liv;
		}
	}

	//  Update the size of the entry from the top of the stack.
	void update_size(struct loop_info_val *liv)
	{
		if(!stack->empty())
		{
			delete (*stack)[stack->size() - 1]->size;
			(*stack)[stack->size() - 1]->size = liv;
		}
	}
};

//  "loop_begin" instruction address location information.
class loop_begin_stack
{
private:
	typedef vector<struct address_info *> loop_begin_vec;
	// Stack containing address specific information corresponding to
	// "loop_begin" instructions.
	loop_begin_vec *stack_;

	//  Free dynamically allocated memory.
	void destroy()
	{
		for (loop_begin_vec::iterator stack_it = stack_->begin();
			stack_it != stack_->end(); ++stack_it)
		{
			delete *stack_it;
		}

		delete stack_;
	}

public:
	//  Constructor.
	loop_begin_stack() :
		stack_(new loop_begin_vec())
	{
	}

	//  Copy constructor.
	loop_begin_stack(const loop_begin_stack &lbs) :
		stack_(new loop_begin_vec())
	{
		for (loop_begin_vec::iterator stack_it = lbs.stack_->begin();
			stack_it != lbs.stack_->end(); ++stack_it)
		{
			stack_->push_back(new struct address_info(**stack_it));
		}
	}

	//  Operator=.
	const loop_begin_stack &operator=(const loop_begin_stack &lbs)
	{
		if (&lbs != this)
		{
			destroy();
			stack_ = new loop_begin_vec();
			for (loop_begin_vec::iterator stack_it = lbs.stack_->begin();
				stack_it != lbs.stack_->end(); ++stack_it)
			{
				stack_->push_back(new struct address_info(**stack_it));
			}
		}

		return *this;
	}

	//  Destructor.
	~loop_begin_stack()
	{
		destroy();
	}

	//  Push the information related to the current address location on the
	// stack.
	void push()
	{
		struct address_info *lbi = new struct address_info(now_seg,
			frag_now, frag_now_fix());
		stack_->push_back(lbi);
	}

	//  Remove the entry from the top of the stack.
	void pop()
	{
		if (stack_->empty())
		{
			return;
		}

		struct address_info *lbi = stack_->back();
		stack_->pop_back();
		delete lbi;
	}

	//  Get the entry from the top of the stack.
	struct address_info *top()
	{
		if (stack_->empty())
		{
			return NULL;
		}
		else
		{
			return stack_->back();
		}
	}
};

// Flag to indicate whether to check assembler rules.
static int check_asm_rules = 1;

// Flag to indicate assembler rule violations
int num_rule_errors = 0;

// This string stores the set of characters we use to terminate the
// instruction-name token.  The minimal set does not include characters also
// seen in instruction names.
static const char *terminating_chars     = 0;
static const char *min_terminating_chars = 0;

// Here we store comment definitons that are not single characters (e.g. "//");
static const char **comment_strs = 0;
static int num_comment_strs = 0;

// The same for line comments (valid only at the beginning of the line).
const char **line_comment_strs = 0;

int num_line_comment_strs = 0;

// Packet grouping characterss
static const char *instr_separators   = 0;
static const char *packet_begin_chars = 0;
static const char *packet_end_chars   = 0;
static const char *default_packet_sep = 0;
static const char *expected_end = 0; 
static int   queue_size = 1;
static bool adl_show_warnings = false;
static bool adl_try_longest_instr = false; // --big-mem

// Prefix fields
struct adl_prefix_fields *prefix_fields ATTRIBUTE_UNUSED = 0; 
static int  num_prefix_fields ATTRIBUTE_UNUSED = 0;

// Prefix instructions
struct adl_instr *prefix_instrs = 0;
int num_prefix_instrs = 0;
unsigned max_prefix_size = 0;


// Line label symbol
symbolS *curr_label = 0;

// Pre assembler handler, before each instruction is assembled, used to force encoding
static adl_pre_asm_t pre_asm_handler[NUM_CORES] = {0, 0};

// Post assembler handler, after each instruction updates prefix fields
static adl_post_asm_t post_asm_handler[NUM_CORES] = {0, 0};

// Post packet assembler handler, after each instructions packet
static adl_post_packet_asm_t post_packet_asm_handler[NUM_CORES] = {0, 0};

// Post packet rule checker, after each packet
static adl_post_packet_checker_t post_packet_rule_checker[NUM_CORES] = {0, 0};

// For reseting prefix counters.
static adl_reset_prefix_counters_t reset_prefix_counters_handler[NUM_CORES] = {0, 0};

// Stores all instructions indexed by ADL name.
static struct hash_control *all_instrs_hash[NUM_CORES] = { 0, 0 };

using modifier_map_t = unordered_map<modifier_t, const struct modifier_info *>;
// Modifier information related to the used operands.
static modifier_map_t modifier_map[NUM_CORES];

static struct adl_name_pair *fields_by_index[NUM_CORES]     = {0, 0};
static int                   num_fields_by_index[NUM_CORES] = {0, 0};
static int                   num_fields[NUM_CORES]          = {0, 0};

// Pointers to installed information about relocations, if relevant.
static const reloc_howto_type *relocs = 0;
static const struct adl_name_pair *relocs_by_index = 0;
static const struct adl_int_pair *reloc_offsets = 0;
static int num_relocs = 0;
static int num_relocs_by_index = 0;
static int num_reloc_offsets = 0;

/* last bundle of microinstructions */
static InstrBundle last_instr_bundle;
/* level of loop imbrication */
static int in_loop = 0;
/* an assembler directive is encountered */
static bool assembler_directive = false;

/* the fragment which contains the space allocated for the last bundle */
static fragS *last_frag = NULL;

/* flag for case insensitivity of instruction mnemonics and operands */
bool ignore_case = true;

/* counts the float numbers received by an assembler directive */
int num_floats;

// Save dynamically allocated strings (instruction mnemonics and register/mode
// names) when the case is ingored in order to reclaim the allocated memory at
// the end.
static string_pool *str_pool = new string_pool();

// Mapping between sections and the instruction type flags related to them.
section_state *ss = new section_state();

// Stack containing loop specific information.
static loop_info_val_stack *lis = new loop_info_val_stack();

// Stack containing "loop_begin" address information.
static loop_begin_stack *lbs = new loop_begin_stack();

// Instruction resulted from expanding VSPA1 "st Iu17, I32" pseudoinstruction.
static char *expand_instr = NULL;

/* flag for skipping the checking for a pseudoinstruction */
static bool skip_macro_check = false;

/* flag for skipping the checking for a generic instruction */
static bool skip_generic_check = false;

/* the labels before the current macroinstruction */
static vector<symbolS *> labels;

extern bool enable_dprec;
extern bool enable_hprec;

extern size_t num_vcpu_modifiers;
extern struct modifier_info vcpu_modifiers[];
extern size_t num_ippu_modifiers;
extern struct modifier_info ippu_modifiers[];

void adl_set_check_asm_rules(int i)
{
  check_asm_rules = i;
}

// Handle multichar comments here.
static void handle_comments (char *str) {
  int i;
  for (i = 0 ;i != num_comment_strs; ++i) {
    const char *comm = comment_strs[i];
    char *p;
    if ((p = strstr(str,comm))) {
      *p = '\0';
      return;
    }
  }
}

// Stores the current instruction table index
int current_table ATTRIBUTE_UNUSED = 0; // TBD use it

extern void s_switch (int arg);

static const pseudo_typeS potable[] = {
  {"switch", s_switch, 0},
  {NULL, NULL, 0} /* End sentinel.  */
};

typedef struct {
  int field_id;
  int iface_id;
  int value;
  const char *enum_value;
} PreSetInfo;

static PreSetInfo   PreSetInfos[MAX_INSTR_FIELDS];
static int PreSetInfoCtr = 0;

string InstrInfo::empty_string("");

InstrBundles instrInfos;                   // Holds all instruction information 

static bool parallel_arch = false;         // True is we have parallel architecture
static bool assembler_fields = false;      // True if we have assembler fields
static bool prefix_saved = false;          // True:  savePrefix has been called.

static unsigned curgrp() { return instrInfos.current_group(); }

// Check if an opcode has a particular attribute,
bool opcode_has_attr(const struct adl_opcode *opcode, unsigned attr)
{
	if (!opcode || !opcode->attrs)
	{
		return false;
	}

	return (opcode->attrs->attrs & (1ULL << attr));
}

// Return true if an instruction has a given attribute.
bool InstrInfo::instrHasAttr(unsigned attr) const
{
	return opcode_has_attr(opcode, attr);
}

// Return true if an instruction has a given attribute.
bool InstrInfo::instrSrcHasAttr(unsigned attr) const
{
	return opcode_has_attr(src_opcode, attr);
}

// Return true if an instruction has an attribute and that attribute has a give
// numerical value.
bool InstrInfo::instrHasAttrVal(unsigned attr, unsigned val) const
{
  if (!opcode || !opcode->attrs)
  {
    return false;
  }

  // Make sure the attribute exists and some values exist.
  if (!(opcode->attrs->attrs & (1ULL << attr))
      || !opcode->attrs->vals)
  {
    return false;
  }

  // Now search the array
  const adl_instr_attr_val *v = opcode->attrs->vals;
  while (v->attr)
  {
    if (v->attr == (1ULL << attr)
        && !v->str && v->num == val)
    {
      return true;
    }
    ++v;
  }
  return false;
}

// Return true if an instruction has an attribute and that attribute has a
// given string value.
bool InstrInfo::instrHasAttrVal(unsigned attr, const string &val) const
{
  if (!opcode || !opcode->attrs)
  {
    return false;
  }

  // Make sure the attribute exists and some values exist.
  if (!(opcode->attrs->attrs & (1ULL << attr))
      || !opcode->attrs->vals)
  {
    return false;
  }

  // Now search the array
  const adl_instr_attr_val *v = opcode->attrs->vals;
  while (v->attr)
  {
    if (v->attr == (1ULL << attr)
        && v->str && val == v->str)
    {
      return true;
    }
    ++v;
  }
  return false;
}

// Check if an opcode has an attribute, in which case its value is returned
bool opcode_get_attr_val(struct adl_opcode *opcode, unsigned attr,
    std::string &val)
{
  if (!opcode || !opcode->attrs)
  {
    return false;
  }

  /* make sure the attribute exists and some values exist */
  if (!(opcode->attrs->attrs & (1ULL << attr)) || !opcode->attrs->vals)
  {
    return false;
  }

  /* now search the array */
  const adl_instr_attr_val *v = opcode->attrs->vals;
  while (v->attr)
  {
    if ( v->attr == (1ULL << attr) && v->str)
    {
      val.append(v->str);
      return true;
    }
    ++v;
  }
  return false;
}

// Return true if an instruction has an attribute and the value of the attribute
// if present
bool InstrInfo::instrGetAttrVal(unsigned attr, std::string &val) const
{
    return opcode_get_attr_val(src_opcode, attr, val)
        || opcode_get_attr_val(opcode, attr, val);
}

void InstrInfo::alloc_operand_values(int p_num_operand_values)
{
  num_operand_values = p_num_operand_values;
  free(operand_values); 
  if (p_num_operand_values) {
    operand_values = (expressionS*)xmalloc(sizeof(expressionS)*num_operand_values);
    memset(operand_values,0,sizeof(expressionS)*num_operand_values);
  } else {
    operand_values = 0;
  }
}

// This changes the opcode and resizes the operand_values array, if necessary.
void InstrInfo::set_opcode(adl_opcode *p_opcode)
{
  opcode = p_opcode;
  if (opcode->num_operands > num_operand_values) {
    expressionS *new_opvals = (expressionS*)xmalloc(sizeof(expressionS)*opcode->num_operands);
    memset(new_opvals,0,sizeof(expressionS)*opcode->num_operands);
    if (operand_values) {
      memcpy(new_opvals,operand_values,sizeof(expressionS)*num_operand_values);
    }
    free(operand_values);
    operand_values = new_opvals;
    num_operand_values = opcode->num_operands;
  }
}

void InstrInfo::init(char *p_s,
    adl_opcode *p_opcode,
    adl_opcode *p_src_opcode,
    expressionS *p_operand_values,
    const Args &raw_args,
    int p_line_number,
    int p_maxfields,
    int p_instr_table,
    const char *p_instr_table_name)
{
  str = p_s;
  opcode = p_opcode;
  src_opcode = p_src_opcode;

  int max_arg = -1;
  for (int i = 0; i!= p_opcode->num_operands; ++i) {
    const struct adl_operand *operand = &p_opcode->operands[i];
    max_arg = max(max_arg,operand->arg);
    if (operand->arg >= raw_args.size()) {
      // We might not have some args if we're dealing with assembler
      // instructions.
      args.push_back(string());
    } else {
      args.push_back(string(raw_args[operand->arg].str,raw_args[operand->arg].len));
    }
  }

  int num_operands = p_opcode->num_operands;
  if (p_src_opcode && p_src_opcode->num_operands > num_operands)
  {
    num_operands = p_src_opcode->num_operands;
  }
  alloc_operand_values(max(max_arg + 1, num_operands));
  memcpy(operand_values,p_operand_values,sizeof(expressionS)*num_operand_values);

  line_number = p_line_number;
  instr_table = p_instr_table;
  instr_table_name = p_instr_table_name;
  maxfields = p_maxfields;
}

// For a given bundle, set prefix data for each instruction.
void InstrBundle::set_prefix_fields()
{
  // Reset all counters.
  (reset_prefix_counters_handler[core])();

  // Iterate over all instructions.  If a prefix setter exists, then run it.
  int index = (_has_prefix) ? 1 : 0;
  for (iterator iter = begin(); iter != end(); ++iter,++index) {
    struct adl_operand *operands = iter->opcode->operands;
    for (int j=0; j < iter->opcode->num_operands; ++j) {    
      struct adl_operand &operand = operands[j];
      if (operand.ptr->pfx_setter) {
        expressionS *ex = &iter->operand_values[operand.arg];
        // If a prefix-counter function exists, then query it for the index to
        // use.
        int pindex = (iter->opcode->prefix_counter) ? (iter->opcode->prefix_counter)(operand.ptr->id,index) : index;
        // Then set the value.
        (operand.ptr->pfx_setter)(ex->X_add_number,pindex,curgrp());
      }
    }
    // Then adjust any counters, if necessary.
    if (iter->opcode->mod_prefix_counters) {
      (iter->opcode->mod_prefix_counters)();
    }
  }
}

// Reset prefix fields values to defaults.
static void reset_prefix(int grp) {
  int i;
  struct adl_prefix_field **pfx_fields = prefix_fields[grp].fields;
  for (i = 0; i != num_prefix_fields; ++i) {
    pfx_fields[i]->value = pfx_fields[i]->default_value;
  }
}

// This must be called *early* so that we set up the output file properly.
void adl_setup_endianness(bool big_endian)
{
  target_big_endian = big_endian;
}

//  Check for a "set.loop" instruction that uses labels for the beginning and
// the end of the loop, in which case the three operand instruction it's
// replaced with a two operand instruction, where the second operand consists
// of the difference between the end label and the beginning label.
void handle_set_loop_with_labels(InstrInfo &instr)
{
	if (!check_set_loop_labels(instr))
	{
		return;
	}

	expressionS expr;
	memset(&expr, 0, sizeof(expressionS));
	expr.X_op = O_subtract;
	expr.X_add_symbol = instr.operand_values[2].X_add_symbol;
	expr.X_op_symbol = instr.operand_values[1].X_add_symbol;
	memset(&instr.operand_values[2], 0, sizeof(expressionS));
	memcpy(&instr.operand_values[1], &expr, sizeof(expressionS));
}

//  Check for instructions that are not yet implemented in the simulator.
static void check_unimplemented_instr(InstrInfo &instr)
{
	if (check_unimplemented(instr))
	{
		as_warn("VSPA2 \'%s\' instruction is not available in the simulator.",
			instr.str.c_str());
	}
}

//  Check for invalid instructions.
static void check_invalid_instr(InstrInfo &instr)
{
	if (check_invalid(instr))
	{
		as_warn("\'%s\' is an invalid instruction.", instr.str.c_str());
	}
}

//  Check restrictions for multiple registers save/restore instructions
// ("pushm", "popm", "stm", "ldm").
static void check_variable_length_instr(InstrInfo &instr)
{
	bool mask_32;
	if (!check_var_instr(instr, mask_32))
	{
		return;
	}

	const string &instr_str = instr.str;
	string::size_type pos = instr_str.find(' ');
	if (pos == string::npos)
	{
		return;
	}

	const string &instr_args = instr_str.substr(pos + 1);
	char *args = strdup(instr_args.c_str());
	assert(args);
	for (char *p = args; *p; ++p)
	{
		*p = TOLOWER(*p);
	}

	static const size_t max_regs = 32;
	size_t min_index = max_regs, max_index = 0, num_regs = 0;
	stringstream ss("");
	bool indexes[max_regs];
	std::fill_n(indexes, max_regs, false);
	bool rag_regs = false;
	for (char *arg = strtok(args, ","); arg; arg = strtok(NULL, ","))
	{
		size_t val = 0, factor = 1;
		char *p;
		for (p = &arg[strlen(arg) - 1]; p >= arg && ISDIGIT(*p); --p)
		{
			val += factor * (*p - '0');
			factor *= 10;
		}

		if (factor == 1)
		{
			if (!strcmp(arg, "rv"))
			{
				val = 3;
			}
			else if (!strcmp(arg, "rst"))
			{
				val = 4;
			}
			else
			{
				continue;
			}
		}

		size_t index, len =  p - arg + 1;
		// There are 5 RAG register sets: rS0 rS1 rS2  rV rSt.
		// Mask index in instruction:       4   3   2   1   0.
		const static size_t max_rag_idx = 4;
		if (len == 1)
		{
			if (*arg == 'a')
			{
				if (mask_32 || val < 4)
				{
					index = val + 12;
				}
				else
				{
					index = val - 4;
				}
			}
			else if (*arg = 'g')
			{
				index = val;
			}
			else
			{
				continue;
			}
		}
		else if (len == 2)
		{
			if (*arg == 'a' && *(arg + 1) == 's')
			{
				index = val;
			}
			else if (*arg == 'r' && (*(arg + 1) == 's' || *(arg + 1) == 'v'))
			{
				index = max_rag_idx - val;
				rag_regs = true;
			}
		}
		else if ((len == 3
			&& *arg == 'r'
			&& *(arg + 1) == 's'
			&& *(arg + 2) == 't'))
		{
			index = max_rag_idx - val;
			rag_regs = true;
		}
		else
		{
			continue;
		}

		if (indexes[index])
		{
			as_bad("The list of saved/restored registers contains duplicates:"
				" %s", arg);
			free(args);
			return;
		}
		else
		{
			indexes[index] = true;
		}

		ss << arg << ",";

		if (index < min_index)
		{
			min_index = index;
		}
		if (index > max_index)
		{
			max_index = index;
		}
		num_regs++;
	}

	// The multiple save/restore instructions can handle as much registers as
	// can fit into a data memory line (2 * AU_NUM 32-bit words).
	// Each aX, asX and gX register occupies one 32-bit word on the stack
	// => maximum 2 * AU_NUM registers.
	// Each rX register set occupies 8 16-bit slots (4 32-bit words), one slot
	// for each component register => maximum AU_NUM / 2 registers.
	const size_t max_num_regs = (rag_regs ? AU_NUM / 2 : AU_NUM * 2);
	if (num_regs > max_num_regs)
	{
		as_bad("Exceeded maximum number of %u saved/restored registers: %u",
			max_num_regs, num_regs);
	}
	// For rX registers any combination can be used, unlike aX, asX or gX,
	// where the registers should belong to the same partition.
	else if (!rag_regs
		&& (max_index / max_num_regs) != (min_index / max_num_regs))
	{
		string str = ss.str();
		if (!str.empty())
		{
			str[str.size() - 1] = '\0';
		}
		as_bad("Invalid combination of saved/restored registers: \"%s\"",
			str.c_str());
	}

	free(args);
}

//  Check if an instruction is a "st Iu17, I32" pseudoinstruction.
bool check_st_Iu17_I32(InstrInfo &instr)
{
	if (strncasecmp(instr.str.c_str(), "st ", 3)
		|| instr.opcode->num_operands != 2)
	{
		return false;
	}

	// Check that the range of the first operand matches Iu17.
	const size_t min_val = 0, max_val = (1 << 17) - 1;
	adl_opcode *opcode = instr.opcode;
	for (size_t i = 0, e = opcode->num_operands; i < e; i++)
	{
		adl_operand &operand = opcode->operands[i];
		if (operand.arg == 0)
		{
			if ((operand.flags & ADL_ABSOLUTE)
				&& (operand.lower == min_val)
				&& (operand.upper == max_val))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	return false;
}

//  Handle VSPA1 "st Iu17, I32" pseudoinstruction, which results in two
// instructions when the second operand is an immediate that doesnt't fit in 16
// bits or a symbolic expression. Also, if the second operand is a negative
// value, a "st.s Iu17, Is16" instruction is generated.
void handle_pseudoinstructions(InstrInfo &instr)
{
	if (!instr.src_opcode || !check_is_pseudo(instr.opcode))
	{
		return;
	}

	assert(check_st_Iu17_I32(instr));
	expressionS *expr = &instr.operand_values[1];
	if (expr->X_op == O_constant)
	{
		offsetT add_number = expr->X_add_number;
		bool sign = (add_number & 0x00008000)
			&& (add_number & 0xFFFF0000);

#define SIGN_EXT_32B_TO_64B_MASK 0xFFFFFFFF80000000LL
#define ZERO_EXT_32B_TO_64B_MASK 0x80000000LL
		
		// Floating point or integer value that doesn't fit in 16 bits.
		// Must make sure that the value is a 32 bit value that is 
		// sign extended or zero extended.
		if (expr->X_md > 16
			|| (!expr->X_md
				&& ((add_number & SIGN_EXT_32B_TO_64B_MASK) == SIGN_EXT_32B_TO_64B_MASK // negative, sign extended 32bit int
					|| (add_number & SIGN_EXT_32B_TO_64B_MASK) == 0x0 // positive 32bit int
					|| (add_number & SIGN_EXT_32B_TO_64B_MASK) == ZERO_EXT_32B_TO_64B_MASK) //negative, zero extended 32bit int
				&& (add_number & 0xFFFF8000) != 0xFFFF8000 //not a negative 16bit int
				&& (add_number & 0xFFFF0000) != 0)) //not a positive 16bit int
		{
			// Update the operand of the instruction to store only the 16 least
			// significant bits of the value.
			unsigned value = ((add_number >> 16) & 0xFFFF);
			expr->X_add_number &= 0xFFFF;

			// Create a second instruction that stores the 16 most significant
			// bits of the value.
			stringstream ss;
			ss << "st.h " << instr.args[0] << "," << value;
			expand_instr = strdup(ss.str().c_str());
			assert(expand_instr);
		}

		if (sign)
		{
			string opcode;
			if (get_instance_name(instr, opcode))
			{
				instr = replaceInstr(instr, opcode);
			}
		}
	}
	else
	{
		// Update the operand of the instruction to store only the 16 least
		// significant bits of the expression.
		expressionS new_expr;
		memset(&new_expr, 0, sizeof(expressionS));
		new_expr.X_op = O_bit_and;
		new_expr.X_add_symbol = make_expr_symbol(expr);
		new_expr.X_op_symbol=  expr_build_uconstant(0xFFFF);
		instr.operand_values[1] = new_expr;

		// Create a second instruction that stores the 16 most significant
		// bits of the expression.
		stringstream ss;
		ss << "st.h " << instr.args[0] << ",((" << instr.args[1]
			<< ")>>16)&0xFFFF";
		expand_instr = strdup(ss.str().c_str());
		assert(expand_instr);
	}
}

//  Handle operations executed on instructions right after they are parsed.
void vspa_post_asm(unsigned width, int group)
{
	InstrBundle &bundle = instrInfos[group];
	unsigned size = bundle.group_size();
	if (size == 0)
	{
		return;
	}

	InstrInfo &instr = bundle.get_instr(size - 1);

	handle_pseudoinstructions(instr);
	handle_set_loop_with_labels(instr);
	check_variable_length_instr(instr);
	check_instr_restrictions(instr);
	check_invalid_instr(instr);
#if defined(_VSPA2_)
	check_unimplemented_instr(instr);
#endif
}

//  Free the memory allocated for strings corresponding to the lower case
// versions of the instruction mnemonics and register/mode names.
static void free_string_pool()
{
	delete str_pool;
}

//  Initialize modifier information map.
static void init_modifier_info(int core)
{
	const struct modifier_info *entries;
	size_t num_entries;

	if (core == CORE_VCPU)
	{
		num_entries = num_vcpu_modifiers;
		entries = vcpu_modifiers;
	}
	else
	{
		num_entries = num_ippu_modifiers;
		entries = ippu_modifiers;
	}

	auto &map = modifier_map[core];
	for (size_t i = 0; i < num_entries; i++)
	{
		const struct modifier_info *mi = &entries[i];
		map[mi->modifier] = mi;
	}
}

// General purpose setup stuff.
void adl_setup_general(const char *mtc,
	const char *tc,
	bool is_parallel,
	adl_pre_asm_t pre_asm,
	adl_post_asm_t post_asm,
	adl_post_packet_asm_t post_packet_asm,
	adl_post_packet_checker_t post_packet_checker,
	int q_size,
	bool show_warnings,
	bool big_mem)
{
	adl_show_warnings = show_warnings;
	adl_try_longest_instr = big_mem;
	terminating_chars = tc;
	min_terminating_chars = mtc;
	parallel_arch = is_parallel;

	pre_asm_handler[init_core] = pre_asm;
	post_asm_handler[init_core]  = post_asm;
	post_packet_asm_handler[init_core] = post_packet_asm;
	post_packet_rule_checker[init_core] = post_packet_checker;

	// Add in the .switch pseudo-op so that we can switch instruction tables in
	// the middle of a file.
	pop_insert(potable);
	queue_size = q_size;
	instrInfos.init(q_size);
	if (!init_core)
	{
#if defined(_VSPA2_)
		if (vspa1_on_vspa2)
		{
			load_instr_mapping(ignore_case);
		}
#endif
		mwinfo_section_init();

		// Schedule a routine to free the memory allocated for instruction
		// mnemonics and register/mode names.
		if (ignore_case)
		{
			xatexit(free_string_pool);
		}
	}

	if (init_core == CORE_VCPU && post_asm == NULL)
	{
		post_asm_handler[init_core] = vspa_post_asm;
	}

	init_modifier_info(init_core);
}

void adl_setup_comments(const char **cs, int ncs, const char **lcs, int nlcs) 
{
  comment_strs = cs;
  num_comment_strs = ncs;
  line_comment_strs = lcs;
  num_line_comment_strs = nlcs;
}

void adl_setup_instr_separators(const char *instr_separators_p)
{
  instr_separators = instr_separators_p;
}

void adl_setup_grouping( const char *pbc, const char *pec)
{
  packet_begin_chars = pbc;
  packet_end_chars   = pec;
  int i = strlen(packet_begin_chars);
  if (i >= 1 && (packet_begin_chars[i-1] == packet_end_chars[i-1])) { // Add support for a single "\n" as packet grouping symbol
    default_packet_sep = &packet_begin_chars[i-1]; 
  } else {
    default_packet_sep = 0; 
  }
}

void adl_setup_prefix(struct adl_prefix_fields *pfx_fields, int num_pfx_fields, struct adl_instr* pfx_instrs ATTRIBUTE_UNUSED,
                      int num_pfx_instrs ATTRIBUTE_UNUSED,adl_reset_prefix_counters_t reset_prefix_counters)
{
  prefix_fields = pfx_fields;
  num_prefix_fields = num_pfx_fields;
  prefix_instrs  = pfx_instrs;
  num_prefix_instrs = num_pfx_instrs;
  int i;
  for (i = 0; i != num_prefix_instrs; ++i) {
    struct adl_opcode* opcode = &(prefix_instrs[i].opcodes[0]);
    if (opcode->size > max_prefix_size) {
      max_prefix_size = opcode->size;
    }
  }
  for(i=0; i != queue_size; ++i) {
    reset_prefix(i);
  }
  reset_prefix_counters_handler[init_core] = reset_prefix_counters;
}
// This process .switch pseudo-ops.  The argument is passed to a call-back, which
// should then process is.
void s_switch(int argc ATTRIBUTE_UNUSED) // TBD
{
  const int BufSize = 500;
  char arg[BufSize];
  char *s;

  int n = 0;

  if (*input_line_pointer++ != '-') { // TBD
    AS_FATAL(_("Improperly formed .switch directive."));
  }

  // Get the command.
  int c = *input_line_pointer++;

  // Now get the argument.
  for (s = input_line_pointer;
       ((!is_end_of_line[(unsigned char) *input_line_pointer] && *input_line_pointer != ' ' && *input_line_pointer != '\t')); 
       input_line_pointer++,n++);
  strncpy(arg,s,n);
  arg[n] = 0;
  md_parse_option(c,arg);
  demand_empty_rest_of_line();
}

struct hash_control *adl_setup_asm_instructions (struct adl_asm_instr *instructions,int num_instructions)
{
  struct hash_control *instr_hash;
  instr_hash = hash_new(); 

  // Now populate it.
  {
    struct adl_asm_instr *instr  = instructions;
    struct adl_asm_instr *instr_end = instructions + num_instructions;

    for ( ; instr != instr_end; ++instr) {
      // Insert the instruction.
      const char *rc = hash_insert(instr_hash,instr->name,(PTR)instr);
      if (rc != NULL) {
        AS_FATAL(_("Internal assembler error for instruction %s:  %s"),instr->name,rc);
      }
    }
  }

  return instr_hash;
}

struct hash_control *adl_setup_instr_pfx_fields (const char *field_values[],int num_items)
{
  struct hash_control *field_hash;
  field_hash = hash_new(); 

  // Now populate it.
  {
    const char **fv     = field_values;
    const char **fv_end = field_values + num_items;

    for ( ; fv != fv_end; ++fv) {
      // Insert the instruction.
      const char *rc = hash_insert(field_hash,*fv,(void*)1);
      if (rc != NULL) {
        AS_FATAL(_("Internal assembler error for instruction-prefix-field value %s:  %s"),*fv,rc);
      }
    }
  }

  return field_hash;
}

// This intalls the enumerated fields values hash and the by-index array used
// for error/warning messages.
void adl_setup_instrfields(struct adl_field *fields, int num_fields_p,
	struct adl_name_pair *fields_by_index_p, int num_fields_by_index_p)
{
  fields_by_index[init_core]     = fields_by_index_p;
  num_fields[init_core]          = num_fields_p;
  num_fields_by_index[init_core] = num_fields_by_index_p;
}

// Stores any relocation information.
void adl_setup_relocations(const reloc_howto_type *relocs_p, int num_relocs_p,
    const struct adl_name_pair *relocs_by_index_p,int num_relocs_by_index_p,
    const struct adl_int_pair *reloc_offsets_p, int num_reloc_offsets_p)
{
  relocs          = relocs_p;
  num_relocs      = num_relocs_p;

  relocs_by_index     = relocs_by_index_p;
  num_relocs_by_index = num_relocs_by_index_p;

  reloc_offsets = reloc_offsets_p;
  num_reloc_offsets = num_reloc_offsets_p;
}

bool adl_have_custom_relocs()
{
  return num_relocs != 0;
}

static void setup_operands(adl_opcode *op,adl_field *operands)
{
  for (int f = 0; f != op->num_operands; ++f) {
    if (op->operands[f].index < num_fields[init_core]) {
      op->operands[f].ptr = &operands[op->operands[f].index];
    }
    if (operands[op->operands[f].index].assembler || operands[op->operands[f].index].pfx_setter) {
      assembler_fields = true;
    }
  }
}

//  This installs the opcodes into the instruction hash.
struct hash_control *adl_setup_instructions(struct adl_instr *instructions,
	int num_instructions,
	struct adl_field *operands)
{
	struct hash_control *instr_hash;
	regex_t *rec;
	int reg_err;

	if (!all_instrs_hash[init_core])
	{
		all_instrs_hash[init_core] = hash_new();
	}

	// Create the hash.
	instr_hash = hash_new();

	// Now populate it.
	for (struct adl_instr *instr = instructions, *instr_end = instructions
		+ num_instructions; instr != instr_end; ++instr)
	{
		// Reorder opcodes to implement family precedence restrictions.
		reorder_opcodes(instr);

		// Insert the instruction.
		const char *rc;
		if (ignore_case)
		{
			rc = hash_insert(instr_hash, str_pool->add_lowercase(instr->name),
				(PTR) instr);
		}
		else
		{
			rc = hash_insert(instr_hash, instr->name, (PTR) instr);
		}

		if (rc != NULL)
		{
			AS_FATAL(_("Internal assembler error for instruction %s: %s"),
				instr->name, rc);
		}

		// For each opcode-entry associated with this instruction name, perform
		// any needed setup.
		for (struct adl_opcode *op = instr->opcodes, *op_end = instr->opcodes
			+ instr->num_opcodes; op != op_end; ++op)
		{
			// Insert entry into the all-instructions-hash. Duplicates may
			// exist if multiple tables reference the same instruction, so
			// ignore an 'exists' error.
			const char *rc = hash_insert(all_instrs_hash[init_core], op->name,
				(PTR)op);

			// Compile the regular expression.
			rec = static_cast<regex_t *>(xmalloc(sizeof(regex_t)));
			if (!rec)
			{
				AS_FATAL(_("Error in allocating memory for regex of '%s'"),
					instr->name);
			}

			int reg_cflags = REG_EXTENDED;
			if (ignore_case)
			{
				reg_cflags |= REG_ICASE;
			}
			// Fix regular expression for opcodes with no operands.
			if (!strcmp(op->regex, "$"))
			{
				reg_err = regcomp(rec, "^$", reg_cflags);
			}
			else
			{
				reg_err = regcomp(rec, op->regex, reg_cflags);
			}

			if (reg_err != 0)
			{
				const int BufSize = 80;
				char msg[BufSize];
				regerror(reg_err, rec, msg, BufSize);
				regfree(rec);
				AS_FATAL(_("Error in compiling regex for %s:%s"), instr->name,
					msg);
			}
			op->regcomp = rec;

			// Initialize the operand arrays.
			setup_operands(op,operands);
			for (int t_iter = 0; t_iter != op->num_alias_targets; ++t_iter)
			{
				setup_operands(&op->alias_targets[t_iter], operands);
			}
		}
	}

	return instr_hash;
}

// This installs register names into the register name hash.
struct hash_control *adl_setup_name_hash(const struct adl_name_pair *names,
	int num_names,
	const char *msg)
{
	struct hash_control *hash;

	// Create the hash.
	hash = hash_new();

	// Now populate it.
	const struct adl_name_pair *n_end = names + num_names;
	for (const struct adl_name_pair *n = names; n != n_end; ++n)
	{
		// Insert the value.

		const char *rc;
		if (ignore_case)
		{
			rc = hash_insert(hash, str_pool->add_lowercase(n->name),
				(PTR) n);
		}
		else
		{
			rc = hash_insert(hash, n->name, (PTR) n);
		}

		if (rc != NULL)
		{
			AS_FATAL(_("Internal assembler error for %s %s: %s"), msg, n->name,
				rc);
		}
	}

	return hash;
}

bool namePairComp(const adl_name_pair &a,const adl_name_pair &b) { return a.index < b.index; };

// Given a field index, return a useful name, or 0 if not found.
static const char *get_field_name(unsigned index)
{
  adl_name_pair tmp;
  tmp.name  = 0;
  tmp.index = index;

  pair<adl_name_pair*,adl_name_pair*> p =
      equal_range(&fields_by_index[core][0],
                  &fields_by_index[core][num_fields_by_index[core]],
                  tmp,
                  namePairComp);

  if (p.first != p.second)
  {
    return p.first->name;
  }
  else
  {
    return 0;
  }
}

// Same as above, but always returns something.  If not found, returns "unknown".
static const char *get_field_name_safe(unsigned index)
{
  if (const char *n = get_field_name(index)) {
    return n;
  } else {
    return "<unknown>";
  }
}

static void set_constant(expressionS *expr, offsetT value)
{
  expr->X_op = O_constant;
  expr->X_add_number = value;
  expr->X_add_symbol = NULL;
  expr->X_op_symbol = NULL;
}

template <typename T>
struct NameComp {
  bool operator()(const T &x,const T &y) {
    return strcmp(x.name,y.name) < 0;
  }
  bool operator()(const T &x,const char *y) {
    return strcmp(x.name,y) < 0;
  }
  bool operator()(const char *x,const T &y) {
    return strcmp(x,y.name) < 0;
  }
};

// Generic binary search.  Expects the object to have a name member of const
// char *.  Returns a pointer to the item, or 0 if not found.
template <typename T>
T *bfind(T *array,unsigned num,const char *arg)
{
  T tmp;
  tmp.name = arg;
  pair<T*,T*> p = equal_range(array,&array[num],tmp,NameComp<T>());
  if (p.first != p.second) {
    return p.first;
  } else {
    return 0;
  }
}

// Generic binary search.  Expects the object to have a name member of const
// char *.  Returns a pointer to the item, or 0 if not found.  Hack: I want this
// to be a constant comparison, but that's difficult with using a length, since
// only the input has an extent, but the data array contains normal 0-terminated
// strings.  So, temporarily, I 0-terminate the input.
template <typename T>
T *bfind_n(T *array,unsigned num,const char *arg,int len)
{
  T val;
  val.name = arg;
  char tmp = arg[len];
  ((char*)arg)[len] = 0;
  pair<T*,T*> p = equal_range(array,&array[num],val,NameComp<T>());
  ((char*)arg)[len] = tmp;
  if (p.first != p.second) {
    return p.first;
  } else {
    return 0;
  }
}

// Looks up an enumerated field for a corresponding value and then stores that in the expression.
static bool find_enum_option(expressionS *expr,const char *arg,enum_fields *enums)
{
  const enum_field  *ef = bfind<enum_field>(enums->enums,enums->num,arg);
  if (ef) {
    set_constant(expr,ef->value);
    return true;
  }
  return false;
}

/*
 *  Find an enumerated field value in a list of enumerated fields
 */
static enum_field *find_enum(enum_fields *enums, const char *arg, int len) {
  enum_field *ef = enums->enums;
  enum_field *ef_end = enums->enums + enums->num;

  char tmp = arg[len];
  ((char*) arg)[len] = 0;

  for ( ; ef != ef_end; ef++) {
    if ((ignore_case && !strcasecmp(ef->name, arg)) ||
      (!ignore_case  && !strcmp(ef->name, arg))) {
        break;
      }
  }

  ((char*) arg)[len] = tmp;
  if (ef != ef_end) {
    return ef;
  }

  return NULL;
}

static bool find_enum_option(expressionS *expr, const Arg &arg,
                             enum_fields *enums)
{
  /* find_enum is used instead of bfind_n because when the case is ignored 
  the order of the fields may not be the same and binary search would not 
  find the field */
  const enum_field *ef = find_enum(enums, arg.str, arg.len);
  if (ef) {
    set_constant(expr, ef->value);
    return true;
  }
  return false;
}

static expressionS *get_enum_expr(enum_fields **enums,int field_id,int iface_id,InstrInfo &info) 
{
  struct adl_opcode *opcode = info.opcode;
  if (!opcode) AS_FATAL(_("Empty InstrInfo object:  No opcode associated with this instruction item."));
  expressionS *ex = 0;
  int i;
  for (i=0; i != opcode->num_operands; ++i) {
    const struct adl_operand *operand = &opcode->operands[i];
    const struct adl_field *field = operand->ptr;
    if (field && (field->iface_id == field_id || field->iface_id == iface_id) && field->enums != NULL) {
      ex = &(info.operand_values[operand->arg]);
      *enums = field->enums;
      return ex;
    }
  }
  return ex;
}

static expressionS *get_enum_expr(enum_fields **enums,int field_id,int iface_id,int pos,int group) 
{
  return get_enum_expr(enums,field_id,iface_id,instrInfos.get_instr(group,pos));
}

// get the expression corresponding to a field_id, in instruction pos at group gr
static expressionS *get_expr(int field_id,int iface_id,InstrInfo &info) 
{
  struct adl_opcode *opcode     = info.opcode;
  struct adl_opcode *src_opcode = info.src_opcode;
  if (!opcode) AS_FATAL(_("Empty InstrInfo object:  No opcode associated with this instruction item."));
  expressionS *ex = 0;
  if (src_opcode) {
    // This is a shorthand, so we use the source opcode for field data.
    for (int i=0; i != src_opcode->num_operands; ++i) {
      const adl_operand &operand = src_opcode->operands[i];
      if (operand.index == field_id || operand.index == iface_id) {
        ex = &(info.operand_values[operand.arg]);
      }
    }
  } else {
    for (int i=0; i != opcode->num_operands; ++i) {
      const adl_operand &operand = opcode->operands[i];
      const adl_field &field = *operand.ptr;
      if (field.iface_id == field_id || field.iface_id == iface_id) {
        ex = &(info.operand_values[operand.arg]);
      }
    }
  }
  return ex;
}

// get the expression corresponding to a field_id, in instruction pos at group gr
static expressionS *get_expr(int field_id,int iface_id,int pos,int group) 
{
  InstrInfo &info = instrInfos.get_instr(group,pos);
  return get_expr(field_id,iface_id,info);
}

int adl_get_instr_position(const InstrInfo &ii,int group)
{
  return (&ii - &instrInfos[group][0]);
}

int adl_get_current_group(void)
{
  return curgrp();
}

int adl_group_size(int group) 
{
  return instrInfos[group].group_size();
}

bool adl_query_field(int field_id,int iface_id,const InstrInfo &info)
{
  return get_expr(field_id,iface_id,const_cast<InstrInfo&>(info));
}

void  adl_set_field(int field_id,int iface_id,int value,InstrInfo &info)
{
  // save_instr incremented the group counter // group counter or instr counter inside the group?
  expressionS* ex = get_expr(field_id,iface_id,info);
  
  if (ex) {
    set_constant(ex,value);
  } else {
    AS_BAD(_("Bad field access:  Field %s does not exist for this instruction."),get_field_name_safe(field_id));
  }
}

void  adl_set_field(int field_id,int iface_id,int value,int pos,int group)
{
  adl_set_field(field_id,iface_id,value,instrInfos.get_instr(group,pos));
}


void adl_set_enum_field(int field_id,int iface_id,const char *value,InstrInfo &info)
{
  enum_fields *enums = 0;
  expressionS* ex = get_enum_expr(&enums,field_id,iface_id,info);
  
  if (ex) {
    if (!find_enum_option(ex,value,enums)) {
      AS_BAD(_("Unknown enumeration value for field %s."),get_field_name_safe(field_id));
    }
  } else {
    AS_BAD(_("Bad field access:  Field %s does not exist for this instruction."),get_field_name_safe(field_id));
  }
}

void adl_set_enum_field(int field_id,int iface_id,const char *value,int pos,int group)
{
  adl_set_enum_field(field_id,iface_id,value,instrInfos.get_instr(group,pos));
}

unsigned  adl_get_field(int field_id,int iface_id,const InstrInfo &info)
{
  expressionS* ex = get_expr(field_id,iface_id,const_cast<InstrInfo&>(info));
  if (ex) {
    if (ex->X_op == O_constant) {
      return ex->X_add_number; 
    } else {
      AS_BAD(_("Bad field access:  Field %s does not contain a constant."),get_field_name_safe(field_id));
    }
  } else {
    AS_BAD(_("Bad field access:  Field %s does not exist for this instruction."),get_field_name_safe(field_id));
  }

  return -1;
}

unsigned  adl_get_field(int field_id,int iface_id,int pos,int group)
{
  return adl_get_field(field_id,iface_id,instrInfos.get_instr(group,pos));
}


void *adl_get_dest(int pos, int group) 
{
  InstrInfo &info = instrInfos.get_instr(group,pos);
  struct adl_opcode *opcode = info.opcode;
  if (!opcode) AS_FATAL(_("Empty InstrInfo object:  No opcode associated with this instruction item."));

  if (opcode->dests) {
    return (*opcode->dests)();
  }
  return 0;
}

void adl_set_prefix_field_slice(struct adl_prefix_field *pfield, int index, int value)
{
  unsigned shift = pfield->field_width*index;
  if (target_big_endian) {
    uint32_t mask  = (pfield->mask << shift);
    pfield->value = ((value << shift) & mask) | (pfield->value & ~mask);
  } else {
    uint32_t mask  = (pfield->mask >> shift);
    pfield->value = ((value << shift) & mask) | (pfield->value & ~mask);
    
  }
}

bool adl_pre_set_field(int field_id,int iface_id, int value)
{
  PreSetInfos[PreSetInfoCtr].field_id = field_id;
  PreSetInfos[PreSetInfoCtr].iface_id = iface_id;
  PreSetInfos[PreSetInfoCtr].value = value;
  PreSetInfos[PreSetInfoCtr].enum_value = 0;
  
  ++PreSetInfoCtr;
  return true;
}

bool adl_pre_set_enum_field(int field_id,int iface_id,const char *enum_value)
{
  PreSetInfos[PreSetInfoCtr].field_id = field_id;
  PreSetInfos[PreSetInfoCtr].iface_id = iface_id;
  PreSetInfos[PreSetInfoCtr].value = 0;
  PreSetInfos[PreSetInfoCtr].enum_value = enum_value;
  
  ++PreSetInfoCtr;
  return true;
}

//  Given an instruction object, this replaces it with the named instruction
// and returns the new item. The number of operands must match.
InstrInfo replaceInstr(const InstrInfo &instr, const string &new_name)
{
	// Find the new instruction.
	struct adl_opcode *new_op = static_cast<struct adl_opcode *>(
		hash_find(all_instrs_hash[core], new_name.c_str()));

	if (new_op)
	{
		InstrInfo new_instr(instr);
		// If the instruction is an alias the original opcode is used.
		if (new_op->num_alias_targets == 1 && new_op->alias_targets)
		{
			new_instr.set_opcode(new_op->alias_targets);
		}
		else
		{
			new_instr.set_opcode(new_op);
		}
		return new_instr;
	}
	else
	{
		AS_FATAL(_("replaceInstr:  No instruction named %s"), new_name.c_str());
		return InstrInfo();
	}
}

// Returns an instruction object which references the named instruction.
InstrInfo createInstr(int num_args, const char *new_name, ...)
{
  // Find the new instruction.
  struct adl_opcode *new_op = (struct adl_opcode *) hash_find(all_instrs_hash[core],new_name);

  if (new_op) {
    InstrInfo new_instr;

    new_instr.alloc_operand_values(num_args);
    new_instr.set_opcode(new_op);
    new_instr.src_opcode = 0;

    va_list ap;
    va_start(ap,new_name);
    int i = 0;                          
    for ( ; (i != num_args); ++i) {
      int arg = va_arg(ap, int);       
      expressionS *ex = &new_instr.operand_values[i];
      set_constant(ex,arg);
    }
    va_end(ap);
    new_instr.maxfields = i;
    new_instr.line_number = instrInfos[curgrp()][0].line_number; //
    return new_instr;
  } else {
    AS_FATAL(_("createInstr:  No instruction named %s"),new_name);
    return InstrInfo();
  }
}


// Returns an instruction object which references the named instruction.
// Combine micro instructions into a macro one
// The fields of the macro instruction must be all instruction-type 
// The total number of fields of macro instruction should be equal to that of arguments. 
// Use "-1" as an argument to indicate an empty field
InstrInfo combineInstr(int num_args, const char *name, ...)
{
    InstrInfo instr;
    
    // Find the new instruction, opcode_macro should be kept as it is, for it is "static" for all similar formats
    adl_opcode *opcode_macro = (adl_opcode *) hash_find(all_instrs_hash[core],name);
    
    if (!opcode_macro) {
      AS_FATAL(_("combineInstr:  No instruction is named %s"), name);
      return InstrInfo();
    }
    
    if (opcode_macro->num_operands != num_args) {
      AS_FATAL(_("combineInstr: The number of arguments is invalid: %d"), num_args);
      return InstrInfo();
    }    

    // Create a "dynamic" copy of opcode, so we can patch it
    adl_opcode *opcode = (adl_opcode*)malloc(sizeof(adl_opcode));  // Create a dynamic copy
    if (opcode) {
        memcpy(opcode, opcode_macro, sizeof(adl_opcode));
    } else {
      AS_FATAL(_("combineInstr: Unable to combine instructions"));
      return InstrInfo();
    }
    
    int total_num_operands = 0;
    int total_num_exprs = 0;
    int total_num_operand_values = 0;
    va_list ap;
    int instr_ord;

    va_start(ap, name);
    for (int i = 0; i != num_args; ++i) { // The 1st round deals with opcode and calcualte the number of operands
        instr_ord = va_arg(ap, int);

        if ((instr_ord < -1) || (instr_ord >= int(instrInfos[curgrp()].size()))) {  // int() type-conversion is required!
            AS_FATAL(_("combineInstr: The specified instruction index %d is invalid"), instr_ord);
            return InstrInfo();
        }

        if (instr_ord == -1) continue;

        InstrInfo *xx = &instrInfos[curgrp()][instr_ord];

        // Check the width of the micro instruction
        if (xx->opcode->width > 32) {
            AS_FATAL(_("combineInstr: The width of %s is over the limit for micro instructions(32 bits): %d"), xx->opcode->name, xx->opcode->width);
            return InstrInfo();
        }

        if (xx->opcode->width >= opcode_macro->width) {
            AS_FATAL(_("combineInstr: The width of the micro instruction, %d, is greater than or equal to that of the macro one, %d"), 
                        xx->opcode->width, opcode_macro->width);
            return InstrInfo();
        }

        total_num_operands += xx->opcode->num_operands;
        total_num_exprs += xx->opcode->num_exprs;
        total_num_operand_values += xx->num_operand_values;

        unsigned * opc = xx->opcode->opcode;
        unsigned opc_value = (target_big_endian) ? opc[0] : opc[opcode->max_width/32-1]; // Thanks to the width limit of 32 bits
        opc_value = opc_value >> (32 - xx->opcode->width);   // Right adjustification
        const adl_operand * opr = &opcode_macro->operands[i];

        // Inserts a value using the inserter function for a specified field
        // void insert_value(unsigned *instr, uint64_t val, const struct adl_operand *operand)
        insert_value(opcode->opcode, opc_value, opr);  // Patch the opcode at opcode, which is "dynamic", so mutable
    }
    va_end(ap);

    instr.maxfields = total_num_operand_values;
    instr.is_macro = true;

    // Set the line number as that of 1st micro instruction in the group; 
    // Usually all micro instructions are in the same line anyway
    instr.line_number = instrInfos[curgrp()][0].line_number;

    opcode->num_operands = total_num_operands;
    opcode->num_exprs = total_num_exprs;

    instr.alloc_operand_values(total_num_operand_values); // num_operand_values is set inside alloc_operand_values()

    instr.opcode = opcode;
    instr.src_opcode = 0;       // No shorthand for explicitly created macro instructions

    // Allocate for opcode->operands
    if (total_num_operands) {
        opcode->operands = (adl_operand*)malloc(sizeof(adl_operand)*total_num_operands);
        if (!opcode->operands) {
            AS_FATAL(_("combineInstr: Unable to combine micro instructions"));
            return InstrInfo();
        }
    } else {
        opcode->operands = 0;
    }

    // Allocate for instr.operand_macro
    if (total_num_operand_values) {
        instr.operand_macro = (adl_operand_macro*)malloc(sizeof(adl_operand_macro)*total_num_operand_values);
        if (!instr.operand_macro) {
            AS_FATAL(_("combineInstr: Unable to combine micro instructions"));
            return InstrInfo();
        }
    } else {
        instr.operand_macro = 0;
    }

	instr.args.clear();
	int op_val_index = 0;
	int operand_index = 0;
	int arg_base = 0;
	va_start(ap, name);
	for (int i = 0; i != num_args; ++i)
	{
		// The 2nd round fills in the operands, etc.
		instr_ord = va_arg(ap, int);

		if (instr_ord == -1)
		{
			continue;
		}
		InstrInfo *xx = &instrInfos[curgrp()][instr_ord];
		bool inst_is_loop = false;

		if (check_set_loop_labels(xx->opcode))
		{
			inst_is_loop = true;
		}

		for (int j = 0; j != xx->num_operand_values; ++j, ++op_val_index)
		{
			expressionS *opv = &xx->operand_values[j];
			memcpy(&instr.operand_values[op_val_index], opv, 
				sizeof(expressionS));
			assert(instr.operand_macro);
			instr.operand_macro[op_val_index].width = xx->opcode->width;
			instr.operand_macro[op_val_index].operand
				= &opcode_macro->operands[i];
			if (inst_is_loop)
			{
				instr.operand_macro[op_val_index].operand->flags |= ADL_FROM_LOOP_INST;
			}
			instr.operand_macro[op_val_index].max_width
				= opcode_macro->max_width;
			instr.operand_macro[op_val_index].instr_width
				= opcode_macro->width;
			instr.operand_macro[op_val_index].shift = 0;
			instr.operand_macro[op_val_index].arg_base = arg_base;
		}

		for (int j = 0; j != xx->args.size(); ++j)
		{
			instr.args.push_back(xx->args[j]); 
		}

		for (int j = 0; j != xx->opcode->num_operands; ++j, ++operand_index)
		{
			adl_operand *opr = &xx->opcode->operands[j];
			memcpy(&opcode->operands[operand_index], opr, sizeof(adl_operand));
			assert(opcode->operands);
			opcode->operands[operand_index].arg += arg_base;
		}

		arg_base += xx->num_operand_values;
	}
	va_end(ap);

	return instr;
}

// Returns an instruction object which references the named instruction.
// Patch a macro instruction at the 32 LSBs with a micro one
// The instruction-type fields of macro instruction should be continuous
// The width of macro instruction should be greater than that of micro ones
// The width of micro instruction should be less than or equal to 32 bits
// The patch area should be limited within the 32 LSBs of the macro instruction
// The micro instruction should not be a macro instruction by itself
// Note that the operand values of the created instruction may include some O_illeagl ones
InstrInfo patchInstr(const InstrInfo &macro, const InstrInfo &micro, int shift)
{
    if (micro.is_macro) {
        AS_FATAL(_("patchInstr: The specified patch instruction is invalid"));
        return InstrInfo();
    }

   if (micro.opcode->width + shift > 64) {
        AS_FATAL(_("patchInstr: The specified instruction patch location %d is invalid"), shift);
        return InstrInfo();
    }

    if (micro.opcode->width >= macro.opcode->width) {
        AS_FATAL(_("patchInstr: The width of the micro instruction, %d, is greater than or equal to that of the macro one, %d"), 
                    micro.opcode->width, macro.opcode->width);
        return InstrInfo();
    }

    InstrInfo instr;

    // Create a "dynamic" copy of macro.opcode, so we can patch it
    adl_opcode *opcode = (adl_opcode*)malloc(sizeof(adl_opcode));  // Create a dynamic copy
    if (opcode) {
        memcpy(opcode, macro.opcode, sizeof(adl_opcode));
    } else {
      AS_FATAL(_("patchInstr: Unable to patch instruction"));
      return InstrInfo();
    }

    int total_num_operands = macro.opcode->num_operands + micro.opcode->num_operands;
    int total_num_operand_values = macro.num_operand_values + micro.num_operand_values;

    // Patch the opcode of macro instruction with that from micro instruction
    unsigned *opc = micro.opcode->opcode;
    unsigned opc_value = (target_big_endian) ? opc[0] : opc[opcode->max_width/32-1]; // Thanks to the width limit of 32 bits
    opc_value = opc_value >> (32 - micro.opcode->width);   // Right adjustification to get the value
    insert_value_simple(opcode->opcode, opc_value, macro.opcode->max_width, macro.opcode->width, shift);  // Patch the opcode at opcode, which is "dynamic", so mutable

    instr.maxfields = total_num_operand_values;
    instr.is_macro = true;

    // Set the line number as that of the macro instruction 
    instr.line_number = macro.line_number;

    opcode->num_operands = total_num_operands;
    opcode->num_exprs = macro.opcode->num_exprs + micro.opcode->num_exprs;

    instr.alloc_operand_values(total_num_operand_values); // num_operand_values is set inside alloc_operand_values()

    instr.opcode = opcode;
    instr.src_opcode = macro.src_opcode;    // Keep the src_opcode as it is for the patched instruction

    // Allocate for opcode->operands
    if (total_num_operands) {
        opcode->operands = (adl_operand*)malloc(sizeof(adl_operand)*total_num_operands);
        if (!opcode->operands) {
            AS_FATAL(_("patchInstr: Unable to patch instruction"));
            return InstrInfo();
        }
    } else {
        opcode->operands = 0;
    }

    // Allocate for instr.operand_macro
    if (total_num_operand_values) {
        instr.operand_macro = (adl_operand_macro*)malloc(sizeof(adl_operand_macro)*total_num_operand_values);
        if (!instr.operand_macro) {
            AS_FATAL(_("patchInstr: Unable to patch instruction"));
            return InstrInfo();
        }
    } else {
        instr.operand_macro = 0;
    }

	instr.args.clear();
	int op_val_index = 0;
	int operand_index = 0;

	// Fill in the operands, etc.
	for (int j = 0; j != macro.num_operand_values; ++j, ++op_val_index)
	{
		expressionS * opv = &macro.operand_values[j];
		memcpy(&instr.operand_values[op_val_index], opv, sizeof(expressionS));

		assert(instr.operand_macro);
		// Set all the fields for operand_macro.
		if (macro.is_macro)
		{
			// If already macro instruction, keep the values.
			instr.operand_macro[op_val_index].operand
				= macro.operand_macro[op_val_index].operand;
			instr.operand_macro[op_val_index].shift
				= macro.operand_macro[op_val_index].shift;
			instr.operand_macro[op_val_index].width
				= macro.operand_macro[op_val_index].width;
			instr.operand_macro[op_val_index].max_width
				= macro.operand_macro[op_val_index].max_width;
			instr.operand_macro[op_val_index].instr_width
				= macro.operand_macro[op_val_index].instr_width;
			instr.operand_macro[op_val_index].arg_base
				= macro.operand_macro[op_val_index].arg_base;
		}
		else
		{ 
			// Set width == instr_width for operand from macro instruction.
			// The operand is not O_illegal, i.e., not instruction-type, so set
			// operand = 0 for simpler processing.
			instr.operand_macro[op_val_index].operand = 0;
			instr.operand_macro[op_val_index].shift = 0;
			instr.operand_macro[op_val_index].width = macro.opcode->width;
			instr.operand_macro[op_val_index].max_width
				= macro.opcode->max_width;
			instr.operand_macro[op_val_index].instr_width
				= macro.opcode->width;
			instr.operand_macro[op_val_index].arg_base = 0;
		}
	}

	for (int j = 0; j != macro.args.size(); ++j)
	{
		instr.args.push_back(macro.args[j]); 
	}

	for (int j = 0; j != macro.opcode->num_operands; ++j, ++operand_index)
	{
		adl_operand * opr = &macro.opcode->operands[j];
		memcpy(&opcode->operands[operand_index], opr, sizeof(adl_operand));
	}

	int arg_base = macro.num_operand_values/*macro.args.size()*/;

	for (int j = 0; j != micro.num_operand_values; ++j, ++op_val_index)
	{
		expressionS * opv = &micro.operand_values[j];
		memcpy(&instr.operand_values[op_val_index], opv, sizeof(expressionS));

		// Set values for operand from micro instruction
		instr.operand_macro[op_val_index].width = micro.opcode->width;
		instr.operand_macro[op_val_index].operand = 0;
		instr.operand_macro[op_val_index].max_width = macro.opcode->max_width;
		instr.operand_macro[op_val_index].instr_width = macro.opcode->width;
		instr.operand_macro[op_val_index].shift = shift;
		instr.operand_macro[op_val_index].arg_base = arg_base;
	}

	for (int j = 0; j != micro.args.size(); ++j)
	{
		instr.args.push_back(micro.args[j]); 
	}

	for (int j = 0; j != micro.opcode->num_operands; ++j, ++operand_index)
	{
		adl_operand *opr = &micro.opcode->operands[j];
		memcpy(&opcode->operands[operand_index], opr, sizeof(adl_operand));
		assert(opcode->operands);
		opcode->operands[operand_index].arg += arg_base;
	}

	return instr;
}

/*
 *  Parse VSPA half precision fixed format constants defined through assembler
 * directives
 */
static char *adl_atofix(LITTLENUM_TYPE *words) {
  char *start = input_line_pointer;

  /* find the end of the current constant */
  while (!is_end_of_line[*input_line_pointer] && 
      (*input_line_pointer != ' ') &&
      (*input_line_pointer != ',')) {
    input_line_pointer++;
  }

  /* generated the encoding for the constant */
  char c = *input_line_pointer;
  *input_line_pointer = 0;
  gen_fixed_float16(start, words);
  *input_line_pointer = c;

  return input_line_pointer;
}

/*
 *  Machine dependent floating point constants enconding
 */
extern "C" char *md_atof(int type, char *litp, int *sizep)
{
  LITTLENUM_TYPE words[MAX_LITTLENUMS];
  int i, prec = 0, ieee = 1;
  char *end;

  /* determine precision and type */
  switch (type) {
    case 'r':
    case 'R':
      prec = 1;
      ieee = 0;
      break;
    case 'h':
    case 'H':
      if (enable_hprec) {
        prec = 1;
      } else {
        as_bad("half precision float not supported");
        *sizep = 0;
      }
      break;
    case 's':
    case 'S':
    case 'f':
    case 'F':
      prec = 2;
      break;
    case 'd':
    case 'D':
      if (enable_dprec) {
        prec = 4;
      } else {
        as_bad("double precision float not supported");
        *sizep = 0;
      }
      break;
    default:
      *sizep = 0;
      return "Unrecognized or unsupported floating point constant";
  }

  /* parse and encode the value */
  if (ieee) {
    end = atof_ieee(input_line_pointer, type, words);
  } else {
    end = adl_atofix(words);
  }

  if (end) {
    input_line_pointer = end;
  }

  /* write the value */
  *sizep = prec * sizeof(LITTLENUM_TYPE);
  for (i = prec - 1; i >= 0; i--) {
    md_number_to_chars (litp, (valueT) words[i], sizeof (LITTLENUM_TYPE));
    litp += sizeof (LITTLENUM_TYPE);
  }

  /* increment the number of floating point constants for the current
  directive */
  num_floats++;

  return NULL;
}

/* Write a value out to the object file, using the appropriate endianness.  */
extern "C" void md_number_to_chars (char *buf, valueT val, int n)
{
  if (target_big_endian)
    number_to_chars_bigendian (buf, val, n);
  else
    number_to_chars_littleendian (buf, val, n);
}

// For big-endian, we proceed forwards, for little-endian, we start from the
// back and go forwards.  The md_number_to_chars function takes care of the
// byte-swapping of the individual words.
void md_big_number_to_chars (char *buf, unsigned* val, int n)
{
 int ind = 0;
  while (true) {
    if (ind > (MAX_INSTR_WORDS - 1)) {
      AS_BAD(_("Internal assembler error, instruction size exceeded maximal length."));
    }
    if (n <= 4) {
      // Right justify.
      unsigned tmp = val[ind] >> 8*(4-n);
      md_number_to_chars (buf, (valueT)(tmp), (n));
      // We are done.
      break;
    } else {
      md_number_to_chars (buf, (valueT)(val[ind]), 4);
      // Proceed to the next word.
      n -= 4;
      buf +=4;
    }
    // increment array pointer
    ++ind;
  }

}

// Looks up a register by name.
// str: Register name to find.
// expr: If found, contains expression information about the register.
// return: true if found, false if not.
static bool find_reg_name(expressionS *expr,
	struct hash_control *reg_hash,
	const Arg &arg)
{
	if (!ISALPHA(*(arg.str)))
	{
		return false;
	}

	string reg = arg.to_str();
	if (ignore_case)
	{
		std::transform(reg.begin(), reg.end(), reg.begin(), ::tolower);
	}
	const struct adl_name_pair *regname =
		static_cast<const struct adl_name_pair *>(
			hash_find(reg_hash, reg.c_str()));

	if (regname)
	{
		expr->X_op = O_register;
		expr->X_add_number = regname->index;
		expr->X_add_symbol = NULL;
		expr->X_op_symbol = NULL;

		return true;
	}

	return false;
}

// Inserts a 32-bit value into the instruction, make sure the value with shift don't exceed the 64 bit limit
void insert_value_simple(unsigned *instr, uint64_t val, unsigned max_width, unsigned instr_width, unsigned shift)
{

    assert(instr_width <= max_width);
    // assert(instr_width > 32);
    // assert(max_width > 32);
    // assert(shift < 32);
    unsigned displacement = max_width - instr_width;
    unsigned num = max_width/32 - 1;  // max_width is the maximum instruction width round to 32-bits words
    uint64_t bv, bv0, bv1 = 0;  // bv0 stores the LSW
  
    if (target_big_endian) { 
        bv0 = instr[num];
        if (max_width > 32) bv1 = instr[num-1];
    }
    else {
        bv0 = instr[0];
        if (max_width > 32) bv1 = instr[1];
    }

    bv = (bv1 << (32 - displacement)) | (bv0 >> displacement);
    val = val << shift;
    bv |= val;

    if (target_big_endian) { 
        instr[num] = uint32_t(bv << displacement);
        if (max_width > 32) instr[num-1] = uint32_t(bv >> (32 - displacement));
    }
    else {
        instr[0] = uint32_t(bv << displacement);
        if (max_width > 32) instr[1] = uint32_t(bv >> (32 - displacement));
    }
    
}
// Inserts a value using the inserter function for a specified field.
void insert_value(unsigned *instr, uint64_t val, const struct adl_operand *operand)
{
     
  const struct adl_field *field = operand->ptr;

  assert(field);
  // Correct for any shifting.
  val >>= operand->leftshift;
  if (operand->checker) {
    (*field->inserter)(instr,(*operand->checker)(val,false));
  } else {
    (*field->inserter)(instr,val);
  }
}

// [Overloaded for macro instruction] Inserts a value using the inserter function for a specified field 
void insert_value(unsigned *instr, uint64_t val, const struct adl_operand *operand, 
                  const adl_operand_macro * operand_macro) 
{
    unsigned opc[MAX_INSTR_WORDS] = {0}; // Initialized to zero
    insert_value(opc, val, operand);     // Build micro opc

    // unsigned opc_value = (target_big_endian) ? opc[0] : opc[operand_macro->max_width/32-1];    // Thanks to the width limit of 32 bits
    unsigned opc_value = opc[0]; // Thanks to the width limit of 32 bits
    opc_value = opc_value >> (32 - operand_macro->width);  // Right adjustification

    if (operand_macro->operand) {  // If the operand = 0, then use insert_value_simple() instead insert_value()
        insert_value(instr, opc_value, operand_macro->operand);  // Insert the micro opc into macro opc
    } else {
        insert_value_simple(instr, opc_value, operand_macro->max_width, operand_macro->instr_width, operand_macro->shift);
    }
}

// Inserts a value generated via an expression.  To be safe, we clear the value first,
// since it's possible that this may be called twice: Once, before a fixup, then again,
// once a fixup value is known.
void insert_modifier(unsigned *insn,const struct adl_operand *operand, expressionS *exprs, uint32_t line_number)
{
  const struct adl_field *field = operand->ptr;

  assert(field && operand->modifier && exprs);

  uint64_t val = operand->modifier(exprs, line_number);

  (*field->clearer)(insn);

  insert_value(insn,val,operand);
}

// [Overloaded for macro instruction] Inserts a value generated via an expression
void insert_modifier(unsigned *insn, const struct adl_operand *operand, expressionS *exprs, uint32_t line_number, 
                     const adl_operand_macro * operand_macro)
{
    if (operand_macro->width < operand_macro->instr_width) {
        // It is an operand from micro instruction
        unsigned opc[MAX_INSTR_WORDS] = {0}; // Initialized to zero

        /* modifiers consider operand value offsets relative to the first
        operand value of the microinstruction, not of the macroinstruction;
        that's why we have to provide them with a list of operand values having
        the first element the value of the first operand of the
        microinstruction */
        insert_modifier(opc, operand, exprs + operand_macro->arg_base, line_number);     // Modify micro opc

        // unsigned opc_value = (target_big_endian) ? opc[0] : opc[operand_macro->max_width/32-1];    // Thanks to the width limit of 32 bits
        unsigned opc_value = opc[0]; // Thanks to the width limit of 32 bits  
        opc_value = opc_value >> (32 - operand_macro->width);  // Right adjustification    

        if (operand_macro->operand) {
            // If the operand = 0, then use insert_value_simple() instead insert_value()
            insert_value(insn, opc_value, operand_macro->operand);  // Insert the micro opc into macro opc    
        } else {
            insert_value_simple(insn, opc_value, operand_macro->max_width, operand_macro->instr_width, operand_macro->shift);
        }
    }
    else {
        // It is an operand from macro instruction 
        insert_modifier(insn, operand, exprs, line_number);
    }
}

static bool is_whitespace(char c) 
{
  return (c == ' ' || c == '\t'); 
}

static void skip_to_whitespace(char **ptr)
{
  while (**ptr && !is_whitespace(**ptr)) {
    ++(*ptr);
  }
}

static void skip_whitespaces(char **ptr) 
{
  while (**ptr && is_whitespace(**ptr)) {
    ++(*ptr);
  }
}

static void skip_operand(char **ptr) 
{
  while (**ptr && !is_whitespace(**ptr) && **ptr != ',') {
    ++(*ptr);
  }
}

static void skip_operands(char **ptr) 
{
  while (**ptr && !is_whitespace(**ptr)) {
    ++(*ptr);
  }
}

// The idea here is that an instruction generally consists of a name, optional
// abutting flags, whitespice, and operands.  When we come across something we
// don't know, this skips the possible abutting flags, whitespace, and then the
// operands.
static char *skip_to_next(char *ptr)
{
  skip_to_whitespace(&ptr);
  skip_whitespaces(&ptr);
  skip_operands(&ptr);
  return ptr;
}

static bool is_grouping_char(char c) {
  return (strchr(packet_begin_chars,c) || strchr(packet_end_chars,c));
};

// We reject expresstions that use register names.
// It is  "reg + reg" or "reg - reg" expressions
// We also reject expressions of the form "<sym" or ">sym" as it can happen in
// StarCore.
static bool bad_expression(const Arg &arg, struct hash_control *reg_hash)
{
	char *op = 0;
	char c;

	if (arg.str && ((arg.str[0] == '<') || (arg.str[0] == '>')))
	{
		return true;
	}

	assert(arg.str);
	if ((op = strchr(arg.str, '+')) || (op = strchr(arg.str, '-')))
	{
		c = *op;
		*op = '\0';
		if (hash_find_n(reg_hash, arg.str, arg.len))
		{
			*op = c;
			return true;
		}
		if (*(op + 1) && hash_find(reg_hash, op + 1))
		{
			*op = c;
			return true;
		}
		*op = c;
	}
	return false;
}

static void  clear_preset_info(void)
{
  int i;
  for (i=0; i < PreSetInfoCtr; ++i) {
    PreSetInfos[i].enum_value = 0;
  }
  PreSetInfoCtr = 0;
}

static bool has_preset_fields(struct adl_opcode *opcode) 
{
  int i;
  struct adl_operand *operands = opcode->operands;
  for (i=0; i < PreSetInfoCtr; ++i) {
    int j;
    for (j=0; j < opcode->num_operands; ++j) {
      if (operands[j].ptr->iface_id == PreSetInfos[i].iface_id || operands[j].ptr->iface_id == PreSetInfos[i].field_id) {
        break;
      }
    }
    if (j == opcode->num_exprs) {
      return false;
    }
  }
  return true;
}

extern "C" reloc_howto_type *adl_elf_info_to_howto(unsigned code)
{
  if (!relocs)
  {
    return NULL;
  }

  for (int i = 0; i != num_relocs; i++)
  {
    if (relocs[i].type == code)
    {
      return &relocs[i];
    }
  }

  return NULL;
}

// If we have relocations installed, then return a pointer to the relocation in
// the array, otherwise fall back to the standard bfd routine.
const reloc_howto_type *adl_reloc_type_lookup(bfd *abfd, bfd_reloc_code_real_type code)
{
	reloc_howto_type *reloc = adl_elf_info_to_howto(code);

	if (reloc)
	{
		return reloc;
	}

	return bfd_reloc_type_lookup(abfd, code);
}


// If we have relocation installed, then try to get the relocation from the
// installed table.  Otherwise, call the hook function.
bfd_reloc_code_real_type adl_get_reloc_by_name(const char *str,expressionS *ex,const struct adl_operand *operand)
{
  if (relocs_by_index) {

    // We have relocations installed, so look this one up.  We isolate
    // everything to the right of the @, e.g. for foo@bar, we want "bar".
    char ident[20];
    char *str2;
    char ch;

    while (*str  && *str != '@') {
      ++str;
    }

    if (!*str || *str++ != '@')
      return BFD_RELOC_UNUSED;

    for (ch = *str, str2 = ident; (str2 < ident + sizeof (ident) - 1 && (ISALNUM (ch) || ch == '@')); ch = *++str) {
      *str2++ = TOLOWER (ch);
    }
  
    *str2 = '\0';

    if (const adl_name_pair *r = bfind<adl_name_pair>((adl_name_pair*)relocs_by_index,num_relocs_by_index,ident)) {
      return (bfd_reloc_code_real_type)relocs[r->index].type;
    }
    
    return BFD_RELOC_UNUSED;
  } else {
    return acb_reloc_hook((char*)str,ex,operand);
  }
}

/*
 *  Get the relocation type using the relocation name 
 */
bfd_reloc_code_real_type adl_get_reloc_type_by_name(const char *reloc_name) {
  const adl_name_pair *reloc = relocs_by_index;
  const adl_name_pair *reloc_end = relocs_by_index + num_relocs_by_index;

  if (!reloc) {
   return BFD_RELOC_UNUSED;
  }

  for (; reloc != reloc_end; reloc++) {
    if (!strcmp(reloc->name, reloc_name)) {
      return (bfd_reloc_code_real_type)relocs[reloc->index].type;
    }
  }

  return BFD_RELOC_UNUSED;
}

// This deals with setting assembler fields, which are fields that the user sets
// via the assembler, but not directly from assembly input on a per-instruction
// basis.  Since these fields can be set via various hooks which occur before
// the instruction is encountered, we check those values and set the fields if
// necessary.
static bool handle_assembler_fields(struct adl_opcode *opcode, expressionS* operand_values,int start) 
{
  int j;
  struct adl_operand *operands = opcode->operands;
  for (j=start; j < opcode->num_operands; ++j) {
    // set assembler fields from their preset or default values
    const struct adl_operand *operand = &opcode->operands[j];
    if (operand->ptr && operand->ptr->assembler) {
      expressionS *ex = &operand_values[operand->arg];
      int i;
      for (i=0; i < PreSetInfoCtr; ++i) {
        const struct adl_field *field = operand->ptr;
        if (field->iface_id == PreSetInfos[i].field_id || field->iface_id == PreSetInfos[i].iface_id) {
          if (PreSetInfos[i].enum_value != NULL) {
            enum_fields *ef = field->enums;
            if (!ef || !find_enum_option(ex,PreSetInfos[i].enum_value,ef)) {
              return false;
            }
          } else {
            set_constant(ex,PreSetInfos[i].value);
          }
          break;
        }
      }
      if (i == PreSetInfoCtr) {
        set_constant(ex,operands[j].ptr->default_value);
      }
    } 
  } 
  return true;
}

static bool set_default_option(const Arg &arg,int opindex,expressionS *ex,struct adl_opcode *opcode)
{
  // If it is not an empty string of a field that have default value then throw error
  if (opcode->operands[opindex].ptr->default_value == -1) {
    if (arg.str && (strncmp(arg.str,EMPTY_STRING,arg.len)!= 0)) {
      return false;
    }
  }
  else{
    set_constant(ex,opcode->operands[opindex].ptr->default_value);
  }
  return true;
}

// Swap entries in pmatch array
static void swap_match(Args &args,int i, int j) 
{
  Arg tmp = args[i];
  args[i] = args[j];
  args[j] = tmp;
}

// Handles permutable and prefix fields in syntax.  This then puts the items
// into the proper expression positions so that we can then simply iterate
// through the arguments and not worry about order.
static void reorder_perm_fields(Args &args, struct adl_opcode *opcode) 
{
	if (opcode->num_prm_operands == 0 && opcode->num_pfx_operands == 0) 
	{
		return;
	}

	size_t  i = 0; //current operand
	int ii = 1; //operand to swap
	for ( ; (i < args.size()) && (i != opcode->p_end); ++i) 
	{
		Arg &arg = args[i];
		// Need to backup
    
		enum_fields *ef = opcode->enums[i];
    
		expressionS ex;
		bfd_boolean found = FALSE;   
		found = find_enum_option(&ex,arg,ef);    
		if ((!found ) && (ii <= (opcode->p_end-1)))
		{
			swap_match(args, i, ii);     
			i--;
			ii++;
		}
		else
		{     
			ii = i + 2 ;  // after increment  ii equals to i+1
		}
	}
}

//  This transfers argument data from the regular expression data structure to
// the Args array. We don't actually copy data, but just record actual pointers
// and lengths.  Note that there may be holes in the input arguments due to
// enumerated fields with empty strings. Therefore, we have to iterate over
// everything, pushing empty elements on, then erase trailing empties.
static bool copy_operands(Args &args,
	char *&instr_end,
	struct adl_opcode *opcode,
	char *s,
	regmatch_t *pmatch,
	int maxfields)
{
	// First, add on empty items for any missing prefix fields which weren't
	// used. Any items in 'args' right now represents just what we've found in
	// terms of prefix items.
	for (int i = args.size(); i != opcode->num_pfx_operands; ++i)
	{
		// Dummy item.
		args.push_back(Arg(s, 0));
	}

	int last = 0;
	for (int i = 1; i != maxfields + 1; ++i)
	{
		if (pmatch[i].rm_eo < 0)
		{
			// Empty item: Push on a placeholder.
			args.push_back(Arg(s, 0));
			// We count it if we're still in the perm-field range. Otherwise,
			// for example, if we just have perm fields, we wouldn't handle it
			//correctly.
			if (args.size() <= opcode->num_prm_operands)
			{
				last = args.size();
			}
		}
		else
		{
			// Normal item.
			args.push_back(Arg(&s[pmatch[i].rm_so],
				pmatch[i].rm_eo - pmatch[i].rm_so));
			last = args.size();
		}
	}
	// Now remove trailing empties.
	args.erase(args.begin() + last, args.end());
	instr_end = &s[pmatch[0].rm_eo];

	// Return true if the end of the match is whitespace, eol or eos, or
	// matches against an end-of-packet character.
	return (is_whitespace(*instr_end)
		|| *instr_end == '\n'
		|| *instr_end == 0
		|| is_grouping_char(*instr_end));
}

//  Check if a register name exists in the register hash.
static bool find_register_name(struct hash_control *reg_hash, const char *reg)
{
	string name(reg);
	if (ignore_case)
	{
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	}

	return hash_find(reg_hash, name.c_str());
}

//  Check if an operand expression contains a symbol considered keyword.
static bool check_register_expression(expressionS *ex,
	struct hash_control *reg_hash)
{
	switch(ex->X_op)
	{
		case O_symbol:
		case O_symbol_rva:
		case O_uminus:
		case O_bit_not:
		case O_logical_not:
			return (S_GET_SEGMENT(ex->X_add_symbol) == expr_section)
				? check_register_expression(&ex->X_add_symbol->sy_value,
					reg_hash)
				: S_GET_SEGMENT(ex->X_add_symbol) == absolute_section
					? true
					: !find_register_name(reg_hash,
						S_GET_NAME(ex->X_add_symbol));
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
					? check_register_expression(&ex->X_add_symbol->sy_value,
						reg_hash)
					: S_GET_SEGMENT(ex->X_add_symbol) == absolute_section
						? true
						: !find_register_name(reg_hash,
							S_GET_NAME(ex->X_add_symbol)))
				&& ((S_GET_SEGMENT(ex->X_op_symbol) == expr_section)
					? check_register_expression(&ex->X_op_symbol->sy_value,
						reg_hash)
					: S_GET_SEGMENT(ex->X_op_symbol) == absolute_section
						? true
						: !find_register_name(reg_hash,
							S_GET_NAME(ex->X_op_symbol)));
			break;

		default:
			return true;
	}
}

//  Process instruction operands.
static bool process_operands(struct adl_opcode *opcode,
	Args &args,
	int maxfields,
	expressionS *operand_values,
	struct hash_control *reg_hash,
	bool &unresolved_symbol,
	bool &hprec_err,
	bool &dprec_err,
	bool &dp_macro_err)
{
	// Check if preset fields exists.
	if (!has_preset_fields(opcode))
	{
		return false;
	}

	reorder_perm_fields(args, opcode);
	
	unresolved_symbol = false;
	int i;
	for (i = 0; i != args.size(); ++i)
	{
		expressionS *ex = &operand_values[i];
		const Arg &arg = args[i];

		// Are we dealing with an enumerated field? If so, check it against
		// our allowed list.
		enum_fields *ef = opcode->enums[i];

		if (ef)
		{
			if (!find_enum_option(ex, arg, ef))
			{
				// If empty string, then try to set a default value. Otherwise,
				// produce error due to bad enumerated value.
				if (!arg.empty())
				{
					return false;
				}

				if (!set_default_option(arg, i, ex, opcode))
				{
					return false;
				}
			}
		}
		else
		{
			// Check to see if it's a register name.
			if (!find_reg_name(ex, reg_hash, arg))
			{
				// No, so parse as an expression. The expression parser takes
				// its input from input_line_pointer, rather than an argument,
				// for some reason.
				if (bad_expression(arg, reg_hash))
				{
					return false;
				}

				// Wrapper for expression().
				parse_expr(ex, arg.str, arg.len, unresolved_symbol, hprec_err,
					dprec_err, dp_macro_err);

				// Check against an expression including symbols with keyword
				// names.
				if (!check_register_expression(ex, reg_hash))
				{
					return false;
				}
			}
		}

		// Handle the operand.
		if (ex->X_op == O_illegal)
		{
			AS_BAD (_("Operand %d: illegal operand"), i);
		}
		else if (ex->X_op == O_absent)
		{
			AS_BAD (_("Operand %d: missing operand"), i);
		}
	}

	for (int opindex = i; opindex != maxfields; ++opindex)
	{
		memset(static_cast<void *>(&operand_values[opindex]), 0,
			sizeof(expressionS));
	}
	
	// Check for too many operands.
	if (i > opcode->num_operands)
	{
		return false;
	}
	else
	{
		if (assembler_fields)
		{
			return handle_assembler_fields(opcode,operand_values,0);
		}
		return true;
	}
}

#if defined(_VSPA2_)
//  Check for a wide immediate operand. If the value doesn't fit 9 bits it is
// checked to be multiple of the memory line size. The determined number of
// memory lines should also be fit 9 bits.
static bool check_wide_imm_operand(const struct adl_operand *operand,
	bfd_int64_t value)
{
	const bfd_int64_t max_is17 = 65535ll;
	const bfd_int64_t min_is17 = -65536ll;
	const bfd_int64_t max_iu17 = 131071ll;
	const bfd_int64_t min_iu17 = 0ll;
	bfd_int64_t line_size_dmem_unit, max_unit, min_unit;

	if (vspa1_on_vspa2)
	{
		line_size_dmem_unit = 2 * AU_NUM;
		max_unit = 127;
		min_unit = -128;
	}
	else
	{
		line_size_dmem_unit = 4 * AU_NUM;
		max_unit = 255;
		min_unit = -256;
	}

	// Signed value.
	if ((operand->flags & ADL_SIGNED)
		&& operand->lower == min_is17
		&& operand->upper == max_is17
		&& (value > max_unit
			|| value < min_unit))
	{
		if (value > (max_unit * line_size_dmem_unit)
			|| value < (min_unit * line_size_dmem_unit)
			|| (value % line_size_dmem_unit) != 0)
		{
			return false;
		}
	}
	// Negative unsigned value.
	else if ((operand->flags == 0)
		&& operand->lower == min_iu17
		&& operand->upper == max_iu17
		&& value > -min_unit)
	{
		if (value > (-min_unit * line_size_dmem_unit)
			|| (value % line_size_dmem_unit) != 0)
		{
			return false;
		}
	}
	
	return true;
}
#else
//  Check for a wide immediate operand. If the value doesn't fit 8 bits it is
// checked to be multiple of the memory line size. The determined number of
// memory lines should also be fit 8 bits.
static bool check_wide_imm_operand(const struct adl_operand *operand,
	bfd_int64_t value)
{
	const bfd_int64_t max_is15 = 16383ll;
	const bfd_int64_t min_is15 = -16384ll;
	const bfd_int64_t max_iu15= 32767ll;
	const bfd_int64_t min_iu15 = 0ll;
	const bfd_int64_t max_is8 = 127ll;
	const bfd_int64_t min_is8 = -128ll;
	const bfd_int64_t line_size = 2 * AU_NUM;
	const bfd_int64_t max_line_size = 128;

	// Signed value.
	if ((operand->flags & ADL_SIGNED)
		&& operand->lower == min_is15
		&& operand->upper == max_is15
		&& (value > max_is8
			|| value < min_is8))
	{
		if (value > (max_is8 * line_size)
			|| value < (min_is8 * line_size)
			|| (value % line_size) != 0)
		{
			return false;
		}
	}
	// Negative unsigned value.
	else if ((operand->flags == 0)
		&& operand->lower == min_iu15
		&& operand->upper == max_iu15
		&& value > -min_is8)
	{
		if (value > (-min_is8 * line_size)
			|| (value % line_size) != 0)
		{
			return false;
		}
	}

	return true;
}
#endif

//  Check if a field corresponding to an instruction's operand can fit a value
// with a particular width.
static bool check_field_width(struct adl_opcode *opcode, int pos, int width)
{
	if (!opcode->num_alias_targets)
	{
		adl_operand *operands = opcode->operands;
		for (int i = 0; i < opcode->num_operands; i++)
		{
			if (operands[i].arg != pos)
			{
				continue;
			}

			const struct adl_field *field = operands[i].ptr;
			assert(field);
			return width <= field->width;
		}

		return false;
	}

	bool result = true;
	for (int i = 0; i < opcode->num_alias_targets; i++)
	{
		result &= check_field_width(&opcode->alias_targets[i], pos, width);
	}

	return result;
}

//  Check if a value defined by the number of bits needed for encoding is in
// the range of a pseudoinstruction's operand.
bool check_pseudo_operand_range(const struct adl_operand *operand, size_t size)
{
	bfd_int64_t lower, upper;

	assert(size <= 64);

	if (operand->flags & ADL_SIGNED)
	{
		lower = (size < 64) 
			? -(bfd_int64_t)(1ULL << (size - 1)) 
			: (1ULL << (size - 1));
		upper = (1ULL << (size - 1)) - 1;
	}
	else if (operand->flags & ADL_EXT_SIGNED)
	{
		lower = (size < 64)
			? -(bfd_int64_t)(1ULL << (size - 1))
			: (1ULL << (size - 1));
		upper = (1ULL << size) - 1;
	}
	else
	{
		lower = 0;
		upper = (1ULL << size) - 1;
	}

	if (lower < operand->lower || upper > operand->upper)
	{
		return false;
	}

	return true;
}

//  Validate operands at the end of each instruction parsing step.
static bool valid_input(struct adl_opcode *opcode, expressionS *operand_values)
{
	for (int i = 0; i != opcode->num_operands; ++i)
	{
		const struct adl_operand *operand = &opcode->operands[i];
		expressionS *ex = &operand_values[operand->arg];

		// Skip if we're dealing with an assembler field, since we didn't get
		// any input from the syntax string.
		if (operand->ptr && operand->ptr->assembler)
		{
			continue;
		}

		if (operand->validator != NULL
			&& !operand->validator(operand_values))
		{
			return false;
		}

		// Skip if we're dealing with a modifier function.
		if (operand->modifier)
		{
			continue;
		}

		// Reject if we have a register value but the field is not a register
		// field.
		if (ex->X_op == O_register)
		{
			if (!(operand->flags & ADL_REGISTER))
			{
				return false;
			}

			if ((static_cast<unsigned long>(ex->X_add_number) < operand->lower)
				|| (static_cast<unsigned long>(ex->X_add_number)
					> operand->upper))
			{
				return false;
			}
		}

		// Next, do range checking on the input and reject as necessary.
		if (ex->X_op == O_constant)
		{
			// First, check the bounds.
			if (ex->X_md)
			{
				// We have a floating point constant.
				if (check_is_pseudo(opcode))
				{
					// For pseudoinstructions that are handled as aliases, we
					// don't have field information and the field information
					// of the parent instruction may not be accurate.
					if (!check_pseudo_operand_range(operand, ex->X_md))
					{
						return false;
					}
				}
				else if (!check_field_width(opcode, i, ex->X_md))
				{
					return false;
				}
			}
			else if (operand->flags & ADL_SIGNED)
			{
				if ((ex->X_add_number < operand->lower)
					|| (ex->X_add_number > operand->upper))
				{
					return false;
				}
			}
			else if (operand->flags & ADL_EXT_SIGNED)
			{
				bfd_int64_t value = ex->X_add_number;
				if (value < 0)
				{
					value = -value;
				}
				if ((ex->X_add_number < operand->lower)
					|| (static_cast<unsigned long>(value) > operand->upper))
				{
					return false;
				}
			}
			else
			{
				if ((static_cast<unsigned long>(ex->X_add_number)
						< operand->lower)
					|| (static_cast<unsigned long>(ex->X_add_number)
						> operand->upper))
				{
					return false;
				}
			}

			// Then make sure that the low-bits are clear, according to the
			// mask value.
			if ((operand->mask) && (ex->X_add_number & operand->mask) != 0)
			{
				return false;
			}

			// If an operand check exists, make sure it passes.
			if (operand->checker
				&& !operand->checker(
					static_cast<unsigned long>(ex->X_add_number), true))
			{
				return false;
			}

			// Check if we have a wide immediate, in which case it should be
			// divisible by the size of a memory line and the number of lines
			// should fit a signed immediate on 8 bits for VSPA1 and 9 bits for
			// VSPA2.
			if (check_wide_imm(opcode)
				&& (check_disabled(opcode)
				 || !check_wide_imm_operand(operand, ex->X_add_number)))
			{
				return false;
			}
		}
	}

	// If a cross-operand-value checker function exists, make sure that it
	// passes.
	if (opcode->argchecker && !(opcode->argchecker)(operand_values))
	{
		return false;
	}

	return true;
}


static void execute_action(struct adl_opcode *opcode, expressionS *operand_values, uint32_t line_number, int maxfields,unsigned grp)  
{ 
  DeclArray(struct adl_operand_val,vals,maxfields);
  int i = 0; 
  for( ; i < opcode->num_operands; ++i) { 
    const struct adl_operand *operand = &opcode->operands[i]; 
    expressionS *ex = &operand_values[operand->arg]; 
    if (operand->modifier) { 
      insert_modifier(&vals[i].val,operand,operand_values,line_number); 
    } else if (ex->X_op == O_register || ex->X_op == O_constant) { 
      insert_value(&vals[i].val,ex->X_add_number,operand); // We don't need to use overloaded version here
    } else  if (ex->X_op == O_symbol) {
      vals[i].symbol = ex->X_add_symbol;
    } else { 
      AS_BAD (_("Can not handle fix")); 
    } 
  }
  // TBD
  (*opcode->action)(&vals[0],grp); 
}

//  Check if a symbol is a placeholder for a symbolic expression.
static inline bool sym_is_expr(symbolS *sym)
{
	return !strcmp(S_GET_NAME(sym), FAKE_LABEL_NAME)
		&& (S_GET_SEGMENT(sym) == expr_section
			|| S_GET_SEGMENT(sym) == absolute_section);
}

//  Dump symbolic expression.
string dump_expression(const expressionS &expr)
{
	stringstream ss;
	string op;
	bool unary = false, index = false;
	switch (expr.X_op)
	{
	case O_constant:
		ss << expr.X_add_number;
		return ss.str();
	case O_symbol:
	case O_symbol_rva:
		unary = true;
		op = "";
		break;
	case O_uminus:
		unary = true;
		op = "-";
		break;
	case O_bit_not:
		unary = true;
		op = "~";
		break;
	case O_logical_not:
		unary = true;
		op = "!";
		break;
	case O_multiply:
		op = "*";
		break;
	case O_divide:
		op = "/";
		break;
	case O_modulus:
		op = "%";
		break;
	case O_left_shift:
		op = "<<";
		break;
	case O_right_shift:
		op = ">>";
		break;
	case O_bit_inclusive_or:
		op = "|";
		break;
	case O_bit_or_not:
		op = "|~";
		break;
	case O_bit_exclusive_or:
		op = "^";
		break;
	case O_bit_and:
		op = "&";
		break;
	case O_add:
		op = "+";
		break;
	case O_subtract:
		op = "-";
		break;
	case O_eq:
		op = "==";
		break;
	case O_ne:
		op = "!=";
		break;
	case O_lt:
		op = "<";
		break;
	case O_le:
		op = "<=";
		break;
	case O_ge:
		op = ">=";
		break;
	case O_gt:
		op = ">";
		break;
	case O_logical_and:
		op = "&&";
		break;
	case O_logical_or:
		op = "||";
		break;
	case O_index:
		index = true;
		break;
	case O_md1:
		op = "<<<";
		break;
	case O_md2:
		op = ">>>";
		break;

	case O_illegal:
	case O_absent:
	case O_register:
	case O_big:
	default:
		assert(false);
		as_bad("Illegal expression.\n");
		return "";
	}

	string left = sym_is_expr(expr.X_add_symbol)
		? dump_expression(expr.X_add_symbol->sy_value)
		: S_GET_NAME(expr.X_add_symbol);
	if (unary)
	{
		ss << "(" << op << left;
	}
	else
	{
		string right = sym_is_expr(expr.X_op_symbol)
			? dump_expression(expr.X_op_symbol->sy_value)
			: S_GET_NAME(expr.X_op_symbol);

		if (index)
		{
			ss << left;
			ss << "[";
			ss << right;
			ss << "]";
			return ss.str();
		}

		ss << "(" << left << " " << op << " " << right;
	}

	if (expr.X_add_number > 0)
	{
		ss << " + " << expr.X_add_number;
	}
	else if (expr.X_add_number < 0)
	{
		ss << " - " << -expr.X_add_number;
	}
	ss << ")";

	return ss.str();
}

//  Check if the values of the operands used by a modifier to compute the value
// to be encoded are constant.
static void check_modifier_operands(modifier_t modifier, const InstrInfo *info,
									size_t arg_base)
{
	const auto &map = modifier_map[core];
	const auto mod_it = map.find(modifier);

	// The modifier does not use any operand value.
	if (mod_it == map.end())
	{
		return;
	}

	const auto mi = mod_it->second;
	const auto indexes = mi->operands;
	for (size_t i = 0, e = mi->op_num; i < e; i++)
	{
		size_t index = arg_base + indexes[i];
		assert(index < info->num_operand_values);
		if (info->operand_values[index].X_op != O_constant)
		{
			char *file_name;
			as_where(&file_name, NULL);
			as_bad_where(file_name, info->line_number, "Expression \'%s\' "
				"must evaluate to a constant value.",
				dump_expression(info->operand_values[index]).c_str());
		}
	}
}

static void build_instruction(unsigned *insn,
	InstrInfo *info,
	adl::adl_fixup *fixups,
	int *num_fixups)
{
	int maxfields = info->maxfields;
	struct adl_opcode *opcode = info->opcode;
	expressionS *operand_values = info->operand_values;
	int num_operand_values = info->num_operand_values;
	adl_operand_macro *operand_macro = info->operand_macro;
	bool is_macro = info->is_macro;
	int need_operands = 0;
	expressionS *operand_values_fixup = NULL;
	// Do we have any modifier functions in any of the operands? If so, this
	// implies that we need to have the operand values stored for later use.
	for (int i = 0; i != opcode->num_operands; ++i)
	{
		if (opcode->operands[i].modifier)
		{
			need_operands = 1;
		}
	}

	int line_number = info->line_number;
	int fc = 0;
	for (int i = 0; i != opcode->num_operands; ++i)
	{
		const struct adl_operand *operand = &opcode->operands[i];
		expressionS *ex = &operand_values[operand->arg];
		adl_operand_macro *opr_macro = 0;

		if (is_macro)
		{
			assert(operand_macro);
			opr_macro = &operand_macro[operand->arg];
		}

		if (operand->modifier)
		{
			// A modifier should be applied only to constant values.
			check_modifier_operands(operand->modifier, info, is_macro
				? opr_macro->arg_base : 0);

			if (is_macro)
			{
				insert_modifier(insn, operand, operand_values, line_number,
					opr_macro);
			}
			else
			{
				insert_modifier(insn, operand, operand_values, line_number);
			}
		}
		else if (ex->X_op == O_register)
		{
			// Simple integer to be loaded directly into the instruction.
			if (is_macro && (opr_macro->width < opr_macro->instr_width))
			{
				// Short-circuit evaluation.
				insert_value(insn, ex->X_add_number, operand, opr_macro);
			}
			else
			{
				insert_value(insn, ex->X_add_number, operand);
			}
		}
		else if (ex->X_op == O_constant)
		{
			const string &arg = info->get_arg(i);
			// Callback to handle relocations in the argument.
			acb_const_hook((char *) arg.c_str(), ex, operand);
			// Simple integer to be loaded directly into the instruction.
			if (is_macro && (opr_macro->width < opr_macro->instr_width))
			{
				// Short-circuit evaluation.
				insert_value(insn, ex->X_add_number, operand, opr_macro);
			}
			else
			{
				insert_value(insn, ex->X_add_number, operand);
			}
		}
		else if (ex->X_op == O_illegal)
		{
			// In this context, an illegal value means that one wasn't
			// specified. So, we just ignore it.
			// Note that instruction-type fields generate "O_illegal".
		}
		else
		{
			// Need a fixup. Value will be 0 in the instruction until the fixup
			// is applied later. Call a callback in order to determine whether
			// we have a relocation. The callback should return a relocation
			// value, or BFD_RELOC_UNUSED if not applicable.

			if (fc >= maxfields)
			{
				AS_FATAL(_("too many fixups"));
			}

			fixups[fc].exp = *ex;
			fixups[fc].is_relative = (operand->flags & ADL_RELATIVE);
			fixups[fc].reloc = operand->ptr->reloc_type == -1
				? BFD_RELOC_UNUSED
				: static_cast<bfd_reloc_code_real_type>(
					operand->ptr->reloc_type);
			fixups[fc].line_number = line_number;
			fixups[fc].order = info->order;

			// Do we need the operands? Only if we have modifiers in other
			// fields.
			adl_opcode *opcode_fixup = 0;

			if (need_operands || is_macro)
			{
				opcode_fixup = static_cast<adl_opcode *>(
					xmalloc(sizeof(adl_opcode)));
				memcpy(opcode_fixup, opcode, sizeof(adl_opcode));
				assert(opcode->operands);
				assert(opcode->num_operands);
				opcode_fixup->operands = static_cast<struct adl_operand *>(
					xmalloc(sizeof(struct adl_operand)
						* opcode->num_operands));
				memcpy(opcode_fixup->operands, opcode->operands,
					sizeof(struct adl_operand) * opcode->num_operands);
				// Rewrite operand.
				operand = &opcode_fixup->operands[i];

				if (need_operands)
				{
					operand_values_fixup = static_cast<expressionS *>(
						xmalloc(sizeof(expressionS)
							* info->num_operand_values));
					memcpy(operand_values_fixup, operand_values,
						sizeof(expressionS) * info->num_operand_values);
				}
			}

			fixups[fc].opcode = opcode_fixup;
			fixups[fc].operand_values = operand_values_fixup;
			fixups[fc].operand = operand;

			if (is_macro)
			{
				fixups[fc].is_macro = true;
				// Make a new copy.
				adl_operand_macro *operand_macro_fixup
					= static_cast<adl_operand_macro *>(
						xmalloc(sizeof(adl_operand_macro)));
					memcpy(operand_macro_fixup, opr_macro,
						sizeof(adl_operand_macro));
				fixups[fc].operand_macro = operand_macro_fixup;
			}
			else
			{
				fixups[fc].is_macro = false;
				fixups[fc].operand_macro = 0;
			}

			++fc;
		}
	}

	*num_fixups = fc;
}




// This tries to find and assemble appropirate prefix instruction.
// It is not very optimal but may be it is enough.
// Note: Since assembler action code may modify a previous packet prefix
//       the prefix selection algorithm is incorrect in the genral case.
//       It does 
void savePrefix(InstrBundle &grp) 
{
  int i;
  int prefix = -1;
  bool need_prefix = false; // We always need prefix

  // We've called this function, so avoid calling it again.
  prefix_saved = true;

  struct adl_prefix_field **pfx_fields = prefix_fields[curgrp()].fields;
  // We cycle through all prefixes until we find appropriate one.
  for (i = 0; i != num_prefix_instrs; ++i) {
    // Now go through all prefix fields and if we see that its value different to default we 
    // check that the current prefix instruction can handle this field;
    int j;

    struct adl_opcode* opcode = &(prefix_instrs[i].opcodes[0]);

    for (j = 0; j != num_prefix_fields; ++j) {
      if (pfx_fields[j]->value != pfx_fields[j]->default_value) {
        // OK, have to handle this field, currently we take the first available.
        // TBD: check operand width, we'll take prefix that can handle the value.
        need_prefix = true;
        int k;
        bool found = false;
        for (k = 0; k != opcode->num_operands; ++k) {
          if (opcode->operands[k].prefix_field_id == j || opcode->operands[k].prefix_field_type_id == j) {
            found = true;
            break;
          }
        }
        if (!found) {
          goto NEXT;
        }
      }
    }
    // If we got here this prefix will suit our needs.
    prefix = i;

  NEXT:;
  }

  if (!need_prefix) {
    return;
  }

  if (prefix == -1) {
    AS_FATAL(_("Cannot find appropriate prefix."));
  }
  InstrInfo &info = grp.get_prefix();
  struct adl_opcode* prefix_instr ATTRIBUTE_UNUSED = &(prefix_instrs[prefix].opcodes[0]);
  info.opcode = prefix_instr;
  info.alloc_operand_values(prefix_instr->num_operands);
  grp.set_has_prefix(true);
}

// This tries to find and assemble appropirate prefix instruction.
// It is not very optimal but may be it is enough.
// Returns whether a prefix was actually written
static void write_prefix(int grp) 
{
  if (!instrInfos[grp].has_prefix()) {
    return;
  }

  InstrInfo &info = instrInfos.get_prefix(grp);

  struct adl_prefix_field **pfx_fields = prefix_fields[grp].fields;
  struct adl_opcode* prefix_instr ATTRIBUTE_UNUSED = info.opcode;

  unsigned instr[MAX_INSTR_WORDS];
  int k;
  // Set opcode
  for (k = 0; k != MAX_INSTR_WORDS; ++k) { 
    instr[k] = prefix_instr->opcode[k];
  };
  // Set operands, assume it is just a value
  for (k = 0; k != prefix_instr->num_operands; ++k) {
    const struct adl_operand *operand = &prefix_instr->operands[k];
    if (operand->prefix_field_id < 0) {
      continue;
    }

    // Is a prefix field- use the default value unless otherwise set.
    int value = pfx_fields[operand->prefix_field_id]->value;
    // Find out whether the field or the "type" of the field were set
    if (value == pfx_fields[operand->prefix_field_id]->default_value && operand->prefix_field_type_id != -1) {
      int type_value = pfx_fields[operand->prefix_field_type_id]->value;
      if (type_value != pfx_fields[operand->prefix_field_type_id]->default_value) {
        value = type_value;
      }
    }
    insert_value (instr,(uint64_t) value,operand);
  }

  // Instruction is all set, now write
  int instr_sz   = prefix_instr->size;
  uint32_t addr_mask = prefix_instr->addr_mask;
  write_instr(info.f,instr,instr_sz,frag_now,addr_mask);

  instrInfos[grp].set_has_prefix(false);
}

static void alloc_instr_bufs(int grp)
{
  // We allocate the prefix (if applicable) in the appropriate location in the
  // bundle.  Re-ordering should be done, so we can allocate instruction buffer
  // space.
  bool allocated_prefix = false;
  bool need_pfx = instrInfos[grp].has_prefix();
  unsigned pfx_index = instrInfos[grp].prefix_index();
  for (unsigned i = 0; i != instrInfos[grp].size(); ++i) {
    if (i == pfx_index && need_pfx) {
      alloc_instr_buf(instrInfos[grp].get_prefix());
      allocated_prefix = true;
    }
    InstrInfo &info = instrInfos[grp][i];
    alloc_instr_buf(info);
  }
  // Handle prefix-at-end-of-buffer situation.
  if (!allocated_prefix && need_pfx) {
    alloc_instr_buf(instrInfos[grp].get_prefix());
  }
}

/*
 * Set the flag in order to indicate that we have encountered an assembler
 * directive
 */
void adl_set_pseudo_op()
{
  assembler_directive = true;
}

/*
 *  Check if a string ends with a certain suffix
 */
bool endswith(const char *str, const char *suffix, unsigned suffix_len)
{
	unsigned len = strlen(str);
	if (len <= suffix_len)
	{
		return false;
	}

	return !strncmp(str + len - suffix_len, suffix, suffix_len);
}

/*
 * When the compaction is done the label values should be adujsted so that
 * they are not at the end of the packet; the lables will reffer the middle
 * of the packet (after the first 4 bytes)
 */
static void adjust_label_values()
{
	vector<symbolS *>::iterator label_it;
	for (label_it = labels.begin(); label_it != labels.end(); label_it++)
	{
		symbolS *label = *label_it;
		S_SET_VALUE(label, S_GET_VALUE(label) - 4);
	}
}

/*
 *  Check if the current packet is preceded by labels other than those
 * used for debug purposes
 */
static bool check_labels()
{
	vector<symbolS *>::iterator label_it;
	static const char *suffix = ".debugsymbol";
	static const unsigned suffix_len = 12;
	for (label_it = labels.begin(); label_it != labels.end(); label_it++)
	{
		const char *label_name = S_GET_NAME(*label_it);
		/* it's not a debug label */
		if (!endswith(label_name, suffix, suffix_len))
		{
			return true;
		}
	}

	return false;
}




//  Check that the current instruction  bundle can be compacted with the last
// one, in which case the current bundle is combined with the last
static bool compact_instr(InstrBundle &current_instr_bundle, int last_group)
{
	bool compaction = false;
	bool label = check_labels();
	int posX1, posS1, posX2, posS2;
	bool gen1, gen2;
	size_t prev_ctrl_cnt
		= ss->get_section_flags(now_seg)->prev_control_counter;

	if (!in_loop
		&& !label
		&& check_compactable(last_instr_bundle, posX1, posS1, gen1)
		&& check_compactable(current_instr_bundle, posX2, posS2, gen2)
		&& instrInfos[last_group].size() == 1
		&& !assembler_directive
		&& prev_ctrl_cnt > 2)
	{
		// Save the space allocated to the last instruction bundle.
		char *f = instrInfos[last_group][0].f;
		// Clear the new bundle.
		instrInfos[last_group].clear();
		// Add the corresponding "swbreak" instruction if present.
#if defined(_VSPA2_)
		if (posX1 != -1)
		{
			if (posX2 != -1)
			{
				instrInfos[last_group].push_back(createInstr(0, "swbreak"));
			}
			instrInfos[last_group].push_back(createInstr(0, "swbreak"));
		}
		else if (posX2 != -1)
		{
			instrInfos[last_group].push_back(createInstr(0, "opX_nop"));
			instrInfos[last_group].push_back(createInstr(0, "swbreak"));
		}
#else
		if (posX1 != -1)
		{
			if (posX2 != -1)
			{
				instrInfos[last_group].push_back(createInstr(0, "swbreak"));
			}
			else
			{
				instrInfos[last_group].push_back(
					createInstr(0, "swbreak_lower"));
			}
		}
		else if (posX2 != -1)
		{
			instrInfos[last_group].push_back(createInstr(0, "swbreak_upper"));
		}
#endif
		// Add the "opS" instructions.
		if (gen1)
		{
			std::string opcode;
			if (get_instance_name(last_instr_bundle[posS1], opcode))
			{
				instrInfos[last_group].push_back(
					replaceInstr(last_instr_bundle[posS1], opcode.c_str()));
			}
			else
			{
				as_fatal("Cannot instantiate generic instruction");
			}
		}
		else
		{
			instrInfos[last_group].push_back(last_instr_bundle[posS1]);
		}

		if (gen2)
		{
			std::string opcode;
			if (get_instance_name(current_instr_bundle[posS2], opcode))
			{
				instrInfos[last_group].push_back(
					replaceInstr(current_instr_bundle[posS2], opcode.c_str()));
			}
			else
			{
				as_fatal("Cannot instantiate generic instruction");
			}
		}
		else
		{
			instrInfos[last_group].push_back(current_instr_bundle[posS2]);
		}

		for (unsigned i = 0; i < instrInfos[last_group].size(); i++)
		{
			instrInfos[last_group][i].order = i;
		}

		// Combine the two compacted macroinstructions.
		int current_group = curgrp();
		instrInfos.set_current_group(last_group);

		if (post_packet_asm_handler[core])
		{
			(*post_packet_asm_handler[core])(instrInfos[last_group],
				last_group);
			current_instr_bundle.clear();
		}

		// Set the allocated space.
		instrInfos[last_group][0].f = f;
		instrInfos.set_current_group(current_group);

		adjust_label_values();

		compaction = true;
	}

	// Update the level of loop imbrication.
	if (check_loop_end(last_instr_bundle))
	{
		lbs->pop();
		lis->pop();
		in_loop--;
	}

	// Save the current instruction bundle.
	last_instr_bundle = current_instr_bundle;

	assembler_directive = false;

	return compaction;
}

/*
 *  Check if there is more than one "opS" instruction in the user defined
 * macroinstruction
 */
static bool check_opS_instr(InstrBundle &instr_bundle, int &pos)
{
  int num_opS = 0;
  for (unsigned int i = 0; i < instr_bundle.group_size(); i++) {
    if (check_opS_instr(instr_bundle.get_instr(i))) {
      num_opS++;
      pos = i + 1;
    }
  }

  return num_opS > 1;
}

/*
 *  Check if the bundle consists in a single "null" instruction
 */
static bool check_null_bundle(InstrBundle &instr_bundle)
{
  if (instr_bundle.group_size() != 1) {
    return false;
  }

  return check_null_instr(instr_bundle.get_instr(0));
}

//  Check delay slot restrictions.
static void check_delay_slot_restrictions(
									const bool &prev_rts,
									const bool &prev_jmp_jsr,
									const struct jmp_jsr_info &prev_jj_info,
									const bool &loop_begin, const bool &rts,
									const bool &jmp_jsr,
									const struct jmp_jsr_info &jj_info)
{
	if (prev_rts)
	{
		if (loop_begin)
		{
			as_bad("found \'loop_begin\' in the delay slot of \'rts\'");
		}
		else if (rts)
		{
			as_bad("found \'rts\' in the delay slot of \'rts\'");
		}
		else if (jmp_jsr)
		{
			as_bad("found \'" JMP_JSR_INST "\' in the delay slot of a \'rts\'");
		}
	}

	if (prev_jmp_jsr)
	{
		const char *type = prev_jj_info.cond ? "" : "un";
		if (loop_begin)
		{
			as_bad("found \'loop_begin\' in the delay slot of "
				"%sconditional \'" JMP_JSR_INST "\'", type);
		}
		else if (rts)
		{
			as_bad("found \'rts\' in the delay slot of %sconditional "
				"\'" JMP_JSR_INST "\'", type);
		}
	
		if (jmp_jsr && !jj_info.cond)
		{
			if (jj_info.jmp_mnemonic)
			{
				if (!prev_jj_info.jmp_mnemonic)
				{
					as_bad("found unconditional \'jmp\' in the delay slot of "
						"%sconditional \'" JMP_JSR_INST "\'", type);
				}
#if !defined(_VSPA2_)
				else
				{
					as_warn("found unconditional \'jmp\' in the delay slot of "
						"%sconditional \'jmp\'", type);
				}
#endif
			}
			else
			{
				as_bad("found unconditional \'" JMP_JSR_INST "\' in the delay "
					"slot of %sconditional \'" JMP_JSR_INST "\'", type);
			}
		}
	}
}

/*
 *  Generate a new BigFoot symbol name
 */
static string get_bf_symbol_name()
{
  static long id = 0;
  static const char *prefix = "__BF_";
  static const char *suffix = "_symbol";
  stringstream ss("");
  ss << prefix
    << id++
    << suffix;

  return ss.str();
}

/*
 *  Create a new BigFoot symbol and add the corresponsding entry in the
 * ".MW_INFO" in order to mark it as such
 */
static symbolS *create_bf_symbol(segT seg, fragS *frag, valueT where)
{
  symbolS *symbol = symbol_new(get_bf_symbol_name().c_str(), seg, where, frag);
  symbol_get_tc(symbol)->is_bf = 1;

  return symbol;
}

//  Check for DMEM access/mvip inconsistencies, and generate
// appropriate ".MW_INFO" entries if necessary
static void check_dmem_ipwrite_inconsistency_linear(bool dmem, bool ipwrite)
{
	if (dmem && ipwrite)
	{
		as_warn("Found mvip instruction 2 cycles after data access instruction, which represents an instance of the double ip write hardware issue (TKT292594).");
	}
}

//  Check for DMEM access/change-of-flow inconsistencies, and generate
// appropriate ".MW_INFO" entries if necessary
static void check_dmem_cof_or_mvip_inconsistency(bool dmem, bool rts, bool jmp_jsr,
	bool loop_end, struct coff_info &info)
{
	if (!(rts || jmp_jsr || loop_end))
	{
		return;
	}

	if (loop_end)
	{
		// There are 2 cases for loops:
		// a. Do not place any instructions of a 1 or 2 instruction loop
		// within 3 instructions before a PMEM boundary.
		// b. If a 3+ instruction loop crosses a PMEM boundary, do not
		// locate a DRI 3 instructions before the last.
		struct address_info *lbi = lbs->top();
		if (!lbi)
		{
			return;
		}

		// We assume case b.
		valueT loop_end_addr = frag_now_fix();
		enum cof_type type = cof_loop;
		char delay = -2;

		if (dmem_coff)
		{
			// 2 * 8 = two instruction words.
			if ((loop_end_addr - lbi->where) < 2 * 8)
			{
				// Case a.
				type = cof_loop_one_two;
				delay = 0;
			}
			else if (!dmem)
			{
				// More than 3 instructions in loop but no dmem access before
				// "loop_end".
				return;
			}
		}

		symbolS *target_symbol = create_bf_symbol(lbi->seg, lbi->frag,
			lbi->where);
		symbolS *coff_symbol = create_bf_symbol(now_seg, frag_now,
			loop_end_addr);
		mwinfo_save_bf_info(type, coff_symbol, target_symbol, 0, delay);
	}
	else if (!dmem)
	{
		// All the other patterns need a dmem.
		return;
	}

	if (rts)
	{
		symbolS *coff_symbol = create_bf_symbol(now_seg, info.frag,
			info.where);
		mwinfo_save_bf_info(cof_rts, coff_symbol, NULL, 0, -2);
	}
	else if (jmp_jsr)
	{
		symbolS *coff_symbol = create_bf_symbol(now_seg, info.frag, info.where);
		mwinfo_save_bf_info(info.type, coff_symbol, info.target, info.imm, -2);
	}
}

//  Check for DMEM access/change-of-flow inconsistencies, and generate
// appropriate ".MW_INFO" entries if necessary.
static void check_dmem_cof_or_mvip_inconsistency(bool dmem, bool rts, bool jmp_jsr,
	struct coff_info &info, char delay)
{
	if (!(dmem && (rts || jmp_jsr)))
	{
		return;
	}

	symbolS *coff_symbol = create_bf_symbol(now_seg, info.frag, info.where);
	mwinfo_save_bf_info(info.type, coff_symbol, info.target, info.imm, delay);
}

//  Check for DMEM access/change-of-flow inconsistencies, and generate
// appropriate ".MW_INFO" entries if necessary.
static void check_dmem_cof_or_mvip_inconsistency(bool loop_begin_dmem,
	bool loop_end,
	bool rts,
	bool jmp_jsr,
	struct coff_info &cinfo,
	const loop_info *li, bool mvip)
{
	if (!(loop_begin_dmem && (rts || jmp_jsr || mvip)))
	{
		return;
	}
	
	// Check if the size of the hardware loop is 1.
	if (!loop_end)
	{
		const loop_info_val *liv = li->size;
		if (!(liv->is_const() && liv->get_value() == 1) &&
			!(liv->is_diff() && liv->diff_is_zero()))
		{
			return;
		}
	}

	// Check the iteration count of the loop.
	const loop_info_val *liv = li->iter_cnt;
	if (!(liv->is_unknown()
		|| liv->is_reg()
		|| (liv->is_const() && liv->get_value() > 1)))
	{
		return;
	}

	if (mvip)
	{
		as_warn("Found mvip instruction 2 cycles after data access instruction, which represents an instance of the double ip write hardware issue (TKT292594).");
	}
	// Generate ".MW_INFO" entries.
	else if (rts)
	{
		symbolS *coff_symbol = create_bf_symbol(now_seg, cinfo.frag,
			cinfo.where);
		mwinfo_save_bf_info(cof_rts, coff_symbol, NULL, 0, -1);
	}
	else if (jmp_jsr)
	{
		symbolS *coff_symbol = create_bf_symbol(now_seg, cinfo.frag,
			cinfo.where);
		mwinfo_save_bf_info(cinfo.type, coff_symbol, cinfo.target, cinfo.imm,
			-1);
	}
}

//  Extract information related to "jmp", "jsr" and "rts" instructions.
static void extract_coff_info(const InstrBundle &instr_bundle,
							  const int jmp_jsr_index, bool &jmp_jsr,
							  const bool &rts, struct coff_info &info)
{
	memset(&info, 0, sizeof(struct coff_info));

	if (rts)
	{
		info.type = cof_rts;
		info.frag = frag_now;
		info.where = frag_now_fix();
		return;
	}

	if (jmp_jsr_index == instr_bundle.group_size())
	{
		return;
	}

	info.frag = frag_now;
	info.where = frag_now_fix();

	const InstrInfo &instr = instr_bundle.get_instr(jmp_jsr_index);

	jmp_jsr = true;

	// Check if the "jmp/jsr" uses a register or an immediate/symbol.
	assert(instr.opcode
		&& instr.opcode->num_operands >= 2
		&& instr.num_operand_values >= 2);
	if (instr.opcode->operands[1].flags & ADL_REGISTER)
	{
		info.type = cof_jmp_jsr_reg;
	}
	else if (instr.operand_values[1].X_op == O_symbol)
	{
		info.target = instr.operand_values[1].X_add_symbol;
		info.type = cof_jmp_jsr_label;
	}
	else if (instr.operand_values[1].X_op == O_constant)
	{
		info.imm = instr.operand_values[1].X_add_number;
		info.type = cof_jmp_jsr_imm;
	}
}

//  Extract loop specific information from "set.loop" instructions.
static void extract_set_loop_info(InstrBundle &instr_bundle,
	int set_loop_index)
{
	InstrInfo &instr = instr_bundle[set_loop_index];
	assert(instr.opcode
		&& instr.opcode->num_operands >= 1
		&& instr.num_operand_values >= 1);

	bool iter_cnt_is_reg = false;
	for (size_t i = 0; i < instr.opcode->num_operands; i++)
	{
		if (!instr.opcode->operands[i].arg
			&& (instr.opcode->operands[i].flags & ADL_REGISTER))
		{
			iter_cnt_is_reg = true;
			break;
		}
	}

	// Extract loop iteration count.
	if (iter_cnt_is_reg)
	{
		lis->update_iter(new loop_info_val_reg());
	}
	else if (instr.operand_values[0].X_op == O_constant)
	{
		lis->update_iter(new loop_info_val_const(
			instr.operand_values[0].X_add_number));
	}
	else
	{
		lis->update_iter(new loop_info_val_unkn());
	}

	// Extract loop size.
	if ((instr.opcode->num_operands == 2 && instr.num_operand_values == 2)
		|| (instr.opcode->num_operands == 3 && instr.num_operand_values == 3))
	{
		if (instr.opcode->num_operands == 2
			&& instr.operand_values[1].X_op == O_constant)
		{
			lis->update_size(new loop_info_val_const(
				instr.operand_values[1].X_add_number));
		}
		else if (instr.operand_values[1].X_op == O_subtract
			&& (instr.opcode->num_operands == 2
				|| instr.operand_values[2].X_op == O_illegal))
		{
			lis->update_size(new loop_info_val_diff(
				instr.operand_values[1].X_op_symbol,
				instr.operand_values[1].X_add_symbol));
		}
	}
}

//  Check if the current instruction bundle contains control flow and DMEM
// access instructions, information used for short macroinstructions compaction
// and delay slots restrictions enforcing.
static void check_instr_bundle(InstrBundle &instr_bundle, bool &rts,
							   bool &jmp_jsr, struct jmp_jsr_info &jj_info,
							   bool &dmem, bool &loop_begin, bool &loop_end,
							   struct loop_info *&li, struct coff_info &cinfo)
{
	bool reg;
	bool mvip;
	size_t jmp_jsr_index, set_loop_index;

	// Scan the current instruction bundle to find control and DMEM access
	// instructions.
	check_control_dmem_instrs(instr_bundle, loop_begin, loop_end,
		set_loop_index, rts, jmp_jsr_index, jj_info, dmem, mvip);

	jmp_jsr = false;
	extract_coff_info(instr_bundle, jmp_jsr_index, jmp_jsr, rts, cinfo);
	li = lis->top()->clone();
	assert(li);

	struct section_flags *sf = ss->get_section_flags(now_seg);

	// Check the delay slot restrictions.
	check_delay_slot_restrictions(sf->prev2_rts, sf->prev2_jmp_jsr,
		sf->prev2_jj_info, loop_begin, rts, jmp_jsr, jj_info);
	check_delay_slot_restrictions(sf->prev_rts, sf->prev_jmp_jsr,
		sf->prev_jj_info, loop_begin, rts, jmp_jsr, jj_info);

	// Update the level of loop imbrication.
	if (loop_begin)
	{
		lbs->push();
		lis->push();
		in_loop++;
	}

	// Check for DMEM access/change-of-flow inconsistencies (BigFoot).
	if (dmem_coff || dmem_mvip)
	{
		if (dmem_mvip)
		{
			check_dmem_ipwrite_inconsistency_linear(sf->prev2_dmem, mvip);
		}
		check_dmem_cof_or_mvip_inconsistency(sf->prev2_dmem, rts, jmp_jsr, loop_end, cinfo);

		// Check for delay slot DMEM access inconsistencies.
		check_dmem_cof_or_mvip_inconsistency(dmem, sf->prev2_rts, sf->prev2_jmp_jsr,
			sf->prev2_coff_info, 2);
		check_dmem_cof_or_mvip_inconsistency(dmem, sf->prev_rts, sf->prev_jmp_jsr,
			sf->prev_coff_info, 1);
		check_dmem_cof_or_mvip_inconsistency(sf->prev_dmem & sf->prev_loop_begin,
				sf->prev_loop_end, rts, jmp_jsr, cinfo, sf->prev_loop_info, mvip);
	}

	// Extract loop specific information.
	if (set_loop_index < instr_bundle.group_size())
	{
		extract_set_loop_info(instr_bundle, set_loop_index);
	}

	// Increment control counter.
	sf->inc_control_counter(jmp_jsr || rts);
}

//  Collect empty sections.
static void collect_empty_sections(bfd *abfd ATTRIBUTE_UNUSED, asection *sec,
								   void *arg)
{
	std::unordered_set<asection *> *sections
		= static_cast<std::unordered_set<asection *> *>(arg);

	// Skip reg and expr internal assembler sections because they are removed
	// later by GAS.
	if (seg_not_empty_p(sec) == 0
		&& sec != reg_section
		&& sec != expr_section)
	{
		sections->insert(sec);
	}
}

//  Renumber sections.
static void renumber_sections(bfd *abfd ATTRIBUTE_UNUSED, asection *sec,
							  void *arg)
{
	int *countp = static_cast<int *>(arg);

	sec->index = *countp;
	++*countp;
}

//  Collect and remove empty sections that don't have symbols other than the
// section symbols referencing them.
static void remove_empty_sections()
{
	std::unordered_set<asection *> sections;

	// Collect empty sections.
	bfd_map_over_sections(stdoutput, collect_empty_sections, &sections);

	if (sections.empty())
	{
		return;
	}

	// Check if the symbol table contains symbols other than the section
	// symbols that reference a section proposed for removal. If this is the
	// case, the section will not be removed.
	for (symbolS *sym = symbol_rootP; sym; sym = symbol_next(sym))
	{
		if (!sym->bsym || (sym->bsym->flags & BSF_SECTION_SYM))
		{
			continue;
		}

		segT sec = S_GET_SEGMENT(sym);
		if (sections.find(sec) != sections.end())
		{
			sections.erase(sec);
		}
	}

	// Remove sections.
	for (std::unordered_set<asection *>::iterator its = sections.begin(),
		ite = sections.end(); its != ite; ++its)
	{
		asection *sec = *its;
		bfd_section_list_remove(stdoutput, sec);
		symbol_remove(section_symbol(sec), &symbol_rootP, &symbol_lastP);
	}

	// Renumber sections.
	stdoutput->section_count -= sections.size();
	int i = 0;
	bfd_map_over_sections(stdoutput, renumber_sections, &i);

	// When removing empty sections there is a chance that the symbol table
	// will be empty, so it won't be generated. Since the linker always expects
	// a symbol table, we trick the assembler to generate the symbol table by
	// telling it that the object has relocations.
	if (bfd_get_symcount(stdoutput) == 0)
	{
		stdoutput->flags |= HAS_RELOC;
	}
}

//  Free the memory allocated for the section state flags, remove empty
// sections and collect information related to functions.
void adl_end()
{
	delete lis;
	delete ss;
	delete lbs;

	remove_empty_sections();

#if defined(_VSPA2_)
	if (vspa1_on_vspa2)
	{
		clear_instr_mapping();
	}
#endif

	collect_func_info();
}

/*
 *  Check if the current instruction contains a "loop_end" instruction, in
 * which case it is attached to the previous bundle
 */
static void check_compact_loop_end(InstrBundle &current_instr_bundle,
                                        int last_group)
{
  unsigned pos = 0;
  InstrBundle &last_bundle = instrInfos[last_group];

  if (!check_loop_end(current_instr_bundle, pos)) {
    goto end;
  }

  if (last_instr_bundle.empty()) {
    as_bad("single loop_end at the beginning of the code section");
    goto end;
  }

  {
    /* save the space allocated to the last instruction bundle */
    char *f = last_bundle[0].f;

    /* update the previous bundle */
    last_bundle.clear();
    last_bundle.insert(last_bundle.begin(), last_instr_bundle.begin(),
        last_instr_bundle.end());
    last_bundle.push_back(current_instr_bundle[pos]);

    /* if the current bundle becomes empty a "nop" instruction is added */
    if (current_instr_bundle.size() == 1) {
      current_instr_bundle.push_back(createInstr(3, "nop", 0, 0, 0));
    }
    current_instr_bundle.erase(current_instr_bundle.begin() + pos);

    for (unsigned i = 0; i < last_bundle.size(); i++) {
      last_bundle[i].order = i;
    }

    /* combine the compacted instructions */
    int current_group = curgrp();
    instrInfos.set_current_group(last_group);

    if (post_packet_asm_handler[core]) {
      (*post_packet_asm_handler[core])(last_bundle, last_group);
    }

    /* set the space allocated */
    last_bundle[0].f = f;
    instrInfos.set_current_group(current_group);
  }

end:
  last_instr_bundle = current_instr_bundle;
}

static void write_instrs(void)
{
	unsigned instr_arr[MAX_INSTR_WORDS];
	unsigned *instr = instr_arr;
	 int last_group = parallel_arch ? (curgrp() + 1) % queue_size : 0;

	// Toggle the fact that we have not yet saved the prefix.
	prefix_saved = false;

	// Run post packet handler and any final rules. Note: Re-ordering might
	// take place here.  First, set the order members to reflect the initial
	// order.
	for (unsigned i = 0; i != instrInfos[curgrp()].size(); ++i)
	{
		instrInfos[curgrp()][i].order = i;
	}

	if (core == CORE_VCPU)
	{
		// Check for more than one VSPA "opS" family instruction in the same
		// macroinstruction/
		int pos;

		if (check_opS_instr(instrInfos[curgrp()], pos))
		{
			as_bad("Invalid opS instruction at the position of %d", pos);
		}

#if !defined(_VSPA2_)
		//  Check configuration specific instruction bundle restrictions.
		if (AU_NUM == 64)
		{
			check_bundle_restrictions(instrInfos[curgrp()]);
		}
#endif

		check_rv_write_restriction(instrInfos[curgrp()]);

		// Check for control instructions in the current bundle.
		bool rts, jmp_jsr, dmem, loop_begin, loop_end;
		struct coff_info cinfo;
		struct loop_info *li = NULL;
		struct jmp_jsr_info jj_info;
		check_instr_bundle(instrInfos[curgrp()], rts, jmp_jsr, jj_info, dmem,
			loop_begin, loop_end, li, cinfo);

		// When compaction is done we have to generate debug info for the
		// compacted instruction.
		if (compact_instr(instrInfos[curgrp()], last_group))
		{
			dwarf2_emit_insn(8);
			ss->update_section_flags(now_seg, rts, jmp_jsr, jj_info, dmem, cinfo,
				loop_begin, loop_end, li);
		}
		else
		{
			if (check_null_bundle(instrInfos[curgrp()]))
			{
				struct section_flags *sf = ss->get_section_flags(now_seg);
				sf->dec_control_counter();
				instrInfos[curgrp()].clear();
				delete li;
			}
			else if (instrInfos[curgrp()].size())
			{
				ss->update_section_flags(now_seg, rts, jmp_jsr, jj_info, dmem,
					cinfo, loop_begin, loop_end, li);
			}
			else
			{
				delete li;
			}
		}

		// Check for generic instructions and choose the proper family.
		check_generic_instrs(instrInfos[curgrp()]);
	}
	else
	{
		check_compact_loop_end(instrInfos[curgrp()], last_group);
	}

	if (instrInfos[curgrp()].size())
	{
		if (post_packet_asm_handler[core])
		{
			(*post_packet_asm_handler[core])(instrInfos[curgrp()],curgrp());
		}
		if (post_packet_rule_checker[core] && check_asm_rules)
		{
			(*post_packet_rule_checker[core])(instrInfos[curgrp()],curgrp());
		}

		// Emit debug information for the current instruction (only for VCPU).
		if (core == CORE_VCPU)
		{
			dwarf2_emit_insn(0);
		}
	}

	if (parallel_arch && !prefix_saved)
	{
		savePrefix(instrInfos[curgrp()]);
	}

	alloc_instr_bufs(curgrp());

	// The above processes the current group while the below writes out the
	// last group processed before. Need adl_cleanup() to write out all the
	// groups held in the queue when the assembly is done. Pay attention to
	// when the cleanup is processed. Make sure the instructions written out
	// in the cleanup are processed properly.
	bool wrote_prefix = false;
	// Now write instructions. If the last group is empty, the loop is skipped.
	for (unsigned i = 0; i != instrInfos[last_group].size(); ++i)
	{
		// Write prefix in the appropriate location.
		if (!wrote_prefix && i == instrInfos[last_group].prefix_index())
		{
			write_prefix(last_group);
		wrote_prefix = true;
		}
		
		InstrInfo &info = instrInfos[last_group][i];
		// Initial instruction setup.
		struct adl_opcode *opcode = info.opcode;

		if(opcode == NULL)
		{
			continue;
		}

		for (int k = 0; k != MAX_INSTR_WORDS; ++k)
		{
			instr[k] = opcode->opcode[k];
		}

		int instr_sz = opcode->size;
		uint32_t addr_mask = opcode->addr_mask;
		// Uses *_now variables, may be need to take them out ?
		// handle_itable_change(InstrInfos[last_group][i].instr_table_name,
		// InstrInfos[last_group][i].instr_table);
		// Now process all of the operands, placing them into the instruction
		// or registering fixups, if needed.
		int num_fixups = 0 ;
		DeclArray(adl_fixup, fixups, info.maxfields);

		build_instruction(instr, &info, fixups, &num_fixups);

		// Write out the instruction.
		char *f = info.f;
		fragS *tmp_frag = frag_now;
		if (last_group != curgrp())
		{
			frag_now = last_frag;
		}
		write_instr(f, instr, instr_sz, frag_now, addr_mask);

		// Handle fixups here.
		handle_fixups(f, frag_now, fixups, num_fixups, instr_sz);
		frag_now = tmp_frag;
	}

	// Catch the situation where the prefix should be at the end.
	if (!wrote_prefix)
	{
		write_prefix(last_group);
	}

	// Reset prefix.
	reset_prefix(last_group);

	// Clear the last group and reserve the space for the next group coming
	instrInfos[last_group].clear(); 
	instrInfos.set_current_group(parallel_arch ? last_group : 0);

	// Label is associated with a VLES, reset it when packet is written.
	curr_label = 0;

	labels.clear();

	// Save the current fragment.
	last_frag = frag_now;
}

static bool zero_operands_is_valid(struct adl_opcode *opcode)
{
  // Zero operands for this instruction, so no operands in assembly is OK.
  if (opcode->num_operands == 0) {
    return true;
  }
  // Also valid if all operands are 'assembler' operands, meaning that they're
  // not set directly on the assembly line, but rather via other hooks.
  for (int i = 0; i != opcode->num_operands; ++i) {
    if (opcode->operands[i].ptr->assembler == 0) {
      // Nope!  Found a non-assembler field, so we *do* expect to find something
      // on the assembler line.
      return false;
    }
  }
  // It's OK- only assembler fields are present.
  return true;
}

// Extract any prefix flags from the input assembly line.
static char * handle_prefix_values(Args &args,char *str,struct hash_control *instr_pfx_fields_hash,const char *tc,int &fin)
{
  // First, is it a prefix value?  If so, record for later use.  Repeat until no more prefix fields found.
  int n;
  do {
    // Find end of first word.
    n = strcspn(str,tc);

    // Then lower-case it in place.
    for (char *s = str; s != &str[n]; ++s) {
      *s = TOLOWER(*s);
    }
    if (instr_pfx_fields_hash && hash_find_n(instr_pfx_fields_hash,str,n)) {
      // It is a prefix field value, so record and continue.
      args.push_back(Arg(str,n));
      str += n;
      skip_whitespaces(&str);
    } else {
      // We don't have any prefix fields, or it doesn't match, so continue.
      break;
    }
  } while (1);
  fin = n;
  return str;
}

static bool handle_assembly_instrs(char *str,int n,struct hash_control *asm_op_hash)
{
  // Next, check if it is a "assembler instruction"
  const struct adl_asm_instr * asm_instr = (const struct adl_asm_instr *) ((asm_op_hash) ? hash_find_n(asm_op_hash,str,n) : 0);

  if (asm_instr) {
    if (asm_instr->action) {
      (asm_instr->action)(curgrp());
    }
    return true;
  }     

  return false;
}

// For simple instructions, we just call save_instr.  However, for shorthands we
// have to add on the target instruction(s) and map the shorthands operands to
// each target.
void save_instr_internal(InstrBundles &instrInfos,char *s, adl_opcode *opcode, adl_opcode *src_opcode, expressionS *operand_values,
                         const Args &args, int line_number,  int maxfields, int instr_table, const char *instr_table_name) 
{
  if (opcode->num_alias_targets) {
    for (int t_iter = 0; t_iter != opcode->num_alias_targets; ++t_iter) {
      adl_opcode     *trg = &opcode->alias_targets[t_iter];

      // Save this target instruction.  Note the recursive call: I want to be
      // able to handle arbitrary levels of indirection.
      save_instr_internal(instrInfos,s,trg,opcode,operand_values,args,line_number,maxfields,instr_table,instr_table_name);
    }
  } else {
    // Not a shorthand, so just save this item. // The item is saved into current group internally.
    save_instr(instrInfos,s,opcode,src_opcode,operand_values,args,line_number,maxfields,instr_table,instr_table_name);
    if (opcode->action) { 
      execute_action(opcode,operand_values,line_number,maxfields,curgrp()); 
    }
  }
}

// Find an end-of-instruction marker.  If present, null it out and return a
// pointer to that terminator.  If not present, returns 0.
char *find_eoi_marker(char *str)
{
  if (!instr_separators) return 0;
  size_t r = strcspn(str,instr_separators); // Search for the length not consisting of the end-of-instruction characters.
  if (!r) return 0;                         // 0 search meant that we were passed an empty string.
  char *next = &str[r];
  if (*next) {                              // If the string doesn't point to null, then null it out and return it.
    *next = 0;
    return next;
  }
  return 0;                                 // We're at the end of the string, so we consider that a no-separator situation.
}

// Given a default string and a string returned by find_eoi_marker, returns the
// latter if it's non-null, otherwise the first.
void restore_eoi_marker(char *eoi)
{
  if (eoi) {
    // Should only ever get a non-null string here if instr_separators isn't
    // null, since that's checked by find_eoi_marker.
    *eoi = instr_separators[0];
  }
}

char *next_instr(char *defstr,char *eoi_str)
{
  return (eoi_str) ? eoi_str+1 : defstr;
}

/*
 *  Convert a string to a long value and signal if any conversion errors
 * occured
 */
bool parse_value(const char *string, long &value) {
  char *end;

  errno = 0;
  value = strtol(string, &end, 0);

   /* input out of range */
  if ((value == LONG_MIN || value == LONG_MAX) && errno == ERANGE) {
    return false;
  }

  /* no conversion possible */
  if (end == string) {
    return false;
  }

  /*trailing grabage */
  if (*end != '\0') {
    return false;
  }

  return true;
}

/*
 *  Check for a "__VRA_ELEM()" macro and expand it if present
 */
static bool handle_vra_elem_macro(char **args, char **num)
{
  char *macro_end = strchr(*args, ')');
  if (!macro_end)
  {
    return false;
  }

  char *operands = *args + 11;
  *macro_end = 0;
  char *comma = strchr(operands, ',');
  if (!comma)
  {
    return false;
  }

  /* parse macro operands and compute macro value */
  *comma = 0;
  long op1, op2;
  if (!parse_value(operands, op1) || !parse_value(comma + 1, op2))
  {
    return false;
  }
  long result = op1 * 256 + op2;

  /* advance to the end of the macro; expect comma or end of string */
  *args = macro_end + 1;
  if (**args == ',')
  {
    *args = *args + 1;
  }
  else if (**args)
  {
    return false;
  }

  char number[128];
  sprintf(number, "%ld", result);
  *num = strdup(number);

  return true;
}

/*
 *  Parse regular instruction operand delimited by a comma or the end of the
 * operand list
 */
static void handle_operand(char **args, char **op)
{
  char *comma = strchr(*args, ',');
  if (!comma) {
    *op = strdup(*args);
    *args += strlen(*args);
  } else {
    *comma = 0;
    *op = strdup(*args);
    *args = comma + 1;
  }
}

/*
 *  Free the memory allocated for the instruction's operands
 */
static void free_operands(vector<char *> ops)
{
  for (vector<char *>::iterator op = ops.begin(); op != ops.end(); op++) {
    free(*op);
  }
}

//  Check if an instruction is a "set*.VRA*", which uses the "__VRA_ELEM()"
// macro; in this case the macro is expanded for each operand and the new
// instruction is assembled.
static bool check_built_in_macro(char *str,
	struct hash_control *op_hash, 
	struct hash_control *asm_op_hash ATTRIBUTE_UNUSED,
	struct hash_control *reg_hash,
	struct hash_control *instr_pfx_fields_hash,
	int maxfields, uint32_t line_number,
	int instr_table, const char *instr_table_name)
{
	static const char *vra_elem_prefix = "__VRA_ELEM(";
	static const unsigned vra_elem_prefix_len = 11;
	size_t fin = strcspn(str, min_terminating_chars);
	if (fin == 0)
	{
		return false;
	}

	int (*compare)(const char *, const char *, size_t) = ignore_case
		? strncasecmp : strncmp;
	// Check mnemonic.
	if (compare(str, "set.VRAincr", fin)
		&& compare(str, "set.VRAptr", fin)
		&& compare(str, "setA.VRAptr", fin)
		&& compare(str, "setB.VRAptr", fin)
		&& compare(str, "set.VRArange1", fin)
		&& compare(str, "set.VRArange2", fin))
	{
		return false;
	}

	// Parse operands and expand macros.
	char *instr = strdup(str + fin);
	char *args = instr, *op;
	bool macro = false;
	skip_whitespaces(&args);
	vector<char *> operands;
	while(*args)
	{
		if (!compare(args, vra_elem_prefix, vra_elem_prefix_len))
		{
			if (handle_vra_elem_macro(&args, &op))
			{
				macro = true;
				operands.push_back(op);
			}
			else
			{
				free(instr);
				free_operands(operands);
				return false;
			}
		}
		else
		{
			handle_operand(&args, &op);
			operands.push_back(op);
		}
	}

	// Nothing happens if there is no macro.
	if (!macro)
	{
		free_operands(operands);
		free(instr);
		return false;
	}

	// Create a new instruction using the value resulted after macro expansion.
	char new_instr[512];
	memcpy(new_instr, str, fin);
	new_instr[fin] = ' ';
	args = new_instr + fin + 1;
	for (vector<char *>::iterator op = operands.begin();
		op != operands.end(); op++)
	{
		unsigned len = strlen(*op);
		memcpy(args, *op, len);
		args[len] = ',';
		args += len + 1;
		free(*op);
	}
	args[-1] = 0;

	// Assemble expanded instruction.
	skip_macro_check = true;
	adl_assemble_instr(new_instr, op_hash, asm_op_hash, reg_hash,
		instr_pfx_fields_hash, maxfields,line_number, instr_table,
		instr_table_name);
	skip_macro_check = false;

	free(instr);
	return true;
}

//  Check if a "st Iu17, I32" pseudoinstruction must be expanded, in which case
// a "st.h Iu17, Iu16" is generated by the assembler.
static void check_expand_macro(struct hash_control *op_hash,
							struct hash_control *asm_op_hash ATTRIBUTE_UNUSED,
							struct hash_control *reg_hash,
							struct hash_control *instr_pfx_fields_hash,
							int maxfields, uint32_t line_number,
							int instr_table, const char *instr_table_name)
{
	if (!expand_instr)
	{
		return;
	}

	// Assemble and build instruction.
	adl_assemble_instr(expand_instr, op_hash, asm_op_hash, reg_hash,
		instr_pfx_fields_hash, maxfields,line_number, instr_table,
		instr_table_name);
	write_instrs();

	free(expand_instr);
	expand_instr = NULL;
}

//  Assemble single instruction.
char *adl_assemble_instr(char *str, struct hash_control *op_hash,
						 struct hash_control *asm_op_hash ATTRIBUTE_UNUSED,
						 struct hash_control *reg_hash,
						 struct hash_control *instr_pfx_fields_hash,
						 int maxfields, uint32_t line_number, int instr_table,
						 const char *instr_table_name)
{
	skip_whitespaces(&str);
	int op_index;
	const struct adl_instr *instr;
	struct adl_opcode *opcode;
	const int BufSize = 512;
	char *orig_str = str;
	bool hprec_err = false, dprec_err = false, dp_macro_err = false;

	char *eoi = find_eoi_marker(str);

	// Check for a "set*.VRA*" pseudoinstruction.
	if (core == CORE_VCPU
		&& !skip_macro_check
		&& check_built_in_macro(str, op_hash, asm_op_hash, reg_hash,
			instr_pfx_fields_hash, maxfields,line_number, instr_table,
			instr_table_name))
	{
		return next_instr(skip_to_next(str), eoi);
	}

#ifdef _VSPA2_
	// Flag for skipping the check for compatibility mode.
	static bool skip_comp_check = false;

	if (core == CORE_VCPU
		&& vspa1_on_vspa2
		&& !skip_comp_check
		&& check_vspa1_instruction(str, op_hash, asm_op_hash, reg_hash,
			instr_pfx_fields_hash, maxfields, line_number, instr_table,
			instr_table_name, min_terminating_chars, ignore_case,
			skip_comp_check))
	{
		return next_instr(skip_to_next(str), eoi);
	}
#endif

	// This very outer loop is for checking instruction name termination. On
	// the first attempt, we use a stricter check that includes characters
	// found in instruction names. The second check uses a looser check, but
	// we try anyway since there may be duplication between enumerations used
	// in abutting fields and characters used in actual instruction names.
	for (int instr_tc_check = 0; instr_tc_check != 2; ++instr_tc_check)
	{
		// This stores the raw operand value parsed by the regular expression.
		const int nmatch = maxfields + 1;
		DeclArray(regmatch_t, pmatch, nmatch);

		// The raw argument data. We put prefix info plus regular expression
		// data into this vector.
		Args args;

		// This will store all of the operands that we parse.
		DeclArray(expressionS, operand_values, maxfields);
		memset(operand_values, 0, maxfields * sizeof(expressionS));

		int fin;
		const char *tc = ((instr_tc_check == 0) ?
			min_terminating_chars : terminating_chars);

		str = orig_str;
		char *instr_end = str;

		// Remove trailing whitespaces from the instruction.
		size_t size = strlen(str);
		while (size > 0 && is_whitespace(str[size - 1]))
		{
			str[--size] = 0;
		}

		// Are there any flags that might precede the instruction name ?
		str = handle_prefix_values(args, str, instr_pfx_fields_hash, tc, fin);

		// Before any processing is done, run the pre-assembly handler, if one
		// exists.
		if (pre_asm_handler[core])
		{
			(*pre_asm_handler[core])(curgrp());
		}

		// Check to see if we have any assembly instructions. These are special
		// pseudo instructions which just modify behavior during assembly.
		if (handle_assembly_instrs(str, fin, asm_op_hash))
		{
			str += fin;
			skip_whitespaces(&str);
			restore_eoi_marker(eoi);
			return str;
		}

		// Lookup the opcode in the table, produce an error if not found.
		instr = static_cast<const struct adl_instr *>(
			hash_find_n(op_hash, str, fin));
		if (!instr)
		{
			// Try again if we can, using a looser check; maybe the instruction
			// is there but has an abutting field.
			if (!instr_tc_check)
			{
				continue;
			}

			if (is_grouping_char(*instr_end))
			{
				AS_BAD(_("Wrong grouping character"));
			}
			else
			{
				AS_BAD(_("Unrecognized instruction: '%s'"), orig_str);
			}
			return next_instr(skip_to_next(&str[fin]), eoi);
		}

		// Loop over the possible opcodes for this instruction. If we don't
		// find a match against its regular expression, we skip to the next
		// instruction.
		// If we come to the end of our list, then it's an error.
		// Assuming we get a regular expression match, we then do range and
		// type checking. If we don't match, we again skip to the next
		// instruction. This allows us to do a limited form of function
		// overloading: you can have instructions of the same name that either
		// take different numbers of arguments, or different types (register
		// vs. immediate values) and of different values.
		opcode = 0;
		str = &str[fin];
		unsigned orig_args = args.size();
		for (op_index = 0; op_index != instr->num_opcodes; ++op_index)
		{
			// Remove any arguments we might have from a prior instruction
			// attempt.
			args.erase(args.begin() + orig_args, args.end());

			// Try to use the instruction with longest immediate.
			// This flag is set only by the DevTech assembler integration, it
			// signals that one of the operands is an unresolved symbol, in
			// which case the equivalent opcode with the longest immediate
			// operand (if defined), is tested for a match
			bool is_unresolved_symbol = true;
			opcode = &instr->opcodes[op_index];
			// Now parse the operands.  We do this using regular expressions
			// matching.
			if (regexec((const regex_t *) opcode->regcomp,
					str, nmatch, pmatch, 0) == REG_NOMATCH)
			{
				continue;
			}

			// Transfer regular expression arguments.  This returns false if
			// the regular expression match ends with non-whitespace or
			// string-end. This implies that the regular expression wasn't as
			// complete as it could be, e.g. we could have matched against 3
			// parameters, but only matched against two.
			if (!copy_operands(args, instr_end, opcode, str,
					pmatch, maxfields))
			{
				continue;
			}

			bool hp_err = false, dp_err = false, dpm_err = false;
			// Process each input operand.  This just handles converting raw
			// text into expressions.
			if (!process_operands(opcode, args, maxfields, operand_values,
					reg_hash, is_unresolved_symbol, hp_err, dp_err, dpm_err))
			{
				continue;
			}
			hprec_err = hp_err;
			dprec_err = dp_err;
			dp_macro_err = dpm_err;

			// Now do input validation.
			if (valid_input(opcode, operand_values))
			{
				// Input validated, so we can use this instruction.
				// An unresolved symbol exits, the instruction with longest
				// immediate is defined and it is not the current instruction
				// try it as well. If inputs are invalid so will be those of
				// the longest instruction
				if (adl_try_longest_instr
					&& is_unresolved_symbol
					&& opcode->longest != -1
					&& opcode->longest != op_index)
				{
					// This will store all of the operands that we parse.
					DeclArray(expressionS, longest_values, maxfields);
					memset(longest_values, 0, maxfields * sizeof(expressionS));

					struct adl_opcode *long_opcode =
						&instr->opcodes[opcode->longest];
					// Do not expect a regex match here, since the DevTech
					// assembler will give "unknown_immed".
					bool hp_err = false, dp_err = false, dpm_err = false;
					if (process_operands(long_opcode, args, maxfields,
							longest_values, reg_hash, is_unresolved_symbol,
							hp_err, dp_err, dpm_err)
						&& valid_input(long_opcode, longest_values))
					{
						// The instruction with the longest immed can be used.
						// Use the operand values of that instruction.
						memcpy((void *) operand_values,
							(void *) longest_values, sizeof(longest_values));
						opcode  = long_opcode;
						hprec_err = hp_err;
						dprec_err = dp_err;
						dp_macro_err = dpm_err;
					}
				}

				break;
			}
		}

		if (op_index == instr->num_opcodes)
		{
			if (!instr_tc_check)
			{
				// Try again if we can, since maybe there's another version of
				// the instruction which will match but didn't originally
				// because we have an abutting field.
				continue;
			}
			else
			{
				AS_BAD(_("%s:  No version of instruction %s matched the given"
					" arguments."), orig_str, instr->name);
				// If we never matched against a regular expression, then try
				// to advance past operands to the next possible instruction.
				// Otherwise, we have already matched against operands and
				// found some problem, so instr_end should have already been
				// updated.
				if (instr_end == orig_str)
				{
					return next_instr(skip_to_next(instr_end), eoi);
				}
				else
				{
					return next_instr(instr_end, eoi);
				}
			}
		}

		if (hprec_err)
		{
			as_bad("half precision float not supported");
		}
		if (dprec_err)
		{
			as_bad("double precision float not supported");
		}
		if (dp_macro_err)
		{
			as_bad("invalid parameter for the double precision float macro");
		}

		save_instr_internal(instrInfos, orig_str, opcode, 0, operand_values,
			args, line_number, maxfields, instr_table, instr_table_name);

		if (post_asm_handler[core])
		{
			(*post_asm_handler[core])((opcode->width), curgrp());
		}
		if (opcode->checker && check_asm_rules)
		{
			(*(opcode->checker))(instrInfos[curgrp()].back(), curgrp());
		}

		clear_preset_info();

		// If we succeed, then we're done.
		return next_instr(instr_end,eoi);
	}
}

static bool end_of_packet(char **p_str) {
  // If no grouping defined - choose grouping method
  if (!expected_end) {
    const char *p = strchr(packet_begin_chars,**p_str);
    if ( p && *p ) {
      expected_end = &packet_end_chars[p-packet_begin_chars]; 
      ++(*p_str);
      skip_whitespaces(p_str);
      return false;
    } else {
      expected_end = default_packet_sep;
      return false;
    }
  }

  skip_whitespaces(p_str);
  if (!**p_str) {
    if (*expected_end == '\n') {
      expected_end = 0;
      return true;
    } else {
      return false;
    }
  }
  // First check for end packet
  if (**p_str == *expected_end) {
    expected_end = 0;
    ++(*p_str);
    skip_whitespaces(p_str);
    // Start of packet may follow immideatly
    const char *p = strchr(packet_begin_chars,**p_str);
    if (p && *p) {
      expected_end = &packet_end_chars[p-packet_begin_chars];
      ++(*p_str);
      skip_whitespaces(p_str);
    }
    return true;
  } else {
    return false;
  }
}
static uint32_t adl_line_number=0;
static char adl_input_str[240];

extern "C" void adl_assemble(char *str,struct hash_control *op_hash,
                              struct hash_control *asm_op_hash,
                              struct hash_control *reg_hash,
                              struct hash_control *instr_pfx_fields_hash,
                              int maxfields,
                              int instr_table,
                              const char *instr_table_name,
                              uint32_t line_number)
{
  init_line_assemble(line_number);

  if (adl_show_warnings) {
    adl_line_number = line_number;
    strncpy(adl_input_str,str,240);
  }

  /* handle multicharacter comments */
  handle_comments(str);
  if (parallel_arch) {
    // if grouping is defined by new line we understand it here of it is
    // a single line that indicates end of packet we commit instructions
    bool done = false;
    if (end_of_packet(&str)){
      done = !(bool)(*str);
      write_instrs();
      if (core == CORE_VCPU)
      {
        check_expand_macro(op_hash, asm_op_hash, reg_hash,
          instr_pfx_fields_hash, maxfields, line_number,
          instr_table, instr_table_name);
      }
    }

    while (!done && *str) {
      str = adl_assemble_instr(str, op_hash, asm_op_hash, reg_hash,
              instr_pfx_fields_hash, maxfields, line_number, instr_table,
              instr_table_name);
      /* check grouping */
      if (end_of_packet(&str)) {
        done = !(bool)(*str);
        write_instrs();
        if (core == CORE_VCPU)
        {
          check_expand_macro(op_hash, asm_op_hash, reg_hash,
            instr_pfx_fields_hash, maxfields, line_number,
            instr_table, instr_table_name);
        }
      }
    }
  } else {
    adl_assemble_instr(str, op_hash, asm_op_hash, reg_hash,
      instr_pfx_fields_hash, maxfields, line_number,
      instr_table, instr_table_name);
    write_instrs();
    if (core == CORE_VCPU)
    {
      check_expand_macro(op_hash, asm_op_hash, reg_hash, instr_pfx_fields_hash,
        maxfields, line_number, instr_table, instr_table_name);
    }
  }
}

/* Convert a machine dependent frag.  We never generate these. */
void md_convert_frag (bfd *abfd ATTRIBUTE_UNUSED,asection *sec ATTRIBUTE_UNUSED,fragS *fragp ATTRIBUTE_UNUSED)
{
  abort ();
}

/* We have no need to default values of symbols.  */
symbolS *md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  if (adl_show_warnings) {
    printf("Warning: line %d: undefined symbol: %s\n Input: %s \n\n",adl_line_number,name,adl_input_str);
  }
  return 0;
}


/* We don't have any form of relaxing.  */
int md_estimate_size_before_relax (fragS *fragp ATTRIBUTE_UNUSED, asection *seg ATTRIBUTE_UNUSED)
{
  abort ();
  return 0;
}

/*
 *  Write the instructions from the instruction queue
 */
void flush_instruction_queue()
{
	for (int i = 0; i != queue_size; ++i)
	{
		write_instrs();
	}
}

extern void adl_cleanup(void)
{
	flush_instruction_queue();

	if (num_rule_errors)
	{
		AS_BAD(_("%d assembler rule violations were found."), num_rule_errors);
	}

}

extern void adl_set_curr_label(symbolS *s)
{
	curr_label = s;
	labels.push_back(s);

	if (core == CORE_VCPU)
	{
		symbol_set_debug_info(s);
		dwarf2_emit_label(s);
	}
}

#ifdef _MSC_VER

#include <intrin.h>

static uint32_t __inline clz(uint32_t x)
{
	unsigned long r = 0;
	return _BitScanReverse(&r,x);
	return (sizeof(x)*8-1) - r;
}

static uint32_t __inline ctz(uint32_t x)
{
	unsigned long r = 0;
	return _BitScanForward(&r,x);
	return r;
}

static uint32_t __inline popcnt(uint32_t x)
{
	int c;
	for (c = 0; x; c++) {
		x &= x - 1; // clear the least significant bit set
	}
	return c;
}

#else

static uint32_t inline clz(uint32_t x)
{
	return __builtin_clz(x);
}

static uint32_t inline ctz(uint32_t x)
{
	return __builtin_ctz(x);
}

static uint32_t inline popcnt(uint32_t x)
{
	return __builtin_popcount(x);
}

#endif

unsigned count_leading_zeros(offsetT n,unsigned s)
{
  if (n == 0) {
    return 0;
  } else {
    return clz((uint32_t)n) - (sizeof(offsetT)*8 - s);
  }
}

unsigned count_trailing_zeros(offsetT n)
{
  if (n == 0) {
    return 0;
  } else {
    return ctz((uint32_t)n);
  }
}

unsigned count_ones(offsetT n)
{
  return popcnt((uint32_t)n);
}

