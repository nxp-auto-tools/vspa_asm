#define VSPA_ASM_VERSION "2.115"
#define VSPA_ADL_VERSION "#57"

// 2.115
//  NewWork CMPVSPA-3893, CMPVSPA-3894: Add ADL #58 to the assembler
//  Add ADL #58 in vspa3 assembler
// 2.114
//  Bugfix CMPVSPA-4094 CMPVSPA-4102: fix issue with combining two Ops instructions
//  if one is a mv gx, sp
//  Fix issue with combining two Ops instructions if one is a mv gx,sp (some
//  fields in the operand_values expression were incorrectly initialized)
// 2.113
//  Bugfix CMPVSPA-4094 CMPVSPA-4101: fix issue with combining two Ops instructions
//  if one is a mv to sp
//  Fix issue with combining two Ops instructions if one is a mv to sp
// 2.112
//  Bugfix CMPVSPA-4094 CMPVSPA-4095: allow mv gx,sp and mv sp,gx to be changed
//  from opB to opS
//  Allow mv gx,sp and mv sp,gx to be changed from opB to opS
// 2.111
//  Bugfix CMPVSPA-4061 CMPVSPA-4067: Fix issue on rdsk test with changing mvB
//  to mvS
//   Fix issue on rdsk test with changing mvB to mvS (correct the conditions so
//   that mv_cc instruction is only generated for mv gx,gy)
// 2.110
//  Bugfix CMPVSPA-4061 CMPVSPA-4066: Correct issue in replacing opB mv to opS
//  mv_cc for mv gx,gy
//  Correct issue in replacing opB mv to opS mv_cc for mv gx,gy
// 2.109
//  Bugfix CMPVSPA-4061 CMPVSPA-4063: allow mv gx,gy to be changed from opB to opS
//  Allow mv gx,gy to be changed from opB to opS
// 2.108
//  Bugfix CMPVSPA-3966/CMPVSPA-3957 - Fix st/ldB warning
//  CMPVSPA-3957 - Fix st/ldB warning
// 2.107
//  Bugfix CMPVSPA-4007 - Transform "Error: found a mv rx,ry between load
//  instructions" into warning
//  Transform "Error: found a mv rx,ry between load instructions" into warning
//  and add option for error
// 2.106
//  Bugfix CMPVSPA-3993, CMPVSPA-3994: Added full path in the DW_AT_name for
//  the DW_TAG_compile_unit of the built assembly files
// 2.105
//  Bugfix CMPVSPA-3973/CMPVSPA-3981 - remove assembler restriction on accepting
//  two ops instructions
//  Remove assembler restriction on accepting two ops instructions
//  Bugfix CMPVSPA-3944/CMPVSPA-3980 - Correct s_cmp_bit encoding for IPPU
//  Correct s_cmp_bit encoding for IPPU
// 2.104
//  Bugfix CMPVSPA-3966/CMPVSPA-3983 - Set NAU for default ISA
//  CMPVSPA-3983 - Set NAU for default ISA
// 2.103
//  Bugfix CMPVSPA-3936/CMPVSPA-3955 - Add fft restrictions
//  CMPVSPA-3955 - Add fft restrictions
// 2.102
//  Bugfix CMPVSPA-3936/CMPVSPA-3955 - Add machine state
//  CMPVSPA-3955 - Add fft restrictions
// 2.101
//  Bugfix CMPVSPA-3975 - Compiler incorrectly flags some legal usages of "mv Rx,
//  Ry" instruction
//  Compiler incorrectly flags some legal usages of "mv Rx, Ry" instruction
//   Build
//    2.100
//     Bugfix CMPVSPA-3975 - Compiler incorrectly flags some legal usages of
//     "mv Rx,
//     Ry" instruction
//     Compiler incorrectly flags some legal usages of "mv Rx, Ry" instruction
//  Typo in external/binutils-adl/win-build/bfd/version.h
//  Bugfix CMPVSPA-3975 - Compiler incorrectly flags some legal usages of "mv Rx,
//  Ry" instruction
//  Compiler incorrectly flags some legal usages of "mv Rx, Ry" instruction
// 2.100
//  Bugfix CMPVSPA-3975 - Compiler incorrectly flags some legal usages of "mv Rx,
//  Ry" instruction
//  Compiler incorrectly flags some legal usages of "mv Rx, Ry" instruction
// 2.99
//  Bugfix CMPVSPA-3945, CMPVSPA-3953 - comp - Any scalar mac instruction should
//  not be found in the cycle immediately following any scalar multiply instruction
//  Any scalar mac instruction should not be found in the cycle immediately
//  following any scalar multiply instruction - assembler should generate error
// 2.98
//  Bugfix CMPVSPA-3945, CMPVSPA-3953 - Any scalar mac instruction should not
//  be found in the cycle immediately following any scalar multiply instruction
//  Any scalar mac instruction should not be found in the cycle immediately
//  following any scalar multiply instruction - assembler should generate error
// 2.97
//  Bugfix CMPVSPA-3942/CMPVSPA-3957 - Add ldB/st restriction (ERR050465)
//  CMPVSPA-3957 - Add ldB/st restriction (ERR050465)
// 2.96
//  Bugfix CMPVSPA-3921, CMPVSPA-3827 - no error report for illegal parallel
//  instruction
//  We cannot have an OpS and an OpA/OpB in the same packet - assembler should
//  generate error
// 2.95
//  NewWork CMPVSPA-3840, CMPVSPA-3873: Add support to adjust debug information
//  path
// 2.94
//  Bugfix CMPVSPA-3899, CMPVSPA-3822 - Restrictions for mv rx,ry
//  Bugfix CMPVSPA-3899 CMPVSPA-3900 - [LAX][asm] Generate error for mv rx,ry
//  between load instructions
//  Bugfix CMPVSPA-3822 CMPVSPA-3869 - [LAX] Report conflicting OpV ld.<mode>,
//  ldB rX & mv Rx, Ry
// 2.93
//  Bugfix CMPVSPA-3868 CMPVSPA-3884 - [vspa3] S1zeros from S1mode is not
//  available on LA9358HW (SeaEagle)
// 2.92
//  Bugfix CMPVSPA-3868 CMPVSPA-3884 - [vspa3] S1zeros from S1mode is not
//  available on LA9358HW (SeaEagle)
//  Bugfix CMPVSPA-3909 CMPVSPA-3918 - [vspa3] compiler should report error for
//  violation of set.loop constrain
//  Bugfix CMPVSPA-3899, CMPVSPA-3822 - Restrictions for mv rx,ry
//  Bugfix CMPVSPA-3899 CMPVSPA-3900 - [LAX][asm] Generate error for mv rx,ry
//  between load instructions
//  Bugfix CMPVSPA-3822 CMPVSPA-3869 - [LAX] Report conflicting OpV ld.<mode>,
//  ldB rX & mv Rx, Ry
// 2.91
//  NewWork CMPVSPA-3761, CMPVSPA-3762: Add ADL #57 to the assembler
//  Add ADL #57 in vspa3 assembler
// 2.90
//  Bugfix CMPVSPA-3863 CMPVSPA-3864 - [vspa3] Relax short loop in the last
//  vleses of long loop restriction
// 2.89
//  Bugfix CMPVSPA-3825 CMPVSPA-3861 - [vspa3] Emit error for mv_rx_ry in the
//  next two vleses after load
// 2.88
//  Bugfix CMPVSPA-3805 CMPVSPA-3807 - [vspa3] Correctly handle wide_imm values
//  on 16au (further fix for ld.laddr version)
// 2.87
//  Bugfix CMPVSPA-3805 CMPVSPA-3807 - [vspa3] Correctly handle wide_imm values
//  on 16au.
//  Bugfix CMPVSPA-3646, CMPVSPA-3654: Assembler should report error for loop
//  delay slot restriction
//  Assembler should report error for loop delay slot restriction
// 2.86
//  CMPVSPA-3803 - [vspa3] ld instruction should always have priority to assemble
//  as an OpA - fix 1
// 2.85
//  CMPVSPA-3803 - [vspa3] ld instruction should always have priority to assemble
//  as an OpA
//  CMPVSPA-3803 - [vspa3] ld instruction should always have priority to assemble
//  as an OpA
// 2.84
//  Bugfix CMVSPA-3790 CMPVSPA-3792: Fix encoding issue in assembler
//  When transforming ld_br_agx_agy instruction to ldb_agx_agy instruction,
//  first two operands need to be switched in order (ported fix for adl 55)
// 2.83
//  Revert "Bugfix CMPVSPA-3646, CMPVSPA-3654: Assembler should report error
//  for loop delay slot restriction"
//  This reverts commit 097d4ca5efbec465e1c59dfe5e00158891234e31.
//  NewWork CMPVSPA-3486, CMPVSPA-3487: Add ADL #55 to the assembler
//  Add ADL #55 in vspa3 assembler
// 2.82
//  Bugfix CMPVSPA-3646, CMPVSPA-3654: Assembler should report error for loop
//  delay slot restriction
//  Assembler should report error for loop delay slot restriction
// 2.81
//  Bugfix CMPVSPA-3626, CMPVSPA-3627: Fix encoding issue in assembler
//  When transforming ld_br_agx_agy instruction to ldb_agx_agy instruction,
//  first two operands need to be switched in order
// 2.80
//  CMPVSPA-3623, CMPVSPA-3625: CCP_PRESENT instructions are not working
//  Legacy check for CCP_PRESENT instructions was not removed when multiple ISA
//  support was added.
// 2.79
//  Bugfix CMPVSPA-3593, CMPVSPA-3622: Implement checker for mv rx,ry in assembler
//   Implement checker for mv rx,ry in assembler
// 2.78
//  CMPVSPA-3584, CMPVSPA-3586: [IPPU] The jump instruction does not work properly.
//  IPPU relocations were not properly handled and were mistaken as VCPU
//  relocations
//  (mainly from the ADL framework update when integrating VSPA3 ADL description
//  #51)
// 2.77
//  Bugfix CMPVSPA-3551, CMPVSPA-3552: Delete incorrect version file from
//  assembly build
//  Delete incorrect version file from assembly build
// 2.76
//  Bugfix CMPVSPA-3350, CMPVSPA-3518: Modify PARAM_ATTR_MIN and PARAM_ATTR_MAX
//   Fix assembler build by changing the values in the param enum
// 2.75
//  Bugfix CMPVSPA-3350, CMPVSPA-3516: Fix vspa architecture elf number
//  Fix vspa architecture elf number
//  Bugfix CMPVSPA-3350, CMPVSPA-3517: Fix assembler build by changing the values
//  in the param enum
//   Fix assembler build by changing the values in the param enum
// 2.74
//  Bugfix CMPVSPA-3350, CMPVSPA-3516: Fix assembler build by renaming structures
//  differently in ippu and vcpu
//   Fix assembler build by renaming structures differently in ippu and vcpu
// 2.73
//  NewWork CMPVSPA-3350, CMPVSPA-3350: Add ADL #54 to the assembler
//  Add ADL #54 in vspa3 assembler
// 2.72
//  NewWork CMPVSPA-3467, CMPVSPA-3468: Fix relocation issue
//  Fix relocation issue with latest adl framework
// 2.71
//  CMPVSPA-3441, CMPVSPA-3442: extend .laddr workaround for 2AU
//  NewWork CMPVSPA-3296, CMPVSPA-3440: Fix build issues on libraries with ADL
//  51 asm
//   Fix build issues on libraries with ADL 51 asm
// 2.70
//  BuildFix : corrected linux makefile
// 2.69
//  BuildFix: correct linux makefile
// 2.68
//  NewWork CMPVSPA-3296, CMPVSPA-3297: Add ADL #51 to the assembler
//  Add ADL #51 in vspa3 assembler and also integrate ADL framework 3.19.1
// 2.67
//  CMPVSPA-3343: update laxcc default ISA - update target ISAs checked / with
//  impact on HW errata
// 2.66
//  BugFix CMPVSPA-3339, CMPVSPA-3369: Correct workaround for .laddr instructions
//  on vspa3 on 16au to skip instructions which have .laddr from assembly
// 2.65
//  BugFix CMPVSPA-3339, CMPVSPA-3365: Add workaround for .laddr instructions
//  on vspa3 on 16au
// 2.64
//  NewWork CMPVSPA-3255, CMPVSPA-3256: Add ADL #50 to the assembler
//  Add ADL #50 in vspa3 assembler
// 2.63
//  NewWork CMPVSPA-3244, CMPVSPA-3259: relax HW errata in the assembler
//  restriction checking based on the selected ISA
// 2.62
//  NewWork CMPVSPA-3200, CMPVSPA-3201: Add ADL #49 to the assembler
//  Add ADL #49 in vspa3 assembler
// 2.61
//  NOJIRA: small change in the assembler to allow no ISA option without warning
// 2.60
//  NOJIRA: small fix in the ADL #48 for the assembler
// 2.59
//  BugFix NoJira: Fix issue in interpreting -isa command line option
//  When interpreting the command line options, the VSPA speciffic command line
//  option interpreter
//  function should return 1 when it recognize the option.
//  BugFix NoJira: Fix issue in setting ISA id when none specified in the
//  command line
//  When no isa is specified in the command line, use the default value to encode
//  in the object's file elf header.
//  Fix crash in asm when incorrect isa name is used in the command line.
//  NewWork CMPVSPA-3098, CMPVSPA-3099: Add ADL #48 to the assembler
//  Add ADL #48 in vspa3 assembler
// 2.58
//  NOJIRA: small changes to accommodate the linux build for multiple ISA feature
// 2.57
//  NewWork CMPVSPA-3227, CMPVSPA-2441: Add support for mutiple isa in assembler
//  Add support for multiple isa in assembler
// 2.56
//  NewWork CMPVSPA-3098, CMPVSPA-3099: Add ADL #47 to the assembler
//  Add ADL #47 in vspa3 assembler
// 2.55
//  NewWork CMPVSPA-3070, CMPVSPA-3071: Add ADL #45 to the assembler
//  Add ADL #45 in vspa3 assembler
// 2.54
//  Bugfix CMPVSPA-3033, CMPVSPA-3034: Add ADL #44 to the assembler
//  Add ADL #44 in vspa3 assembler
// 2.53
//  Bugfix CMPVSPA-2920, CMPVSPA-2921: Add ADL #43 to the assembler
//  Add ADL #43 in vspa3 assembler
// 2.52
//  Bugfix CMPVSPA-2910, CMPVSPA-2981: Add assembler checks for hw errata for
//  vspa3 assembler
//  Add assembler checks for hw errata ERR011386 / VSPA A-011386 in vspa3 assembler
// 2.51
//  Bugfix CMPVSPA-2842, CMPVSPA-2893: Correctly add workaround for relocation
//  offset
//  Correctly add workaround for relocation offset
// 2.50
//  NewWork CMPVSPA-2842, CMPVSPA-2843: Update vspa ADL description to ADL#41
//  Update ADL description to ADL #41 in the assembler
// 2.49
//  Bugfix CMPVSPA-2876, CMPVSPA-2877: Remove condition which doesn't allow
//  complex expressions
//  Remove condition which doesn't allow complex expressions
// 2.48
//  NewWork CMPVSPA-2792, CMPVSPA-2848: Update ADL framework to ADL 3.17.7
//  Update ADL framework to ADL 3.17.7 in the assembler
// 2.47
//  CAF
//  Cleanup of unused binary files for pushing into CAF
//  Bugfix CMPVSPA-2828, CMPVSPA-2830: Update core variable based on sec flags
//  when dumping relocs
//  Update core variable based on sec flags before dumping relocs
// 2.46
//  Bugfix CMPVSPA-2706, CMPVSPA-2711: Always accept s1qline mode on vspa3
//   Always accept s1qline mode on vspa3, regardless of core_type
// 2.45
//  Bugfix CMPVSPA-2733, CMPVSPA-2734: Integrate ADL #39 in the assembler
//  Integrate ADL #39 in the assembler
// 2.44
//  BugFix CMPVSPA-2401, CMPVSPA-2532: Interpret all align info as byte-address
//  alignment
//  Interpret the .align info as byte-address alignment. Hence, no shift of the
//  align value must be performed.
// 2.43
//  Bugfix CMPVSPA-2691, CMPVSPA-2692: Integrate ADL #38 in the assembler
//  Integrate ADL #38 in the assembler
// 2.42
//  Bugfix CMPVSPA-2681, CMPVSPA-2683: Correct memory access issues in assembler
//  Correct two index out of bounds issues in the assembler
// 2.41
//  Bugfix CMPVSPA-2652, CMPVSPA-2654: Correctly set up IPPU relocations
//  Set up IPPU relocations along the VCPU relocations and use each when
//  appropriate
// 2.40
//  Bugfix CMPVSPA-2572, CMPVSPA-2602: Don't generate warnings for IPPU
//  instructions
//  Don't generate warnings that instructions are not available for IPPU
//  instructions
// 2.39
//  Bugfix CMPVSPA-2397, CMPVSPA-2594: Don't dump low_pc and high_pc in debug
//  abbrev if we don't have a text section
//  Don't dump low_pc and high_pc in debug abbrev if we don't have a text section
//  or we have an empty one
//  Bugfix CMPVSPA-2527, CMPVSPA-2528: Integrate ADL #37 in the assembler
//  Integrate ADL #37 in the assembler
// 2.38
//  Bugfix CMPVSPA-2397, CMPVSPA-2431: Don't dump low_pc and high_pc in debug
//  info if we don't have a text section
//  Don't dump low_pc and high_pc in debug info if we don't have a text section
//  or we have an empty one
// 2.37
//  Bugfix CMPVSPA-2510 CMPVSPA-2534: Report an error when the number of
//  instructions in a loop is too big
//  Report an error when the number of instructions in a loop is too big and it
//  can't be correctly encoded
//  Bugfix CMPVSPA-2555, CMPVSPA-2562: Report an error for opB ld Rn instructions
//  which are not grouped with another ld
//  Don't allow opB ld Rn instructions which are not grouped with another ld
//  (there are opVld instructions which are not ld, and which should not be
//  grouped with an opB ld)
// 2.36
//  Bugfix CMPVSPA-2514 CMPVSPA-2522: Report an error when the start or end
//  symbol of a function is undefined
//  Report an error when the start or end symbol of a function is undefined and
//  the size of the function can't be computed
// 2.35
//  NewWork CMPVSPA-2487 CMPVSPA-2492: Change banner
//  Change banner  and error messages to remove VSPA name
// 2.34
//  NewWork CMPVSPA-2449 CMPVSPA-2450: Add ADL #36 in the assembler
//  Add ADL #36 generated files in the assembler
// 2.33
//  BugFix CMPVSPA-2475, CMPVSPA-2476: Fix asm compaction issue
//  Fix compaction issue: when checking if an instruction can be compacted, the
//  assembler checks if an instruction can be replaced, if it's a opB, with an OpS
//  version. In this moment, the instruction is checked only if it has the OpBS
//  attribute but the assembler doesn't check also the replacement constraints.
// 2.32
//  NewWork CMPVSPA-2309 CMPVSPA-2310: Add ADL #35 in the assembler
//  Add ADL #35 generated files in the assembler, including support for excluding
//  instructions with attribute instr_ccp
// 2.31
//  Bugfix CMPVSPA-2398 CMPVSPA-2469: Fix an assembler compilation issue
//  Fix an assembler compilation issue on release and a memory leak
// 2.30
//  Bugfix CMPVSPA-2398 CMPVSPA-2419: Fix in function which extracts jmp_jsr info
//  The function which extracts jmp_jsr info for an instruction (if a change
//  of flow instruction is jmp or jsr, and conditional or not) was not updated
//  for vspa3
// 2.29
//  Bugfix CMPVSPA-2381 CMPVSPA-2389: Fix relocation expressions with symbols
//  If a relocation expression contains a symbol, the unresolved marker might
//  not have been set even it should have been.
// 2.28
//  Bugfix CMPVSPA-2353 CMPVSPA-2358: Fix the address of symbols on ippu
//  Only do label address readjusting on vcpu, as it's npt needed on ippu
// 2.27
//  NewWork CMPVSPA-2270 CMPVSPA-2271: Add ADL #34 in the assembler
//  Add ADL #34 generated files in the assembler
// 2.26
//  Bugfix CMPVSPA-2341 CMPVSPA-2342: Change the sorting algorithm in
//  reorder_opcodes
//  Change the sorting algorithm in reorder_instructions to use a stable sorting
//  algorithm (to not reorder opcodes with the same priority)
// 2.25
//  BugFix CMPVSPA-2291, CMPVSPA-2295, CMPVSPA-2293, CMPVSPA-2294: Fix asm
//  relocation issue
//  WA CMPVSPA-2302: Fix issue in applying relocation: when the field where the
//  relocation should be applied has size < 32 bits, the assembler was overwriting
//  the data. This is the case for R_VSPA_1 (8 bits) and R_VSPA_2 (16 bits),
//  relocations that are used in the debug sections.
// 2.24
//  Bugfix CMPVSPA-2280: Add workaround for incorrect left shift of addend
//  in relocations
//  The assembler incorrectly left shifts the addend in R_VSPA_DMEM_20 and
//  R_VSPA_DMEM_21 relocations
// 2.23
//  Bugfix CMPVSPA-2260: Add workaround for incorrect wide_imm instructions error
//  Add workaround for incorrect error reported by assembler on some correct
//  wide_imm 16au instructions
//  NewWork CMPVSPA-2258: Remove debug printf in the assembler
//  Remove debug printf in the assembler
// 2.22
//  NewWork CMPVSPA-2179 CMPVSPA-2180 CMPVSPA-2211: Update adl generated files
//  in the assembler to ADL #33
//  Update adl generated files to ADL #33 and add workaround for CMPVSPA-2211;
//  also removed workarounds for CMPVSPA-2061 and CMPVSPA-2067
// 2.21
//  Bugfix CMPVSPA-2234: Add workaround for missing wide_imm checker
//  Manually add checker for wide_imm immediate to reject instruction if the
//  immediate doesn't have correct value
// 2.20
//  NewWork CMPVSPA-2101 CMPVSPA-2102: Update adl generated files in the assembler
//  to ADL #31
//  Update adl generated files to ADL #31
// 2.19
//  BugFix CMPVSPA-2176, CMPVSPA-2200: Add workaround for the CMPVSPA-2175.
//  Add workaround for the CMPVSPA-2175: Relocation R_VSPA_LAB_IND_32 must have
//  shift value 0.
// 2.18
//  Bugfix CMPVSPA-2085 CMPVSPA-2087: Add workaround for offset  in relocations
//  Add workaround for offset!=0 in relocations (force it to 0)
// 2.17
//  NewWork CMPVSPA-1973 CMPVSPA-1974: Update adl generated files in the assembler
//  to ADL #29
//  Update adl generated files to ADL #29 and correct a regular expression
//  in transform.pl
//  manually updated the changes between versions
// 2.16
//  Bugfix CMPVSPA-2084 CMPVSPA-2088: Add workaround for ADL issue in
//  opDSCheckIfExchangable
//  Add workaround for ADL issue in opDSCheckIfExchangable (return an invalid
//  value from getOpValue for expressions that are not constants)
// 2.15
//  Bugfix CMPVSPA-1980 CMPVSPA-1981: Correctly fix the set.loop issue
//  Correctly fix the set loop issue (remove code in the assembler that
//  handled set.loop instructions in a special way, as the value is now
//  correctly computed and encoded by the modifier/inserter functions in the
//  adl generated files)
//
//  BugFix CMPVSPA-2063, CMPVSPA-2066: Indexed access is incorrectly encoded
//  as post-incremented
//  Modify the first implementation of the WA: Limit the ignoring of instr_inst
//  attributes to only LD and ST instructions.
// 2.14
//  BugFix CMPVSPA-2063, CMPVSPA-2066: Indexed access is incorrectly encoded
//  as post-incremented
//  Indexed access is incorrectly encoded as post-incremented. This is due
//  to replacement
//  of OpC instruction with an incorrect OpS : ld_gY_agXIs18 is replaced with
//  ldS_GX_agY_u_Is9_ld
//  instead of ldS_GX_agY_Is9_ld.
//  The change consists in a WA: all instr_inst attributes are ignored.
// 2.13
//  Bugfix CMPVSPA-2060 CMPVSPA-2061: Add workaround for mvB gx,gy compaction issue
//  Add workaround for mvB gx,gy compaction issue (it is transformed into
//  mvS_agx_agy which can't accept two g regs), the workaround is to disable
//  compaction in this case
// 2.12
//  BugFix CMPVSPA-2024, CMPVSPA-2026: BugFix in .size expression computation
//  After updating ADL to version 3.16.24, an issue was introduced. In function
//  "elf_frob_symbol", if the size is a constant (O_constant), in the switch
//  statement
//  "break" is missing.
// 2.11
// BugFix CMPVSPA-2050, CMPVSPA-2051: Update adl to latest version
// Update adl to latest version 3.16.24
// 2.10
// BugFix CMPVSPA-2024, CMPVSPA-2025: Add workaround for wrong size computation
// Add WA for wrong symbol size computation.
// If we have function _X that has no FuncEnd_X symbol, but size is defined
// .global _X
// _X
// .type _X, @function
// ...
// .size _X, . - _X
// When generating debug info, FuncEnd_X symbol will be automatically emitted :
// FuncEnd_X = _X + size
// It's value is correct in symbol table.
// However, size of _X has wrong value(2 * actual size).This is due to adding
// to the constant value previously computed the value of pc(".")
//
// BugFix CMPVSPA-2017, CMPVSPA-2030: Add workaround for wrong relocation offset
// The relocation offset is incorrectly computed as macro - instruction's offset in
// section + instruction - field bit offset.This is incorrect and breaks the
// convention between linker and assembler.
// 2.9
//  BugFix CMPVSPA-1980, CMPVSPA-2011: Add workaround for segfault
//  Add WA for SegFault on ewl/EWL_C/src/math_longdouble.c
//   The index for a set.loop is incorrectly modified
//  somewhere to -1. Hence the memory is incorrectly accessed.
//  This function computes the value for the set.loop instruction:
//    set.loop CNT, (Loop_end - Loop_start)
//  (Loop_end - Loop_start) is always the 2nd argument
// 2.8
//  Bugfix CMPVSPA-1943 CMPVSPA-1944: Add ADL #27
//  Add ADL #27 in the assembler
// 2.7
//  Bugfix CMPVSPA-1916 CMPVSPA-1917: Add ADL #25
//  Add ADL #25 in the assembler
// 2.6
//  Bugfix CMPVSPA-1930 CMPVSPA-1935: Fix issue in computing expressions
//  Fix issue in computing expressions between local symbols
//  Update VSPA_ADL_VERSION variable to 24
//  Update VSPA_ADL_VERSION variable to 24
// 2.5
//  Bugfix CMPVSPA-1923 CMPVSPA-1924: Correct label addresses dumped by asm
//  Correct label addresses dumped by asm
//  Update VSPA_ADL_VERSION variable to 24
// 2.4
//  Buildfix: Fix Linux build
//  Fix linux build by placing attribute always_inline on function def not
//  implementation
// 2.3
//  NewWork CMPVSPA-1870 CMPVSPA-1869: Add ADL #24 in the assembler
//  Add ADL #24 support in the assembler
//  NewWork CMPVSPA-1871 CMPVSPA-1869 CMPVSPA-1778: Remove instr_lut WA in the asm
//  Remove the WA for missing instr_lut attribute in the assembler
//  BuildFix: Update ADL number to #24
//  Update VSPA_ADL_VERSION variable to 24
// 2.2
// NewWork CMPVSPA-1812 CMPVSPA-1815: Add ADL release #23
//  Add release23 of the ADL to the assembler
// 2.1
// NewWork CMPVSPA-1711 CMPVSPA-1712
//  Add initial support for VSPA3
// 1.176
//  BugFix CMPVSPA-1114, CMPVSPA-1107: When computing the fixup value to replace
//  hardware loop instruction count into the
//  difference (EndLable - StartLabel), the assembler must make sure that the
//  value can be written
//  in the immediate instruction field:
//  set.loop X, loopStart, loopEnd => set.loop X, InstrCnt
//  where InstrCnt = (loopEnd - loopStart) / 8.
//  The fix adds a check that the InstrCnt can be encoded or emits an error
//  otherwise.
// 1.175
//  Other: Fix a few build warnings.
//  Other: Standardize version file in order to be able to parse it by a script
//  (for auto versioning)
// 1.174
// A new version of fix cmpvspa-927(previous fix incorrectly gave an error for
//	mv.. rv,... 
// 1.173
// NewWork CMPVSPA-978: Add new assembler directive .stack_effect. Based on
// the information provided by this directive, the assembler will emit 
// mw_info type 8 which is used by the linker to compute the stack effect
// of each function.
// 1.172
// Fix cmpvspa-927: don't allow wr.X and mv/fill/lsb2rf [rv] in parallel in 
//     assembler
// 1.171
// CMPVSPA-1053: Update VCPU2 ADL to 6afc773c7786610ede5f57aa9eb4ca9b06860859
//    (vspa2_db_r.ARCH-1.45.xml)
//    - Add instr_fnop attribute on loop_end pseudo instruction
// 1.170
// CMPVSPA-1053: Update VCPU2 ADL to 3a7d9f2f7070df3850e72d7e40939ab121536445
//    (vspa2_db_r.ARCH-1.44.xml)
//    - setB.loop fix for field location issue: created new label instrFields
//    defines; and created new relocation defines.
//    - Added S0 mux modes: group2nr & group2nc; Added S1 mux modes: interp2nr
//    & interp2nc.
// 1.169
// CMPVSPA-900: Update VCPU2 ADL to 9e9c5efd5e5ff04970bd218654abf7d8125c318f 
//    (vspa2_db_r.ARCH-1.43.xml)
//    - fix some issues related to ldm/stm instructions
// 1.168
// - Bugfix for CMPVSPA-1039: Incorrect handling of st I17, I32 instruction
// The pseudo-instruction st I17, I32 should be replaced by two instructions
//	st.s i17, (i32 & 0xFFFF)
//	st.h i17, ((i32 >> 16) & 0xFFFF)
// when necessary.
// Values are read from the input asm file and stored in a 64bit signed int as 
// follows:
//    - if it has sign, it is stored as sign extended 
//        e.g. -0xABCD -> 0xFFFF FFFF FFFF 5433
//    - if it has no sign, it is stored as zero extended
//        e.g. 
//        1. 0xABCD -> 0x0000 0000 0000 ABCD
//        2. even though the instruction field is a 32 bit value, where 
//            -0xABCD = 0xFFFF 5433
//            0xFFFF 5433  is stored as 0x0000 0000 FFFF 5433
// Hence when checking if a value is a 16bit or a 32bit value, we must take in
// consideration also the zero extended values.
// 1.167
// - Bugfix for CMPVSPA-912: unable to assemble
//           setB.loop agX, LABEL_BEG, LABEL_END
//	 This instruction is actually an alias for "setB.loop AGX, INSTR_LOOP_CNT".
// Operand INSTR_LOOP_CNT must be computed by the assembler using 
// "LOOP_END_XYZ - LOOP_BEG_XYZ" In the processing step when the assembler
// tries to make the replacement, the instruction is already replaced by the 
// macro - instruction and the assembler has no information about the source
// instruction of the operand.
// 1.166
// - Update VCPU2 ADL model, which adds the missing instr_ctrl attribute to the
// set.vrarange* instructions and also adds .clr as an alias for .eq and .set
// as an alias for .ne (the changes are part of CMPVSPA-842)
// 1.165
// -Bugfix CMPVSPA-852 (restrict bigfoot3 checker to DMA_XFER_CTRL reg)
// 1.164
// -Bugfix CMPVSPA-741 (add bigfoot3 checker in asm)
// 1.163
// - New Work for: CMPVSPA-377: Update  VCPU2 ADL model which adds ldm/stm 
//  instructions
// -Bugfix for CMPVSPA-721: Assembler doesn't dump empty DIE at end of 
// compilation unit
// 1.162
// - Bugfix for CMPVSPA-694: Issue in debug_info for type of asm functions
// 1.161
// - Bugfix for CMPVSPA-634: Check for the asm .size directive value correctness
//	after section fragment relaxation step (compute of start address).
// 1.160
// - Update VCPU1 and VCPU2 ADL models, which add back aX operand to VSPA1
// "set.br Iu3" => "set.br aX, Iu3" and agX operand to VSPA2 "set.br Iu4" =>
// "set.br agX, Iu4" (JIRA CMPVSPA-553, CMPVSPA-578). Also, instr_load
// attribute is removed from instructions that don't load directly from memory
// (loads to VRA).
// 1.159
// - Modify mechanism that collects information about functions' start/end
// symbols and .debug_info/.debug_line contents start/end symbols to allow
// multiple functions to start or end at the same address (JIRA CMPVSPA-589,
// CMPVSPA-604).
// 1.158
// - Modify routine that collects function start and end symbols to not display
// a warning for the case when a function does not have an end symbol, but the
// assembler is able to successfully generate the symbol by using the value and
// the size of the function's start symbol (JIRA CMPVSPA-594, CMPVSPA-601).
// 1.157
// - Update VCPU1 and VCPU2 ADL models, which include changes related to the
// bit reversal instructions (JIRA CMPVSPA-553, CMPVSPA-578):
//   - remove aX operand from the syntax of VSPA1 "set.br aX, Iu3" and VSPA2
//   "set.br aX, Iu4" instructions;
//   - mark VSPA1 "st.br(.llr_mode) [aX]+aY" and VSPA2 "st.br(.llr_mode)
//   [agX]+agY" instructions as illegal; the assembler displays a warning for
//    each encounter of these instructions.
// 1.156
// - Update routine that checks the restrictions for instructions with variable
// number of arguments to be able to handle the following upcoming VSPA2
// instructions:
//   ldm [sp+Is10], rX;
//   stm rX, [sp+Is10].
// 1.155
// - Update release notes file to include instructions for building the
// assembler by using the Makefile.
// 1.154
// - Force alignment of code sections to the size of the program memory word,
// which is 64 bits for VCPU and 32 bits for IPPU (JIRA CMPVSPA-416,
// CMPVSPA-426).
// 1.153
// - Fix Cygwin32 GCC build.
// - Update IPPU2 ADL model, which adds support for half-word addressing (JIRA
// CMPVSPA-382, CMPVSPA-419). The width of the immediate field was extended for
// the following instructions:
//   mv ippu_reg, I18 => mv ippu_reg, I19;
//   set.range aY, asA, Iu18 => set.range aY, asA, Iu19.
// The following instructions were added:
//   add aX, Is8;
//   add aX, mY;
//   add.cb aX, Is8;
//   add.cb aX, mY.
// 1.152
// - Fix Cygwin GCC build.
// - Update IPPU2 ADL model, which adds the following instructions
//  (JIRA CMPVSPA-323, CMPVSPA-410):
//    ld.type [aX+iY]+/-Is4, elem_offset, wr_offset, latch_mode;
//    st.type [aX+iY]+/-Is4, elem_offset, rd_offset, write_mode.
// 1.151
// - Remove Windows XP support.
// 1.150
// - Update VCPU2 ADL model, which adds new opVau "mafac" instruction (JIRA
// CMPVSPA-257, CMPVSPA-284).
// 1.149
// - Automatically add end of function symbol when .size directive is present
// in assembly source file. The symbol will be used when generating mw_info
// type 6 (remove_block) entries. Add checking that .size directive has same
// value as (symbol_end - symbol_start) (JIRA CMPVSPA-309, CMPVSPA-307).
// 1.148
// - Extract from the ADL model the information related to what operand values
// each modifier uses to compute the final value to be encoded in the opcode
// and use it to check that all those operand values evaluate to constants
// (JIRA CMPVSPA-260, CMPVSPA-269).
// 1.147
// - Fix instruction grouping check to not allow an opV and an opD instruction
// in the same packet (JIRA CMPVSPA-259, CMPVSPA-264).
// 1.146
// - Modify delay slot restriction check to allow an unconditional "jmp"
// instruction in the delay slot of another "jmp" instruction. On VSPA1, a
// warning is issued in situations like this (JIRA CMPVSPA-202, CMPVSPA-246).
// 1.145
// - Update VCPU2 ADL model, which adds the new opC "swi(.cc) Iu16"
// instruction. The instruction is modeled as a macro that includes both opC
// "swi(.cc) Iu16" and opZ "break" (JIRA CMPVSPA-223, CMPVSPA-232).
// 1.144
// - Update VCPU2 ADL model, which splits the following instructions (JIRA
// CMPVSPA-175/CMPVSPA-177):
//     mv NCO_REG, gX => mv nco_freq, gX; mv nco_phase, gX; mv nco_k, gX;
//     mv gX, NCO_REG => mv gX, nco_freq; mv gX, nco_phase; mv gX, nco_k.
// 1.143
// - Remove empty sections with no other symbols than the section symbols
// referencing them (JIRA CMPVSPA-112, CMPVSPA-115).
// 1.142
// - Update VCPU1 and VCPU2 ADL models, which fix the range of the immediate
// value operands of the following "set.loop" instructions (JIRA CMPVSPA-121/
// CMPVSPA-128):
//   - VSPA1:
//     set.loop Iu10;
//     set.loop Iu10, Iu10;
//     set.loop Iu10, Iu17, Iu17;
//     set.loop asX, Iu10;
//   where:
//     Iu10 => [1, 1024];
//   - VSPA2:
//     setB.loop Iu11;
//     setC.loop Iu16;
//     set.loop Iu16, Iu10;
//     set.loop Iu16, Iu25, Iu25;
//     set.loop agX, Iu10;
//   where:
//     Iu10 => [1, 1024];
//     Iu11 => [1, 2048];
//     Iu16 => [1, 65536].
// 1.141
// - Add projects and solution for Visual Studio 2013 (JIRA CMPSC-143/
// CMPSC-154).
// 1.140
// - Modify generation of ".debug_line" information to reset the machine state
// register at the beginning of a function in order to allow stripping the
// function's corresponding ".debug_line" entries for debug information
// generated by the assembler. Generate ".MW_INFO" type 6 information (remove
// block when stripping symbol) for function/object symbols in order to allow
// stripping the data from ".debug_line" and ".debug_info" associated to the
// symbol. This kind of information is generated only for functions for which a
// function end symbol ("F<func_name>_end" or "FuncEnd<func_name>", where
// <func_name> is the name of the function symbol) is present in the symbol
// table (JIRA CMPVSPA-86/CMPVSPA-92, CQ:ENGR00348984).
// 1.139
// - Update VCPU1 ADL model, which adds new "st Iu17, I32" pseudoinstruction
// and a new "R_VSPA_LAB_16" relocation for the following opD instructions:
//   - "st Iu17, Iu16";
//   - "st.s Iu17, Iu16";
//   - "st.h Iu17, Iu16";
// - Modify the way the VSPA1 "st Iu17, I32" pseudoinstruction is handled in
// order to allow symbolic expressions for the second operand (JIRA CMPVSPA-62/
// CMPVSPA-75). For "st _x, _expr", where "_expr" is a symbolic expression, the
// assembler will generate and encode two instructions:
//   st _x, (_expr) & 0xFFFF;
//   st.h _x, ((_expr) >> 16) & 0xFFFF.
// 1.138
// - Update VCPU2 ADL model, which adds new "instr_lut" attribute for LUT
// instructions ("lut.hsw", "set.lut Iu2", "lut.fwr [gX]+Is6",
// "lut.hwr [gX]+Is6", "lut.rd") and "instr_set_smode" attribute for
// "set.Smode ((S0conj,) (S0sign,) S0mode,) (S1mode,) (S2mode)".
// - Implement checks for VCPU2 LUT specific restrictions: all above mentioned
// LUT instructions and the "S1qline" value of the S1mode operand of
// "set.Smode ((S0conj,) (S0sign,) S0mode,) (S1mode,) (S2mode)" are not
// supported on all AU SP and DP (CQ:ENGR00344541).
// 1.137
// - Update VCPU2 ADL model, which:
//   - Splits "jmp(.cc)" and "jsr(.cc)" to separate the instructions whose
//   qualifiers depend on "cc" register from those that depend on "au_an" and
//   "au_ap" registers;
//   - Adds "instr_set_prec" attribute to "set.prec S0prec, S1prec, S2prec,
//   AUprec, Vprec".
// - Implement checks for VCPU2 AU count and core precision restrictions:
//   - "set.prec S0prec, S1prec, S2prec, AUprec, Vprec" doesn't support
//   "double" mode for SXprec, AUprec and Vprec for 64AU SP.
// 1.136
// - Fix typographical mistakes in error messages displayed while checking
// delay slot restrictions (Rally US4890 / CQ:ENGR00343473).
// - Update VCPU1 ADL model, which splits "lsb2rf(.sr) [r7V], gX" into
// "lsb2rf [r7V], gX" and "lsb2rf(.sr) [r7V], gX". Update VCPU2 ADL model,
// which adds/removes the family letter to/from the mnemonics of the following
// instructions (Rally US4876):
//     opA "add(.laddr) aU, Is9" => "addA(.laddr) aU, Is9";
//     opA "addA aU, aV, aW" => "add aU, aV, aW";
//     opA "subA aU, aV, aW" to "sub aU, aV, aW";
//     opB "set.loop ITER_CNT" => "setB.loop ITER_CNT";
//     opC "add aV, aU, Is19" => "addC aV, aU, Is19";
//     opS "set.creg creg, Iu4" => "setS.creg creg, Iu4".
// 1.135
// - Update VCPU2 ADL model, which adds new generic "ld Rx" aliases for opB
// "ldB Rx" and opVld "ld.normal Rx" and a set of rules to instantiate such
// generic instructions inside a packet to comply to the VSPA2 packet formats.
// A "ld Rx" instruction is encoded as a "ld.normal Rx", unless we have another
// opVld instruction in the packet, in which case it is encoded as a "ldB Rx"
// (Rally US4786).
// 1.134
// - Update VCPU2 ADL model, which changes the syntax of opS "set.creg creg,
// Iu4" to "setS.creg creg, Iu4". Add VSPA1 on VSPA2 translation rule from
// "set.creg creg, Iu4" to "setS.creg creg, Iu4" (Rally US4844 /
// CQ:ENGR00341627).
// 1.133
// - Update VCPU2 ADL model, which removes redundant alias of "setB.loop Iu11"
// and adds new generic alias for "setB.creg creg, Iu4" (Rally US4786).
// 1.132
// - Update VCPU2 ADL model, which fixes the encoding of the "ldm"/"stm"
// instructions in the presence of negative offsets (Rally US4606) and the
// definition of the "set.cgu Iu5" instruction (ENGR00340689).
// 1.131
// - Fix Klocwork issues (Rally US4728).
// 1.130
// - Fix Klocwork issues (Rally US4728).
// - Update IPPU1 and IPPU2 ADL models, which rename the "indx" register to
// "vindx_ptr", allow I17 for "mv mX, Is17" on IPPU1 and I18 for "mv mX, Is18"
// on IPPU2 and restrict the ranges of immediate offsets to signed values
// (ENGR00339156).
// 1.129
// - Inhibit debug information emitting for IPPU code specified via the
// "-mippu" command line option or the ".ippu" assembler directive
// (ENGR00338677).
//  - Fix debug information in ".debug_abbrev" section to specify
// "DW_children_yes" for the "DW_TAG_compile_unit" entry, since
// "DW_TAG_base_type", "DW_TAG_base_subprogram" and "DW_TAG_variable" are its
// children (ENGR00338706).
// 1.128
//  - Update VCPU1 ADL model, which changes the Is31 operand of the
// "set.nco {radix2, singles, normal}, Iu10, Is31" instruction to I31 and fixes
// the way the conjugate bit is set: 0 - if bit 30 is 1 (negative values) and 1
// if bit 30 is 0 (positive values) (Rally US4603). Update GCC translation
// script rule that translates from VSPA1 GCC "set nco {radix2, singles,
// normal}, Iu10, Iu4, Iu3, Iu2(, conj)" to VSPA1 iScape "set.nco {radix2,
// singles, normal}, Iu10, Is31" to take into account the new semantic of the
// VSPA1 iScape instruction.
// 1.127
//  - Update VCPU2 ADL model, which splits the "ldm" instruction in multiple
// instructions, one for each potential number of arguments. This simplifies
// the regular expression used to match the instruction, significantly
// reducing the time needed to parse the instruction. Add check for duplicates
// in the list of operands of the instructions with variable number of
// arguments (Rally US4606).
//  - Update VCPU1 and VCPU2 ADL models, which remove the "Rxl1" and "RxRyl1"
// right rotate modes.
// 1.126
//  - Update VCPU2 ADL, which removes the opZ "loop_break" instruction; the
// instruction is now included in the encoding of the opC
// "loop_break(.cc) Iu25" instruction (ENGR00337861).
// 1.125
//  - Update VCPU2 ADL, which fixes the family of the "nco" instruction
// (ENGR00337508).
// 1.124
//  - Modify GCC translation script rule that translates from GCC VSPA1
// "set nco {radix2, singles, normal}, Iu10, Iu4, Iu3, Iu2(, conj)" to iScape
// VSPA1 "set.nco {radix2, singles, normal}, Iu10, Is31". The new rule
// generates a negative Is31 value when encountering "conj" instead of
// generating a positive Iu31 value with bit 30 set to 1 (ENGR00336588).
//  - Allow "dp" option for the "core_type" command line parameter and update
// the values used for core precision type encoding in the ELF header (sp = 0,
// dp = 1, spl = 2 (VSPA2 only), dpl = 3 (VSPA2 only)).
//  - Fix the parsing of the "core_type" command line parameter to enable the
// right core precision type when receiving "dpl" on VSPA2.
// 1.123
//  - Add checks for special AFD440 (VSPA1) restrictions (ENGR00333056,
// ENGR00332837):
//    - "atan" is supported only on 4AU;
//    - "rcp" and "rcp.e" are not supported on 64AU SP;
//    - "rrt" and "rrt.e" are not supported on all AU DP;
//    - "cmp_even" mode of "set.xtrm" instruction is not supported on any AU
//   configuration;
//    - "cmp aX, Iu17" does not modify C condition carry flag and clears the
//   V flag; unsigned conditional instructions (.hi, .lo, .hs, .ls, .cs, .cc,
//   .vs, .vc) can't be used in conjunction with this instruction;
//    - "addA(.laddr) aX, Is8" is not supported on 64AU;
//    - "set.prec" instruction doesn't support "half" mode for SXprec and
//   Vprec on 64AU and "double" mode for SXprec, AUprec and Vprec for all AU
//   SP;
//    - "rd SX" and "rrot" can't be grouped with "set.range" on 64AU;
//    - "setA.VRAptr" and "setB.VRAptr" can't be grouped together on 64AU.
//  - Update the VCPU2 ADL, which allows "sp" as operand for the following
// instructions:
//     cmpS.z gZ, Iu16;
//     cmpD(.cc) gY, I32.
// 1.122
//  - Update VCPU2 ADL, which removes the restrictions imposed to the operands
// of the "fill.d/mv.d [rV], gX:gX+1" and "fill.q/mv.q [rV], gX:gX+1:gX+2:gX+3"
// instructions, which state that X should be even for "fill.d/mv.d" and
// multiple of 4 for "fill.q/mv.q" (ENGR00334284).
//  - Update VCPU1 ADL, which includes a fix for the definition of the "Is31"
// operand of the "set.nco {radix2, singles, normal}, Iu10, Is31" instruction,
// which should allow signed values (ENGR00335167).
// 1.121
//  - Add restriction check for instructions with variable number of operands
// such as "pushm/popm" (VSPA1 and VSPA2) and "stm/ldm" (VSPA2), which can
// save/restore a number of registers equal to the number of words in a memory
// line. There is an additional restriction that forbids mixing registers
// belonging to different intervals, where such an interval is obtained by
// splitting the complete ordered list of registers allowed by a specific
// instruction in groups with a size equal to the number of words in a memory
// line (ENGR00333054).
// 1.120
//  - Fix generation of ".MW_INFO" information for hardware loops with size of
// one or two instructions (ENGR00333783).
// 1.119
//  - Update VCPU1 and VCPU2 ADL models, which include fixes related to the
// relocations generated for opS microinstructions packed in XSVZ
// macroinstructions (ENGR00333008).
//  - Update the VCPU2 ADL, which additionally allows generating relocations
// for the following instructions:
//     addD(.ucc)(.cc) gX, gY, I32 (sp allowed);
//     addD(.ucc)(.cc) gX, I32 (sp allowed);
//     andD(.cc) gX, gY, I32;
//     andD(.cc) gX, I32;
//     cmpD(.cc) gX, I32;
//     floatx2n gX, gY, I32;
//     lfsr gX, gY, I32;
//     mpyD gX, gY, I32;
//     mvip gX, Iu9, Iu32;
//     mvip Iu9, gX, Iu32;
//     mvip Iu9, Iu32;
//     orD(.cc) gX, gY, I32;
//     orD(.cc) gX, I32;
//     push I32;
//     subD(.ucc)(.cc) gX, gY, I32 (sp allowed);
//     subD(.ucc)(.cc) gX, I32 (sp allowed);
//     xorD(.cc) gX, gY, I32;
//     xorD(.cc) gX, I32.
//  - Change the GCC translation script to ignore directives if they are
// preceded by spaces and fix translation of "mv R7[rV], gX" (ENGR00332648).
//  - Generate ".MW_INFO" entries for the BigFoot case that involves a DMEM
// access inside a hardware loop of size 1 with an iteration count bigger than
// 1 immediately followed (the two instruction bundles are consecutive) by an
// instruction bundle containing a change-of-flow instruction
// ("jmp"/"jsr"/"rts") (ENGR00330506).
// 1.118
//  - Update the VCPU2 ADL, which adds a new syntax for the following
// instructions:
//     mv.d [rV], gX -> mv.d [rV], gX:gX+1
//     fill.d [rV], gX -> fill.d [rV], gX:gX+1
//     mv.q [rV], gX -> mv.q [rV], gX:gX+1:gX+2:gX+3
//     fill.q [rV], gX -> fill.q [rV], gX:gX+1:gX+2:gX+3
// 1.117
//  - Update the VCPU2 ADL, which contains a fix for the computation of the
// number of lines for wide immediate operands, which are expressed in number
// of half-words when the VSPA1onVSPA2 mode is disabled and in number of words
// otherwise. Update wide immediate value checker to reflect the changes from
// the ADL.
// 1.116
//  - Update VCPU1 ADL, which include the following changes:
//    - Add generic aliases for:
//       addA aX, Is8;
//       addA aX, aX, Is8;
//       addA.laddr aX, Is8;
//       addA.laddr aX, aX, Is8;
//    - Allow relocations for:
//       ld H, Iu17;
//       st Iu17, H;
//       add gX, gX, I32;
//       add gX, I32;
//       andl gX, gX, Iu32;
//       andl gX, Iu32;
//       cmp gX, I32;
//       lfsr gX, I32;
//       mvip gX, Iu9, Iu32;
//       mvip Iu9, gX, Iu32;
//       mvip Iu9, Iu32;
//       orl gX, gX, Iu32;
//       orl gX, Iu32;
//       push I32;
//       sub gX, gX, I32;
//       sub gX, I32;
//       xorl gX, gX, I32;
//       xorl gX, I32;
//       add aX, aX, Is17;
//       add aX, Is17;
//       add asX, aY, Is17;
//       cmp aX, Iu17.
//  - Update VCPU2 ADL, which adds a wide immediate alias for
// "add(.laddr) aX, Is9".
// 1.115
//  - Update VCPU2 ADL, which adds the following instructions:
//    - opB:
//      hfixtofloatsp gX, gY;
//      floatsptohfix gX, gY;
//      floathptofloatsp gX, gY;
//      floatsptofloathp gX, gY;
//    - opC:
//      ldC agY, [sp+Is18] (it now allows aY as destination);
//      ldC.u agY, [sp+Is18]; (it now allows aY as destination);
//      stC [sp+Is18], agY; (it now allows aY as source);
//      stC.u [sp+Is18], agY; (it now allows aY as source);
//      sthC [sp]+Is18, gY;
//      sthC [sp+Is18], gY;
//      sthC.u [sp+Is18], gY;
//      ldhC gY, [sp]+Is18;
//      ldhC.s gY, [sp]+Is18;
//      ldhC gY, [sp+Is18];
//      ldhC.s gY, [sp+Is18];
//      ldhC.u gY, [sp+Is18];
//      ldhC.u.s gY, [sp+Is18];
//      add aU, sp, Is19;
//    - opS:
//      ld.x2 gZ, [agX+/-agY];
//      ld.u.x2 gZ, [agX+/-agY];
//      ld.x2 gZ, [agX]+/-agY;
//      st.x2 [agX+/-agY], gZ;
//      st.u.x2 [agX+/-agY], gZ;
//      st.x2 [agX]+/-agY, gZ.
// 1.114
//  - Fix the check for an undefined symbol to take into account the symbols
// included in complex expressions (ENGR00326679).
// 1.113
//  - Fix VSPA1onVSPA2 translation rule for "pop gX" (ENGR00326887).
//  - Update VCPU2 ADL, which adds missing "instr_jmp_jsr" attributes for some
// "jmp/jsr" instructions. This allows preventing compaction in the delay slot
// of those "jmp/jsr" instructions (ENGR00326885).
// 1.112
//  - Reject immediate instruction operands that include garbage characters at
// the end.
//  - Update VCPU1 ADL, which allows relocation for "set.range aX, asY, Iu17"
// and VCPU2 ADL, which allows relocation for "set.range aX, agY, Iu19"
// (ENGR00326679).
// 1.111
//  - Update VCPU1, VCPU2 ADL, which contains fixes for the range of the
// "xtrm_n" operand of the "set.xtrm" instruction (ENGR00326325).
//  - VCPU1 ADL contains new definitions for pre/post-increment immediates that
//  may be expressed as mutiple of line size. These changes are reflected in
// the checker used to validate the values of the immediates.
// 1.110
//  - Update the VCPU2 ADL, which contains the following changes:
//    - Allow "sp" as operand for opD "add/and/sub" instructions
//    (ENGR00326181);
//    - Remove "rsub.s sp, Is16" variation of the "rsub.s gX, Is16"
//    instruction (ENGR00326180);
//    - Update encoding for "wr.even" (ENGR00326177);
//    - Change "set.xtrm" operand names from "cmp_all/cmp_even" to "all/even"
//    (ENGR00325993).
//  - Update VSPA1onVSPA2 translation rule for "set.xtrm".
//  - Fix Linux build VSPA1onVSPA translation issue by updating the version of
//  the GCC used to build the assembler to 4.9.1 (ENGR00326148).
// 1.109
//  - Generate error when encountering relocation expression operands for
// fields that don't allow relocations (ENGR00317012).
//  - Clean-up code that generate relocations when building instructions to
//  remove the search for the type of relocation as part of the symbol.
// 1.108
//  - Fix VS2008 solution and projects.
// 1.107
//  - Add check for instruction operand expressions to reject those that
// include symbols with keyword names such as register or mode names
// (ENGR00316900).
//  - Remove unordered_map and unordered_set definitions needed by older GCC
// versions for the Linux build.
// 1.106
//  - The alignment of a section is no longer taken into consideration when
// computing its offset and size (ENGR00324546, ENGR00324547).
// 1.105
//  - Update VCPU2 ADL, which removes "and(S)(.z)/or(S)/xor(S) gX, gX, Iu16"
// aliases.
// 1.104
//  - VSPA2 Linux build fix.
// 1.103
//  - Update VCPU1 ADL, which includes fixes for "mv gX, sp" and "mv sp, gX"
//  opS/opB instantiation information (ENGR00324148).
//  - Fix local symbol check when applying ".MW_INFO" section symbol index
//  fixups (related to ENGR00324183).
//  - Enhance VSPA1onVSPA2 translator to allow translation rules that apply
// only to a specific operand, identified by its position. This allows defining
// translation rules that convert VSPA1 long forms of instructions such as
// "add gX, gX, I16" to the corresponding short forms (ENGR00321568,
// ENGR00323655). Update VSPA1onVSPA2 translator to use C++11 regular
// expressions, which allow negative lookahead.
// 1.102
//  - Update VCPU1 ADL, which includes "ovsf_dl_sc_y" and "ovsf_dl_sc_y_bak"
//  CGU registers (ENGR00324106).
// 1.101
//  - Fix ".MW_INFO" symbol handling to allow local symbols (ENGR00323679).
//  - Enable sorting for all the opcodes of a mnemonic. This is enforced by the
//  keyword checking that prevents parsing registers as symbols.
