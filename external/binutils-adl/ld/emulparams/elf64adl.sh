. ${srcdir}/emulparams/elf32adlcommon.sh
ELFSIZE=64
OUTPUT_FORMAT="elf64-adl"
TEXT_START_ADDR=0x1000
#SEGMENT_SIZE=0x10000000
ARCH=adl:common64
unset EXECUTABLE_SYMBOLS
unset SDATA_START_SYMBOLS
unset SDATA2_START_SYMBOLS
unset SBSS_START_SYMBOLS
unset SBSS_END_SYMBOLS
unset OTHER_END_SYMBOLS
unset OTHER_RELRO_SECTIONS
BSS_PLT=
