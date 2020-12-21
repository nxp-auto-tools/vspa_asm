/*
 *  Check if an instruction bundle contains a "loop_end" instruction, in which
 * case its position is returned
 */
bool check_loop_end(adl::InstrBundle &instr_bundle, unsigned &pos)
{
	unsigned size = instr_bundle.size();
	for (unsigned i = 0; i < size; i++)
	{
		adl::InstrInfo instr = instr_bundle[i];
		if (instr.instrHasAttr(instr_loop_end))
		{
			pos = i;
			return true;
		}
	}

	return false;
}