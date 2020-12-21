/*
 * the maximum size for a loop defined between two lables, which can be encoded
 * in the 10-bit field of the opcode
 */
#define MAX_LOOP_SIZE ((1 << 10) - 1)

/*
 *  Check if an instruction bundle contains an instruction with a certain
 * attribute
 */
bool bundle_has_attr(adl::InstrBundle &instr_bundle, unsigned int attr)
{
  for (unsigned int i = 0; i < instr_bundle.group_size(); i++)
  {
    adl::InstrInfo instr = instr_bundle.get_instr(i);
    if (instr.instrHasAttr(attr))
    {
      return true;
    }
  }

  return false;
}

/*
 *  Check if an instruction bundle contains a "loop_end" instruction
 */
bool check_loop_end(adl::InstrBundle &instr_bundle)
{
  return bundle_has_attr(instr_bundle, instr_loop_end);
}

/*
 *  Check if an instruction is a set.loop with labels
 */
bool check_set_loop_labels(adl::InstrInfo &instr)
{
  return instr.instrHasAttr(instr_loop_label);
}

//  Check if an opcode corresponds to a "set.loop" with labels.
bool check_set_loop_labels(const struct adl_opcode *opcode)
{
	return opcode && opcode_has_attr(opcode, instr_loop_label);
}

//  Check if an instruction bundle can be compacted.
bool check_compactable(adl::InstrBundle &instr_bundle,
	int &posX,
	int &posS,
	bool &gen)
{
	unsigned size = instr_bundle.size();
	bool other = false;
	posS = posX = -1;
	gen = false;

	for (unsigned i = 0; i < size; i++)
	{
		adl::InstrInfo instr = instr_bundle[i];

		if(instr.instrHasAttr(instr_opS))
		{
			if (posS == -1)
			{
				posS = i;
			}
			else
			{
				return false;
			}
		}
		else if (instr.instrHasAttr(instr_opX))
		{
			posX = i;
		}
		else if((instr.instrHasAttr(instr_nop)
				&& !instr.instrHasAttr(instr_fnop))
			|| (instr.instrHasAttr(instr_opBS)
				&& !notCompactableBS(instr)))
		{
			if (posS == -1)
			{
				posS = i;
				gen = true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			other = true;
		}
	}

	if (posS == -1 || other)
	{
		return false;
	}

	return true;
}


/*
 *  Check if an instruction belongs to the "opS" family
 */
bool check_opS_instr(adl::InstrInfo &instr)
{
  return instr.instrHasAttr(instr_opS);
}

/*
 *  Check if an istruction is a "null"
 */
bool check_null_instr(adl::InstrInfo &instr)
{
  return instr.instrHasAttr(instr_null);
}

//  Check if a jmp/jsr instruction has a jmp mnemonic and if it is conditional
// or not.
static struct jmp_jsr_info extract_jmp_jsr_info(const adl::InstrInfo &instr)
{
	char *mnemonic = (char*)malloc(strlen(instr.opcode->name) + 1);
	strcpy(mnemonic,instr.opcode->name);
	char* underscore = strchr(mnemonic, '_');
	if (underscore)
	{
		*underscore = '\0';
	}
	bool is_jmp = !strcmp(mnemonic,"jmp");
	free(mnemonic);
	return {is_jmp, instr.args[0]!=""
		&& instr.args[0] != "al" && instr.args[0] != "nv"};
}

//  Check if an instruction bundle contains "loop_begin", "loop_end",
// "set.loop", "rts", "jmp", "jsr" or DMEM access instructions.
void check_control_dmem_instrs(adl::InstrBundle &instr_bundle,
							   struct machine_state &ms)
{
	bool is_memory_load = false;
	bool is_load_into_Rx = false;
	ms.set_loop_instr = ms.loop_begin = ms.loop_end = ms.rts = ms.dmem = ms.mvip = ms.load = ms.move = ms.load_R = ms.scalar_mac = ms.mpy = false;
	ms.set_loop_index = ms.jmp_jsr_index = instr_bundle.group_size();
	for (unsigned int i = 0; i < instr_bundle.group_size(); i++)
	{
		const adl::InstrInfo &instr = instr_bundle.get_instr(i);
		if (instr.instrHasAttr(instr_loop_begin)
			|| instr.instrSrcHasAttr(instr_loop_begin))
		{
			ms.loop_begin = true;
		}
		else if (instr.instrHasAttr(instr_loop_end)
			|| instr.instrSrcHasAttr(instr_loop_end))
		{
			ms.loop_end = true;
		}
		else if (instr.instrHasAttr(instr_rts))
		{
			ms.rts = true;
		}
		else if (instr.instrHasAttr(instr_jmp_jsr))
		{
			ms.jmp_jsr_index = i;
			ms.jj_info = extract_jmp_jsr_info(instr);
		}
		else if (strncmp(instr.instrName(), "ld", 2) == 0)
		{
			std::string str = instr.opcode->regex;
			size_t pos = str.find(",");
			if (pos != std::string::npos && str.find(",", pos) == std::string::npos)
			{
				ms.load = true;
			}
			// check if we have ld [aX]+aY as we cannot have a mv rX,ry between this one and a ld Rx
			// we are between such loads
			if (!strstr(instr.str.c_str(), ",") && strncmp(instr.str.c_str(), " R", 2) && strncmp(instr.str.c_str(), " r", 2))
			{
				// a mv in parallel with a memory load is legal (as long as there we are not inside of a pair)
				// so for a mv in the same packet, move is set to false
				if (ms.move && ms.between_loads == 0)
				{
					ms.move = false;
				}
				is_memory_load = true;
				ms.between_loads++;
			}
			// check if we have ld Rx as we cannot have a mv rX,ry between this one and a ld Rx
			// we exit in between such loads
			if (strncmp(instr.str.c_str(), " R", 2) == 0 || strncmp(instr.str.c_str(), " r", 2) == 0 )
			{
				assert(ms.between_loads);
				ms.between_loads--;
				// we cannot have a mv rX,ry in the same packet with a ld Rx
				ms.load_R = true;
				is_load_into_Rx = true;
			}
		}
		else if ((instr.instrHasAttr(instr_loop)
			|| instr.instrSrcHasAttr(instr_loop))
			&& strstr(instr.instrName(), "set_loop"))
		{
			ms.set_loop_instr = true;
		}
		// a mv in parallel with a memory load is legal, so move is set to true only if a ld is not in the same packet 
		if (strcmp(instr.instrName(), "mv_Rx_Ry") == 0)
		{
			// if we already had a ld in the same packet - then the difference between ld and ld.normal must be greater than 1
			// aka a new pair just started
			if (is_memory_load && ms.between_loads > 1)
			{
				ms.move = true;
			}
			// if we did not have a ld in the same packet - then the difference between ld and ld.normal must be greater than 0
			if (!is_memory_load && ms.between_loads > 0)
			{
				ms.move = true;
			}

			if (is_load_into_Rx)
			{
				ms.move = true;
			}
		}
		if (strcmp(instr.instrName(), "mac_cc_gZ_gX_gY") == 0 || strcmp(instr.instrName(), "fmac_cc_gZ_gX_gY") == 0)
		{
			ms.scalar_mac = true;
		}
		else if (strncmp(instr.instrName(), "mpy", 3) == 0)
		{
			ms.mpy = true;
		}
	} 
}

//  Get the opcode name for the alternative instance of a generic instruction.
bool get_instance_name(const adl::InstrInfo &instr, std::string &opcode)
{
	if (instr.instrHasAttr(instr_nop)
		&& !instr.instrHasAttr(instr_fnop))
	{
		opcode = "nop_s";
		return true;
	}

	std::string value("");
	if (instr.instrGetAttrVal(instr_inst, value))
	{
		opcode = value;
		return true;
	}

	return false;
}

/*
 *  Check if the bundle contains instaces of generic instructions, which can be
 * replaced other specific instances to improve microinstruction packing and
 * code compaction
 */
void check_generic_instrs(adl::InstrBundle &instr_bundle)
{
}

static bfd_uint64_t _set_loop_modifier(const expressionS *operands, unsigned __cia)
{
  int loop_size = ((operands[2].X_add_number) - (operands[1].X_add_number)) / 8;

  if (loop_size > MAX_LOOP_SIZE)
  {
    as_fatal("The size of the loop at line %d cannot be encoded using 10 bits", __cia);
    return 0;
  }

  return loop_size;
}

//  Check if an instruction is not supported by the simulator.
bool check_unimplemented(const adl::InstrInfo &instr)
{
	if (!instr.instrHasAttr(instr_vcpu1) && !instr.instrHasAttr(instr_vcpu2) && !instr.instrHasAttr(instr_vcpu3))
	{
		return true;
	}
	return false;
}

//  Check if an instruction is not supported by the simulator.
bool check_invalid(const adl::InstrInfo &instr)
{
	return false;
}

//  Check if an opcode allows a wide immediate.
bool check_wide_imm(const struct adl_opcode *opcode)
{
	return opcode_has_attr(opcode, instr_wide_imm);
}

//  Check if an opcode is disabled in the current core configuration.
bool check_disabled(const struct adl_opcode *opcode)
{
	return false;
}

//  Check if an instruction has a variable number of arguments. If this is the
// case, it also checks if the mask used for the saved/restored registers has a
// width of 32 bits.
bool check_var_instr(const adl::InstrInfo &instr, bool &mask32)
{
	if (instr.instrHasAttr(instr_var))
	{
		mask32 = instr.instrHasAttr(instr_opD);
		return true;
	}
	else
	{
		return false;
	}
}

// Reorder opcodes to implement family precedence restrictions:
// - opA > opC, opS
// - opB > opS
// - opS > opC, opD
// => opA, opB > opS > opC, opD
void reorder_opcodes(struct adl_instr *instr)
{
	struct opcode_info
	{
		long position;
		int precedence;
	};
	bool change = false;
	
	struct opcode_info *info = new struct opcode_info[instr->num_opcodes];
	for (long i = 0; i < instr->num_opcodes; i++)
	{
		info[i].position = i;
		struct adl_opcode *opcode = &instr->opcodes[i];
		if (opcode_has_attr(opcode, instr_opV))
		{
			info[i].precedence = 0;
		}
		else if (opcode_has_attr(opcode, instr_opB))
		{
			info[i].precedence = 1;
		}
		else if (opcode_has_attr(opcode, instr_opA))
		{
			info[i].precedence = 2;
		}
		else if (opcode_has_attr(opcode, instr_opS))
		{
			info[i].precedence = 3;
		}
		else if (opcode_has_attr(opcode, instr_opC))
		{
			info[i].precedence = 4;
		}
		else if (opcode_has_attr(opcode, instr_opD))
		{
			info[i].precedence = 5;
		}
		else if (opcode_has_attr(opcode, instr_fnop))
		{
			info[i].precedence = 6;
		}
		else if (opcode_has_attr(opcode, instr_done))
		{
			info[i].precedence = 7;
		}
		else if (opcode_has_attr(opcode, instr_opX))
		{
			info[i].precedence = 8;
		}
		else if (opcode_has_attr(opcode, instr_opZ))
		{
			info[i].precedence = 9;
		}
		else
		{
			info[i].precedence = 10;
		}
	}

	// Reorder opcodes: the opcodes with the lowest precedence must be first.
	bool c = true;
	while (c)
	{
		c = false;
		for (long i = 0; i < instr->num_opcodes - 1; i++)
		{
			if (info[i].precedence > info[i + 1].precedence)
			{
				struct opcode_info aux_info = info[i+1];
				info[i + 1] = info[i];
				info[i] =  aux_info;
				struct adl_opcode aux_opcode = instr->opcodes[i + 1];
				instr->opcodes[i + 1] = instr->opcodes[i];
				instr->opcodes[i] = aux_opcode;
				change = true;
				c = true;
			}
		}
	}

	if (!change)
	{
		delete[] info;
		return;
	}

	// Reverse permutation.
	long *pos_rev = new long[instr->num_opcodes];
	for (long i = 0; i < instr->num_opcodes; i++)
	{
		pos_rev[info[i].position] = i;
	}

	// Update opcode links.
	for (long i = 0; i < instr->num_opcodes; i++)
	{
		if (instr->opcodes[i].longest >= 0)
		{
			instr->opcodes[i].longest = pos_rev[instr->opcodes[i].longest];
		}
	}

	delete[] info;
	delete[] pos_rev;
}

void check_rv_write_restriction(const adl::InstrBundle &instr_bundle)
{
	int pos_write = -1;
	int pos_rv = -1;
	size_t bundle_size = instr_bundle.group_size();
	for (size_t i = 0; i < bundle_size; i++)
	{
		const adl::InstrInfo &instr = instr_bundle.get_instr(i);
		if (strncmp(instr.instrName(), "wr", 2) == 0)
		{
			pos_write = i;
		}
		if (strncmp(instr.instrName(), "mv", 2) == 0
			|| strncmp(instr.instrName(), "fill", 4) == 0
			|| strncmp(instr.instrName(), "lsb2rf", 6) == 0)
		{
			std::string str = instr.opcode->regex;
			if (str.find("[rV") != std::string::npos
				|| str.find("[r7V") != std::string::npos)
			{
				pos_rv = i;
			}
		}
	}
	if (pos_rv >= 0 && pos_write >= 0)
	{
		as_bad("Instructions \"%s\" and \"%s\" are not allowed to be in the "
			"same packet",
			instr_bundle.get_instr(pos_write).str.c_str(),
			instr_bundle.get_instr(pos_rv).str.c_str(),
			AU_NUM, core_type == core_type_sp ? "SP" : "DP");
	}
}

void check_mv_load_restriction(const adl::InstrBundle &instr_bundle)
{
	int pos_load = -1;
	int pos_mv = -1;
	size_t bundle_size = instr_bundle.group_size();
	char *name1 = NULL;
	char * name2 = NULL;

	//ERR011386 / VSPA A-011386
	if (!restriction_checked_on_isa(&check_mv_load_restriction, isa_id)) return;

	for (size_t i = 0; i < bundle_size; i++)
	{
		const adl::InstrInfo &instr = instr_bundle.get_instr(i);
		if (strncmp(instr.instrName(), "st", 2) == 0)
		{
			std::string str = instr.opcode->regex;
			if (str.find(",") == std::string::npos)
			{
				pos_load = i;
				name1 = (char*)malloc(strlen(instr.instrName())+1);
				strcpy(name1,instr.instrName());
				char* p=strchr(name1,'_');
				if (p)
				{
					*p='\0';
				}
			}
		}
		if (strncmp(instr.instrName(), "stld", 4) == 0)
		{
			std::string str = instr.opcode->regex;
			size_t pos = str.find(",");
			if (pos != std::string::npos && str.find(",", pos) == std::string::npos)
			{
				pos_load = i;
				name1 = (char*)malloc(strlen(instr.instrName())+1);
				strcpy(name1,instr.instrName());
				char* p=strchr(name1,'_');
				if (p)
				{
					*p='\0';
				}
			}
		}
		if (strncmp(instr.instrName(), "ldB_Rx", 6) == 0)
		{
			pos_mv = i;
			name2=(char*)malloc(strlen(instr.instrName())+1);
			strcpy(name2,instr.instrName());
			char *p=strchr(name2,'_');
			if (p)
			{
				*p='\0';
			}
		}
		if (strncmp(instr.instrName(), "mv", 2) == 0
			|| strncmp(instr.instrName(), "fill", 4) == 0
			|| strncmp(instr.instrName(), "lsb2rf", 6) == 0)
		{
			std::string str = instr.opcode->regex;
			if (str.find("[rV") != std::string::npos)
			{
				pos_mv = i;
				name2=(char*)malloc(strlen(instr.instrName())+1);
				strcpy(name2,instr.instrName());
				char *p=strchr(name2,'_');
				if (p)
				{
					*p='\0';
				}
			}
		}
		if (strncmp(instr.instrName(), "mv", 2) == 0) 
		{
			std::string str = instr.opcode->regex;
			if (str.find("[rs0") != std::string::npos)
			{
				pos_mv = i;
				name2 = (char*)malloc(strlen(instr.instrName())+1);
				strcpy(name2,instr.instrName());
				char *p=strchr(name2,'_');
				if (p)
				{
					*p='\0';
				}
			}
		}
	}
	if (pos_load >= 0 && pos_mv >= 0)
	{
		as_bad("Instructions \"%s%s\" and \"%s%s\" are not allowed to be in the "
			"same packet",
			name1,
			instr_bundle.get_instr(pos_load).str.c_str(),
			name2,
			instr_bundle.get_instr(pos_mv).str.c_str(),
			AU_NUM, core_type == core_type_sp ? "SP" : "DP");
	}
}

void check_mv_loadstore_restriction2(const adl::InstrBundle &instr_bundle)
{
	int pos_load = -1;
	int pos_mv = -1;
	size_t bundle_size = instr_bundle.group_size();
	char *name1 = NULL;
	char * name2 = NULL;

	for (size_t i = 0; i < bundle_size; i++)
	{
		const adl::InstrInfo &instr = instr_bundle.get_instr(i);
		if (strncmp(instr.instrName(), "st", 2) == 0)
		{
			std::string str = instr.opcode->regex;
			if (str.find(",") == std::string::npos)
			{
				pos_load = i;
				name1 = (char*)malloc(strlen(instr.instrName())+1);
				strcpy(name1,instr.instrName());
				char* p=strchr(name1,'_');
				if (p)
				{
					*p='\0';
				}
			}
		}
		if (strncmp(instr.instrName(), "stld", 4) == 0)
		{
			std::string str = instr.opcode->regex;
			size_t pos = str.find(",");
			if (pos != std::string::npos && str.find(",", pos) == std::string::npos)
			{
				pos_load = i;
				name1 = (char*)malloc(strlen(instr.instrName())+1);
				strcpy(name1,instr.instrName());
				char* p=strchr(name1,'_');
				if (p)
				{
					*p='\0';
				}
			}
		}
		if (strncmp(instr.instrName(), "ld", 2) == 0)
		{
			std::string str = instr.opcode->regex;
			size_t pos = str.find(",");
			if (pos != std::string::npos && str.find(",", pos) == std::string::npos)
			{
				pos_load = i;
				name1 = (char*)malloc(strlen(instr.instrName())+1);
				strcpy(name1,instr.instrName());
				char* p=strchr(name1,'_');
				if (p)
				{
					*p='\0';
				}
			}
		}
		if (strcmp(instr.instrName(), "mv_Rx_Ry") == 0)
		{
			pos_mv=i;
			name2=(char*)malloc(strlen(instr.instrName())+1);
			strcpy(name2,instr.instrName());
			char *p=strchr(name2,'_');
			if (p)
			{
				*p='\0';
			}
		}
	}
	if (pos_load >= 0 && pos_mv >= 0)
	{
		as_bad("Instructions \"%s%s\" and \"%s%s\" are not allowed to be in the "
			"same packet",
			name1,
			instr_bundle.get_instr(pos_load).str.c_str(),
			name2,
			instr_bundle.get_instr(pos_mv).str.c_str(),
			AU_NUM, core_type == core_type_sp ? "SP" : "DP");
	}
}


void check_loadstore_restriction(const adl::InstrBundle &instr_bundle)
{
	int pos_load = -1;
	int pos_store = -1;
	size_t bundle_size = instr_bundle.group_size();
	char *name1 = NULL;
	char * name2 = NULL;

	//ERR050465 
	if (!restriction_checked_on_isa(&check_loadstore_restriction, isa_id)) return;

	for (size_t i = 0; i < bundle_size; i++)
	{
		const adl::InstrInfo &instr = instr_bundle.get_instr(i);
		if (strncmp(instr.instrName(), "st", 2) == 0)
		{
			std::string str = instr.opcode->regex;
			if (str.find(",") == std::string::npos)
			{
				pos_store = i;
				name1 = (char*)malloc(strlen(instr.instrName())+1);
				strcpy(name1,instr.instrName());
				char* p=strchr(name1,'_');
				if (p)
				{
					*p='\0';
				}
			}
		}
		if (strncmp(instr.instrName(), "stld", 4) == 0)
		{
			std::string str = instr.opcode->regex;
			size_t pos = str.find(",");
			if (pos != std::string::npos && str.find(",", pos) == std::string::npos)
			{
				pos_store = i;
				pos_load = i;
				name1 = (char*)malloc(strlen(instr.instrName())+1);
				strcpy(name1,instr.instrName());
				char* p=strchr(name1,'_');
				if (p)
				{
					*p='\0';
				}
			}
		}
		if (	strncmp(instr.instrName(), "ld", 2) == 0
			&&	!instr.instrHasAttr ( instr_opVldB )
			&&	!instr.instrHasAttr ( instr_opVld )
			&&	strncmp(instr.instrName(), "ldB_Rx", 6)
			&&	(	instr.instrHasAttr ( instr_opAB ) || instr.instrHasAttr ( instr_opBA )
				||	instr.instrHasAttr ( instr_opB )) )
		{
			std::string str = instr.opcode->regex;
			pos_load = i;
			name2 = (char*)malloc(strlen(instr.instrName())+1);
			strcpy(name2,instr.instrName());
			char* p=strchr(name2,'_');
			if (p)
			{
				*p='\0';
			}
		}
	}
	if (pos_load >= 0 && pos_store >= 0)
	{
		if (pos_load == pos_store)
		{
			as_warn("Instruction \"%s%s\" is not allowed to be in the "
				"same packet if they refer the same memory bank",
				name1,
				instr_bundle.get_instr(pos_load).str.c_str(),
				AU_NUM, core_type == core_type_sp ? "SP" : "DP");
		}
		else
		{
			as_warn("Instructions \"%s%s\" and \"%s%s\" are not allowed to be in the "
				"same packet if they refer the same memory bank",
				name1,
				instr_bundle.get_instr(pos_store).str.c_str(),
				name2,
				instr_bundle.get_instr(pos_load).str.c_str(),
				AU_NUM, core_type == core_type_sp ? "SP" : "DP");
		}
	}
}

void check_ld_Vldb_restriction(const adl::InstrBundle &instr_bundle)
{
	int pos_Vld1 = -1;
	int pos_Vld2 = -1;
	size_t bundle_size = instr_bundle.group_size();
	for (size_t i = 0; i < bundle_size; i++)
	{
		const adl::InstrInfo &instr = instr_bundle.get_instr(i);
		if (instr. instrHasAttr ( instr_opVld ))
		{
			if (pos_Vld1 == -1)
			{
				pos_Vld1 = i;
			}
			else if (pos_Vld2 == -1)
			{
				pos_Vld2 = i;
			}
		}
	}
	if (pos_Vld1 >=0 && pos_Vld2 >=0 )
	{
		const adl::InstrInfo &instr1 = instr_bundle.get_instr(pos_Vld1);
		const adl::InstrInfo &instr2 = instr_bundle.get_instr(pos_Vld2);
		if (instr1. instrHasAttr ( instr_opVldB ) && instr1 . instrHasAttrVal ( instr_inst , "ldB_Rx_opB" ))
		{
			if (strncmp(instr2.instrName(), "ld", 2) != 0)
			{
				char* name1=(char*)malloc(strlen(instr1.instrName())+1);
				strcpy(name1,instr1.instrName());
				char* p=strchr(name1,'_');
				if (p)
				{
					*p='\0';
				}
				char* name2=(char*)malloc(strlen(instr2.instrName())+1);
				strcpy(name2,instr2.instrName());
				p=strchr(name2,'_');
				if (p)
				{
					*p='\0';
				}
				as_bad("Instruction \"%s%s\" can only be grouped with a ld(.x) opVld instruction, "
					"not with \"%s%s\"",
					name1,
					instr1.str.c_str(),
					name2,
					instr2.str.c_str());
			}
		}
		else if (instr2. instrHasAttr ( instr_opVldB ) && instr2 . instrHasAttrVal ( instr_inst , "ldB_Rx_opB" ))
		{
			if (strncmp(instr1.instrName(), "ld", 2) != 0)
			{
				char* name1=(char*)malloc(strlen(instr1.instrName())+1);
				strcpy(name1,instr1.instrName());
				char* p=strchr(name1,'_');
				if (p)
				{
					*p='\0';
				}
				char* name2=(char*)malloc(strlen(instr2.instrName())+1);
				strcpy(name2,instr2.instrName());
				p=strchr(name2,'_');
				if (p)
				{
					*p='\0';
				}
				as_bad("Instruction \"%s%s\" can only be grouped with a ld(.x) opVld instruction, "
					"not with \"%s%s\"",
					name2,
					instr2.str.c_str(),
					name1,
					instr1.str.c_str());
			}
		}
	}
}

//  Check VSPA2 and VSPA3 configuration specific instruction restrictions.
void check_instr_restrictions(const adl::InstrInfo &instr)
{
	if (!instr_present_on_isa(instr,isa_id))
	{
		as_bad("%s instruction is not supported in this %s architecture"
					" in this core configuration (%uAU %s)",
					instr.opcode->name, isa_name, AU_NUM,
					core_type == core_type_sp ? "SP" : "DP");
	}
	if (instr.instrHasAttr(instr_set_prec)
		&& (core_type == core_type_sp || core_type == core_type_spl))
	{
		int (*compare)(const char *str1, const char *str2) = ignore_case
			? strcasecmp : strcmp;
		const std::vector<std::string> &args = instr.args;
		static const char *precs[] = { "S0prec", "S1prec", "S2prec", "AUprec",
			"Vprec" };

		for (size_t i = 0, num_precs = ARRAY_SIZE(precs); i < num_precs; i++)
		{
			if (!compare(args[i].c_str(), "double"))
			{
				as_bad("\"%s\" precision mode is not supported for %s"
					" in this core configuration (%uAU %s)",
					args[i].c_str(), precs[i], AU_NUM,
					core_type == core_type_sp ? "SP" : "DP");
			}
		}
	}
	else if (core_type == core_type_sp || core_type == core_type_dp)
	{
		if (instr.instrHasAttr(instr_lut))
		{
			as_bad("\"%s\" is not supported in this core configuration "
				"(%uAU %s)", instr.str.c_str(), AU_NUM,
				core_type == core_type_sp ? "SP" : "DP");
		}
	}
}

//  Check VSPA3 configuration specific instruction unavailable modes.
void check_instr_unavailable_modes(const adl::InstrInfo &instr)
{
	// On VSPA3_LA9358 S1zeros mode is not supported (the only isa on which is not supported)
	if (isa_id == VSPA3_LA9358 && strstr(instr.str.c_str(), "S1zeros"))
	{
		as_bad("\"%s\" mode for instruction %s is not supported in this %s architecture"
					" in this core configuration (%uAU %s)",
					instr.str.c_str(), instr.opcode->name, isa_name, AU_NUM,
					core_type == core_type_sp ? "SP" : "DP");
	}
}

//  Check if an instruction opcode corresponds to the "st Iu17, I32"
// pseudoinstruction.
bool check_is_pseudo(const struct adl_opcode *opcode)
{
	// On VSPA2 this is an actual instruction, not a pseudoinstruction.
	return false;
}
