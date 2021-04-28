#include <algorithm>
#include <iostream>
#include <sstream>
#include <regex>
#include <unordered_map>

struct param_mapping;

#define ARRAY_AND_SIZE(a) a, ARRAY_SIZE(a)

#if defined(_MSC_VER) && (_MSC_VER == 1500)
using namespace std::tr1;
#endif

#include "asm-translate.h"
#include "adl-asm-impl.h"

using namespace std;

struct param_mapping
{
	const char *old_name;
	const char *new_name;
};

//  Mnemonic mapping information.
struct mnemonic_mapping
{
	// New mnemonic.
	const char *mnemonic;
	// Regular expression corresponding to the instruction's operands.
	regex *op_reg_exp;
	// Position of operand for which the operand mapping applies.
	int position;
	// Operand mapping.
	unordered_map<string, string> *operands;
};

//  Instruction mapping information.
struct instr_mapping
{
	// Old mnemonic.
	const char *old_mnemonic;
	// Regular expression to be checked for the operands.
	const char *op_reg_exp;
	// New mnemonic.
	const char *new_mnemonic;
	// Operand position (from 0 to the number of operands - 1) for which the
	// operand mappings apply. A value of -1 means that the mappings apply to
	// all the operands.
	int position;
	// List of operand mappings.
	struct param_mapping *params;
	// Operand mappings list size.
	unsigned num_params;
};

struct param_mapping as_regs[] = {
	{ "as0", "a4" },
	{ "as1", "a5" },
	{ "as2", "a6" },
	{ "as3", "a7" },
	{ "as4", "a8" },
	{ "as5", "a9" },
	{ "as6", "a10" },
	{ "as7", "a11" },
	{ "as8", "a12" },
	{ "as9", "a13" },
	{ "as10", "a14" },
	{ "as11", "a15" },
	{ "as12", "a16" },
	{ "as13", "a17" },
	{ "as14", "a18" },
	{ "as15", "a19" },
};

struct param_mapping push_g_regs[] = {
	{ "g0", "[sp]+1,g0" },
	{ "g1", "[sp]+1,g1" },
	{ "g2", "[sp]+1,g2" },
	{ "g3", "[sp]+1,g3" },
	{ "g4", "[sp]+1,g4" },
	{ "g5", "[sp]+1,g5" },
	{ "g6", "[sp]+1,g6" },
	{ "g7", "[sp]+1,g7" },
	{ "g8", "[sp]+1,g8" },
	{ "g9", "[sp]+1,g9" },
	{ "g10", "[sp]+1,g10" },
	{ "g11", "[sp]+1,g11" },
};

struct param_mapping pop_g_regs[] = {
	{ "g0", "g0,[sp-1]" },
	{ "g1", "g1,[sp-1]" },
	{ "g2", "g2,[sp-1]" },
	{ "g3", "g3,[sp-1]" },
	{ "g4", "g4,[sp-1]" },
	{ "g5", "g5,[sp-1]" },
	{ "g6", "g6,[sp-1]" },
	{ "g7", "g7,[sp-1]" },
	{ "g8", "g8,[sp-1]" },
	{ "g9", "g9,[sp-1]" },
	{ "g10", "g10,[sp-1]" },
	{ "g11", "g11,[sp-1]" },
};

struct param_mapping r7V_regs[] = {
	{ "[r7V]", "[rV]" },
};

struct param_mapping smode_regs[] = {
	{ "S0normal", "S0hlinecplx" },
	{ "S0straight", "S0straight" },
	{ "S0ones", "S0real1" },
	{ "S0zeros", "S0zeros" },
	{ "S0abs", "S0abs" },
	{ "S0singles", "S0hword" },
	{ "S0coef1", "S0i1r1i1r1" },
	{ "S0coef2", "S0i1i1r1r1" },
	{ "S0fft1", "S0fft1" },
	{ "S0fft2", "S0fft2" },
	{ "S0fft3", "S0fft3" },
	{ "S0fft4", "S0fft4" },
	{ "S0fft5", "S0fft5" },
	{ "S0fftn", "S0fftn" },
	{ "S1normal", "S1hlinecplx" },
	{ "S1straight", "S1straight" },
	{ "S1ones", "S1real1" },
	{ "S1conj", "S1real_conj" },
	{ "S1nco", "S1nco" },
	{ "S1reverse", "S1cgu_r2c_re0" },
	{ "S1udfc", "S1i2i1r2r1" },
	{ "S1udfr", "S1udfr" },
	{ "S1r2c", "S1r2c" },
	{ "S1r2c_conj", "S1r2c_conj" },
	{ "S1r2c_im0", "S1r2c_im0" },
	{ "S1r2c_re0", "S1r2c_re0" },
	{ "S1cgu_udfc", "S1cgu_i2i1r2r1" },
	{ "S1cgu_normal", "S1cgu_normal" },
	{ "S1cgu_r2c_im0", "S1cgu_r2c_im0" },
	{ "S1cgu_r2c_re0", "S1cgu_r2c_re0" },
	{ "S2normal", "S2hlinecplx" },
	{ "S2ones", "S2real1" },
	{ "S2zeros", "S2zeros" },
	{ "S2coef1", "S2i1r1i1r1" },
	{ "S2fft1", "S2fft1" },
	{ "S2fft2", "S2fft2" },
	{ "S2fft3", "S2fft3" },
	{ "S2fft4", "S2fft4" },
	{ "S2fft5", "S2fft5" },
	{ "S2fftn", "S2fftn" },
};

struct param_mapping gg_regs[] = {
	{ "g0", "g0,g0" },
	{ "g1", "g1,g1" },
	{ "g2", "g2,g2" },
	{ "g3", "g3,g3" },
	{ "g4", "g4,g4" },
	{ "g5", "g5,g5" },
	{ "g6", "g6,g6" },
	{ "g7", "g7,g7" },
	{ "g8", "g8,g8" },
	{ "g9", "g9,g9" },
	{ "g10", "g10,g10" },
	{ "g11", "g11,g11" },
};

struct param_mapping rm_g_regs[] = {
	{ "g0", "" },
	{ "g1", "" },
	{ "g2", "" },
	{ "g3", "" },
	{ "g4", "" },
	{ "g5", "" },
	{ "g6", "" },
	{ "g7", "" },
	{ "g8", "" },
	{ "g9", "" },
	{ "g10", "" },
	{ "g11", "" },
};

struct param_mapping rm_sp_regs[] = {
	{ "sp", "" },
};

struct param_mapping set_xtrm_regs[] = {
	{ "cmp_even", "even,index" },
	{ "cmp_all", "all,index" },
};

const char *gx_gx_imm_no_reg_regex = "^g((0,g0)|(1,g1)|(2,g2)|(3,g3)|(4,g4)|(5,g5)"
	"|(6,g6)|(7,g7)|(8,g8)|(9,g9)|(10,g10)|(11,g11)),"
	"(?!((g([0-9]|1[01]))|sp|H))([^},[ ]+)$";
const char *gx_gx_gy_regex = "^g((0,g0)|(1,g1)|(2,g2)|(3,g3)|(4,g4)|(5,g5)"
	"|(6,g6)|(7,g7)|(8,g8)|(9,g9)|(10,g10)|(11,g11)),g([0-9]|1[01])$";
const char *sp_sp_imm_regex = "^sp,sp,(?!((g([0-9]|1[01]))|sp|H))([^},[ ]+)$";
const char *gx_gy_imm_regex = "^g([0-9]|1[01]),g([0-9]|1[01]),([^},[ ]+)$";
const char *gx_imm_regex = "^g([0-9]|1[01]),([^},[ ,]+)$";
const char *gx_regex = "^g([0-9]|1[01])$";
const char *gx_gy_regex = "^g([0-9]|1[01]),g([0-9]|1[01])$";
const char *gx_asy_regex = "^g([0-9]|1[01]),as([0-9]|1[0-5])$";
const char *asx_gy_regex = "^as([0-9]|1[0-5]),g([0-9]|1[01])$";
const char *a04_sp_sp_a04_regex = "^(a[0-3],sp)|(sp,a[0-3])$";

struct instr_mapping instrs[] = {
	{ "mv", gx_gy_regex, "mv.al", -1, NULL, 0 },
	{ "mv", a04_sp_sp_a04_regex, "mvS", -1, NULL, 0 },
	{ "mv", asx_gy_regex, "mvB", -1, ARRAY_AND_SIZE(as_regs) },
	{ "mv", gx_asy_regex, "mvB", -1, ARRAY_AND_SIZE(as_regs) },
	{ "mv", NULL, NULL, -1, ARRAY_AND_SIZE(as_regs) },
	{ "mvB", NULL, NULL, -1, ARRAY_AND_SIZE(as_regs) },
	{ "mvS", NULL, NULL, -1, ARRAY_AND_SIZE(as_regs) },
	{ "add", NULL, NULL, -1, ARRAY_AND_SIZE(as_regs) },
	{ "sub", NULL, NULL, -1, ARRAY_AND_SIZE(as_regs) },
	{ "set.creg", NULL, "setS.creg", -1, NULL, 0 },
	{ "set.range", NULL, NULL, -1, ARRAY_AND_SIZE(as_regs) },
	{ "pushm", NULL, NULL, -1, ARRAY_AND_SIZE(as_regs) },
	{ "popm", NULL, NULL, -1, ARRAY_AND_SIZE(as_regs) },
	{ "set.loop", NULL, NULL, -1, ARRAY_AND_SIZE(as_regs) },
	{ "mv.h", NULL, NULL, -1, ARRAY_AND_SIZE(r7V_regs) },
	{ "mv.w", NULL, NULL, -1, ARRAY_AND_SIZE(r7V_regs) },
	{ "lsb2rf", NULL, NULL, -1, ARRAY_AND_SIZE(r7V_regs) },
	{ "lsb2rf.sr", NULL, NULL, -1, ARRAY_AND_SIZE(r7V_regs) },
	{ "set.Smode", NULL, NULL, -1, ARRAY_AND_SIZE(smode_regs) },
	{ "Rrot", NULL, "ror", -1, NULL, 0 },
	{ "mad", NULL, "rmad", -1, NULL, 0 },
	{ "mac", NULL, "rmac", -1, NULL, 0 },
	{ "st.h", NULL, "sth", -1, NULL, 0 },
	{ "st.s", NULL, "st", NULL, 0 },
	{ "wr.normal", NULL, "wr.hlinecplx", -1, NULL, 0 },
	{ "push", gx_regex, "st", -1, ARRAY_AND_SIZE(push_g_regs) },
	{ "pop", gx_regex, "ld.u", -1, ARRAY_AND_SIZE(pop_g_regs) },
	{ "adda", NULL, "add", NULL, 0 },
	{ "dit", NULL, "rmad", NULL, 0 },
	{ "set.xtrm", NULL, NULL, -1, ARRAY_AND_SIZE(set_xtrm_regs) },
	{ "set.Rrot", NULL, "set.Rot", -1, NULL, 0 },
	{ "andl", gx_gy_imm_regex, "andD", -1, NULL, 0 },
	{ "orl", gx_gy_imm_regex, "orD", -1, NULL, 0 },
	{ "xorl", gx_gy_imm_regex, "xorD", -1, NULL, 0 },
	{ "andl", gx_imm_regex, "andD", -1, ARRAY_AND_SIZE(gg_regs) },
	{ "orl", gx_imm_regex, "orD", -1, ARRAY_AND_SIZE(gg_regs) },
	{ "xorl", gx_imm_regex, "xorD", -1, ARRAY_AND_SIZE(gg_regs) },
	{ "add", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "add.z", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "add.s", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "add", sp_sp_imm_regex, NULL, 0, ARRAY_AND_SIZE(rm_sp_regs) },
	{ "add.s", sp_sp_imm_regex, NULL, 0, ARRAY_AND_SIZE(rm_sp_regs) },
	{ "add.z", sp_sp_imm_regex, NULL, 0, ARRAY_AND_SIZE(rm_sp_regs) },
	{ "sub", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "sub.z", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "sub.s", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "sub", sp_sp_imm_regex, NULL, 0, ARRAY_AND_SIZE(rm_sp_regs) },
	{ "sub.s", sp_sp_imm_regex, NULL, 0, ARRAY_AND_SIZE(rm_sp_regs) },
	{ "sub.z", sp_sp_imm_regex, NULL, 0, ARRAY_AND_SIZE(rm_sp_regs) },
	{ "rsub", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "rsub.z", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "rsub.s", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "mpy", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "mpy.z", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "mpy.s", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "div", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "div.z", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "div.s", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "rdiv", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "rdiv.z", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "rdiv.s", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "mod", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "mod.z", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "mod.s", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "rmod", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "rmod.z", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "rmod.s", gx_gx_imm_no_reg_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "floatx2n", gx_gx_gy_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
	{ "lfsr", gx_gx_gy_regex, NULL, 0, ARRAY_AND_SIZE(rm_g_regs) },
};

// Mnemonic mapping.
unordered_map<string, vector<struct mnemonic_mapping *> > mnemonics;
// Operand mapping index by the operand mapping list.
unordered_map<struct param_mapping *,
	unordered_map<string, string> *> operand_maps;
unordered_map<string, regex *> regex_map;

// Load instruction and operand mappings.
void load_instr_mapping(bool ignore_case)
{
	for (unsigned i = 0, size = (sizeof(instrs) / sizeof(instrs[0]));
		i < size; i++)
	{
		struct instr_mapping &im = instrs[i];
		string old_mnemonic(im.old_mnemonic);
		if (ignore_case)
		{
			transform(old_mnemonic.begin(), old_mnemonic.end(),
				old_mnemonic.begin(), ::tolower);
		}

		regex *regexp;
		if (im.op_reg_exp)
		{
			unordered_map<string, regex *>::iterator regex_it
				= regex_map.find(im.op_reg_exp);
			
			if (regex_it == regex_map.end())
			{
				if (ignore_case)
				{
					regexp = new regex(im.op_reg_exp, regex::icase);
				}
				else
				{
					regexp = new regex(im.op_reg_exp);
				}
				regex_map[im.op_reg_exp] = regexp;
			}
			else
			{
				regexp = regex_it->second;
			}
		}
		else
		{
			regexp = NULL;
		}

		struct mnemonic_mapping *mapping =
			static_cast<struct mnemonic_mapping *>(
				malloc(sizeof(struct mnemonic_mapping)));
		mapping->mnemonic = im.new_mnemonic;
		mapping->op_reg_exp = regexp;
		mapping->position = im.position;
		mapping->operands = NULL;

		if (im.num_params)
		{
			unordered_map<struct param_mapping *, unordered_map<string,
				string> *>::iterator op_map_it = operand_maps.find(im.params);
			if (op_map_it == operand_maps.end())
			{
				unordered_map<string, string> *op_mapping =
					new unordered_map<string, string>();
				for (unsigned j = 0; j < im.num_params; j++)
				{
					struct param_mapping &param_map = im.params[j];
					string old_param(param_map.old_name);
					if (ignore_case)
					{
						transform(old_param.begin(), old_param.end(),
							old_param.begin(), ::tolower);
					}
					(*op_mapping)[old_param] = param_map.new_name;
				}

				operand_maps[im.params] = op_mapping;
				mapping->operands = op_mapping;
			}
			else
			{
				mapping->operands = op_map_it->second;
			}
		}

		unordered_map<string, vector<struct mnemonic_mapping *> >::iterator
			mnemonic_it = mnemonics.find(old_mnemonic);
		if (mnemonic_it == mnemonics.end())
		{
			vector<struct mnemonic_mapping *> mappings;
			mappings.push_back(mapping);
			mnemonics[old_mnemonic] = mappings;
		}
		else
		{
			mnemonic_it->second.push_back(mapping);
		}
	}
}

// Free the memory allocated for instruction and operand mappings.
void clear_instr_mapping()
{
	for (unordered_map<struct param_mapping *,
		unordered_map<string, string> *>::iterator map_it =
		operand_maps.begin(); map_it != operand_maps.end(); ++map_it)
	{
		delete map_it->second;
	}

	// Free the memory allocatted for mnemonic mappings and operand regular
	// expressions, if present.
	for (unordered_map<string, vector<struct mnemonic_mapping *> >::iterator
		mnemonic_it = mnemonics.begin(); mnemonic_it != mnemonics.end();
		++mnemonic_it)
	{
		vector<struct mnemonic_mapping *> &mappings = mnemonic_it->second;
		for (vector<struct mnemonic_mapping *>::iterator mapping_it =
			mappings.begin(); mapping_it != mappings.end(); ++mapping_it)
		{
			struct mnemonic_mapping *mapping = *mapping_it;
			free(mapping);
		}
	}

	// Free the memory allocated to regular expressions.
	for (unordered_map<string, regex *>::iterator regex_it = regex_map.begin();
		regex_it !=  regex_map.end(); ++regex_it)
	{
		delete regex_it->second;
	}

	mnemonics.clear();
	operand_maps.clear();
	regex_map.clear();
}

// Check for a VSPA1 instruction that whose syntax requires translation in
// order to be assembler for VSPA2.
bool check_vspa1_instruction(char *str,
	struct hash_control *op_hash,
	struct hash_control *asm_op_hash,
	struct hash_control *reg_hash,
	struct hash_control *instr_pfx_fields_hash,
	int maxfields,
	uint32_t line_number,
	int instr_table,
	const char *instr_table_name,
	const char *min_terminating_chars,
	bool ignore_case,
	bool &skip_check)
{
	unsigned fin = strcspn(str, min_terminating_chars);
	string instr(str);

	string mnemonic = instr.substr(0, fin);
	if (ignore_case)
	{
		transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(),
			::tolower);
	}

	bool change = false;
	stringstream new_instr;
	unordered_map<string, vector<struct mnemonic_mapping *> >::iterator
		mnemonic_it = mnemonics.find(mnemonic);
	unordered_map<string, string> *operand_map = NULL;
	int operand_position = -1;
	if (mnemonic_it == mnemonics.end())
	{
		new_instr << mnemonic;
	}
	else
	{
		// Check operand regular expression if there is one.
		vector<struct mnemonic_mapping *> &mappings = mnemonic_it->second;
		vector<struct mnemonic_mapping *>::iterator mapping = mappings.end();
		for (vector<struct mnemonic_mapping *>::iterator mapping_it =
			mappings.begin(); mapping_it != mappings.end(); ++mapping_it)
		{
			regex *regexp = (*mapping_it)->op_reg_exp;
			if (regexp)
			{
				string operands = instr.substr(fin + 1);
				if (regex_match(operands, *regexp))
				{
					mapping = mapping_it;
				}
			}
			else if (mapping == mappings.end())
			{
				mapping = mapping_it;
			}
		}

		if (mapping == mappings.end())
		{
			return false;
		}
		operand_map = (*mapping)->operands;
		operand_position = (*mapping)->position;

		if ((*mapping)->mnemonic == NULL)
		{
			new_instr << mnemonic;
		}
		else
		{
			new_instr << (*mapping)->mnemonic;
			change = true;
		}
	}

	if (operand_map == NULL)
	{
		if (change)
		{
			new_instr << instr.substr(fin);
		}
		else
		{
			return false;
		}
	}
	else
	{
		size_t pos = instr.find_first_not_of(" ", fin);
		if (pos == string::npos)
		{
			pos = fin;
		}
		string args = instr.substr(pos);

		size_t prev = 0;
		size_t next = 0;

		new_instr << " ";
		char *delimiters = " ,\n";
		int position = 0;
		while ((next = args.find_first_of(delimiters, prev)) !=
			std::string::npos)
		{
			if ((next - prev != 0))
			{
				string arg = args.substr(prev, next - prev);
				if ((operand_position == -1) || (operand_position == position))
				{
					string new_arg(arg);
					if (ignore_case)
					{
						transform(new_arg.begin(), new_arg.end(),
							new_arg.begin(), ::tolower);
					}
					unordered_map<string, string>::iterator op_it =
						operand_map->find(new_arg);
					if (op_it == operand_map->end())
					{
						new_instr << arg << ",";
					}
					else
					{
						string op = op_it->second;
						if (!op.empty())
						{
							new_instr << op_it->second << ",";
						}
						change = true;
					}
				}
				else
				{
					new_instr << arg << ",";
				}
				position++;
			}
			if ((prev = args.find_first_not_of(delimiters, next)) ==
				std::string::npos)
			{
				prev = next + 1;
			}
		}

		if (prev < args.size())
		{
			string arg = args.substr(prev);
			if ((operand_position == -1) || (operand_position == position))
			{
				string new_arg(arg);
				if (ignore_case)
				{
					transform(new_arg.begin(), new_arg.end(), new_arg.begin(),
					::tolower);
				}
				unordered_map<string, string>::iterator op_it =
					operand_map->find(new_arg);
				if (op_it == operand_map->end())
				{
					new_instr << arg << ",";
				}
				else
				{
					new_instr << op_it->second;
					change = true;
				}
			}
			else
			{
				new_instr << arg << ",";
			}
		}

		if (!change)
		{
			return false;
		}
	}

	string new_str= new_instr.str();
	size_t pos = new_str.size() - 1;
	if (new_str[pos] == ',')
	{
		new_str = new_str.substr(0, pos);
	}
	size_t length = new_str.length() + 1;
	char *new_inst = new char[length];
	memcpy(new_inst, new_str.c_str(), length);

	skip_check = true;
	adl_assemble_instr(new_inst, op_hash, asm_op_hash, reg_hash,
		instr_pfx_fields_hash, maxfields,line_number, instr_table,
		instr_table_name);
	skip_check = false;

	delete[] new_inst;
	return true;
}
