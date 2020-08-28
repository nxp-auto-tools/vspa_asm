CPP_COMPILER = g++
C_COMPILER = gcc

INCLUDE_PATH=-I"./external/binutils-adl/gas" -I"./external/binutils-adl/gas/config" -I"./external/binutils-adl/win-build/gas" -I"./external/binutils-adl/" -I"./external/binutils-adl/include" -I"./external/binutils-adl/bfd" -I"./external/binutils-adl/win-build/bfd" -I"../enterprise_tools/target" #-I"./external/binutils-adl/win-headers"

ifeq ($(ARCH_TARGET), vspa3)
VSPA_VERSION=-D_VSPA3_
else
VSPA_VERSION=
endif

PREPROCESSOR_DEFINITIONS=$(VSPA_VERSION) -DHAVE_CONFIG_H -DGCC_BUILD 
COMPILER_FLAGS=

ifeq ($(ARCH_TARGET), vspa2)
OBJECTS = $(OUTPUT_DIR)/libgas/gas/adl-asm-common.o $(OUTPUT_DIR)/libgas/gas/adl-asm-impl-gas.o $(OUTPUT_DIR)/libgas/gas/adl-asm-impl.o $(OUTPUT_DIR)/libgas/gas/app.o $(OUTPUT_DIR)/libgas/gas/as.o $(OUTPUT_DIR)/libgas/gas/atof-generic.o $(OUTPUT_DIR)/libgas/gas/atof-ieee.o $(OUTPUT_DIR)/libgas/gas/compress-debug.o $(OUTPUT_DIR)/libgas/gas/cond.o $(OUTPUT_DIR)/libgas/gas/depend.o $(OUTPUT_DIR)/libgas/gas/dw2gencfi.o $(OUTPUT_DIR)/libgas/gas/dwarf2dbg.o $(OUTPUT_DIR)/libgas/gas/ecoff.o $(OUTPUT_DIR)/libgas/gas/ehopt.o $(OUTPUT_DIR)/libgas/gas/expr.o $(OUTPUT_DIR)/libgas/gas/flonum-copy.o $(OUTPUT_DIR)/libgas/gas/flonum-konst.o $(OUTPUT_DIR)/libgas/gas/flonum-mult.o $(OUTPUT_DIR)/libgas/gas/frags.o $(OUTPUT_DIR)/libgas/gas/hash.o $(OUTPUT_DIR)/libgas/gas/input-file.o $(OUTPUT_DIR)/libgas/gas/input-scrub.o $(OUTPUT_DIR)/libgas/gas/listing.o $(OUTPUT_DIR)/libgas/gas/literal.o $(OUTPUT_DIR)/libgas/gas/macro.o $(OUTPUT_DIR)/libgas/gas/messages.o $(OUTPUT_DIR)/libgas/gas/obj-elf.o $(OUTPUT_DIR)/libgas/gas/output-file.o $(OUTPUT_DIR)/libgas/gas/ppc-support.o $(OUTPUT_DIR)/libgas/gas/read.o $(OUTPUT_DIR)/libgas/gas/remap.o $(OUTPUT_DIR)/libgas/gas/sb.o $(OUTPUT_DIR)/libgas/gas/stabs.o $(OUTPUT_DIR)/libgas/gas/subsegs.o $(OUTPUT_DIR)/libgas/gas/symbols.o $(OUTPUT_DIR)/libgas/gas/write.o $(OUTPUT_DIR)/libgas/gas/mw-info.o $(OUTPUT_DIR)/libgas/gas/asm-translate.o
else
OBJECTS = $(OUTPUT_DIR)/libgas/gas/adl-asm-common.o $(OUTPUT_DIR)/libgas/gas/adl-asm-impl-gas.o $(OUTPUT_DIR)/libgas/gas/adl-asm-impl.o $(OUTPUT_DIR)/libgas/gas/app.o $(OUTPUT_DIR)/libgas/gas/as.o $(OUTPUT_DIR)/libgas/gas/atof-generic.o $(OUTPUT_DIR)/libgas/gas/atof-ieee.o $(OUTPUT_DIR)/libgas/gas/compress-debug.o $(OUTPUT_DIR)/libgas/gas/cond.o $(OUTPUT_DIR)/libgas/gas/depend.o $(OUTPUT_DIR)/libgas/gas/dw2gencfi.o $(OUTPUT_DIR)/libgas/gas/dwarf2dbg.o $(OUTPUT_DIR)/libgas/gas/ecoff.o $(OUTPUT_DIR)/libgas/gas/ehopt.o $(OUTPUT_DIR)/libgas/gas/expr.o $(OUTPUT_DIR)/libgas/gas/flonum-copy.o $(OUTPUT_DIR)/libgas/gas/flonum-konst.o $(OUTPUT_DIR)/libgas/gas/flonum-mult.o $(OUTPUT_DIR)/libgas/gas/frags.o $(OUTPUT_DIR)/libgas/gas/hash.o $(OUTPUT_DIR)/libgas/gas/input-file.o $(OUTPUT_DIR)/libgas/gas/input-scrub.o $(OUTPUT_DIR)/libgas/gas/listing.o $(OUTPUT_DIR)/libgas/gas/literal.o $(OUTPUT_DIR)/libgas/gas/macro.o $(OUTPUT_DIR)/libgas/gas/messages.o $(OUTPUT_DIR)/libgas/gas/obj-elf.o $(OUTPUT_DIR)/libgas/gas/output-file.o $(OUTPUT_DIR)/libgas/gas/ppc-support.o $(OUTPUT_DIR)/libgas/gas/read.o $(OUTPUT_DIR)/libgas/gas/remap.o $(OUTPUT_DIR)/libgas/gas/sb.o $(OUTPUT_DIR)/libgas/gas/stabs.o $(OUTPUT_DIR)/libgas/gas/subsegs.o $(OUTPUT_DIR)/libgas/gas/symbols.o $(OUTPUT_DIR)/libgas/gas/write.o $(OUTPUT_DIR)/libgas/gas/mw-info.o  $(OUTPUT_DIR)/libgas/gas/target_ISA.o
endif

all: create_folder $(OUTPUT_DIR)/libgas/libgas.a

clean:
	rm -rf $(OUTPUT_DIR)/libgas

$(OUTPUT_DIR)/libgas/libgas.a: $(OBJECTS)
	ar rcs $@ $^
  
$(OUTPUT_DIR)/libgas/gas/target_ISA.o: ../enterprise_tools/target/target_ISA.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/asm-translate.o: external/binutils-adl/gas/asm-translate.C
	$(CPP_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/adl-asm-common.o: external/binutils-adl/gas/adl-asm-common.C
	$(CPP_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/adl-asm-impl-gas.o: external/binutils-adl/gas/adl-asm-impl-gas.C
	$(CPP_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/adl-asm-impl.o: external/binutils-adl/gas/adl-asm-impl.C
	$(CPP_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/app.o: external/binutils-adl/gas/app.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/as.o: external/binutils-adl/gas/as.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/atof-generic.o: external/binutils-adl/gas/atof-generic.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/atof-ieee.o: external/binutils-adl/gas/config/atof-ieee.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/compress-debug.o: external/binutils-adl/gas/compress-debug.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/cond.o: external/binutils-adl/gas/cond.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/depend.o: external/binutils-adl/gas/depend.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/dw2gencfi.o: external/binutils-adl/gas/dw2gencfi.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/dwarf2dbg.o: external/binutils-adl/gas/dwarf2dbg.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/ecoff.o: external/binutils-adl/gas/ecoff.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/ehopt.o: external/binutils-adl/gas/ehopt.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/expr.o: external/binutils-adl/gas/expr.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/flonum-copy.o: external/binutils-adl/gas/flonum-copy.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)


$(OUTPUT_DIR)/libgas/gas/flonum-konst.o: external/binutils-adl/gas/flonum-konst.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/flonum-mult.o: external/binutils-adl/gas/flonum-mult.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/frags.o: external/binutils-adl/gas/frags.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/hash.o: external/binutils-adl/gas/hash.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/input-file.o: external/binutils-adl/gas/input-file.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/input-scrub.o: external/binutils-adl/gas/input-scrub.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/listing.o: external/binutils-adl/gas/listing.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/literal.o: external/binutils-adl/gas/literal.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/macro.o: external/binutils-adl/gas/macro.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/messages.o: external/binutils-adl/gas/messages.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/obj-elf.o: external/binutils-adl/gas/config/obj-elf.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/output-file.o: external/binutils-adl/gas/output-file.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/ppc-support.o: external/binutils-adl/gas/config/ppc-support.C
	$(CPP_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/read.o: external/binutils-adl/gas/read.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/remap.o: external/binutils-adl/gas/remap.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/sb.o: external/binutils-adl/gas/sb.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/stabs.o: external/binutils-adl/gas/stabs.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/subsegs.o: external/binutils-adl/gas/subsegs.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/symbols.o: external/binutils-adl/gas/symbols.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/write.o: external/binutils-adl/gas/write.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libgas/gas/mw-info.o: external/binutils-adl/gas/mw-info.C
	$(CPP_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

create_folder:
	mkdir -p $(OUTPUT_DIR)/libgas/gas
