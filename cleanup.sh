#!/bin/sh

cd enterprise_vspa_asm

rm -rf .git
cd external/binutils-adl
rm aclocal.m4
cd bfd
rm aclocal.m4
cd ..
cd binutils
rm aclocal.m4
cd ..
cd gas
rm aclocal.m4
cd ..
cd intl
rm aclocal.m4
cd ..
cd libiberty
rm aclocal.m4
cd ..
cd opcodes
rm aclocal.m4
cd ..
cd ../../
cd ..

mv enterprise_vspa_asm/* .
rm -rf enterprise_vspa_asm