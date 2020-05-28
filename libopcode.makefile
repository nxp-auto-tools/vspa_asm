CPP_COMPILER = g++
C_COMPILER = gcc

INCLUDE_PATH=-I"./external/binutils-adl/libiberty" -I"./external/binutils-adl/include" -I"./external/binutils-adl/win-build/bfd" -I"./external/binutils-adl/win-headers" -I"./external/binutils-adl/win-build/opcodes" 

ifeq ($(ARCH_TARGET), vspa3)
VSPA_VERSION=-D_VSPA3_
else
VSPA_VERSION=
endif

PREPROCESSOR_DEFINITIONS=$(VSPA_VERSION) -DHAVE_CONFIG_H -DGCC_BUILD
COMPILER_FLAGS=
 
OBJECTS = $(OUTPUT_DIR)/libopcode/opcode/dis-buf.o $(OUTPUT_DIR)/libopcode/opcode/dis-init.o $(OUTPUT_DIR)/libopcode/opcode/disassemble.o $(OUTPUT_DIR)/libopcode/opcode/ppc-support.o 


all: create_folder $(OUTPUT_DIR)/libopcode/libopcode.a

clean: 
	rm -rf $(OUTPUT_DIR)/libopcode

$(OUTPUT_DIR)/libopcode/libopcode.a: $(OBJECTS)
	ar rcs $@ $^

$(OUTPUT_DIR)/libopcode/opcode/dis-buf.o: external/binutils-adl/opcodes/dis-buf.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libopcode/opcode/dis-init.o: external/binutils-adl/opcodes/dis-init.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINFITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libopcode/opcode/disassemble.o: external/binutils-adl/opcodes/disassemble.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libopcode/opcode/ppc-support.o: external/binutils-adl/opcodes/ppc-support.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

create_folder:
	mkdir -p $(OUTPUT_DIR)/libopcode/opcode
