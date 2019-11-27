//
// Copyright (C) 2005 by Freescale Semiconductor Inc.  All rights reserved.
//
// You may distribute under the terms of the Artistic License, as specified in
// the COPYING file.
//

#ifndef _STRINGUTILS_H_
#define _STRINGUTILS_H_

#include <sstream>
#include <string>

#include <iterator> // used only by tests (below)

namespace adl {

  // Converts an arbitrary number type into a string.
  template <class T>
  std::string itos(T n)
  {
    std::ostringstream ss;
    ss << n;
    return ss.str();
  }

  // Converts a boolean into a string.
  inline std::string btos(bool b)
  {
    return (b) ? "true" : "false";
  }

  // Completely generic combiner, that joins strings with spaces by default.
  template < class fwd_container >
  typename fwd_container::value_type
		join (const fwd_container &tokens,const typename fwd_container::value_type &delim = typename fwd_container::value_type(" "))
  {
    typename fwd_container::value_type  result;
    if (false == tokens.empty()) {
      std::ostringstream  os;
      typename fwd_container::const_iterator  strIter;
      for (strIter = tokens.begin();
           strIter != tokens.end(); ) {
        os << *strIter++;
        if (strIter != tokens.end()) {
          os << delim;
        }
      }
      result = os.str().c_str();
    }
    return result;
  }

  /*!
  A fairly generic tokenizer.  A typical way to invoke it is
  using spaces and tabs, e.g.
  
    std::string s = "these   are my       words";
    std::vector< std::string > tl =
      split< typeof(tl) >(s, " \t", true);
  
  (Note that typeof is a GNU extension...if not available just use
  the actual type name instead.)
  
  If "noempty" is true, substrings that evaluate to empty strings
  are omitted from the list (convenient, for instance, to treat a
  series of spaces as a single delimiter).
  */
  template < typename fwd_container, class S >
  fwd_container
  split (const S &joined,const S &tokens,bool noempty = false)
  {
    fwd_container  result;
    if (false == joined.empty()) {
      typename S::size_type  i = 0;
      typename S::size_type  emptyCount = 0;
      for (i = 0; i < joined.npos; ) {
        typename S::size_type  t = joined.find_first_of(tokens, i);
        //assert(t >= i);
        if (i == t) {
          ++emptyCount;
          if ((false == noempty) && (emptyCount > 1)) result.push_back("");
          ++i;
        } else {
          emptyCount = 0;
          S  token(joined, i, t - i);
          bool  keep = (noempty) ? (false == token.empty()) : true;
          if (keep) result.push_back(token);
          //assert(t > i);
          i = t;
        }
      }
    }
    return result;
  }

  // Invoke generic tokenizer with typical defaults for whitespace.
  template < typename fwd_container, class S >
  fwd_container
  split (const S &joined)
  {
    return split< fwd_container,S >(joined, " \t", true/* omit empties */);
  }

}

namespace adl_test {

/*!
For testing only.  Invoke this routine somewhere in your program
to test join() and emit a report to standard output.

The actual output is printed first, then a divider and the proper
output.  Compare them to see if the code is working correctly.
*/
inline void
_test_join (std::ostream &stream) {
  stream << "========== BEGIN TEST\n";
  typedef std::vector< std::string > T;
  T tl;
  tl.push_back("these");
  tl.push_back("are");
  tl.push_back("my");
  tl.push_back("words");
  stream << adl::join(tl," ") << "\n";
  tl.clear();
  tl.push_back("one");
  tl.push_back("two");
  tl.push_back("");
  tl.push_back("three");
  tl.push_back("4");
  tl.push_back("five six seven");
  tl.push_back("");
  tl.push_back("");
  tl.push_back("8");
  tl.push_back("");
  stream << adl::join(tl, ":") << "\n";
  stream << "==========\n";
  stream << "these are my words\n";
  stream << "one:two::three:4:five six seven:::8:\n";
  stream << "========== END TEST\n";
  stream.flush();
}// _test_join

/*!
For testing only.  Invoke this routine somewhere in your program
to test split() and emit a report to standard output.

The actual output is printed first, then a divider and the proper
output.  Compare them to see if the code is working correctly.
*/
inline void
_test_split (std::ostream &stream) {
  stream << "========== BEGIN TEST\n";
  std::string s = "these   are my     words  ";
  typedef std::vector< std::string > type_of_tl;
  type_of_tl tl = adl::split< type_of_tl >(s);
  std::copy(tl.begin(), tl.end(), std::ostream_iterator< std::string >(stream, "\n"));
  s = "one:two::three:4:five six seven:::8:";
  tl = adl::split< type_of_tl,std::string >(s, ":", false);
  std::copy(tl.begin(), tl.end(), std::ostream_iterator< std::string >(stream, "\n"));
  stream << "==========\n";
  stream << "these\nare\nmy\nwords\n";
  stream << "one\ntwo\n\nthree\n4\nfive six seven\n\n\n8\n\n";
  stream << "========== END TEST\n";
  stream.flush();
}// _test_split

}

#endif
