
#ifndef ___target_ISA_h___
#define ___target_ISA_h___

#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif


enum target_parameters_enum
{
	NO_PARAM					= -1,
	RUBY_NAU					= 0,
	FLOAT64_ENABLE				= 1,
	FLOAT16_ENABLE				= 2,
	NCO_PRESENT					= 3,
	RRT_PRESENT					= 4,
	RCP_PRESENT					= 5,
	ATAN_PRESENT				= 6,
	SIN_COS_PRESENT				= 7,
	LOG2_PRESENT				= 8,
	EXP_PRESENT					= 9,
	LUT_TABLE_COUNT				= 10,
	CMM_PRESENT					= 11,
	RF_2SCOMP_PRESENT			= 12,
	LD_ELEM_REORDER				= 13,
	VEC_PRED_PRESENT			= 14,
	VEC_RED_PRESENT				= 15,
	FLOAT16_NCO_PREADD_PRESENT	= 16,
	SCALAR_FP_PRESENT			= 17,
	ST_LLR8_QAM_ENABLE			= 18,
	UNALIGN						= 19,
	THREAD_PROTECTION			= 20,
	CGU_PRESENT					= 21,
	CCP_PRESENT					= 22,
	STACK_OFFSET				= 23,
	CORE_TYPE					= 24,
	TARGET_PARAMETERS_NUM		= 25
};

typedef int target_parameters_ISA[TARGET_PARAMETERS_NUM];

#define TARGET_PARAMETERS_NAME	\
	"RUBY_NAU",						/* 0 */\
	"FLOAT64_ENABLE",				/* 1 */\
	"FLOAT16_ENABLE",				/* 2 */\
	"NCO_PRESENT",					/* 3 */\
	"RRT_PRESENT",					/* 4 */\
	"RCP_PRESENT",					/* 5 */\
	"ATAN_PRESENT",					/* 6 */\
	"SIN_COS_PRESENT",				/* 7 */\
	"LOG2_PRESENT",					/* 8 */\
	"EXP_PRESENT",					/* 9 */\
	"LUT_TABLE_COUNT",				/* 10 */\
	"CMM_PRESENT",					/* 11 */\
	"RF_2SCOMP_PRESENT",			/* 12 */\
	"LD_ELEM_REORDER",				/* 13 */\
	"VEC_PRED_PRESENT",				/* 14 */\
	"VEC_RED_PRESENT",				/* 15 */\
	"FLOAT16_NCO_PREADD_PRESENT",	/* 16 */\
	"SCALAR_FP_PRESENT",			/* 17 */\
	"ST_LLR8_QAM_ENABLE",			/* 18 */\
	"UNALIGN",						/* 19 */\
	"THREAD_PROTECTION",			/* 20 */\
	"CGU_PRESENT",					/* 21 */\
	"CCP_PRESENT",					/* 22 */\
	"STACK_OFFSET",					/* 23 */\
	"CORE_TYPE",					/* 24 */



enum target_ISAs_enum
{
	VSPA3_noisa				= 0,
	VSPA3_generic			= 1,
	VSPA3_LA9358			= 2,
	VSPA3_LAX_64			= 3,
	VSPA3_LAX				= 4,
	VSPA3_Geul				= 5,
	VSPA3_Europa_WB			= 6,
	VSPA3_Europa_NB			= 7,
	TARGET_ISA_NUM			= 8
};

#define TARGET_ISAS_NAME	\
	"VSPA3_noisa",				/* 0 */\
	"VSPA3_generic",			/* 1 */\
	"VSPA3_LA9358",			/* 2 */\
	"VSPA3_LAX_64",				/* 3 */\
	"VSPA3_LAX",				/* 4 */\
	"VSPA3_Geul",				/* 5 */\
	"VSPA3_Europa_WB",			/* 6 */\
	"VSPA3_Europa_NB",			/* 7 */

#define TARGET_ISAS_DETAIL	\
	"VSPA3 before ISA changes (legacy) / no ISA in command line",	/* 0 */\
	"VSPA3 generic",												/* 1 */\
	"VSPA3 Sea-Eagle",												/* 2 */\
	"VSPA3 LAX legacy (64AU)",										/* 3 */\
	"VSPA3 LAX",													/* 4 */\
	"VSPA3 Geul",													/* 5 */\
	"VSPA3 Europa WB",												/* 6 */\
	"VSPA3 Europa NB",												/* 7 */

#define TARGET_ISAS_PREFIX	\
	"",							/* 0 */\
	"gen_",						/* 1 */\
	"se_",						/* 2 */\
	"lax64_",					/* 3 */\
	"lax_",						/* 4 */\
	"geul_",					/* 5 */\
	"eurwb_",					/* 6 */\
	"eurnb_",					/* 7 */

#define TARGET_ISAS_DESCRIPTION	\
	/* VSPA3_noisa */							\
	{											\
		64, /* RUBY_NAU */						\
		0, /* FLOAT64_ENABLE */					\
		0, /* FLOAT16_ENABLE */					\
		1, /* NCO_PRESENT */					\
		1, /* RRT_PRESENT */					\
		1, /* RCP_PRESENT */					\
		1, /* ATAN_PRESENT */					\
		1, /* SIN_COS_PRESENT */				\
		1, /* LOG2_PRESENT */					\
		1, /* EXP_PRESENT */					\
		0, /* LUT_TABLE_COUNT */				\
		1, /* CMM_PRESENT */					\
		1, /* RF_2SCOMP_PRESENT */				\
		0, /* LD_ELEM_REORDER */				\
		1, /* VEC_PRED_PRESENT */				\
		0, /* VEC_RED_PRESENT */				\
		0, /* FLOAT16_NCO_PREADD_PRESENT */		\
		1, /* SCALAR_FP_PRESENT */				\
		0, /* ST_LLR8_QAM_ENABLE */				\
		0, /* UNALIGN */						\
		0, /* THREAD_PROTECTION */				\
		0, /* CGU_PRESENT */					\
		0, /* CCP_PRESENT */					\
		18, /* STACK_OFFSET */					\
		1, /* CORE_TYPE */						\
	},											\
												\
	/* VSPA3_generic */							\
	{											\
		16, /* RUBY_NAU */						\
		0, /* FLOAT64_ENABLE */					\
		1, /* FLOAT16_ENABLE */					\
		1, /* NCO_PRESENT */					\
		1, /* RRT_PRESENT */					\
		1, /* RCP_PRESENT */					\
		0, /* ATAN_PRESENT */					\
		0, /* SIN_COS_PRESENT */				\
		0, /* LOG2_PRESENT */					\
		0, /* EXP_PRESENT */					\
		0, /* LUT_TABLE_COUNT */				\
		1, /* CMM_PRESENT */					\
		0, /* RF_2SCOMP_PRESENT */				\
		1, /* LD_ELEM_REORDER */				\
		1, /* VEC_PRED_PRESENT */				\
		0, /* VEC_RED_PRESENT */				\
		1, /* FLOAT16_NCO_PREADD_PRESENT */		\
		0, /* SCALAR_FP_PRESENT */				\
		0, /* ST_LLR8_QAM_ENABLE */				\
		6, /* UNALIGN */						\
		0, /* THREAD_PROTECTION */				\
		0, /* CGU_PRESENT */					\
		0, /* CCP_PRESENT */					\
		18, /* STACK_OFFSET */					\
		1, /* CORE_TYPE */						\
	},											\
												\
	/* VSPA3_LA9358 */						\
	{											\
		16, /* RUBY_NAU */						\
		0, /* FLOAT64_ENABLE */					\
		1, /* FLOAT16_ENABLE */					\
		1, /* NCO_PRESENT */					\
		1, /* RRT_PRESENT */					\
		1, /* RCP_PRESENT */					\
		0, /* ATAN_PRESENT */					\
		0, /* SIN_COS_PRESENT */				\
		0, /* LOG2_PRESENT */					\
		0, /* EXP_PRESENT */					\
		0, /* LUT_TABLE_COUNT */				\
		1, /* CMM_PRESENT */					\
		0, /* RF_2SCOMP_PRESENT */				\
		1, /* LD_ELEM_REORDER */				\
		1, /* VEC_PRED_PRESENT */				\
		0, /* VEC_RED_PRESENT */				\
		1, /* FLOAT16_NCO_PREADD_PRESENT */		\
		0, /* SCALAR_FP_PRESENT */				\
		1, /* ST_LLR8_QAM_ENABLE */				\
		6, /* UNALIGN */						\
		0, /* THREAD_PROTECTION */				\
		1, /* CGU_PRESENT */					\
		1, /* CCP_PRESENT */					\
		18, /* STACK_OFFSET */					\
		1, /* CORE_TYPE */						\
	},											\
												\
	/* VSPA3_LAX_64 */							\
	{											\
		64, /* RUBY_NAU */						\
		0, /* FLOAT64_ENABLE */					\
		0, /* FLOAT16_ENABLE */					\
		1, /* NCO_PRESENT */					\
		1, /* RRT_PRESENT */					\
		1, /* RCP_PRESENT */					\
		1, /* ATAN_PRESENT */					\
		1, /* SIN_COS_PRESENT */				\
		1, /* LOG2_PRESENT */					\
		1, /* EXP_PRESENT */					\
		0, /* LUT_TABLE_COUNT */				\
		1, /* CMM_PRESENT */					\
		1, /* RF_2SCOMP_PRESENT */				\
		0, /* LD_ELEM_REORDER */				\
		1, /* VEC_PRED_PRESENT */				\
		0, /* VEC_RED_PRESENT */				\
		0, /* FLOAT16_NCO_PREADD_PRESENT */		\
		1, /* SCALAR_FP_PRESENT */				\
		0, /* ST_LLR8_QAM_ENABLE */				\
		0, /* UNALIGN */						\
		0, /* THREAD_PROTECTION */				\
		0, /* CGU_PRESENT */					\
		0, /* CCP_PRESENT */					\
		18, /* STACK_OFFSET */					\
		1, /* CORE_TYPE */						\
	},											\
												\
	/* VSPA3_LAX */								\
	{											\
		16, /* RUBY_NAU */						\
		0, /* FLOAT64_ENABLE */					\
		1, /* FLOAT16_ENABLE */					\
		1, /* NCO_PRESENT */					\
		1, /* RRT_PRESENT */					\
		1, /* RCP_PRESENT */					\
		1, /* ATAN_PRESENT */					\
		1, /* SIN_COS_PRESENT */				\
		1, /* LOG2_PRESENT */					\
		1, /* EXP_PRESENT */					\
		0, /* LUT_TABLE_COUNT */				\
		1, /* CMM_PRESENT */					\
		1, /* RF_2SCOMP_PRESENT */				\
		1, /* LD_ELEM_REORDER */				\
		1, /* VEC_PRED_PRESENT */				\
		1, /* VEC_RED_PRESENT */				\
		1, /* FLOAT16_NCO_PREADD_PRESENT */		\
		1, /* SCALAR_FP_PRESENT */				\
		0, /* ST_LLR8_QAM_ENABLE */				\
		1, /* UNALIGN */						\
		0, /* THREAD_PROTECTION */				\
		0, /* CGU_PRESENT */					\
		0, /* CCP_PRESENT */					\
		21, /* STACK_OFFSET */					\
		1, /* CORE_TYPE */						\
	},											\
												\
	/* VSPA3_Geul */							\
	{											\
		16, /* RUBY_NAU */						\
		0, /* FLOAT64_ENABLE */					\
		1, /* FLOAT16_ENABLE */					\
		1, /* NCO_PRESENT */					\
		1, /* RRT_PRESENT */					\
		1, /* RCP_PRESENT */					\
		0, /* ATAN_PRESENT */					\
		0, /* SIN_COS_PRESENT */				\
		0, /* LOG2_PRESENT */					\
		0, /* EXP_PRESENT */					\
		0, /* LUT_TABLE_COUNT */				\
		1, /* CMM_PRESENT */					\
		0, /* RF_2SCOMP_PRESENT */				\
		1, /* LD_ELEM_REORDER */				\
		1, /* VEC_PRED_PRESENT */				\
		0, /* VEC_RED_PRESENT */				\
		1, /* FLOAT16_NCO_PREADD_PRESENT */		\
		0, /* SCALAR_FP_PRESENT */				\
		1, /* ST_LLR8_QAM_ENABLE */				\
		6, /* UNALIGN */						\
		0, /* THREAD_PROTECTION */				\
		1, /* CGU_PRESENT */					\
		0, /* CCP_PRESENT */					\
		21, /* STACK_OFFSET */					\
		1, /* CORE_TYPE */						\
	},											\
												\
	/* VSPA3_Europa_WB */						\
	{											\
		16, /* RUBY_NAU */						\
		0, /* FLOAT64_ENABLE */					\
		1, /* FLOAT16_ENABLE */					\
		1, /* NCO_PRESENT */					\
		1, /* RRT_PRESENT */					\
		1, /* RCP_PRESENT */					\
		0, /* ATAN_PRESENT */					\
		0, /* SIN_COS_PRESENT */				\
		0, /* LOG2_PRESENT */					\
		0, /* EXP_PRESENT */					\
		0, /* LUT_TABLE_COUNT */				\
		1, /* CMM_PRESENT */					\
		0, /* RF_2SCOMP_PRESENT */				\
		1, /* LD_ELEM_REORDER */				\
		1, /* VEC_PRED_PRESENT */				\
		0, /* VEC_RED_PRESENT */				\
		1, /* FLOAT16_NCO_PREADD_PRESENT */		\
		0, /* SCALAR_FP_PRESENT */				\
		1, /* ST_LLR8_QAM_ENABLE */				\
		6, /* UNALIGN */						\
		0, /* THREAD_PROTECTION */				\
		0, /* CGU_PRESENT */					\
		0, /* CCP_PRESENT */					\
		21, /* STACK_OFFSET */					\
		1, /* CORE_TYPE */						\
	},											\
												\
	/* VSPA3_Europa_NB */						\
	{											\
		2, /* RUBY_NAU */						\
		0, /* FLOAT64_ENABLE */					\
		1, /* FLOAT16_ENABLE */					\
		1, /* NCO_PRESENT */					\
		1, /* RRT_PRESENT */					\
		1, /* RCP_PRESENT */					\
		0, /* ATAN_PRESENT */					\
		0, /* SIN_COS_PRESENT */				\
		0, /* LOG2_PRESENT */					\
		0, /* EXP_PRESENT */					\
		0, /* LUT_TABLE_COUNT */				\
		1, /* CMM_PRESENT */					\
		0, /* RF_2SCOMP_PRESENT */				\
		1, /* LD_ELEM_REORDER */				\
		1, /* VEC_PRED_PRESENT */				\
		0, /* VEC_RED_PRESENT */				\
		1, /* FLOAT16_NCO_PREADD_PRESENT */		\
		0, /* SCALAR_FP_PRESENT */				\
		1, /* ST_LLR8_QAM_ENABLE */				\
		3, /* UNALIGN */						\
		0, /* THREAD_PROTECTION */				\
		0, /* CGU_PRESENT */					\
		0, /* CCP_PRESENT */					\
		21, /* STACK_OFFSET */					\
		1, /* CORE_TYPE */						\
	},											\



#define TARGET_DEFAULT_ISA				VSPA3_generic
#define TARGET_GENERIC_ISA				VSPA3_generic

#define TARGET_ADL_PARAM_PREFIX			"param_"

#define CORE_PARAM_OFFSET				(sizeof(unsigned long) * 8 - TARGET_PARAMETERS_NUM)
#define CORE_PARAM_MASK					~((1 << CORE_PARAM_OFFSET) - 1)


extern const char * target_parameters_name[TARGET_PARAMETERS_NUM];
extern const char * target_ISAs_name[TARGET_ISA_NUM];
extern const char * target_ISAs_detail[TARGET_ISA_NUM];
extern const target_parameters_ISA target_ISAs[TARGET_ISA_NUM];

bool is_valid_ISA(const char *isa_name);
int get_ISA_id(const char *isa_name);
const char *get_ISA_name(int isa_id);
const char *get_ISA_detail(int isa_id);
const char *get_ISA_prefix(int isa_id);
int get_default_ISA_id();
const char *get_default_ISA_name();
int get_generic_ISA_id();
bool is_valid_parameter(const char *param_name);
int get_param_id(const char *param_name);
int get_parameter_value_for_ISA(const char *isa_name, const int param);
int get_parameter_value_for_ISA_id(int isa_id, const int param);
int get_default_au_count_for_ISA_id(int isa_id);
int get_default_core_type_for_ISA_id(int isa_id);



/* feature checking - needed in Front-End and Code Generation */

#define has_SCALAR_FLOAT_SUPPORT(isa_id)		(target_ISAs[isa_id][SCALAR_FP_PRESENT] != 0)

#define has_VECTOR_FLOAT64_SUPPORT(isa_id)		(target_ISAs[isa_id][FLOAT64_ENABLE] != 0)
#define has_VECTOR_FLOAT16_SUPPORT(isa_id)		(target_ISAs[isa_id][FLOAT16_ENABLE] != 0)

#define has_VECTOR_PREDICATE_SUPPORT(isa_id)	(target_ISAs[isa_id][VEC_PRED_PRESENT] != 0)

#define has_VECTOR_REDUCTION(isa_id)			(target_ISAs[isa_id][VEC_RED_PRESENT] != 0)

#define has_ATAN(isa_id)						(target_ISAs[isa_id][ATAN_PRESENT] != 0)
#define has_SIN_COS(isa_id)						(target_ISAs[isa_id][SIN_COS_PRESENT] != 0)
#define has_LOG2(isa_id)						(target_ISAs[isa_id][LOG2_PRESENT] != 0)
#define has_EXP(isa_id)							(target_ISAs[isa_id][EXP_PRESENT] != 0)
#define has_RCP(isa_id)							(target_ISAs[isa_id][RCP_PRESENT] != 0)
#define has_RRT(isa_id)							(target_ISAs[isa_id][RRT_PRESENT] != 0)

#define has_NCO(isa_id)							(target_ISAs[isa_id][NCO_PRESENT] != 0)
#define has_LUT(isa_id)							(target_ISAs[isa_id][LUT_TABLE_COUNT] != 0)


#define has_UNALIGNED_VECTOR_STORE(isa_id)		(target_ISAs[isa_id][UNALIGN] != 0)
#define has_VECTOR_STORE_CONVERSION(isa_id)		(target_ISAs[isa_id][ST_LLR8_QAM_ENABLE] != 0)
#define has_VECTOR_LOAD_CONVERSION(isa_id)		(target_ISAs[isa_id][UNALIGN] != 0)
#define has_VECTOR_LOAD_REORDER(isa_id)			(target_ISAs[isa_id][ST_LLR8_QAM_ENABLE] != 0)

#define TARGET_STACK_OFFSET(isa_id)				(target_ISAs[isa_id][STACK_OFFSET])


/* feature checking could be implemented as inline functions as well */

/*
	bool has_SCALAR_FLOAT_SUPPORT(int isa_id);
	bool has_VECTOR_FLOAT64_SUPPORT(int isa_id);
	bool has_VECTOR_FLOAT16_SUPPORT(int isa_id);
	bool has_VECTOR_PREDICATE_SUPPORT(int isa_id);
	bool has_VECTOR_REDUCTION(int isa_id);
	bool has_ATAN(int isa_id);
	bool has_SIN_COS(int isa_id);
	bool has_LOG2(int isa_id);
	bool has_EXP(int isa_id);
	bool has_RCP(int isa_id);
	bool has_RRT(int isa_id);
	bool has_NCO(int isa_id);
	bool has_LUT(int isa_id);
	bool has_UNALIGNED_VECTOR_STORE(int isa_id);
	bool has_VECTOR_STORE_CONVERSION(int isa_id);
	bool has_VECTOR_LOAD_CONVERSION(int isa_id);
	bool has_VECTOR_LOAD_REORDER(int isa_id);
*/

/*
	bool has_SCALAR_FLOAT_SUPPORT(int isa_id)		{ return (target_ISAs[isa_id][SCALAR_FP_PRESENT] != 0); }

	bool has_VECTOR_FLOAT64_SUPPORT(int isa_id)		{ return (target_ISAs[isa_id][FLOAT64_ENABLE] != 0); }
	bool has_VECTOR_FLOAT16_SUPPORT(int isa_id)		{ return (target_ISAs[isa_id][FLOAT16_ENABLE] != 0); }

	bool has_VECTOR_PREDICATE_SUPPORT(int isa_id)	{ return (target_ISAs[isa_id][VEC_PRED_PRESENT] != 0); }

	bool has_VECTOR_REDUCTION(int isa_id)			{ return (target_ISAs[isa_id][VEC_RED_PRESENT] != 0); }

	bool has_ATAN(int isa_id)						{ return (target_ISAs[isa_id][ATAN_PRESENT] != 0); }
	bool has_SIN_COS(int isa_id)					{ return (target_ISAs[isa_id][SIN_COS_PRESENT] != 0); }
	bool has_LOG2(int isa_id)						{ return (target_ISAs[isa_id][LOG2_PRESENT] != 0); }
	bool has_EXP(int isa_id)						{ return (target_ISAs[isa_id][EXP_PRESENT] != 0); }
	bool has_RCP(int isa_id)						{ return (target_ISAs[isa_id][RCP_PRESENT] != 0); }
	bool has_RRT(int isa_id)						{ return (target_ISAs[isa_id][RRT_PRESENT] != 0); }

	bool has_NCO(int isa_id)						{ return (target_ISAs[isa_id][NCO_PRESENT] != 0); }
	bool has_LUT(int isa_id)						{ return (target_ISAs[isa_id][LUT_TABLE_COUNT] != 0); }


	bool has_UNALIGNED_VECTOR_STORE(int isa_id)		{ return (target_ISAs[isa_id][UNALIGN] != 0); }
	bool has_VECTOR_STORE_CONVERSION(int isa_id)	{ return (target_ISAs[isa_id][ST_LLR8_QAM_ENABLE] != 0); }
	bool has_VECTOR_LOAD_CONVERSION(int isa_id)		{ return (target_ISAs[isa_id][UNALIGN] != 0); }
	bool has_VECTOR_LOAD_REORDER(int isa_id)		{ return (target_ISAs[isa_id][ST_LLR8_QAM_ENABLE] != 0); }
*/



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ___target_ISA_h___ */
