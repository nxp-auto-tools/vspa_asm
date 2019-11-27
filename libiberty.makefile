CPP_COMPILER = g++
C_COMPILER = gcc

INCLUDE_PATH=-I"./external/binutils-adl/libiberty" -I"./external/binutils-adl/include" -I"./external/binutils-adl/win-build/libiberty" #-I"./external/binutils-adl/win-headers" 

ifeq ($(ARCH_TARGET), vspa2)
VSPA_VERSION=-D_VSPA2_
else
VSPA_VERSION=
endif

PREPROCESSOR_DEFINITIONS=$(VSPA_VERSION) -DGCC_BUILD -DHAVE_CONFIG_H -DHAVE_PSIGNAL
COMPILER_FLAGS=

OBJECTS=$(OUTPUT_DIR)/libiberty/iberty/alloca.o $(OUTPUT_DIR)/libiberty/iberty/argv.o $(OUTPUT_DIR)/libiberty/iberty/asprintf.o $(OUTPUT_DIR)/libiberty/iberty/bcmp.o $(OUTPUT_DIR)/libiberty/iberty/bcopy.o $(OUTPUT_DIR)/libiberty/iberty/bzero.o $(OUTPUT_DIR)/libiberty/iberty/choose-temp.o $(OUTPUT_DIR)/libiberty/iberty/concat.o $(OUTPUT_DIR)/libiberty/iberty/cp-demangle.o $(OUTPUT_DIR)/libiberty/iberty/cp-demint.o $(OUTPUT_DIR)/libiberty/iberty/cplus-dem.o $(OUTPUT_DIR)/libiberty/iberty/dyn-string.o $(OUTPUT_DIR)/libiberty/iberty/fdmatch.o $(OUTPUT_DIR)/libiberty/iberty/ffs.o $(OUTPUT_DIR)/libiberty/iberty/fibheap.o $(OUTPUT_DIR)/libiberty/iberty/floatformat.o $(OUTPUT_DIR)/libiberty/iberty/fnmatch.o $(OUTPUT_DIR)/libiberty/iberty/fopen_unlocked.o $(OUTPUT_DIR)/libiberty/iberty/getopt.o $(OUTPUT_DIR)/libiberty/iberty/getopt1.o $(OUTPUT_DIR)/libiberty/iberty/getpwd.o $(OUTPUT_DIR)/libiberty/iberty/getruntime.o $(OUTPUT_DIR)/libiberty/iberty/hashtab.o $(OUTPUT_DIR)/libiberty/iberty/hex.o $(OUTPUT_DIR)/libiberty/iberty/lbasename.o $(OUTPUT_DIR)/libiberty/iberty/lrealpath.o $(OUTPUT_DIR)/libiberty/iberty/make-relative-prefix.o $(OUTPUT_DIR)/libiberty/iberty/make-temp-file.o $(OUTPUT_DIR)/libiberty/iberty/md5.o $(OUTPUT_DIR)/libiberty/iberty/mempcpy.o $(OUTPUT_DIR)/libiberty/iberty/mkstemps.o $(OUTPUT_DIR)/libiberty/iberty/objalloc.o $(OUTPUT_DIR)/libiberty/iberty/obstack.o $(OUTPUT_DIR)/libiberty/iberty/partition.o $(OUTPUT_DIR)/libiberty/iberty/pex-common.o $(OUTPUT_DIR)/libiberty/iberty/pex-one.o  $(OUTPUT_DIR)/libiberty/iberty/pexecute.o $(OUTPUT_DIR)/libiberty/iberty/physmem.o $(OUTPUT_DIR)/libiberty/iberty/random.o $(OUTPUT_DIR)/libiberty/iberty/regex.o $(OUTPUT_DIR)/libiberty/iberty/rindex.o $(OUTPUT_DIR)/libiberty/iberty/safe-ctype.o $(OUTPUT_DIR)/libiberty/iberty/setenv.o $(OUTPUT_DIR)/libiberty/iberty/sigsetmask.o $(OUTPUT_DIR)/libiberty/iberty/snprintf.o $(OUTPUT_DIR)/libiberty/iberty/sort.o $(OUTPUT_DIR)/libiberty/iberty/spaces.o $(OUTPUT_DIR)/libiberty/iberty/splay-tree.o $(OUTPUT_DIR)/libiberty/iberty/stpncpy.o $(OUTPUT_DIR)/libiberty/iberty/strcasecmp.o $(OUTPUT_DIR)/libiberty/iberty/strerror.o $(OUTPUT_DIR)/libiberty/iberty/strncasecmp.o $(OUTPUT_DIR)/libiberty/iberty/strndup.o $(OUTPUT_DIR)/libiberty/iberty/strsignal.o $(OUTPUT_DIR)/libiberty/iberty/strverscmp.o $(OUTPUT_DIR)/libiberty/iberty/unlink-if-ordinary.o $(OUTPUT_DIR)/libiberty/iberty/vasprintf.o $(OUTPUT_DIR)/libiberty/iberty/vfork.o $(OUTPUT_DIR)/libiberty/iberty/waitpid.o $(OUTPUT_DIR)/libiberty/iberty/xatexit.o $(OUTPUT_DIR)/libiberty/iberty/xexit.o $(OUTPUT_DIR)/libiberty/iberty/xmalloc.o $(OUTPUT_DIR)/libiberty/iberty/xmemdup.o $(OUTPUT_DIR)/libiberty/iberty/xstrdup.o $(OUTPUT_DIR)/libiberty/iberty/xstrerror.o $(OUTPUT_DIR)/libiberty/iberty/xstrndup.o # $(OUTPUT_DIR)/libiberty/iberty/pex-win32.o
	
all: create_folder $(OUTPUT_DIR)/libiberty/libiberty.a

clean: 
	rm -rf $(OUTPUT_DIR)/libiberty

$(OUTPUT_DIR)/libiberty/libiberty.a: $(OBJECTS)
	ar rcs $@ $^

$(OUTPUT_DIR)/libiberty/iberty/alloca.o: external/binutils-adl/libiberty/alloca.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/argv.o: external/binutils-adl/libiberty/argv.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/asprintf.o: external/binutils-adl/libiberty/asprintf.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/bcmp.o: external/binutils-adl/libiberty/bcmp.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/bcopy.o: external/binutils-adl/libiberty/bcopy.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/bzero.o: external/binutils-adl/libiberty/bzero.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/choose-temp.o: external/binutils-adl/libiberty/choose-temp.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/concat.o: external/binutils-adl/libiberty/concat.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/cp-demangle.o: external/binutils-adl/libiberty/cp-demangle.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/cp-demint.o: external/binutils-adl/libiberty/cp-demint.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/cplus-dem.o: external/binutils-adl/libiberty/cplus-dem.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/dyn-string.o: external/binutils-adl/libiberty/dyn-string.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/fdmatch.o: external/binutils-adl/libiberty/fdmatch.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/ffs.o: external/binutils-adl/libiberty/ffs.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/fibheap.o: external/binutils-adl/libiberty/fibheap.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/floatformat.o: external/binutils-adl/libiberty/floatformat.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/fnmatch.o: external/binutils-adl/libiberty/fnmatch.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/fopen_unlocked.o: external/binutils-adl/libiberty/fopen_unlocked.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/getopt.o: external/binutils-adl/libiberty/getopt.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/getopt1.o: external/binutils-adl/libiberty/getopt1.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/getpwd.o: external/binutils-adl/libiberty/getpwd.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/getruntime.o: external/binutils-adl/libiberty/getruntime.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/hashtab.o: external/binutils-adl/libiberty/hashtab.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/hex.o: external/binutils-adl/libiberty/hex.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/lbasename.o: external/binutils-adl/libiberty/lbasename.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/lrealpath.o: external/binutils-adl/libiberty/lrealpath.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/make-relative-prefix.o: external/binutils-adl/libiberty/make-relative-prefix.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/make-temp-file.o: external/binutils-adl/libiberty/make-temp-file.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/md5.o: external/binutils-adl/libiberty/md5.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/mempcpy.o: external/binutils-adl/libiberty/mempcpy.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/mkstemps.o: external/binutils-adl/libiberty/mkstemps.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/objalloc.o: external/binutils-adl/libiberty/objalloc.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/obstack.o: external/binutils-adl/libiberty/obstack.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/partition.o: external/binutils-adl/libiberty/partition.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/pex-common.o: external/binutils-adl/libiberty/pex-common.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/pex-one.o: external/binutils-adl/libiberty/pex-one.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/pex-win32.o: external/binutils-adl/libiberty/pex-win32.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/pexecute.o: external/binutils-adl/libiberty/pexecute.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/physmem.o: external/binutils-adl/libiberty/physmem.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/random.o: external/binutils-adl/libiberty/random.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/regex.o: external/binutils-adl/libiberty/regex.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/rindex.o: external/binutils-adl/libiberty/rindex.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/safe-ctype.o: external/binutils-adl/libiberty/safe-ctype.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/setenv.o: external/binutils-adl/libiberty/setenv.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/sigsetmask.o: external/binutils-adl/libiberty/sigsetmask.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/snprintf.o: external/binutils-adl/libiberty/snprintf.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/sort.o: external/binutils-adl/libiberty/sort.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/spaces.o: external/binutils-adl/libiberty/spaces.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/splay-tree.o: external/binutils-adl/libiberty/splay-tree.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/stpncpy.o: external/binutils-adl/libiberty/stpncpy.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/strcasecmp.o: external/binutils-adl/libiberty/strcasecmp.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/strerror.o: external/binutils-adl/libiberty/strerror.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/strncasecmp.o: external/binutils-adl/libiberty/strncasecmp.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/strndup.o: external/binutils-adl/libiberty/strndup.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/strsignal.o: external/binutils-adl/libiberty/strsignal.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/strverscmp.o: external/binutils-adl/libiberty/strverscmp.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/unlink-if-ordinary.o: external/binutils-adl/libiberty/unlink-if-ordinary.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/vasprintf.o: external/binutils-adl/libiberty/vasprintf.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/vfork.o: external/binutils-adl/libiberty/vfork.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/waitpid.o: external/binutils-adl/libiberty/waitpid.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/xatexit.o: external/binutils-adl/libiberty/xatexit.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/xexit.o: external/binutils-adl/libiberty/xexit.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/xmalloc.o: external/binutils-adl/libiberty/xmalloc.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/xmemdup.o: external/binutils-adl/libiberty/xmemdup.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/xstrdup.o: external/binutils-adl/libiberty/xstrdup.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/xstrerror.o: external/binutils-adl/libiberty/xstrerror.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

$(OUTPUT_DIR)/libiberty/iberty/xstrndup.o: external/binutils-adl/libiberty/xstrndup.c
	$(C_COMPILER) $< -c -o $@ $(INCLUDE_PATH) $(PREPROCESSOR_DEFINITIONS) $(COMPILER_FLAGS)

create_folder:
	mkdir -p $(OUTPUT_DIR)/libiberty/iberty
