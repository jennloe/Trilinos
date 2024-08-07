// @HEADER
// *****************************************************************************
//           Amesos2: Templated Direct Sparse Solver Package
//
// Copyright 2011 NTESS and the Amesos2 contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

/**
 * \file   Amesos2_Meta.hpp
 * \author Eric Bavier <etbavie@sandia.gov>
 * \date   Mon Jul 18 15:28:52 2011
 *
 * \brief  Provides some simple meta-programming utilities for Amesos2.
 *
 * The meta-programming utilities provided here might not be nearly as
 * flexible and complete as those provided in the Boost
 * Meta-Programming Library, but they are sufficient for the needs of
 * Amesos2.
 */


#ifndef AMESOS2_META_HPP
#define AMESOS2_META_HPP

#include <type_traits>

#include "Amesos2_config.h"


namespace Amesos2 {

  namespace Meta {

    /**
     * \internal
     * \defgroup amesos2_meta Amesos2 Meta-programming Functions
     * @{
     */

    //////////////////////////////////////////
    // Meta-functions for boolean operators //
    //////////////////////////////////////////

    /* Must define these with the '_' suffix because they are
     * otherwise keywords in C++
     */

    template <bool b1, bool b2>
    struct or_ : public std::false_type {};

    template <bool b>
    struct or_<true,b> : public std::true_type {};

    template <bool b>
    struct or_<b,true> : public std::true_type {};


    template <bool b1, bool b2>
    struct and_ : public std::false_type {};

    template <>
    struct and_<true,true> : public std::true_type {};


    template <bool b>
    struct not_ {};

    template <>
    struct not_<true> : std::false_type {};

    template <>
    struct not_<false> : std::true_type {};

    ////////////////////////////////////////////
    // A meta-programming type-list structure //
    ////////////////////////////////////////////

    struct nil_t {};                // to denote an empty list

    template <typename Head, typename Tail>
    struct type_list {
      typedef type_list<Head,Tail> type;
      typedef Head head;
      typedef Tail tail;
    };

    /**
     * \brief Utility meta-function for creating a type-list
     *
     * Example:
     *
     * \code
     * make_list3<float,double,quad>
     * \encode
     *
     * is a list of three types: \c float , \c double , and \c quad .
     */
    template <typename T1>
    struct make_list1
      : type_list<T1,nil_t>
    { };

    template <typename T1, typename T2>
    struct make_list2
      : type_list<T1,type_list<T2,nil_t> >
    { };

    template <typename T1, typename T2, typename T3>
    struct make_list3
      : type_list<T1, type_list<T2, type_list<T3,nil_t> > >
    { };

    template <typename T1, typename T2, typename T3, typename T4>
    struct make_list4
      : type_list<T1, type_list<T2, type_list<T3, type_list<T4,nil_t> > > >
    { };

    template <typename T1, typename T2, typename T3, typename T4, typename T5>
    struct make_list5
      : type_list<T1, type_list<T2, type_list<T3, type_list<T4, type_list<T5,nil_t> > > > >
    { };

    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    struct make_list6
      : type_list<T1, type_list<T2, type_list<T3, type_list<T4, type_list<T5, type_list<T6,nil_t> > > > > >
    { };

    /* More declarations for larger type lists may be added if necessary */


    /**
     * \brief A utility meta-function to determine whether a given
     * type is found within a type-list
     *
     * \code
     * typedef make_list4<float,double,int,long> t_list;
     * if( type_list_contains<t_list,bool>::value ){
     *   // dead branch
     * } else {
     *   // This will always execute
     * }
     */

    /* SR: We will not use external initialization for the static const types.
     * Combined with template meta programming this fails in Intel compilers
     * 11-13. Moving all the initializations inside the declarations.
     */
    template <typename list, typename elem>
    struct type_list_contains {
      static const bool value = std::conditional_t<
          std::is_same_v<typename list::head, elem>,
          std::true_type,
          type_list_contains<typename list::tail,elem>
        >::value;
    };

    // Base recursive case
    template <typename elem>
    struct type_list_contains<nil_t,elem> {
      static const bool value = false;
    };

    /** @} */

  } // end namespace Meta

} // end namespace Amesos2

#endif  // AMESOS2_META_HPP
