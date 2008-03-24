#ifndef _WIDEN_H
#define _WIDEN_H

#include <string>
#include <vector>
#include <locale>
#include <functional>
#include <iostream>

// Put this class in your personal toolbox...
template<class E,
class T = std::char_traits<E>,
class A = std::allocator<E> >

class Widen : public std::unary_function<
    const std::string&, std::basic_string<E, T, A> >
{
    std::locale loc_;
    const std::ctype<E>* pCType_;

    // No copy-constructor, no assignment operator...
    Widen(const Widen&);
    Widen& operator= (const Widen&);

public:
    // Constructor...
    Widen(const std::locale& loc = std::locale()) : loc_(loc)
    {
        pCType_ = &std::use_facet<std::ctype<E> >(loc);
    }

    // Conversion...
    std::basic_string<E, T, A> operator() (const std::string& str) const
    {
        typename std::basic_string<E, T, A>::size_type srcLen =
            str.length();
        const char* pSrcBeg = str.c_str();
        std::vector<E> tmp(srcLen);

        pCType_->widen(pSrcBeg, pSrcBeg + srcLen, &tmp[0]);
        return std::basic_string<E, T, A>(&tmp[0], srcLen);
    }
}; 

#endif
