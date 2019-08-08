/* Select disassembly routine for specified architecture.
   Copyright (C) 1994-2014 Free Software Foundation, Inc.

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "dis-asm.h"


extern unsigned AdlBfdArch;

disassembler_ftype
disassembler (abfd)
     bfd *abfd;
{
  
  enum bfd_architecture a = bfd_get_arch (abfd);
  disassembler_ftype disassemble;

  if (a == AdlBfdArch) {
    if (bfd_big_endian(abfd)) {
      disassemble = print_insn_big_adl;
    } else {
      disassemble = print_insn_little_adl;
    }
  } else {
    return 0;
  }
  return disassemble;
}

void
disassembler_usage (stream)
     FILE * stream ATTRIBUTE_UNUSED;
{
  print_adl_disassembler_options (stream);

  return;
}

void
disassemble_init_for_target (struct disassemble_info * info ATTRIBUTE_UNUSED)
{
}
