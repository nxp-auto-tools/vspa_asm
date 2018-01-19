/*
 * the maximum size for a loop defined between two lables, which can be encoded
 * in the 10-bit field of the opcode
 */
#define MAX_LOOP_SIZE ((1 << 10) - 1)
#define DMA_XFER_CTRL 0x2f

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
bool check_set_loop_labels(struct adl_opcode *opcode)
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

#if !defined(_VSPA2_)
		if (instr.instrHasAttr(instr_no_comp))
		{
			return false;
		}
#endif

		if(instr.instrHasAttr(instr_opS))
		{
			posS = i;
		}
		else if (instr.instrHasAttr(instr_opX))
		{
			posX = i;
		}
#if defined(_VSPA2_)
		else if((instr.instrHasAttr(instr_nop)
				&& !instr.instrHasAttr(instr_fnop))
			|| instr.instrHasAttr(instr_opBS))
#else
		else if (instr.instrHasAttr(instr_opBS))
#endif
		{
			posS = i;
			gen = true;
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
	const auto space_pos = instr.str.find_first_of(' ');
	const auto &mnemonic = (space_pos == std::string::npos) ? instr.str
		: instr.str.substr(0, space_pos);
	const auto dot_pos = mnemonic.find_first_of('.');

	if (dot_pos == std::string::npos)
	{
		return { mnemonic == "jmp", false };
	}

	const auto &cond = mnemonic.substr(dot_pos + 1);
	// .al (always) and .nv (never) are not actual conditions.
	return { !mnemonic.compare(0, dot_pos, "jmp"),
		(cond != "al") && (cond != "nv") };
}

//  Check if an instruction bundle contains "loop_begin", "loop_end",
// "set.loop", "rts", "jmp", "jsr" or DMEM access instructions.
void check_control_dmem_instrs(adl::InstrBundle &instr_bundle,
							   bool &loop_begin, bool &loop_end,
							   size_t &set_loop, bool &rts, size_t &jmp_jsr,
							   struct jmp_jsr_info &jmp_jsr_info, bool &dmem, bool &mvip)
{
	loop_begin = loop_end = rts = dmem = mvip = false;
	set_loop = jmp_jsr = instr_bundle.group_size();
	for (unsigned int i = 0; i < instr_bundle.group_size(); i++)
	{
		const adl::InstrInfo &instr = instr_bundle.get_instr(i);
		if (instr.instrHasAttr(instr_loop_begin)
			|| instr.instrSrcHasAttr(instr_loop_begin))
		{
			loop_begin = true;
		}
		else if (instr.instrHasAttr(instr_loop_end)
			|| instr.instrSrcHasAttr(instr_loop_end))
		{
			loop_end = true;
		}
		else if (instr.instrHasAttr(instr_rts))
		{
			rts = true;
		}
		else if (instr.instrHasAttr(instr_jmp_jsr))
		{
			jmp_jsr = i;
			jmp_jsr_info = extract_jmp_jsr_info(instr);
		}
#if !defined(_VSPA2_)
		// Check for a "set.loop" instruction.
		else if (instr.instrHasAttr(instr_loop)
			&& instr.instrHasAttr(instr_ctrl))
		{
			set_loop = i;
		}
		// Check for instructions that generate a DMEM access.
		else if (instr.instrHasAttr(instr_store)
			|| (!instr.instrHasAttr(instr_opVr)
				&& instr.instrHasAttr(instr_load)))
		{
			dmem = true;
		}
		//check if instruction is mvip
		else if (strncmp(instr.instrName(), "mvip", 4)==0)
		{
			//only return true for DMA_XFER_CTRL
			if (instr.operand_values[0].X_op == O_constant 
				&& instr.operand_values[0].X_add_number == DMA_XFER_CTRL)
			{
				mvip = true;
			}
		}
#endif
	} 
}

//  Get the opcode name for the alternative instance of a generic instruction.
bool get_instance_name(const adl::InstrInfo &instr, std::string &opcode)
{
#if defined(_VSPA2_)
	if (instr.instrHasAttr(instr_nop)
		&& !instr.instrHasAttr(instr_fnop))
	{
		opcode = "nop_s";
		return true;
	}
#endif

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
#ifndef _VSPA2_
  int extB, posAB;
  extB = 0;
  posAB = -1;

  /* gather information from the bundle */
  for (unsigned int i = 0; i < instr_bundle.group_size(); i++)
  {
    adl::InstrInfo &instr = instr_bundle.get_instr(i);
    if (!instr.instrHasAttr(instr_opB))
    {
      continue;
    }

    extB++;

    assert(instr.opcode);
    if (instr.instrHasAttr(instr_opAB))
    {
      posAB = i;
    }
    else if (!instr.instrHasAttr(instr_opBS))
    {
      continue;
    }

    instr = replaceInstr(instr, instr.opcode->name);
  }

  if (extB != 2 || posAB == -1)
  {
    return;
  }

  adl::InstrInfo &instrAB = instr_bundle.get_instr(posAB);
  std::string opcode_name;
  if (!get_instance_name(instrAB, opcode_name))
  {
    as_fatal("Cannot instantiate generic instruction");
    return;
  }

  instrAB = replaceInstr(instrAB, opcode_name.c_str());
#endif
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
#if defined(_VSPA2_)
	if (!instr.instrHasAttr(instr_vcpu1) && !instr.instrHasAttr(instr_vcpu2))
	{
		return true;
	}
#endif
	return false;
}

//  Check if an instruction is not supported by the simulator.
bool check_invalid(const adl::InstrInfo &instr)
{
	return instr.instrHasAttr(instr_illegal);
}

//  Check if an opcode allows a wide immediate.
bool check_wide_imm(const struct adl_opcode *opcode)
{
	return opcode_has_attr(opcode, instr_wide_imm);
}

//  Check if an opcode is disabled in the current core configuration.
bool check_disabled(const struct adl_opcode *opcode)
{
#if defined(_VSPA2_)
	return false;
#else
	return AU_NUM == 64 && opcode_has_attr(opcode, instr_dis_64au);
#endif
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
	for (long i = 0; i < instr->num_opcodes - 1; i++)
	{
		for (long j = i + 1; j < instr->num_opcodes; j++)
		{
			if (info[i].precedence > info[j].precedence)
			{
				struct opcode_info aux_info = info[i];
				info[i] = info[j];
				info[j] = aux_info;
				struct adl_opcode aux_opcode = instr->opcodes[i];
				instr->opcodes[i] = instr->opcodes[j];
				instr->opcodes[j] = aux_opcode;
				change = true;
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
		instr->opcodes[i].longest = pos_rev[instr->opcodes[i].longest];
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

#if !defined(_VSPA2_)
//  Check configuration specific instruction bundle restrictions
void check_bundle_restrictions(const adl::InstrBundle &instr_bundle)
{
	size_t pos_set_range, num_set_vra_ptr, bundle_size
		= instr_bundle.group_size();
	const size_t num_opV = 4;
	size_t pos_V[num_opV];
	size_t pos_set_vra_ptr[2];
	pos_V[0] = pos_V[1] = pos_V[2] = pos_V[3] = pos_set_range = bundle_size;
	num_set_vra_ptr = 0;

	for (size_t i = 0; i < bundle_size; i++)
	{
		const adl::InstrInfo &instr = instr_bundle.get_instr(i);
		if (instr.instrHasAttr(instr_set_vra_ptr) && num_set_vra_ptr < 2)
		{
			pos_set_vra_ptr[num_set_vra_ptr++] = i;
		}
		else if (instr.instrHasAttr(instr_set_range))
		{
			pos_set_range = i;
		}
		else if (instr.instrHasAttr(instr_opVs0))
		{
			pos_V[0] = i;
		}
		else if (instr.instrHasAttr(instr_opVs1))
		{
			pos_V[1] = i;
		}
		else if (instr.instrHasAttr(instr_opVs2))
		{
			pos_V[2] = i;
		}
		else if (instr.instrHasAttr(instr_opVrot))
		{
			pos_V[3] = i;
		}
	}

	if (num_set_vra_ptr == 2)
	{
		as_bad("Instructions \"%s\" and \"%s\" are not allowed to be in the "
			"same packet in this core configuration (%uAU %s)",
			instr_bundle.get_instr(pos_set_vra_ptr[0]).str.c_str(),
			instr_bundle.get_instr(pos_set_vra_ptr[1]).str.c_str(),
			AU_NUM, core_type == core_type_sp ? "SP" : "DP");
	}

	if (pos_set_range < bundle_size)
	{
		for (size_t i = 0; i < num_opV; i++)
		{
			if (pos_V[i] == bundle_size)
			{
				continue;
			}

			as_bad("Instructions \"%s\" and \"%s\" are not allowed to be in "
				"the same packet in this core configuration (%uAU %s)",
				instr_bundle.get_instr(pos_set_range).str.c_str(),
				instr_bundle.get_instr(pos_V[i]).str.c_str(),
				AU_NUM, core_type == core_type_sp ? "SP" : "DP");
		}
	}
}

//  Check VSPA1 configuration specific instruction restrictions.
void check_instr_restrictions(const adl::InstrInfo &instr)
{
	if (AU_NUM != 4 && (instr.instrHasAttr(instr_en_4au))
		|| (AU_NUM == 64
			&& core_type == core_type_sp
			&& instr.instrHasAttr(instr_dis_64au_sp))
		|| (AU_NUM == 64 && instr.instrHasAttr(instr_dis_64au))
		|| (core_type == core_type_dp && instr.instrHasAttr(instr_dis_all_dp)))
	{
		as_bad("\"%s\" is not supported in this core configuration (%uAU %s)",
			instr.str.c_str(), AU_NUM, core_type == core_type_sp ? "SP"
			: "DP");
	}

	if (AU_NUM == 64 && instr.instrHasAttr(instr_cmp_aX_Iu17))
	{
		as_warn("\"%s\" does not modify C condition carry flag and clears the "
			"V flag; unsigned conditional instructions (.hi, .lo, .hs, .ls, "
			".cs, .cc, .vs, .vc) can't be used in conjunction with this "
			"instruction", instr.str.c_str());
	}

	if (instr.instrHasAttr(instr_set_prec)
		&& (AU_NUM == 64 || core_type == core_type_sp))
	{
		int (*compare)(const char *str1, const char *str2) = ignore_case
			? strcasecmp : strcmp;

		const std::vector<std::string> &args = instr.args;
		static const char *precs[] = { "S0prec", "S1prec", "S2prec", "AUprec",
			"Vprec" };
		if (AU_NUM == 64)
		{
			for (size_t i = 0; i < 3; i++)
			{
				if (!compare(args[i].c_str(), "half"))
				{
					as_bad("\"%s\" precision mode is not supported for %s"
						" in this core configuration (%uAU %s)",
						args[i].c_str(), precs[i], AU_NUM,
						core_type == core_type_sp ? "SP" : "DP");
				}
			}

			if (!compare(args[4].c_str(), "half"))
			{
				as_bad("\"%s\" precision mode is not supported for %s"
					" in this core configuration (%uAU %s)",
					args[4].c_str(), precs[4], AU_NUM,
					core_type == core_type_sp ? "SP" : "DP");
			}
		}

		if (core_type == core_type_sp)
		{
			for (size_t i = 0; i < 5; i++)
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
	}
}

//  Check if an instruction opcode corresponds to the "st Iu17, I32"
// pseudoinstruction.
bool check_is_pseudo(const struct adl_opcode *opcode)
{
	return opcode_has_attr(opcode, instr_pseudo);
}
#else
//  Check VSPA2 configuration specific instruction restrictions.
void check_instr_restrictions(const adl::InstrInfo &instr)
{
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
		else if (instr.instrHasAttr(instr_set_smode))
		{
			int (*compare)(const char *str1, const char *str2) = ignore_case
				? strcasecmp : strcmp;
			const std::vector<std::string> &args = instr.args;
			static const char *s1qline = "S1qline";
			for (std::vector<std::string>::const_iterator ita = args.begin(),
				ita_end = args.end(); ita != ita_end; ++ita)
			{
				if (!compare(ita->c_str(), s1qline))
				{
					as_bad("\"%s\" is not supported for S1mode in this core "
						"configuration (%uAU %s)", ita->c_str(), AU_NUM,
						core_type == core_type_sp ? "SP" : "DP");
				}
			}
		}
	}
}

//  Check if an instruction opcode corresponds to the "st Iu17, I32"
// pseudoinstruction.
bool check_is_pseudo(const struct adl_opcode *opcode)
{
	// On VSPA2 this is an actual instruction, not a pseudoinstruction.
	return false;
}
#endif
