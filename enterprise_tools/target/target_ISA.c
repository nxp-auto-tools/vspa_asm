
#include <string.h>
#include "target_ISA.h"


/* table with the name of the parameters defining the target */
const char * target_parameters_name[TARGET_PARAMETERS_NUM] =
{
	TARGET_PARAMETERS_NAME
};

/* table with the name of the target ISAs that are supported */
const char * target_ISAs_name[TARGET_ISA_NUM] =
{
	TARGET_ISAS_NAME
};

/* table with the details / description of the target ISAs that are supported */
const char * target_ISAs_detail[TARGET_ISA_NUM] = 
{
	TARGET_ISAS_DETAIL
};

/* table with the prefix in library name used for each target ISA */
const char * target_ISAs_prefix[TARGET_ISA_NUM] =
{
	TARGET_ISAS_PREFIX
};

/* table with the target ISAs description (i.e. parameter values for each defined ISA) */
const target_parameters_ISA target_ISAs[TARGET_ISA_NUM] =
{
	TARGET_ISAS_DESCRIPTION
};




/* check if the provided name matches any of the supported target ISA */
bool is_valid_ISA(const char *isa_name)
{
	int isa_cnt;
	if ((isa_name == NULL) || (strlen(isa_name) == 0))
	{
		return false;
	}
	/* find target ISA by name */
	for (isa_cnt = 0; isa_cnt < TARGET_ISA_NUM; isa_cnt++)
	{
		if (strcmp(target_ISAs_name[isa_cnt], isa_name) == 0)
		{
			return true;
		}
	}
	return false;
}

/* get the ID for the provided target ISA name 
(if no target ISA with this name is found, return the ID of the TARGET_DEFAULT_ISA) */
int get_ISA_id(const char *isa_name)
{
	int isa_cnt;
	if ((isa_name == NULL) || (strlen(isa_name) == 0))
	{
		isa_cnt = TARGET_DEFAULT_ISA;
	}
	/* find target ISA by name */
	else
	{
		for (isa_cnt = 0; isa_cnt < TARGET_ISA_NUM; isa_cnt++)
		{
			if (strcmp(target_ISAs_name[isa_cnt], isa_name) == 0)
			{
				break;
			}
		}
	}
	/* default ISA if target not found */
	if (isa_cnt >= TARGET_ISA_NUM)
	{
		isa_cnt = TARGET_DEFAULT_ISA;
	}
	return isa_cnt;
}

/* get the name of a target ISA based on its ID */
const char *get_ISA_name(int isa_id)
{
	if ((isa_id < 0) || (isa_id >= TARGET_ISA_NUM))
	{
		isa_id = TARGET_DEFAULT_ISA;
	}
	return target_ISAs_name[isa_id];
}

/* get the detail for a target ISA based on its ID */
const char *get_ISA_detail(int isa_id)
{
	if ((isa_id < 0) || (isa_id >= TARGET_ISA_NUM))
	{
		isa_id = TARGET_DEFAULT_ISA;
	}
	return target_ISAs_detail[isa_id];
}

/* get the library prefix for a target ISA based on its ID */
const char *get_ISA_prefix(int isa_id)
{
	if ((isa_id < 0) || (isa_id >= TARGET_ISA_NUM))
	{
		isa_id = TARGET_DEFAULT_ISA;
	}
	return target_ISAs_prefix[isa_id];
}

/* get the ID of the default target ISA */
int get_default_ISA_id()
{
	return TARGET_DEFAULT_ISA;
}

/* get the name of the default target ISA */
const char *get_default_ISA_name()
{
	return target_ISAs_name[TARGET_DEFAULT_ISA];
}

/* get the ID of the generic target ISA */
int get_generic_ISA_id()
{
	return TARGET_GENERIC_ISA;
}

/* check if the provided name matches any of the parameters defining the target */
bool is_valid_parameter(const char *param_name)
{
	int param_cnt;
	/* find parameter by name */
	for (param_cnt = 0; param_cnt < TARGET_PARAMETERS_NUM; param_cnt++)
	{
		if (strcmp(target_parameters_name[param_cnt], param_name) == 0)
		{
			return true;
		}
	}
	return false;
}

/* get the ID of the provided parameter name 
(return NO_PARAM if the parameter name is not found) */
int get_param_id(const char *param_name)
{
	int param_cnt;
	/* find parameter by name */
	for (param_cnt = 0; param_cnt < TARGET_PARAMETERS_NUM; param_cnt++)
	{
		if (strcmp(target_parameters_name[param_cnt], param_name) == 0)
		{
			break;
		}
	}
	/* NO_PARAM if parameter not found */
	if (param_cnt >= TARGET_PARAMETERS_NUM)
	{
		param_cnt = NO_PARAM;
	}
	return param_cnt;
}

/* get the value of a parameter for an ISA specified by name
(default target ISA is considered if the ISA name is not recognized) */
int get_parameter_value_for_ISA(const char *isa_name, const int param)
{
	int isa_cnt;
	/* parameter out of range */
	if ((param < 0) || (param >= TARGET_PARAMETERS_NUM))
	{
		return 0;
	}
	/* find target ISA by name */
	for (isa_cnt = 0; isa_cnt < TARGET_ISA_NUM; isa_cnt++)
	{
		if (strcmp(target_ISAs_name[isa_cnt], isa_name) == 0)
		{
			break;
		}
	}
	/* default ISA if target not found */
	if (isa_cnt >= TARGET_ISA_NUM)
	{
		isa_cnt = TARGET_DEFAULT_ISA;
	}
	return target_ISAs[isa_cnt][param];
}

/* get the value of a parameter for an ISA specified by ID
(default target ISA is considered if the ISA ID is wrong) */
int get_parameter_value_for_ISA_id(int isa_id, const int param)
{
	/* parameter out of range */
	if ((param < 0) || (param >= TARGET_PARAMETERS_NUM))
	{
		return 0;
	}	
	/* default ISA if target out of range */
	if ((isa_id < 0) || (isa_id >= TARGET_ISA_NUM))
	{
		isa_id = TARGET_DEFAULT_ISA;
	}
	return target_ISAs[isa_id][param];
}


/* get the default number of AUs for a specified ISA (by ID)
(used for command line options initialization) */
int get_default_au_count_for_ISA_id(int isa_id)
{
	/* default ISA if target out of range */
	if ((isa_id < 0) || (isa_id >= TARGET_ISA_NUM))
	{
		isa_id = TARGET_DEFAULT_ISA;
	}
	return target_ISAs[isa_id][RUBY_NAU];
}

/* get the default core type (SP / DP / SPL / DPL) for a specified ISA (by ID)
(used for command line options initialization) */
int get_default_core_type_for_ISA_id(int isa_id)
{
	/* default ISA if target out of range */
	if ((isa_id < 0) || (isa_id >= TARGET_ISA_NUM))
	{
		isa_id = TARGET_DEFAULT_ISA;
	}
	return target_ISAs[isa_id][CORE_TYPE];
}
