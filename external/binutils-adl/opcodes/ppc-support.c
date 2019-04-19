//
// This file is generated by hand and contains the entry-points that are expected by
// BinUtils.  This file then calls the generated code created by ADL.
//

#include <string.h>
#include <stdarg.h>

#include "sysdep.h"
#include "dis-asm.h"

int ppc_print_insn (bfd_vma memaddr, struct disassemble_info *info,int bigendian);

// Print up to 4 bytes per line, trying to print out as much as possible, based upon the instruction length.
static void update_bytes_per_chunk(int len,struct disassemble_info *info)
{
  if (len % 4 == 0) {
    info->bytes_per_chunk = 4;
  } else if (len % 2 == 0) {
    info->bytes_per_chunk = 2;
  } else {
    info->bytes_per_chunk = 1;
  }
}

int print_insn_little_powerpc(bfd_vma memaddr, struct disassemble_info *info)
{
  int len = ppc_print_insn(memaddr,info,0);
  update_bytes_per_chunk(len,info);
  return len;
}

int print_insn_big_powerpc(bfd_vma memaddr, struct disassemble_info *info)
{
  int len = ppc_print_insn(memaddr,info,1);
  update_bytes_per_chunk(len,info);
  return len;
}

int print_insn_rs6000(bfd_vma memaddr, struct disassemble_info *info)
{
  int len = ppc_print_insn(memaddr,info,1);
  update_bytes_per_chunk(len,info);
  return len;
}

void print_ppc_disassembler_options (FILE *stream ATTRIBUTE_UNUSED) { }


// This prototype duplicates the one in asm/disassembler.h, but I didn't want to include a file
// that's outside of the binutils tree.
int disassemble(char *output,unsigned memaddr,const char *input,int length,int bigendian,
                int print_addr,const char *sep);

static void simple_print_address(bfd_vma memaddr,struct disassemble_info *info)
{
  unsigned addr = memaddr;
  (*info->fprintf_func) (info->stream, "0x%x",addr);
}

static void simple_error_func(int status, bfd_vma memaddr, struct disassemble_info *info)
{
  (*info->fprintf_func) (info->stream, "Error reading from stream %d at address 0x%x.",status,(unsigned)memaddr);
}

static int stream_read_func (bfd_vma memaddr ATTRIBUTE_UNUSED, bfd_byte *myaddr, unsigned int length,
                      struct disassemble_info *info)
{
  char *input = (char *)info->application_data;
  memcpy(myaddr,input,length);
  info->application_data = input + length;
  return 0;
}

// This tracks and updates the stream, so that we don't overwrite what we've just written.
// Note the extra level of indirection:  We're pointing to the character stream, so that we can
// update the current pointer.
static int ATTRIBUTE_PRINTF_2
tracking_sprintf (char **stream, const char *format, ...)
{
  int n;

  va_list args;
  va_start (args,format);
  n = vsprintf(*stream,format,args);
  va_end(args);
  *stream += n;
  return n;
}

//
// Interface for disassembling a stream to a buffer.
//
// output:     Output buffer (disassembled instruction text).
// memaddr:    Memory address of disassembly point.  Only relevant if the generated
//             disassembler contains instruction fields with semantic knowledge about
//             addresses or if print_addr is true.
// input:      The input buffer (binary data).
// length:     Length (in bytes) of input buffer.  The routine will keep
//             disassembling until 'length' is reached.
// bigendian:  If true, the input character stream is in big-endian format..
//             False: In little-endian format.
// print_addr: True:  Print a leading memory address before each disassembled instruction.  This starts with
//             memaddr and advances it as necessary.
// sep:        Separator string for multiple instructions.  This string is printed after each instruction
//             is disassembled.  May be 0, in which case it is ignored.
// return:     Number of bytes disassembled.
int disassemble(char *output,unsigned memaddr,const char *input,int length,int bigendian,
                int print_addr,const char *sep)
{
  disassemble_info info;
  int width = sizeof(bfd_vma) * 2;
  const char *end = input + length;
  unsigned addr = memaddr;
  char *tmp_output = output;

  memset(&info,0,sizeof(disassemble_info));
  info.fprintf_func = (fprintf_ftype)tracking_sprintf;
  info.stream = &tmp_output;
  info.print_address_func = simple_print_address;
  info.read_memory_func = stream_read_func;
  info.memory_error_func = simple_error_func;
  info.application_data = (char*)input;
  info.buffer_length = length;

  **((char**)info.stream) = '\x0';
  while ( (const char*)info.application_data < end ) {
    if (print_addr) {
      (*info.fprintf_func)(info.stream,"0x%0*x:  ",width,addr);
    }
    addr += ppc_print_insn(addr,&info,bigendian);
    if (sep) {
      (*info.fprintf_func)(info.stream,"%s",sep);
    }
  }
  return (addr - memaddr);
}