//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//

//
// A garbage-collected C++ string.
//

#ifndef _GC_STRING_H_
#define _GC_STRING_H_

#include <string>
#include <sstream>

#include "gc/gc_allocator.h"

#include "stringhash.h"

typedef std::basic_string<char,std::char_traits<char>,gc_allocator<char> >  gc_string;
typedef std::basic_ostringstream<char,std::char_traits<char>,gc_allocator<char> > gc_ostringstream;
typedef std::basic_istringstream<char,std::char_traits<char>,gc_allocator<char> > gc_istringstream;

inline gc_string to_gstr(const std::ostringstream &os) { return gc_string(os.str().c_str()); }
inline gc_string to_gstr(const std::string &s) { return gc_string(s.c_str()); }

#ifndef _MSC_VER 

namespace __gnu_cxx {

  // Hash/set support for gc'd strings.
  template<> struct hash<gc_string>
  {
    size_t operator()(const gc_string &__s) const { 
      return __stl_hash_string(__s.c_str()); 
    }
  };

}

#endif

#endif

