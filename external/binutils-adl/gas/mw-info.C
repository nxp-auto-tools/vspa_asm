#include <vector>
#include <unordered_map>
#include <assert.h>

#include "mw-info.h"

using namespace std;

#if defined(_MSC_VER) && (_MSC_VER == 1500)
using namespace std::tr1;
#endif

extern "C" symbolS *section_symbol(segT);

/* content of the "MW_INFO" section, which is kept in the memory */
struct section_content {
  bfd_byte *bytes;        /* content of the section */
  bfd_size_type size;     /* size of the content */
  bfd_size_type capacity; /* allocated capacity for the content */
};

/* symbol pair used to delimit a padding zone which is created by an
alignment */
struct align_symbols {
  symbolS *begin; /* the symbol from the beginning of the zone */
  symbolS *end;   /* the symbol from the end of the zone */
};

/* remove block information */
struct remove_block {
  symbolS *start_symbol;    /* symbol from the beginning of the block */
  symbolS *stripped_symbol; /* symbol associated with the block */
  expressionS *size;        /* size of the block */
};

// BigFoot information.
struct bf_info {
	// Change-of-flow type.
	enum cof_type type;
	// Symbol for the change of flow instruction.
	symbolS *coff_symbol;
	// Symbol to the target instruction.
	symbolS *target_symbol;
	// Immediate value.
	offsetT imm;
	// Offset of the DMEM access instruction
	char delay_offset;
};

// "MW_INFO" section information.
struct mwinfo_section {
	// The corresponding BFD section.
	segT section;
	// The content of the section.
	struct section_content *contents;
	// Map of symbols and offsets used to write the index of the symbol when
	// it's final.
	unordered_map<symbolS *, vector<bfd_size_type> *> *fixups;
	// List of alignment symbols for the padding zones.
	vector<struct align_symbols *> *align_symbols;
	// List of remove block entries.
	vector<struct remove_block *> *remove_block;
	// List of BigFoot entries./
	vector<struct bf_info *> *bf_entries;
	// Counter for alignment pading symbol indexes.
	int align_index;
	// Option value for type 11 entry.
	int option;
};

struct mwinfo_section *mwinfo = NULL;

/*
 *  Initializer for the content of the section
 */
static void init_contents(struct section_content *contents) {
  contents->size = 0;
  contents->capacity = 16;  /* initial capacity */
  contents->bytes = (bfd_byte *) malloc(contents->capacity);
}

/*
 *  Destructor for the content of the section
 */
static void destroy_contents(struct section_content **contents) {
  if ((*contents)->bytes) {
    free((*contents)->bytes);
  }

  free(*contents);
}

/*
 *  Setter for the type 11 entry option when is not already specified
 */
static void mwinfo_set_option_if_undef(int option) {
  if (mwinfo->option == -1) {
    mwinfo->option = option;
  }
}

/*
 *  Allocates space in the buffer that keeps the content of the section and
 * returns a pointer to the beginning of the allocated space
 */
static bfd_byte *alloc_contents(struct section_content *contents,
                     bfd_size_type wanted) {
  bfd_size_type new_size = contents->size + wanted;
  bfd_byte *ret = NULL;
  bool extend = false;

  /* check if we have more space */
  while (new_size > contents->capacity) {
    contents->capacity <<= 1; /* double the capacity */
    extend = true;
  }

  /* check if we need to extend the allocated space */
  if (extend) {
    contents->bytes = (bfd_byte *)
      realloc(contents->bytes, contents->capacity);
  }

  ret = contents->bytes + contents->size;
  contents->size = new_size;

  return ret;
}

//  Allocate space inside de content buffer and updated the size of the
// section.
static bfd_byte *mwinfo_section_more(bfd_size_type wanted)
{
	if (mwinfo->section == NULL)
	{
		flagword applicable = bfd_applicable_section_flags(stdoutput);
		mwinfo->section = subseg_new(MW_INFO_SECTION_NAME, 0);
		bfd_set_section_flags(stdoutput, mwinfo->section,
			applicable & (SEC_READONLY | SEC_HAS_CONTENTS));
	}

	bfd_byte *ret = alloc_contents(mwinfo->contents, wanted);
	bfd_set_section_size(stdoutput, mwinfo->section, mwinfo->contents->size);

	return ret;
}

/*
 *  Returns the current offset in section
 */
static bfd_size_type mwinfo_section_offset() {
  return mwinfo->contents->size;
}


//  Initialize ".MW_INFO" section structures. The contents of this section are
// used by the linker to implement stripping.
void mwinfo_section_init()
{
	mwinfo = static_cast<struct mwinfo_section *>(
		malloc(sizeof(struct mwinfo_section)));
	assert(mwinfo);

	mwinfo->section = NULL;

	mwinfo->contents = static_cast<struct section_content *>
		(malloc(sizeof(struct section_content)));
	assert(mwinfo->contents);
	init_contents(mwinfo->contents);
	mwinfo->fixups = new unordered_map<symbolS *, vector<bfd_size_type> *>();
	mwinfo->align_symbols = new vector<struct align_symbols *>();
	mwinfo->remove_block = new vector<struct remove_block *>();
	mwinfo->bf_entries = new vector<struct bf_info *>();
	mwinfo->align_index = 0;

	// Undefined value.
	mwinfo->option = -1;
}

/*
 *  Dump mw_info type 0 entry; the index is not dumped because at this step it
 * is not known yet;
 */
static bfd_size_type mwinfo_out_type_0(int alignment, int type)
{
  bfd_size_type offset = mwinfo_section_offset();
  bfd_byte *entry = mwinfo_section_more(MW_INFO_TYPE0_SIZE);
  assert(entry);

  *((int *)(entry + 0)) = MW_INFO_TYPE0_SIZE; /* size (11 bytes) */
  *(entry + 4) = 0;                           /* type (type 0) */
  *(entry + 5) = alignment;                   /* alignment */
  *(entry + 6) = type;                        /* type */
  *((int *)(entry + 7)) = 0;                  /* index (not known yet) */

  return offset + 7;
}

/*
 *  Dump mw_info type 3 entry; the index is not dumped because at this step it
 * is not known yet
 */
static bfd_size_type mwinfo_out_type_3(int padding)
{
  bfd_size_type offset = mwinfo_section_offset();
  bfd_byte *entry = mwinfo_section_more(MW_INFO_TYPE3_SIZE);
  assert(entry);

  *((int *)(entry + 0)) = MW_INFO_TYPE3_SIZE; /* size (13 bytes) */
  *(entry + 4) = 3;                           /* type (type 3) */
  *((int *)(entry + 5)) = 0;                  /* index (not known yet) */
  *((int *)(entry + 9)) = padding;            /* padding */

  return offset + 5;
}

/*
 *  Dump mw_info type 6 entry; the indexes are not dumped because at this step
 * they are not known yet
 */
static pair<bfd_size_type, bfd_size_type>
    mwinfo_out_type_6(int block_offset, int size)
{
  bfd_size_type offset = mwinfo_section_offset();
  bfd_byte *entry = mwinfo_section_more(MW_INFO_TYPE6_SIZE);
  pair<bfd_size_type, bfd_size_type> offsets;
  assert(entry);

  *((int *)(entry + 0)) = MW_INFO_TYPE6_SIZE; /* size (21 bytes) */
  *(entry + 4) = 6;                           /* type (type 6) */
  *((int *)(entry + 5)) = 0;                  /* section index (unknown) */
  *((int *)(entry + 9)) = block_offset;       /* block offset */
  *((int *)(entry + 13)) = size;              /* size */
  *((int *)(entry + 17)) = 0;                 /* symbol index (unknown) */

  offsets.first = offset + 5;
  offsets.second = offset + 17;
  return offsets;
}

/*
*  Dump mw_info type 8 entry; the index is not dumped because at this step it
* is not known yet;
*/
static bfd_size_type mwinfo_out_type_8(int stack_effect)
{
	bfd_size_type offset = mwinfo_section_offset();
	bfd_byte *entry = mwinfo_section_more(MW_INFO_TYPE8_SIZE);
	assert(entry);

	*((int *)(entry + 0)) = MW_INFO_TYPE8_SIZE;         /* size (13 bytes) */
	*(entry + 4) = 8;                                   /* type (type 8) */
	*((int *)(entry + 5)) = 0;                          /* symbol index (not known yet) */
	*((unsigned int *)(entry + 9)) = stack_effect;      /* stack effect */

	return offset + 5;
}

/*
 *  Dump mw_info type 10 entry; the index is not dumped because at this step it
 * is not known yet
 */
static bfd_size_type mwinfo_out_type_10(int ref_by_addr)
{
  bfd_size_type offset = mwinfo_section_offset();
  bfd_byte *entry = mwinfo_section_more(MW_INFO_TYPE10_SIZE);
  assert(entry);

  *((int *)(entry + 0)) = MW_INFO_TYPE10_SIZE; /* size (10 bytes) */
  *(entry + 4) = 10;                           /* type (type 10) */
  *((int *)(entry + 5)) = 0;                   /* index (unknown ) */
  *(entry + 9) = ref_by_addr;                  /* padding */

  return offset + 5;
}

/*
 *  Dump mw_info type 11 entry
 */
static void mwinfo_out_type_11(int option)
{
  bfd_size_type offset = mwinfo_section_offset();
  bfd_byte *entry = mwinfo_section_more(MW_INFO_TYPE11_SIZE);
  assert(entry);

  *((int *)(entry + 0)) = MW_INFO_TYPE11_SIZE; /* size (6 bytes) */
  *(entry + 4) = 11;                           /* type (type 11) */
  *(entry + 5) = option;                       /* option value */
}

/*
 *  Dump mw_info type 13 entry
 */
static pair<bfd_size_type, bfd_size_type> mwinfo_out_type_13(int type,
	offsetT imm, char delay_offset)
{
	bfd_size_type offset = mwinfo_section_offset();
	bfd_byte *entry = mwinfo_section_more(MW_INFO_TYPE13_SIZE);
	pair<bfd_size_type, bfd_size_type> offsets;
	assert(entry);

	// Size (15 bytes)
	*((int *)(entry + 0)) = MW_INFO_TYPE13_SIZE;
	// Type (type 13).
	*(entry + 4) = 13;
	// Coff symbol (unknown).
	*((int *)(entry + 5)) = 0;
	if (type == 2)
	{
		// Immediate value.
		*((int *)(entry + 9)) = imm;
	}
	else
	{
		// Target symbol (unknown).
		*((int *)(entry + 9)) = 0;
	}
	// BF type.
	*(entry + 13) = type;
	// Delay offset.
	*(entry + 14) = delay_offset;

	offsets.first = offset + 5;
	offsets.second = offset + 9;

	return offsets;
}


//  Add a fixup for a symbol, in order to write the index of the symbol at the
// corresponding offset in the section, when the order of the symbols in the
// symbol table is final.
static void mwinfo_add_fixup(symbolS *symbol, bfd_size_type offset)
{
	unordered_map<symbolS *, vector<bfd_size_type> *> *fixups = mwinfo->fixups;
	unordered_map<symbolS *, vector<bfd_size_type> *>::iterator fix
		= fixups->find(symbol);

	if (fix == fixups->end())
	{
		// Add symbol to the map.
		vector<bfd_size_type> *offsets = new vector<bfd_size_type>();
		offsets->push_back(offset);
		(*fixups)[symbol] = offsets;
	}
	else
	{
		// Add offset to the existing symbol list of offsets.
		fix->second->push_back(offset);
	}
}

//  The indexes of the symbols in the symbol table are final, so they can
// be written in the ".MW_INFO" section.
static void write_symbol_index(bfd *abfd)
{
	unordered_map<symbolS *, vector<bfd_size_type> *> *fixups = mwinfo->fixups;
	struct section_content *contents = mwinfo->contents;

	// Iterate on symbols and lists of offsets and update the section's
	// contents.
	for (unordered_map<symbolS *, vector<bfd_size_type> *>::iterator fix
		= fixups->begin(); fix != fixups->end(); ++fix)
	{
		symbolS *symbol = fix->first;

		// Compute the index of the symbol in the symbol table.
		int index = 0;
		if (symbol->bsym == NULL)
		{
			struct local_symbol *loc_sym
				= reinterpret_cast<struct local_symbol *>(symbol);
			if (loc_sym->lsy_section == reg_section)
			{
				index = _bfd_elf_symbol_from_bfd_symbol(abfd,
					&loc_sym->u.lsy_sym->bsym);
			}
			else
			{
				as_warn("Local symbol %s used in .MW_INFO section",
					loc_sym->lsy_name);
				index = 0;
			}
		}
		else
		{
			index = _bfd_elf_symbol_from_bfd_symbol(abfd, &symbol->bsym);
		}

		// Write the index at all the corresponding offsets.
		vector<bfd_size_type> *offsets = fix->second;
		for (vector<bfd_size_type>::iterator offset_it = offsets->begin();
			offset_it != offsets->end(); ++offset_it)
		{
			*(reinterpret_cast<int *>(&contents->bytes[*offset_it])) = index;
		}

		delete offsets;
	}

	delete fixups;
}

/*
 *  Free the memory allocated for the information associated to the section,
 * not freed yet
 */
static void mwinfo_destroy() {
  destroy_contents(&mwinfo->contents);
  free(mwinfo);
}

//  Write the section's contents.
static void write_mwinfo(bfd *abfd)
{
	if (mwinfo->section == NULL)
	{
		return;
	}

	asection *section = bfd_get_section_by_name(abfd, MW_INFO_SECTION_NAME);
	if (!section)
	{
		as_bad("error at writing .mw_info section");
		return;
	}

	bfd_set_section_contents(abfd, section, mwinfo->contents->bytes, 0, 
		mwinfo->contents->size);
}


//  Dump type 0 information.
void mwinfo_dump_type_0()
{
	struct mwinfo_fix *fix;
	bfd_size_type offset;

	if (!symbol_rootP)
	{
		return;
	}

	for (symbolS *symp = symbol_rootP; symp; symp = symbol_next(symp))
	{
		int type = -1;

		if (symbol_get_bfdsym(symp)->flags & BSF_OBJECT)
		{
			// VARIABLE.
			type = 1;
		}
		else if ((S_IS_FUNCTION(symp))
			|| (S_IS_EXTERNAL(symp)
				&& (S_GET_SEGMENT(symp)->flags & SEC_CODE)))
		{
			// ENTRY POINT.
			type = 3;
		}
		else if (symbol_get_tc(symp)->is_bf)
		{
			// VSPA_BFLABEL.
			type = 5;
		}

		if (type < 0 || type > 5)
		{
			continue;
		}

		int alignment = 255;
		if (S_IS_COMMON(symp))
		{
			// The alignment is kept as the value of the symbol.
			elf_symbol_type *type_ptr = elf_symbol_from(abfd, symp->bsym);
			int sym_value = S_GET_VALUE(symp);
			if (type_ptr == NULL || type_ptr->internal_elf_sym.st_value == 0)
			{
				sym_value = sym_value >= 16 ? 16 : (1 << bfd_log2(sym_value));
			}
			else
			{
				sym_value = type_ptr->internal_elf_sym.st_value;
			}
			
			// Compute the alignment.
			if (sym_value > 0)
			{
				alignment = bfd_log2(sym_value);
			}
		}
		else
		{
			// The alignment is explicitly specified by the "sym_align" directive.
			if (symbol_get_tc(symp)->alignment > 0)
			{
				alignment = bfd_log2(symbol_get_tc(symp)->alignment);
			}
		}

		// Dump entry.
		bfd_size_type offset = mwinfo_out_type_0(alignment, type);
		// Add fix for the symbol.
		mwinfo_add_fixup(symp, offset);
	}
}

//  Dump type 3 information.
void mwinfo_dump_type_3()
{
	vector<struct align_symbols *> *align_syms = mwinfo->align_symbols;
	for (vector<struct align_symbols *>::iterator sym_it = align_syms->begin();
		sym_it != align_syms->end(); ++sym_it)
	{
		struct align_symbols *syms = *sym_it;
		valueT padding = S_GET_VALUE(syms->end) - S_GET_VALUE(syms->begin);

		// Remove the end symbol from the symbol table.
		symbol_remove(syms->end, &symbol_rootP, &symbol_lastP);

		// Dump_type_3 information.
		bfd_size_type offset = mwinfo_out_type_3(padding);
		// Add fix for the symbol.
		mwinfo_add_fixup(syms->begin, offset);

		free(syms);
	}

	// Destroy the list of alignment symbols.
	delete mwinfo->align_symbols;
}

//  Dump type 6 information.
void mwinfo_dump_type_6()
{
	vector<struct remove_block *> *remove_block = mwinfo->remove_block;
	for (vector<struct remove_block *>::iterator block_it
		= remove_block->begin(); block_it != remove_block->end(); ++block_it)
	{
		// Compute the size of the block.
		int size = 0;
		struct remove_block *block = *block_it;
		expressionS *size_expr = block->size;
		if (size_expr->X_op == O_constant)
		{
			size = size_expr->X_add_number;
		}
		else if (size_expr->X_op == O_subtract)
		{
			segT asec = S_GET_SEGMENT(size_expr->X_add_symbol);
			segT bsec = S_GET_SEGMENT(size_expr->X_op_symbol);
			if ((asec != bsec) || (asec == undefined_section)) 
			{
				as_bad("can't resolve `%s' {%s section} - `%s' {%s section}"),
					S_GET_NAME(size_expr->X_add_symbol), segment_name (asec),
					S_GET_NAME(size_expr->X_op_symbol);
				free(size_expr);
				free(block);
				continue;
			}
			size = S_GET_VALUE(size_expr->X_add_symbol) -
				S_GET_VALUE(size_expr->X_op_symbol);
		}

		int offset = S_GET_VALUE(block->start_symbol);
		segT section = S_GET_SEGMENT(block->start_symbol);
		pair<bfd_size_type, bfd_size_type> offsets;

		// Dump entry.
		offsets = mwinfo_out_type_6(offset, size);
		// Create fixes for the section symbol and the stripped symbol.
		symbolS *sec_sym = section_symbol(section);
		assert(sec_sym);
		mwinfo_add_fixup(sec_sym, offsets.first);
		mwinfo_add_fixup(block->stripped_symbol, offsets.second);

		free(size_expr);
		free(block);
	}

	// Destroy the list of blocks.
	delete mwinfo->remove_block;
}

//  Dump type 8 information - stack effect.
void mwinfo_dump_type_8()
{
	if (!symbol_rootP)
	{
		return;
	}

	for (symbolS *symp = symbol_rootP; symp; symp = symbol_next(symp))
	{
		unsigned int stack_effect = symbol_get_tc(symp)->stack_effect;
		if (stack_effect)
		{
			// Dump entry.
			bfd_size_type offset = mwinfo_out_type_8(stack_effect);
			// Create fixup for the index of the symbol.
			mwinfo_add_fixup(symp, offset);

		}
	}

}

//  Dump type 10 information.
void mwinfo_dump_type_10()
{
	bool found = false;
	if (!symbol_rootP)
	{
		return;
	}

	for (symbolS *symp = symbol_rootP; symp; symp = symbol_next(symp))
	{
		if (symbol_get_tc(symp)->ref_by_addr)
		{
			// Dump entry.
			bfd_size_type offset = mwinfo_out_type_10(1);
			// Create fixup for the index of the symbol.
			mwinfo_add_fixup(symp, offset);

		found = true;
		}
	}

	// If we found at least one type 10 entry, type 11 option is enabled.
	if (found)
	{
		mwinfo_set_option_if_undef(1);
	}
}

//  Dump type 11 information, if defined.
void mwinfo_dump_type_11()
{
	if (mwinfo->option == -1)
	{
		return;
	}

	mwinfo_out_type_11(mwinfo->option);
}

//  Dump type 13 information.
void mwinfo_dump_type_13()
{
	vector<struct bf_info *> *bf_entries = mwinfo->bf_entries;
	for (vector<struct bf_info *>::iterator bf_it = bf_entries->begin();
		bf_it != bf_entries->end();
		++bf_it)
	{
		struct bf_info *bfi = *bf_it;

		// Dump entry.
		pair<bfd_size_type, bfd_size_type> offsets
			= mwinfo_out_type_13(bfi->type, bfi->imm, bfi->delay_offset);

		if (bfi->coff_symbol)
		{
			mwinfo_add_fixup(bfi->coff_symbol, offsets.first);
		}
		if (bfi->target_symbol)
		{
			mwinfo_add_fixup(bfi->target_symbol, offsets.second);
		}

		free(*bf_it);
	}

	// Destroy the list of BF entries.
	delete mwinfo->bf_entries;
}

/*
 *  Create name for an alignment padding symbol
 */
char *get_align_name(int index, bool end) {
  const char *prefix = "__align_";
  const char *suffix = "_end";
  char value[32];
  char *name;
  int len, prefix_len, suffix_len, value_len;
  sprintf(value, "%d", index);

  prefix_len = strlen(prefix);
  value_len = strlen(value);
  len = prefix_len + value_len + 1;
  if (end) {
    suffix_len = strlen(suffix);
    len += strlen(suffix);
  }

  name = (char *) malloc(len);
  assert(name);
  memcpy(name, prefix, prefix_len);
  memcpy(name + prefix_len, value, value_len);
  if (end) {
    memcpy(name + prefix_len + value_len, suffix, suffix_len);
  }
   name[len - 1] = 0;

   return name;
}

/*
 *  Create padding alignment symbol
 */
symbolS *create_align_symbol(bool suffix)
{
  char *sym_name = get_align_name(mwinfo->align_index, suffix);
  symbolS *symbolP = symbol_new(sym_name, now_seg, (valueT)frag_now_fix(), frag_now);
  free(sym_name);
  if (suffix) {
    mwinfo->align_index++;
  }

  return symbolP;
}

/*
 *  Final processing of the content and writing it
 */
void mwinfo_final_processing(bfd *abfd)
{
  write_symbol_index(abfd);
  write_mwinfo(abfd);
  mwinfo_destroy();
}

/*
 *  Save the pair of alignment padding symbols
 */
void mwinfo_save_align_syms(symbolS *sym_begin, symbolS *sym_end)
{
  struct align_symbols *symbols;
  symbols = (struct align_symbols *) malloc(sizeof(struct align_symbols));
  assert(symbols);
  symbols->begin = sym_begin;
  symbols->end = sym_end;
  mwinfo->align_symbols->push_back(symbols);
}

/*
 *  Save the information related to blocks and associated symbols
 */
void mwinfo_save_remove_block(symbolS *start_sym, symbolS *stripped_sym,
                              expressionS *size)
{
  struct remove_block *rb;
  rb = (struct remove_block *) malloc(sizeof(struct remove_block));
  assert(rb);
  rb->start_symbol = start_sym;
  rb->stripped_symbol = stripped_sym;
  rb->size = size;

  mwinfo->remove_block->push_back(rb);
}

//  Save the information related to BigFoot.
void mwinfo_save_bf_info(enum cof_type type,
	symbolS *coff_symbol,
	symbolS *target_symbol,
	offsetT imm,
	char delay_offset)
{
	struct bf_info *bfi;
	bfi = static_cast<struct bf_info *>(malloc(sizeof(struct bf_info)));
	assert(bfi);
	bfi->type = type;
	bfi->imm = imm;
	bfi->coff_symbol = coff_symbol;
	bfi->target_symbol = target_symbol;
	bfi->delay_offset = delay_offset;

	mwinfo->bf_entries->push_back(bfi);
}

/*
 *  Setter for type 11 option value
 */
void mwinfo_set_option(int option)
{
  mwinfo->option = option;
}

/*
 *  Set the type of the section
 */
void mwinfo_set_section_type(Elf_Internal_Shdr *shdr)
{
  shdr->sh_type = SHT_LOPROC + 3;
}
