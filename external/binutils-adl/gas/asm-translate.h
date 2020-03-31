#include <stdint.h>

void load_instr_mapping(bool ignore_case);

void clear_instr_mapping();

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
	bool &skip_check);
