# This shell script emits a C file. -*- C -*-
#   Copyright (C) 2003-2014 Free Software Foundation, Inc.
#
# This file is part of the GNU Binutils.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
# MA 02110-1301, USA.
#

fragment <<EOF

#include "libbfd.h"
#include "elf32-adl.h"
#include "ldlex.h"
#include "ldlang.h"


static void
adl_elf_before_allocation (void)
{
  gld${EMULATION_NAME}_before_allocation ();

  if (link_info.discard == discard_sec_merge)
    link_info.discard = discard_l;

  /* We always need at least some relaxation to handle code alignment.  */
  if (RELAXATION_DISABLED_BY_USER)
    TARGET_ENABLE_RELAXATION;
  else
    ENABLE_RELAXATION;

  link_info.relax_pass = 2;
}


EOF

LDEMUL_BEFORE_ALLOCATION=adl_elf_before_allocation

