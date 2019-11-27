//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//

//
// Miscellaneous macros for code simplification.
//

#ifndef _MACROS_
#define _MACROS_

// Bad MSVC++ bug:  They define min/max as macros, which interferes with the min/max defines in std.
#ifdef _MSC_VER
# define NOMINMAX
#endif

// GCC implements the typeof operator natively, so we use that when possible.
// Otherwise, we switch over to using the Boost implementation, which isn't
// quite as nice because it requires us to register all types which will be
// used.
#ifdef USE_BOOST_TYPEOF

#include "boost/typeof/typeof.hpp"

#include BOOST_TYPEOF_INCREMENT_REGISTRATION_GROUP()

#define ForEach(container,iter)                                         \
  for (BOOST_AUTO(iter,container.begin()), end = container.end(); iter != end; ++iter)

#define RevForEach(container,iter)                                      \
  for (BOOST_AUTO(iter,container.rbegin()), rend = container.rend(); iter != rend; ++iter)

#define Var(x,y) BOOST_AUTO(x,y)

#define AdlTypeof(x) BOOST_TYPEOF(x)

#else

#define AdlTypeof(x) typeof(x)

// Iterate over a data structure.  Note that this can be used for
// constant objects as well as non-constant objects because of the
// use of the 'typeof' operator.
#define ForEach(container,iter) \
       for (typeof(container.begin()) iter = container.begin(), end = container.end(); iter != end; ++iter)

#define RevForEach(container,iter) \
       for (typeof(container.rbegin()) iter = container.rbegin(), rend = container.rend(); iter != rend; ++iter)

// Easy way to declare a variable with the type specified by the rhs.
#define Var(x,y) typeof(y) x = y

// Easy way to create an auto_ptr of an object.  Note that the rhs should return
// a pointer, e.g. APtr(x,new Foo);
#define APtr(x,y) auto_ptr<typeof(*y)> x ( y )

#define BOOST_TYPEOF_REGISTER_TYPE(x)
#define BOOST_TYPEOF_REGISTER_TEMPLATE(x, params)

#endif // USE_BOOST_TYPEOF

// Execute code if 'z' is found in 'y'.  'x' is the iterator assigned the value.
#define IfFind(x,y,z) \
  Var(x,y.find(z)); if (x != y.end())

#define IfNotFind(x,y,z) \
  Var(x,y.find(z)); if (x == y.end())

#define ForRange(count,iter) \
       for (unsigned iter = 0; iter != (count); ++iter)

#define ForRevRange(count,iter) \
       for (int iter = count-1; iter >= 0 ; --iter)

// Throws a runtime error.  User must include sstream for this to work.
#define RError(x) { std::ostringstream ss; ss << x; throw std::runtime_error(ss.str()); }

// Displays a message to stderr and then calls exit(1) to immediately exit.
#define ErrExit(x) { std::cerr << x << '\n'; exit(1); }

#define RAssert(x,y) { if (!(x)) { RError(y); } }

#define VPrint(x) { if (VerboseMode) { std::cout << x; } }

#define DPrint(x) { if (DebugMode) { std::cout << "DEBUG:  " << x; } }

#ifdef DEBUG
#  define DBPrint(x) { std::cerr << x << '\n'; }
#else
#  define DBPrint(x)
#endif


#define MkStr(x,y) { std::ostringstream ss; ss << y; x = ss.str(); }
#define MkGStr(x,y) { gc_ostringstream ss; ss << y; x = ss.str(); }


// This will stringify an arbitrary value.
#define Stringify(x) #x

// Call this to expand 'x' if x is also a preprocessor definition.
#define Expand(x) Stringify(x)

#endif

