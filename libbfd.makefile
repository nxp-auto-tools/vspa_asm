CPP_COMPILER = g++
C_COMPILER = gcc

INCLUDE_PATH=-I"./external/binutils-adl/include" -I"./external/binutils-adl/win-build/bfd" #-I"./external/binutils-adl/win-headers"  

ifeq ($(ARCH_TARGET), vspa3)
VSPA_VERSION=-D_VSPA3_
else
VSPA_VERSION=
endif

PREPROCESSOR_DEFINITIONS=$(VSPA_VERSION) -DHAVE_adl_elf64_vec -DHAVE_adl_elf64_le_vec -DHAVE_adl_elf32_vec -DHAVE_adl_elf32_le_vec -DHAVE_elf64_le_vec -DHAVE_elf64_be_vec -DHAVE_elf32_le_vec -DHAVE_elf32_be_vec -DHAVE_CONFIG_H -DDEBUGDIR=\"C:\\Debug\" -DDEFAULT_VECTOR=adl_elf64_vec -DSELECT_VECS="&adl_elf64_vec,&adl_elf64_le_vec,&adl_elf32_vec,&adl_elf32_le_vec,&elf64_le_vec,&elf64_be_vec,&elf32_le_vec,&elf32_be_vec" -DSELECT_ARCHITECTURES="&bfd_rs6000_arch,&bfd_powerpc_arch" -DGCC_BUILD #-D__USE_MINGW_FSEEK
COMPILER_FLAGS=
 
OBJECTS = $(OUTPUT_DIR)/libbfd/bfd/aix5ppc-core.o $(OUTPUT_DIR)/libbfd/bfd/archive.o $(OUTPUT_DIR)/libbfd/bfd/archive64.o $(OUTPUT_DIR)/libbfd/bfd/archures.o $(OUTPUT_DIR)/libbfd/bfd/bfd.o $(OUTPUT_DIR)/libbfd/bfd/bfdio.o $(OUTPUT_DIR)/libbfd/bfd/bfdwin.o $(OUTPUT_DIR)/libbfd/bfd/binary.o $(OUTPUT_DIR)/libbfd/bfd/cache.o $(OUTPUT_DIR)/libbfd/bfd/coff-rs6000.o $(OUTPUT_DIR)/libbfd/bfd/coff64-rs6000.o $(OUTPUT_DIR)/libbfd/bfd/coffgen.o $(OUTPUT_DIR)/libbfd/bfd/compress.o $(OUTPUT_DIR)/libbfd/bfd/corefile.o $(OUTPUT_DIR)/libbfd/bfd/cpu-powerpc.o $(OUTPUT_DIR)/libbfd/bfd/cpu-rs6000.o $(OUTPUT_DIR)/libbfd/bfd/dwarf1.o $(OUTPUT_DIR)/libbfd/bfd/dwarf2.o $(OUTPUT_DIR)/libbfd/bfd/elf-attrs.o $(OUTPUT_DIR)/libbfd/bfd/elf-eh-frame.o $(OUTPUT_DIR)/libbfd/bfd/elf-strtab.o $(OUTPUT_DIR)/libbfd/bfd/elf-vxworks.o $(OUTPUT_DIR)/libbfd/bfd/elf.o $(OUTPUT_DIR)/libbfd/bfd/elf32-gen.o $(OUTPUT_DIR)/libbfd/bfd/elf32-adl.o $(OUTPUT_DIR)/libbfd/bfd/elf32.o $(OUTPUT_DIR)/libbfd/bfd/elf64-gen.o $(OUTPUT_DIR)/libbfd/bfd/elf64-adl.o $(OUTPUT_DIR)/libbfd/bfd/elf64.o $(OUTPUT_DIR)/libbfd/bfd/elflink.o $(OUTPUT_DIR)/libbfd/bfd/format.o $(OUTPUT_DIR)/libbfd/bfd/hash.o $(OUTPUT_DIR)/libbfd/bfd/ihex.o $(OUTPUT_DIR)/libbfd/bfd/init.o $(OUTPUT_DIR)/libbfd/bfd/libbfd.o $(OUTPUT_DIR)/libbfd/bfd/linker.o $(OUTPUT_DIR)/libbfd/bfd/merge.o $(OUTPUT_DIR)/libbfd/bfd/opncls.o $(OUTPUT_DIR)/libbfd/bfd/reloc.o $(OUTPUT_DIR)/libbfd/bfd/section.o $(OUTPUT_DIR)/libbfd/bfd/simple.o $(OUTPUT_DIR)/libbfd/bfd/srec.o $(OUTPUT_DIR)/libbfd/bfd/stab-syms.o $(OUTPUT_DIR)/libbfd/bfd/stabs.o $(OUTPUT_DIR)/libbfd/bfd/syms.o $(OUTPUT_DIR)/libbfd/bfd/targets.o $(OUTPUT_DIR)/libbfd/bfd/tekhex.o $(OUTPUT_DIR)/libbfd/bfd/verilog.o $(OUTPUT_DIR)/libbfd/bfd/xcofflink.o 


all: create_folder $(OUTPUT_DIR)/libbfd/libbfd.a

clean: 
	rm -rf $(OUTPUT_DIR)/libbfd

$(OUTPUT_DIR)/libbfd/libbfd.a: $(OBJECTS)
	ar rcs $@ $^

$(OUTPUT_DIR)/libbfd/bfd/aix5ppc-core.o: external/binutils-adl/bfd/aix5ppc-core.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/archive.o: external/binutils-adl/bfd/archive.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/archive64.o: external/binutils-adl/bfd/archive64.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/archures.o: external/binutils-adl/bfd/archures.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/bfd.o: external/binutils-adl/bfd/bfd.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/bfdio.o: external/binutils-adl/bfd/bfdio.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/bfdwin.o: external/binutils-adl/bfd/bfdwin.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/binary.o: external/binutils-adl/bfd/binary.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/cache.o: external/binutils-adl/bfd/cache.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/coff-rs6000.o: external/binutils-adl/bfd/coff-rs6000.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/coff64-rs6000.o: external/binutils-adl/bfd/coff64-rs6000.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/coffgen.o: external/binutils-adl/bfd/coffgen.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/compress.o: external/binutils-adl/bfd/compress.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/corefile.o: external/binutils-adl/bfd/corefile.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/cpu-powerpc.o: external/binutils-adl/bfd/cpu-powerpc.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/cpu-rs6000.o: external/binutils-adl/bfd/cpu-rs6000.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/dwarf1.o: external/binutils-adl/bfd/dwarf1.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/dwarf2.o: external/binutils-adl/bfd/dwarf2.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/elf-attrs.o: external/binutils-adl/bfd/elf-attrs.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/elf-eh-frame.o: external/binutils-adl/bfd/elf-eh-frame.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/elf-strtab.o: external/binutils-adl/bfd/elf-strtab.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/elf-vxworks.o: external/binutils-adl/bfd/elf-vxworks.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/elf.o: external/binutils-adl/bfd/elf.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/elf32-gen.o: external/binutils-adl/bfd/elf32-gen.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/elf32-adl.o: external/binutils-adl/bfd/elf32-adl.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/elf32.o: external/binutils-adl/bfd/elf32.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/elf64-gen.o: external/binutils-adl/bfd/elf64-gen.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/elf64-adl.o: external/binutils-adl/bfd/elf64-adl.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/elf64.o: external/binutils-adl/bfd/elf64.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/elflink.o: external/binutils-adl/bfd/elflink.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/format.o: external/binutils-adl/bfd/format.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/hash.o: external/binutils-adl/bfd/hash.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/ihex.o: external/binutils-adl/bfd/ihex.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/init.o: external/binutils-adl/bfd/init.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/libbfd.o: external/binutils-adl/bfd/libbfd.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/linker.o: external/binutils-adl/bfd/linker.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/merge.o: external/binutils-adl/bfd/merge.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/opncls.o: external/binutils-adl/bfd/opncls.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/reloc.o: external/binutils-adl/bfd/reloc.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/section.o: external/binutils-adl/bfd/section.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/simple.o: external/binutils-adl/bfd/simple.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/srec.o: external/binutils-adl/bfd/srec.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/stab-syms.o: external/binutils-adl/bfd/stab-syms.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/stabs.o: external/binutils-adl/bfd/stabs.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/syms.o: external/binutils-adl/bfd/syms.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/targets.o: external/binutils-adl/bfd/targets.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/tekhex.o: external/binutils-adl/bfd/tekhex.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/verilog.o: external/binutils-adl/bfd/verilog.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libbfd/bfd/xcofflink.o: external/binutils-adl/bfd/xcofflink.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

create_folder:
	mkdir -p $(OUTPUT_DIR)/libbfd/bfd
