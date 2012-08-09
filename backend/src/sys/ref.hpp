/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

#ifndef __PF_REF_HPP__
#define __PF_REF_HPP__

#include "sys/atomic.hpp"
#include "sys/alloc.hpp"

namespace pf
{
  class RefCount
  {
  public:
    RefCount() : refCounter(0) {}
    virtual ~RefCount() {}
    INLINE void refInc() { refCounter++; }
    INLINE bool refDec() { return !(--refCounter); }
    Atomic32 refCounter;
  };

  ////////////////////////////////////////////////////////////////////////////////
  /// Reference to single object
  ////////////////////////////////////////////////////////////////////////////////

  template<typename Type>
  class Ref {
  public:
    Type* const ptr;

    ////////////////////////////////////////////////////////////////////////////////
    /// Constructors, Assignment & Cast Operators
    ////////////////////////////////////////////////////////////////////////////////

    INLINE Ref(void) : ptr(NULL) {}
    INLINE Ref(NullTy) : ptr(NULL) {}
    INLINE Ref(const Ref& input) : ptr(input.ptr) { if ( ptr ) ptr->refInc(); }
    INLINE Ref(Type* const input) : ptr(input) { if (ptr) ptr->refInc(); }
    INLINE ~Ref(void) { if (ptr && ptr->refDec()) PF_DELETE(ptr); }

    INLINE Ref& operator= (const Ref &input) {
      if (input.ptr) input.ptr->refInc();
      if (ptr && ptr->refDec()) PF_DELETE(ptr);
      *(Type**)&ptr = input.ptr;
      return *this;
    }

    INLINE Ref& operator= (NullTy) {
      if (ptr && ptr->refDec()) DELETE(ptr);
      *(Type**)&ptr = NULL;
      return *this;
    }

    INLINE operator bool(void) const { return ptr != NULL; }
    INLINE operator Type*(void) const { return ptr; }

    ////////////////////////////////////////////////////////////////////////////////
    /// Properties
    ////////////////////////////////////////////////////////////////////////////////

    INLINE const Type& operator*  (void) const { return *ptr; }
    INLINE const Type* operator-> (void) const { return  ptr; }
    INLINE Type& operator*  (void) { return *ptr; }
    INLINE Type* operator-> (void) { return  ptr; }

    template<typename TypeOut>
    INLINE       Ref<TypeOut> cast()       { return Ref<TypeOut>(static_cast<TypeOut*>(ptr)); }
    template<typename TypeOut>
    INLINE const Ref<TypeOut> cast() const { return Ref<TypeOut>(static_cast<TypeOut*>(ptr)); }
    PF_CLASS(Ref);
  };

  template<typename Type> INLINE bool operator< ( const Ref<Type>& a, const Ref<Type>& b ) { return a.ptr <  b.ptr ; }
  template<typename Type> INLINE bool operator== ( const Ref<Type>& a, NullTy             ) { return a.ptr == NULL  ; }
  template<typename Type> INLINE bool operator== ( NullTy            , const Ref<Type>& b ) { return NULL  == b.ptr ; }
  template<typename Type> INLINE bool operator== ( const Ref<Type>& a, const Ref<Type>& b ) { return a.ptr == b.ptr ; }
  template<typename Type> INLINE bool operator!= ( const Ref<Type>& a, NullTy             ) { return a.ptr != NULL  ; }
  template<typename Type> INLINE bool operator!= ( NullTy            , const Ref<Type>& b ) { return NULL  != b.ptr ; }
  template<typename Type> INLINE bool operator!= ( const Ref<Type>& a, const Ref<Type>& b ) { return a.ptr != b.ptr ; }
}

#endif /* __PF_REF_HPP__ */

