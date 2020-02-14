CPP_COMPILER = g++
C_COMPILER = gcc

ifeq ($(ARCH_TARGET), vspa2)
VSPA_VERSION=-D_VSPA2_
VCPU_FILE=as-vcpu2_spec
IPPU_FILE=as-ippu2_spec
OUTPUT_NAME=vspa2-as
OUTPUT_FOLDER=vspa2
else
VSPA_VERSION=
VCPU_FILE=as-vcpu1_spec
IPPU_FILE=as-ippu1_spec
OUTPUT_NAME=vspa-as
OUTPUT_FOLDER=vspa
endif

INCLUDE_PATH=-I"." -I"./external/binutils-adl/gas" -I"./external/binutils-adl/gas/config" -I"./external/binutils-adl/win-build/gas" -I"./external/binutils-adl/opcodes" -I"./external/binutils-adl/win-build/opcodes" -I"./external/binutils-adl/bfd" -I"./external/binutils-adl/win-build/bfd" -I"./external/binutils-adl/include" -I"./external/binutils-adl/" #-I"./external/binutils-adl/win-headers"

LIBRARY_PATH=-L"./bin/$(OUTPUT_FOLDER)/libiberty" -L"./bin/$(OUTPUT_FOLDER)/libgas" -L"./bin/$(OUTPUT_FOLDER)/libopcode" -L"./bin/$(OUTPUT_FOLDER)/libbfd"
LIRARIES=-Wl,--start-group -lgas -liberty -lbfd -lopcode  -Wl,--end-group 




PREPROCESSOR_DEFINITIONS=-DGCC_BUILD -DNDEBUG -D_CONSOLE $(VSPA_VERSION)
COMPILER_FLAGS=


all: create_folder bin/$(OUTPUT_FOLDER)/$(OUTPUT_NAME)

clean:
	rm -rf bin

bin/$(OUTPUT_FOLDER)/$(OUTPUT_NAME): bin/$(OUTPUT_FOLDER)/assembler/$(VCPU_FILE).o bin/$(OUTPUT_FOLDER)/assembler/$(IPPU_FILE).o bin/$(OUTPUT_FOLDER)/libgas/libgas.a bin/$(OUTPUT_FOLDER)/libiberty/libiberty.a bin/$(OUTPUT_FOLDER)/libopcode/libopcode.a bin/$(OUTPUT_FOLDER)/libbfd/libbfd.a
	$(CPP_COMPILER) $^ $(LIBRARY_PATH) $(LIRARIES) $(COMPILER_FLAGS) -static -Wl,-rpath,./ -o $@

bin/$(OUTPUT_FOLDER)/assembler/as-vcpu1_spec.o: assembler/as-vcpu1_spec.cc
	$(CPP_COMPILER) $(PREPROCESSOR_DEFINITIONS) -c $< $(INCLUDE_PATH) $(COMPILER_FLAGS) -o $@

bin/$(OUTPUT_FOLDER)/assembler/as-vcpu2_spec.o: assembler/as-vcpu2_spec.cc
	$(CPP_COMPILER) $(PREPROCESSOR_DEFINITIONS) -c $< $(INCLUDE_PATH) $(COMPILER_FLAGS) -o $@

bin/$(OUTPUT_FOLDER)/assembler/as-ippu1_spec.o: assembler/as-ippu1_spec.cc
	$(CPP_COMPILER) $(PREPROCESSOR_DEFINITIONS) -c $< $(INCLUDE_PATH) $(COMPILER_FLAGS) -o $@

bin/$(OUTPUT_FOLDER)/assembler/as-ippu2_spec.o: assembler/as-ippu2_spec.cc
	$(CPP_COMPILER) $(PREPROCESSOR_DEFINITIONS) -c $< $(INCLUDE_PATH) $(COMPILER_FLAGS) -o $@

bin/$(OUTPUT_FOLDER)/libgas/libgas.a:
	make --directory="." --file=libgas.makefile OUTPUT_DIR=bin/$(OUTPUT_FOLDER)/ ARCH_TARGET=$(ARCH_TARGET)

bin/$(OUTPUT_FOLDER)/libopcode/libopcode.a:
	make --directory="." --file=libopcode.makefile OUTPUT_DIR=bin/$(OUTPUT_FOLDER)/ ARCH_TARGET=$(ARCH_TARGET)

bin/$(OUTPUT_FOLDER)/libbfd/libbfd.a:
	make --directory="." --file=libbfd.makefile OUTPUT_DIR=bin/$(OUTPUT_FOLDER)/ ARCH_TARGET=$(ARCH_TARGET)

bin/$(OUTPUT_FOLDER)/libiberty/libiberty.a:
	make --directory="." --file=libiberty.makefile  OUTPUT_DIR=bin/$(OUTPUT_FOLDER)/ ARCH_TARGET=$(ARCH_TARGET)

create_folder:
	mkdir -p bin/$(OUTPUT_FOLDER)/assembler
