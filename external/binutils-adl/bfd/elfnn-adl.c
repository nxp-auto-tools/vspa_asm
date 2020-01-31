/* ADL-generic ELF handler, based upon PowerPC, but then heavily modified.

   PowerPC-specific support for 32-bit ELF
   Copyright (C) 1994-2014 Free Software Foundation, Inc.
   Written by Ian Lance Taylor, Cygnus Support.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */


/* This file is based on a preliminary PowerPC ELF ABI.  The
   information may not match the final PowerPC ELF ABI.  It includes
   suggestions from the in-progress Embedded PowerPC ABI, and that
   information may also not match.  */

#include "sysdep.h"
#include <stdarg.h>
#include <assert.h>
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elfxx-adl.h"
#include "dwarf2.h"
#include "elf-linux-psinfo.h"

#define ARCH_SIZE NN

/* N_ONES produces N one bits, without overflowing machine arithmetic.  */
#define N_ONES(n) (((((bfd_vma) 1 << ((n) - 1)) - 1) << 1) | 1)


// Empty do-nothing relocation for the error case.
static reloc_howto_type adl_dummy_howto[] = {
  /* This reloc does nothing.  */
  HOWTO (0,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_ADL_NONE",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */
};

/* Set the howto pointer for a PowerPC ELF reloc.  */

reloc_howto_type *adl_get_howto_array(unsigned *);
reloc_howto_type *adl_elf_type_to_howto(unsigned);
reloc_howto_type *adl_bfd_code_to_howto(unsigned);

//  We tap in here and call the ADL variant that's defined in the generated ADL
//  disassembler
static void
adl_elf_info_to_howto (bfd *abfd ATTRIBUTE_UNUSED,
		       arelent *cache_ptr,
		       Elf_Internal_Rela *dst)
{
  reloc_howto_type *adl_howto = adl_elf_type_to_howto(ELFNN_R_TYPE(dst->r_info));
  if (adl_howto) {
    cache_ptr->howto = adl_howto;
    return;
  }

  if (!cache_ptr->howto)
    {
      (*_bfd_error_handler) (_("%B: invalid relocation type %d"),
                             abfd, ELFNN_R_TYPE (dst->r_info));
      bfd_set_error (bfd_error_bad_value);

      cache_ptr->howto = &adl_dummy_howto[0];
    }
}

// Map generic BFD relocation types to specific types.
static reloc_howto_type *adl_elf_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
                                                    bfd_reloc_code_real_type code)
{
  return adl_bfd_code_to_howto(code);
}

static reloc_howto_type *adl_elf_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
                                                    const char *r_name)
{
  unsigned count;

  reloc_howto_type *howtos = adl_get_howto_array(&count);

  for (unsigned i = 0; i != count; i++) {
    if (howtos[i].name != NULL && strcasecmp (howtos[i].name, r_name) == 0)
      return &howtos[i];
  }
  
  return NULL;
}

bfd_boolean acb_elf_relax_section(bfd *abfd,
                                  asection *isec,
                                  struct bfd_link_info *link_info,
                                  bfd_boolean *again);


bfd_reloc_status_type acb_handle_reloc_external(struct bfd_link_info *info,unsigned type,bfd *abfd,asection *input_section,
                                                Elf_Internal_Rela *rel,bfd_byte *contents,bfd_vma relocation);



typedef void (*reloc_setter)(unsigned *,unsigned,int,bfd_uint64_t);

typedef bfd_uint64_t (*reloc_action)(bfd_uint64_t,bfd_uint64_t,int);

// ADL: Maps howto type to special setter functions.
void adl_get_reloc_funcs(unsigned type,reloc_setter *,reloc_action *,int *size,int *inner_size,int *offset);

// We only need one copy of these.
#if ARCH_SIZE == 32

bfd_reloc_status_type __attribute__((weak)) acb_handle_reloc_external(struct bfd_link_info *info ATTRIBUTE_UNUSED,
                                                                      unsigned type ATTRIBUTE_UNUSED,
                                                                      bfd *abfd ATTRIBUTE_UNUSED,
                                                                      asection *input_section ATTRIBUTE_UNUSED,
                                                                      Elf_Internal_Rela *rel ATTRIBUTE_UNUSED,
                                                                      bfd_byte *contents ATTRIBUTE_UNUSED,
                                                                      bfd_vma relocation ATTRIBUTE_UNUSED)
{
  return bfd_reloc_ok;
}

// Just a stub that can be overridden by a user-supplied library.
bfd_boolean __attribute__((weak)) acb_elf_relax_section(bfd *abfd ATTRIBUTE_UNUSED,
		       asection *isec ATTRIBUTE_UNUSED,
		       struct bfd_link_info *link_info ATTRIBUTE_UNUSED,
		       bfd_boolean *again ATTRIBUTE_UNUSED)
{
  *again = FALSE;
  return TRUE;
}


void populate_data_array(unsigned *data,bfd_vma x,bfd_vma y,unsigned size,int big_endian)
{
  if (size <= 4) {
    // Make sure to left justify the data.
    data[0] = x << (8 * (4-size));
    data[1] = 0;
    data[2] = 0;
    data[3] = 0;
  } else if (big_endian) {
    data[0] = (x >> 32);
    data[1] = (x & 0xffffffff);
    data[2] = (y >> 32);
    data[3] = (y & 0xffffffff);
  } else {
    data[0] = (x & 0xffffffff);
    data[1] = (x >> 32);
    data[2] = (y & 0xffffffff);
    data[3] = (y >> 32);
  }
}

void depopulate_data_array(bfd_vma *x,bfd_vma *y,unsigned *data,unsigned size,int big_endian)
{
  if (size <= 4) {
    *x = data[0] >> (8 * (4-size));
  } else if (big_endian) {
    *x = data[0];
    *x = (*x << 32) | (data[1] & 0xffffffff);
    *y = data[2];
    *y = (*y << 32) | (data[3] & 0xffffffff);
  } else {
    *x  = ((bfd_vma)data[1] << 32);
    *x |= data[0];
    *y  = ((bfd_vma)data[3] << 32);
    *y |= data[2];
  }
}

#endif

/* Relocate a given location using a given value and howto.  */
static bfd_reloc_status_type adl_relocate_contents (reloc_howto_type *howto,
                                                    bfd *input_bfd,
                                                    bfd_vma relocation,
                                                    bfd_vma pc,
                                                    bfd_byte *location)
{
  int size;
  bfd_vma x = 0;
  bfd_vma y = 0;
  bfd_reloc_status_type flag;
  unsigned int rightshift = howto->rightshift;
  unsigned int bitpos = howto->bitpos;
  reloc_setter  rs = 0;
  reloc_action  ra = 0;
  int new_size = 0, inner_size = 0, offset = 0;

  /* If the size is negative, negate RELOCATION.  This isn't very
     general.  */
  if (howto->size < 0)
    relocation = -relocation;

  // ADL: Special function handling is placed here to handle instruction-field
  // based relocations.  This lets us handle split fields, for example.  We're
  // treating this in a specific manner that's not quite compatible with the
  // original definition, in terms of parameters.
  adl_get_reloc_funcs(howto->type,&rs,&ra,&new_size,&inner_size,&offset);
  
  /* Get the value we are going to relocate.  */
  // Override if necessary with the real size, which is based upon the instruction size.
  size = (new_size) ? new_size : (int)bfd_get_reloc_size (howto);
  location -= offset;

  switch (size)
    {
    default:
    case 0:
      abort ();
    case 1:
      x = bfd_get_8 (input_bfd, location);
      break;
    case 2:
      x = bfd_get_16 (input_bfd, location);
      break;
    case 3:
    case 4:
      x = bfd_get_32 (input_bfd, location);
      break;
    case 8:
      x = bfd_get_64 (input_bfd, location);
      break;
    case 16:
      // For very big instructions (128b stuff).
      x = bfd_get_64 (input_bfd, location);
      y = bfd_get_64 (input_bfd, location+8);
    }

  /* Check for overflow.  FIXME: We may drop bits during the addition
     which we don't check for.  We must either check at every single
     operation, which would be tedious, or we must do the computations
     in a type larger than bfd_vma, which would be inefficient.  */
  flag = bfd_reloc_ok;
  if (howto->complain_on_overflow != complain_overflow_dont)
    {
      bfd_vma addrmask, fieldmask, signmask, ss;
      bfd_vma a, b, sum;

      /* Get the values to be added together.  For signed and unsigned
         relocations, we assume that all values should be truncated to
         the size of an address.  For bitfields, all the bits matter.
         See also bfd_check_overflow.  */
      fieldmask = N_ONES (howto->bitsize);
      signmask = ~fieldmask;
      addrmask = (N_ONES (bfd_arch_bits_per_address (input_bfd))
                  | (fieldmask << rightshift));
      a = (relocation & addrmask) >> rightshift;
      b = (x & howto->src_mask & addrmask) >> bitpos;
      addrmask >>= rightshift;

      switch (howto->complain_on_overflow)
        {
        case complain_overflow_signed:
          /* If any sign bits are set, all sign bits must be set.
             That is, A must be a valid negative address after
             shifting.  */
          signmask = ~(fieldmask >> 1);
          /* Fall thru */

        case complain_overflow_bitfield:
          /* Much like the signed check, but for a field one bit
             wider.  We allow a bitfield to represent numbers in the
             range -2**n to 2**n-1, where n is the number of bits in the
             field.  Note that when bfd_vma is 32 bits, a 32-bit reloc
             can't overflow, which is exactly what we want.  */
          ss = a & signmask;
          if (ss != 0 && ss != (addrmask & signmask))
            flag = bfd_reloc_overflow;

          /* We only need this next bit of code if the sign bit of B
             is below the sign bit of A.  This would only happen if
             SRC_MASK had fewer bits than BITSIZE.  Note that if
             SRC_MASK has more bits than BITSIZE, we can get into
             trouble; we would need to verify that B is in range, as
             we do for A above.  */
          ss = ((~howto->src_mask) >> 1) & howto->src_mask;
          ss >>= bitpos;

          /* Set all the bits above the sign bit.  */
          b = (b ^ ss) - ss;

          /* Now we can do the addition.  */
          sum = a + b;

          /* See if the result has the correct sign.  Bits above the
             sign bit are junk now; ignore them.  If the sum is
             positive, make sure we did not have all negative inputs;
             if the sum is negative, make sure we did not have all
             positive inputs.  The test below looks only at the sign
             bits, and it really just
             SIGN (A) == SIGN (B) && SIGN (A) != SIGN (SUM)

             We mask with addrmask here to explicitly allow an address
             wrap-around.  The Linux kernel relies on it, and it is
             the only way to write assembler code which can run when
             loaded at a location 0x80000000 away from the location at
             which it is linked.  */
          if (((~(a ^ b)) & (a ^ sum)) & signmask & addrmask)
            flag = bfd_reloc_overflow;
          break;

        case complain_overflow_unsigned:
          /* Checking for an unsigned overflow is relatively easy:
             trim the addresses and add, and trim the result as well.
             Overflow is normally indicated when the result does not
             fit in the field.  However, we also need to consider the
             case when, e.g., fieldmask is 0x7fffffff or smaller, an
             input is 0x80000000, and bfd_vma is only 32 bits; then we
             will get sum == 0, but there is an overflow, since the
             inputs did not fit in the field.  Instead of doing a
             separate test, we can check for this by or-ing in the
             operands when testing for the sum overflowing its final
             field.  */
          sum = (a + b) & addrmask;
          if ((a | b | sum) & signmask)
            flag = bfd_reloc_overflow;
          break;

        default:
          abort ();
        }
    }

  // Perform any necessary actions on the data.
  if (ra) {
    relocation = ra(relocation,pc,1);
  }

  // If relevant, use the special setter function to insert the data.
  if (rs) {
    unsigned data[4];
    int big_endian = bfd_header_big_endian (input_bfd);
    populate_data_array(data,x,y,size,big_endian);

    relocation >>= (bfd_vma) rightshift;

    // Call the setter function, which both clears the field and then inserts
    // the address.
    rs(data,inner_size,big_endian,relocation);

    depopulate_data_array(&x,&y,data,size,big_endian);
    
  } else {

    /* Put RELOCATION in the right bits.  */
    relocation >>= (bfd_vma) rightshift;
    relocation <<= (bfd_vma) bitpos;
    
    /* Add RELOCATION to the right bits of X.  */
    x = ((x & ~howto->dst_mask)
         | (((x & howto->src_mask) + relocation) & howto->dst_mask));

  }

  /* Put the relocated value back in the object file.  */
  switch (size)
    {
    default:
      abort ();
    case 1:
      bfd_put_8 (input_bfd, x, location);
      break;
    case 2:
      bfd_put_16 (input_bfd, x, location);
      break;
    case 3:
    case 4:
      bfd_put_32 (input_bfd, x, location);
      break;
    case 8:
#ifdef BFD64
      bfd_put_64 (input_bfd, x, location);
#else
      abort ();
#endif
      break;
    case 16:
      bfd_put_64 (input_bfd, x, location);
      bfd_put_64 (input_bfd, y, location+8);
    }

  return flag;
}


static bfd_reloc_status_type adl_final_link_relocate (reloc_howto_type *howto,
                                                      bfd *input_bfd,
                                                      asection *input_section,
                                                      bfd_byte *contents,
                                                      bfd_vma address,
                                                      bfd_vma value,
                                                      bfd_vma addend)
{
  bfd_vma relocation;

  /* Sanity check the address.  */
  if (address > bfd_get_section_limit (input_bfd, input_section))
    return bfd_reloc_outofrange;

  /* This function assumes that we are dealing with a basic relocation
     against a symbol.  We want to compute the value of the symbol to
     relocate to.  This is just VALUE, the value of the symbol, plus
     ADDEND, any addend associated with the reloc.  */
  relocation = value + addend;

  /* If the relocation is PC relative, we want to set RELOCATION to
     the distance between the symbol (currently in RELOCATION) and the
     location we are relocating.  Some targets (e.g., i386-aout)
     arrange for the contents of the section to be the negative of the
     offset of the location within the section; for such targets
     pcrel_offset is FALSE.  Other targets (e.g., m88kbcs or ELF)
     simply leave the contents of the section as zero; for such
     targets pcrel_offset is TRUE.  If pcrel_offset is FALSE we do not
     need to subtract out the offset of the location within the
     section (which is just ADDRESS).  */
  bfd_vma pc = (input_section->output_section->vma + input_section->output_offset);

  if (howto->pc_relative)
    {
      relocation -= pc;
      if (howto->pcrel_offset)
        relocation -= address;
    }

  return adl_relocate_contents (howto, input_bfd, relocation, pc + address, contents + address);
}



/* The RELOCATE_SECTION function is called by the ELF backend linker
   to handle the relocations for a section.

   The relocs are always passed as Rela structures; if the section
   actually uses Rel structures, the r_addend field will always be
   zero.

   This function is responsible for adjust the section contents as
   necessary, and (if using Rela relocs and generating a
   relocatable output file) adjusting the reloc addend as
   necessary.

   This function does not have to worry about setting the reloc
   address or the reloc symbol index.

   LOCAL_SYMS is a pointer to the swapped in local symbols.

   LOCAL_SECTIONS is an array giving the section in the input file
   corresponding to the st_shndx field of each local symbol.

   The global hash table entry for the global symbols can be found
   via elf_sym_hashes (input_bfd).

   When generating relocatable output, this function must handle
   STB_LOCAL/STT_SECTION symbols specially.  The output symbol is
   going to be the section symbol corresponding to the output
   section, which means that the addend must be adjusted
   accordingly.  */

static bfd_boolean
adl_elf_relocate_section (bfd *output_bfd,
			  struct bfd_link_info *info,
			  bfd *input_bfd,
			  asection *input_section,
			  bfd_byte *contents,
			  Elf_Internal_Rela *relocs,
			  Elf_Internal_Sym *local_syms,
			  asection **local_sections)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;
  bfd_boolean ret = TRUE;

  symtab_hdr = &elf_symtab_hdr (input_bfd);
  sym_hashes = elf_sym_hashes (input_bfd);

  rel = relocs;
  relend = relocs + input_section->reloc_count;

  // ADL:  Major hack here:  This is a simple linker driver designed to tie into the generated relocations from ADL.
  for (; rel < relend; rel++) {
    unsigned r_type;
    bfd_vma addend;
    bfd_reloc_status_type r;
    Elf_Internal_Sym *sym;
    asection *sec;
    struct elf_link_hash_entry *h;
    const char *sym_name;
    reloc_howto_type *howto;
    unsigned long r_symndx;
    bfd_vma relocation;
    bfd_boolean unresolved_reloc;
    bfd_boolean warned;

    r_type = ELFNN_R_TYPE (rel->r_info);
    sym = NULL;
    sec = NULL;
    h = NULL;
    unresolved_reloc = FALSE;
    warned = FALSE;
    r_symndx = ELFNN_R_SYM (rel->r_info);
    howto = &adl_dummy_howto[0];

    if (r_symndx < symtab_hdr->sh_info)
      {
        sym = local_syms + r_symndx;
        sec = local_sections[r_symndx];
        sym_name = bfd_elf_sym_name (input_bfd, symtab_hdr, sym, sec);

        relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
      }
    else
      {
        bfd_boolean ignored;

        RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
                                 r_symndx, symtab_hdr, sym_hashes,
                                 h, sec, relocation,
                                 unresolved_reloc, warned, ignored);

        sym_name = h->root.root.string;
      }

    if (sec != NULL && discarded_section (sec))
      {
        /* For relocs against symbols from removed linkonce sections,
           or sections discarded by a linker script, we just want the
           section contents zeroed.  Avoid any special processing.  */
        howto = adl_elf_type_to_howto(r_type);
        assert(howto);
        RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
                                         rel, 1, relend, howto, 0, contents);
      }

    // Extra processing: Let an external library make adjustments before we do
    // the relocation.
    r = acb_handle_reloc_external(info,r_type,input_bfd,input_section,rel,contents,relocation);

    if (r == bfd_reloc_other) {
      // Overload this unused meaning to indicate that we can skip the
      // relocation.
      continue;
    } else if (r == bfd_reloc_ok) {
    
      addend = rel->r_addend;
      howto = NULL;

      howto = adl_elf_type_to_howto(ELFNN_R_TYPE(rel->r_info));

      if (!howto) {
        info->callbacks->einfo
          (_("%P: %B: unknown relocation type %d for symbol %s\n"),
           input_bfd, (int) r_type, sym_name);
      
        bfd_set_error (bfd_error_bad_value);
        return FALSE;
      }

      r = adl_final_link_relocate (howto, input_bfd, input_section, contents,
                                   rel->r_offset, relocation, addend);

    }
    
    if (r != bfd_reloc_ok)
      {
        if (r == bfd_reloc_overflow)
          {
            if (warned)
              continue;
            if (h != NULL
                && h->root.type == bfd_link_hash_undefweak
                && howto->pc_relative)
              {
                /* Assume this is a call protected by other code that
                   detect the symbol is undefined.  If this is the case,
                   we can safely ignore the overflow.  If not, the
                   program is hosed anyway, and a little warning isn't
                   going to help.  */

                continue;
              }

            if (! (*info->callbacks->reloc_overflow) (info,
                                                      (h ? &h->root : NULL),
                                                      sym_name,
                                                      howto->name,
                                                      rel->r_addend,
                                                      input_bfd,
                                                      input_section,
                                                      rel->r_offset))
              return FALSE;
          }
        else
          {
            info->callbacks->einfo
              (_("%P: %H: %s reloc against `%s': error %d\n"),
               input_bfd, input_section, rel->r_offset,
               howto->name, sym_name, (int) r);
            ret = FALSE;
          }
      }
  }


  return ret;
}



#define TARGET_LITTLE_SYM	adl_elfNN_le_vec
#define TARGET_LITTLE_NAME	"elfNN-adlle"
#define TARGET_BIG_SYM		adl_elfNN_vec
#define TARGET_BIG_NAME		"elfNN-adl"
#define ELF_ARCH		bfd_arch_adl
#define ELF_TARGET_ID		PPCNN_ELF_DATA
#define ELF_MACHINE_CODE	EM_VSPA
#define ELF_MAXPAGESIZE		0x10000
#define ELF_MINPAGESIZE		0x1000
#define ELF_COMMONPAGESIZE	0x1000
#define elf_info_to_howto	adl_elf_info_to_howto


#define elf_backend_plt_not_loaded	1
#define elf_backend_can_gc_sections	1
#define elf_backend_can_refcount	1
#define elf_backend_rela_normal		1
#define elf_backend_caches_rawsize	1

#define bfd_elfNN_bfd_reloc_type_lookup		adl_elf_reloc_type_lookup
#define bfd_elfNN_bfd_reloc_name_lookup		adl_elf_reloc_name_lookup
#define bfd_elfNN_bfd_relax_section		    acb_elf_relax_section

#define elf_backend_relocate_section		  adl_elf_relocate_section
#define elf_backend_init_index_section	  _bfd_elf_init_1_index_section

#include "elfNN-target.h"
