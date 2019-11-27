//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//

//
// Hash support for C++ strings.
//

#ifndef _STRING_HASH_H_
#define _STRING_HASH_H_

#include <string.h>

#ifdef _MSC_VER

#include <hash_map>

#else

#undef __DEPRECATED
#include <ext/hash_map>

namespace __gnu_cxx {

  // Hash/set support for strings.
  template<> struct hash<std::string>
  {
    size_t operator()(const std::string &__s) const { 
      return __stl_hash_string(__s.c_str()); 
    }
  };

  // Hash/set support for all pointers.
  template<typename T> struct hash<T *>
  {
    size_t operator()(T *__x) const { 
        std::size_t x = static_cast<std::size_t>(
           reinterpret_cast<std::ptrdiff_t>(__x));
        return x + (x >> 3);
    }
  };

  // Hash/set support for unsigned long long.
  template<> struct hash<unsigned long long>
  {
    size_t operator()(unsigned long long __x) const { 
      return (__x ^ (__x >> 32));
    }
  };

}

#endif

namespace adl {

  struct CmpConstChar {
    bool operator()(const char *x,const char *y) const {
      return strcmp(x,y) == 0;
    }
  };

}

#endif
