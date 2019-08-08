#ifndef _MW_INFO_H_
#define _MW_INFO_H_

#define MW_INFO_SECTION_NAME ".mw_info"
#define MW_INFO_TYPE0_SIZE 11
#define MW_INFO_TYPE3_SIZE 13
#define MW_INFO_TYPE6_SIZE 21
#define MW_INFO_TYPE8_SIZE 13
#define MW_INFO_TYPE10_SIZE 10
#define MW_INFO_TYPE11_SIZE 6
#define MW_INFO_TYPE13_SIZE 15

enum cof_type
{
	cof_no_type = -1,
	cof_jmp_jsr_label = 1,
	cof_jmp_jsr_imm = 2,
	cof_jmp_jsr_reg = 3,
	cof_rts = 4,
	cof_loop = 5,
	cof_loop_one_two = 6,
};

extern "C"
{
# include "as.h"
# include "struc-symbol.h"
# include "libbfd.h"
# include "safe-ctype.h"
}

extern struct mwinfo_section *mwinfo;

void mwinfo_section_init();
void mwinfo_dump_type_0();
void mwinfo_dump_type_3();
void mwinfo_dump_type_6();
void mwinfo_dump_type_8();
void mwinfo_dump_type_10();
void mwinfo_dump_type_11();
void mwinfo_dump_type_13();
void mwinfo_set_option(int option);
void mwinfo_final_processing(bfd *abfd);
char *get_align_name(int index, bool suffix);
symbolS *create_align_symbol(bool suffix);
void mwinfo_save_align_syms(symbolS *sym_begin, symbolS *sym_end);
void mwinfo_save_remove_block(symbolS *sym_begin, symbolS *sym_end,
	expressionS *expr);
void mwinfo_save_bf_info(enum cof_type type, symbolS *coff_symbol,
	symbolS *target_symbol, offsetT imm, char delay_offset);
void mwinfo_set_section_type(Elf_Internal_Shdr *shdr);

#endif
