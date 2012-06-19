/** \file AliasedPair.hpp
 *
 * Define a macro which makes aliases for an STL pair for improved readability.
 * Construct a STL pair class using DEFINE_ALIASED_PAIR(MapPair, int, string, key, value);
 * You can then declare an instance: MapPair pair;
 *   and access the members via pair.first and pair.second as usual
 *   or via pair.key and pair.value
 * NOTE: All behavior may not behave as expected at current moment 
 * D.T. Miller - 24 Feb 2012
 */

#ifndef __ALIASEDPAIR_HPP_
#define __ALIASEDPAIR_HPP_

#include <utility>

#define DEFINE_ALIASED_PAIR(CLASSNAME, TYPE_FIRST, TYPE_SECOND, ALIAS_FIRST, ALIAS_SECOND) \
    class CLASSNAME : public std::pair<TYPE_FIRST,TYPE_SECOND>		\
    {									\
      public:								\
	TYPE_FIRST & ALIAS_FIRST;					\
	TYPE_SECOND & ALIAS_SECOND;					\
	CLASSNAME () : ALIAS_FIRST(first), ALIAS_SECOND(second) {};	\
	CLASSNAME (const std::pair<TYPE_FIRST,TYPE_SECOND> &pa)	:	\
	    std::pair<TYPE_FIRST,TYPE_SECOND>(pa), 			\
	    ALIAS_FIRST(first), ALIAS_SECOND(second) {};		\
        CLASSNAME (const CLASSNAME &apa) :                              \
            std::pair<TYPE_FIRST,TYPE_SECOND>(apa.first, apa.second),   \
            ALIAS_FIRST(first), ALIAS_SECOND(second) {};                \
        CLASSNAME (const TYPE_FIRST &T1, const TYPE_SECOND &T2) :	\
	    std::pair<TYPE_FIRST,TYPE_SECOND>(T1,T2),			\
	    ALIAS_FIRST(first), ALIAS_SECOND(second) {};		\
	CLASSNAME& operator=(const CLASSNAME &rhs) {			\
	    ALIAS_FIRST = rhs.ALIAS_FIRST; ALIAS_SECOND = rhs.ALIAS_SECOND; return (*this); \
	}								\
	CLASSNAME &operator=(const std::pair<TYPE_FIRST,TYPE_SECOND> &rhs) { \
	    ALIAS_FIRST = rhs.first; ALIAS_SECOND = rhs.second; return (*this); \
	}								\
    }

/// CODE EXAMPLE
// DEFINE_ALIASED_PAIR(FooPair, int, std::string, bar, baz);
// FooPair f(1, "bob")
// f.bar = 2;
// std::cout << f.first << " " << f.second << std::endl;
// std::map<int,std::string> map;
// map.insert(f);

#endif // __ALIASEDPAIR_HPP_
 
 
