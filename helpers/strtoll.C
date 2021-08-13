//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//

//
// An implementation of strtoull for systems which do not have one.
//

#include <ctype.h>

#ifdef _MSC_VER
# undef HAVE_STRTOLL
# undef HAVE_STRTOULL

# define LONG_LONG_MIN  _I64_MIN
# define LONG_LONG_MAX  _I64_MAX
# define ULONG_LONG_MAX _UI64_MAX

#else
# include "adl_config.h"
#endif

#include "helpers/BasicTypes.h"

#ifndef HAVE_STRTOLL

extern "C" int64_t strtoll(const char *nptr,char **endptr,int base);

int64_t strtoll(const char *nptr,char **endptr,int base)
{
  int64_t cutoff,cutlim,value;
  char *s = (char*)nptr;

  bool neg = false;
  bool any = false;
  bool ovf = false;

  unsigned char c;

  // Initialize
  value=0;

  // Assume no conversion
  if(endptr) *endptr=s;

  // Skip spaces
  while(*s!='\0' && isspace(*s)) s++;

  // Process sign
  if(*s=='-'){
    neg = true;
    s++;
  }
  else if(*s=='+'){
    s++;
  }

  // Figure base if not given
  if(base<2){
    base=10;
    if(s[0]=='0'){
      base=8;
      if(s[1]=='x' || s[1]=='X') base=16;
    }
  }

  // Process 0x if hexadecimal
  if(base==16 && s[0]=='0' && (s[1]=='x' || s[1]=='X')) s+=2;

  // Overflow detection
  cutoff = neg ? LONG_LONG_MAX+1 : LONG_LONG_MAX;
  cutlim = cutoff%base;
  cutoff = cutoff/base;

  // Process digits
  while((c=*s++)!='\0'){

    // Digit value
    if(isdigit(c)) {
      c=c-'0';
    }
    else if(isalpha(c)) {
      c=toupper(c)-'A'+10;
    }
    else {
      break;
    }

    // Digit value out of range
    if(c>=base) {
      break;
    }

    // Check for overflow
    if(ovf || value>cutoff || (value==cutoff && c>cutlim)) { 
      ovf=true; 
      continue; 
    }

    // Accumulate
    value*=base;
    value+=c;
    any=1;
  }

  // Overflow
  if(ovf){
    value = neg ? LONG_LONG_MIN : LONG_LONG_MAX;
    errno=ERANGE;
  }

  // Negative
  else if(neg){
    value=-value;
  }

  // Return end of number
  if(any){
    if(endptr) {
      *endptr=s;
    }
  }

  // Done
  return value;
}

#endif



#ifndef HAVE_STRTOULL

extern "C" uint64_t strtoull(const char *nptr,char **endptr,int base);


uint64_t strtoull(const char *nptr,char **endptr,int base)
{
  uint64_t cutoff,cutlim,value;
  const char *s = nptr;
  bool neg = false;
  bool any = false;
  bool ovf = false;

  unsigned char c;

  // Initialize
  value=0;

  // Assume no conversion
  if(endptr) *endptr=s;

  // Skip spaces
  while(*s != '\0' && isspace(*s)) s++;

  // Process sign
  if(*s=='-') {
    neg=1;
    s++;
  }
  else if(*s=='+') {
    s++;
  }

  // Figure base if not given
  if(base<2) {
    base=10;
    if(s[0]=='0'){
      base=8;
      if(s[1]=='x' || s[1]=='X') {
        base=16;
      }
    }
  }

  // Process 0x if hexadecimal
  if(base==16 && s[0]=='0' && (s[1]=='x' || s[1]=='X')) s+=2;

  // Overflow detection
  cutoff=ULONG_LONG_MAX;
  cutlim=cutoff%base;
  cutoff=cutoff/base;

  // Process digits
  while((c=*s++)!='\0') {

    // Digit value
    if(isdigit(c)) {
      c=c-'0';
    }
    else if(isalpha(c)) {
      c=toupper(c)-'A'+10;
    }
    else
      break;

    // Digit value out of range
    if(c>=base) break;

    // Check for overflow
    if(ovf || value>cutoff || (value==cutoff && c>cutlim)) { 
      ovf=true; 
      continue; 
    }

    // Accumulate
    value*=base;
    value+=c;
    any=true;
  }

  // Overflow
  if(ovf){
    value=ULONG_LONG_MAX;
    errno=ERANGE;
  }

  // Negative
  else if(neg) {
    value=-value;
  }

  // Return end of number
  if(any){
    if(endptr) {
      *endptr=s;
    }
  }

  // Done
  return value;
}

#endif




