//@HEADER
// ************************************************************************
//
//                        Kokkos v. 4.0
//       Copyright (2022) National Technology & Engineering
//               Solutions of Sandia, LLC (NTESS).
//
// Under the terms of Contract DE-NA0003525 with NTESS,
// the U.S. Government retains certain rights in this software.
//
// Part of Kokkos, under the Apache License v2.0 with LLVM Exceptions.
// See https://kokkos.org/LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//@HEADER
#ifndef KOKKOSBATCHED_TRSV_DECL_HPP
#define KOKKOSBATCHED_TRSV_DECL_HPP

/// \author Kyungjoo Kim (kyukim@sandia.gov)
/// \author Yuuichi Asahi (yuuichi.asahi@cea.fr)

#include "KokkosBatched_Util.hpp"
#include "KokkosBatched_Vector.hpp"

namespace KokkosBatched {

///
/// Serial Trsv
///

template <typename ArgUplo, typename ArgTrans, typename ArgDiag, typename ArgAlgo>
struct SerialTrsv {
  template <typename ScalarType, typename AViewType, typename bViewType>
  KOKKOS_INLINE_FUNCTION static int invoke(const ScalarType /*alpha*/, const AViewType & /*A*/,
                                           const bViewType & /*b*/) {
    assert(false && "Error: encounter dummy impl");
    return 0;
  }
};

///
/// Team Trsv
///

template <typename MemberType, typename ArgUplo, typename ArgTrans, typename ArgDiag, typename ArgAlgo>
struct TeamTrsv {
  template <typename ScalarType, typename AViewType, typename bViewType>
  KOKKOS_INLINE_FUNCTION static int invoke(const MemberType & /*member*/, const ScalarType /*alpha*/,
                                           const AViewType & /*A*/, const bViewType & /*b*/) {
    assert(false && "Error: encounter dummy impl");
    return 0;
  }
};

///
/// TeamVector Trsv
///

template <typename MemberType, typename ArgUplo, typename ArgTrans, typename ArgDiag, typename ArgAlgo>
struct TeamVectorTrsv {
  template <typename ScalarType, typename AViewType, typename bViewType>
  KOKKOS_INLINE_FUNCTION static int invoke(const MemberType & /*member*/, const ScalarType /*alpha*/,
                                           const AViewType & /*A*/, const bViewType & /*b*/) {
    assert(false && "Error: encounter dummy impl");
    return 0;
  }
};

///
/// Selective Interface
///
template <typename MemberType, typename ArgUplo, typename ArgTrans, typename ArgDiag, typename ArgMode,
          typename ArgAlgo>
struct Trsv {
  template <typename ScalarType, typename AViewType, typename bViewType>
  KOKKOS_INLINE_FUNCTION static int invoke(const MemberType &member, const ScalarType alpha, const AViewType &A,
                                           const bViewType &b) {
    int r_val = 0;
    if constexpr (std::is_same_v<ArgMode, Mode::Serial>) {
      r_val = SerialTrsv<ArgUplo, ArgTrans, ArgDiag, ArgAlgo>::invoke(alpha, A, b);
    } else if constexpr (std::is_same_v<ArgMode, Mode::Team>) {
      r_val = TeamTrsv<MemberType, ArgUplo, ArgTrans, ArgDiag, ArgAlgo>::invoke(member, alpha, A, b);
    } else if constexpr (std::is_same_v<ArgMode, Mode::TeamVector>) {
      r_val = TeamVectorTrsv<MemberType, ArgUplo, ArgTrans, ArgDiag, ArgAlgo>::invoke(member, alpha, A, b);
    }
    return r_val;
  }
};

}  // namespace KokkosBatched

#include "KokkosBatched_Trsv_Serial_Impl.hpp"
#include "KokkosBatched_Trsv_Team_Impl.hpp"
#include "KokkosBatched_Trsv_TeamVector_Impl.hpp"

#define KOKKOSBATCHED_SERIAL_TRSV_LOWER_NO_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, DIAG, M, N, ALPHA, A, AS0, AS1, B, BS)  \
  KokkosBatched::Impl::SerialTrsvInternalLower<ALGOTYPE>::invoke(DIAG::use_unit_diag, false, M, ALPHA, A, AS0, AS1, B, \
                                                                 BS)

#define KOKKOSBATCHED_SERIAL_TRSV_LOWER_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, DIAG, M, N, ALPHA, A, AS0, AS1, B, BS)     \
  KokkosBatched::Impl::SerialTrsvInternalUpper<ALGOTYPE>::invoke(DIAG::use_unit_diag, false, N, ALPHA, A, AS1, AS0, B, \
                                                                 BS)

#define KOKKOSBATCHED_SERIAL_TRSV_LOWER_CONJTRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, DIAG, M, N, ALPHA, A, AS0, AS1, B, BS) \
  KokkosBatched::Impl::SerialTrsvInternalUpper<ALGOTYPE>::invoke(DIAG::use_unit_diag, true, N, ALPHA, A, AS1, AS0, B,  \
                                                                 BS)

#define KOKKOSBATCHED_SERIAL_TRSV_UPPER_NO_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, DIAG, M, N, ALPHA, A, AS0, AS1, B, BS)  \
  KokkosBatched::Impl::SerialTrsvInternalUpper<ALGOTYPE>::invoke(DIAG::use_unit_diag, false, M, ALPHA, A, AS0, AS1, B, \
                                                                 BS)

#define KOKKOSBATCHED_SERIAL_TRSV_UPPER_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, DIAG, M, N, ALPHA, A, AS0, AS1, B, BS)     \
  KokkosBatched::Impl::SerialTrsvInternalLower<ALGOTYPE>::invoke(DIAG::use_unit_diag, false, N, ALPHA, A, AS1, AS0, B, \
                                                                 BS)

#define KOKKOSBATCHED_SERIAL_TRSV_UPPER_CONJTRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, DIAG, M, N, ALPHA, A, AS0, AS1, B, BS) \
  KokkosBatched::Impl::SerialTrsvInternalLower<ALGOTYPE>::invoke(DIAG::use_unit_diag, true, N, ALPHA, A, AS1, AS0, B,  \
                                                                 BS)

#define KOKKOSBATCHED_TEAM_TRSV_LOWER_NO_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, MEMBER, DIAG, M, N, ALPHA, A, AS0, AS1, \
                                                                   B, BS)                                            \
  KokkosBatched::TeamTrsvInternalLower<ALGOTYPE>::invoke(MEMBER, DIAG::use_unit_diag, M, ALPHA, A, AS0, AS1, B, BS)

#define KOKKOSBATCHED_TEAM_TRSV_LOWER_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, MEMBER, DIAG, M, N, ALPHA, A, AS0, AS1, B, \
                                                                BS)                                                  \
  KokkosBatched::TeamTrsvInternalUpper<ALGOTYPE>::invoke(MEMBER, DIAG::use_unit_diag, N, ALPHA, A, AS1, AS0, B, BS)

#define KOKKOSBATCHED_TEAM_TRSV_UPPER_NO_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, MEMBER, DIAG, M, N, ALPHA, A, AS0, AS1, \
                                                                   B, BS)                                            \
  KokkosBatched::TeamTrsvInternalUpper<ALGOTYPE>::invoke(MEMBER, DIAG::use_unit_diag, M, ALPHA, A, AS0, AS1, B, BS)

#define KOKKOSBATCHED_TEAM_TRSV_UPPER_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, MEMBER, DIAG, M, N, ALPHA, A, AS0, AS1, B, \
                                                                BS)                                                  \
  KokkosBatched::TeamTrsvInternalLower<ALGOTYPE>::invoke(MEMBER, DIAG::use_unit_diag, N, ALPHA, A, AS1, AS0, B, BS)

#define KOKKOSBATCHED_TEAMVECTOR_TRSV_LOWER_NO_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, MEMBER, DIAG, M, N, ALPHA, A, AS0, \
                                                                         AS1, B, BS)                                  \
  KokkosBatched::TeamVectorTrsvInternalLower<ALGOTYPE>::invoke(MEMBER, DIAG::use_unit_diag, M, ALPHA, A, AS0, AS1, B, \
                                                               BS)

#define KOKKOSBATCHED_TEAMVECTOR_TRSV_LOWER_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, MEMBER, DIAG, M, N, ALPHA, A, AS0,    \
                                                                      AS1, B, BS)                                     \
  KokkosBatched::TeamVectorTrsvInternalUpper<ALGOTYPE>::invoke(MEMBER, DIAG::use_unit_diag, N, ALPHA, A, AS1, AS0, B, \
                                                               BS)

#define KOKKOSBATCHED_TEAMVECTOR_TRSV_UPPER_NO_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, MEMBER, DIAG, M, N, ALPHA, A, AS0, \
                                                                         AS1, B, BS)                                  \
  KokkosBatched::TeamVectorTrsvInternalUpper<ALGOTYPE>::invoke(MEMBER, DIAG::use_unit_diag, M, ALPHA, A, AS0, AS1, B, \
                                                               BS)

#define KOKKOSBATCHED_TEAMVECTOR_TRSV_UPPER_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, MEMBER, DIAG, M, N, ALPHA, A, AS0,    \
                                                                      AS1, B, BS)                                     \
  KokkosBatched::TeamVectorTrsvInternalLower<ALGOTYPE>::invoke(MEMBER, DIAG::use_unit_diag, N, ALPHA, A, AS1, AS0, B, \
                                                               BS)

#define KOKKOSBATCHED_TRSV_LOWER_NO_TRANSPOSE_INTERNAL_INVOKE(MODETYPE, ALGOTYPE, MEMBER, DIAG, M, N, ALPHA, A, AS0,   \
                                                              AS1, B, BS)                                              \
  if constexpr (std::is_same_v<MODETYPE, KokkosBatched::Mode::Serial>) {                                               \
    KOKKOSBATCHED_SERIAL_TRSV_LOWER_NO_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, DIAG, M, N, ALPHA, A, AS0, AS1, B, BS);     \
  } else if constexpr (std::is_same_v<MODETYPE, KokkosBatched::Mode::Team>) {                                          \
    KOKKOSBATCHED_TEAM_TRSV_LOWER_NO_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, MEMBER, DIAG, M, N, ALPHA, A, AS0, AS1, B,    \
                                                               BS);                                                    \
  } else if constexpr (std::is_same_v<MODETYPE, KokkosBatched::Mode::TeamVector>) {                                    \
    KOKKOSBATCHED_TEAMVECTOR_TRSV_LOWER_NO_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, MEMBER, DIAG, M, N, ALPHA, A, AS0, AS1, \
                                                                     B, BS);                                           \
  }

#define KOKKOSBATCHED_TRSV_LOWER_TRANSPOSE_INTERNAL_INVOKE(MODETYPE, ALGOTYPE, MEMBER, DIAG, M, N, ALPHA, A, AS0, AS1, \
                                                           B, BS)                                                      \
  if constexpr (std::is_same_v<MODETYPE, KokkosBatched::Mode::Serial>) {                                               \
    KOKKOSBATCHED_SERIAL_TRSV_LOWER_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, DIAG, M, N, ALPHA, A, AS0, AS1, B, BS);        \
  } else if constexpr (std::is_same_v<MODETYPE, KokkosBatched::Mode::Team>) {                                          \
    KOKKOSBATCHED_TEAM_TRSV_LOWER_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, MEMBER, DIAG, M, N, ALPHA, A, AS0, AS1, B, BS);  \
  } else if constexpr (std::is_same_v<MODETYPE, KokkosBatched::Mode::TeamVector>) {                                    \
    KOKKOSBATCHED_TEAMVECTOR_TRSV_LOWER_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, MEMBER, DIAG, M, N, ALPHA, A, AS0, AS1, B, \
                                                                  BS);                                                 \
  }

#define KOKKOSBATCHED_TRSV_UPPER_NO_TRANSPOSE_INTERNAL_INVOKE(MODETYPE, ALGOTYPE, MEMBER, DIAG, M, N, ALPHA, A, AS0,   \
                                                              AS1, B, BS)                                              \
  if constexpr (std::is_same_v<MODETYPE, KokkosBatched::Mode::Serial>) {                                               \
    KOKKOSBATCHED_SERIAL_TRSV_UPPER_NO_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, DIAG, M, N, ALPHA, A, AS0, AS1, B, BS);     \
  } else if constexpr (std::is_same_v<MODETYPE, KokkosBatched::Mode::Team>) {                                          \
    KOKKOSBATCHED_TEAM_TRSV_UPPER_NO_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, MEMBER, DIAG, M, N, ALPHA, A, AS0, AS1, B,    \
                                                               BS);                                                    \
  } else if constexpr (std::is_same_v<MODETYPE, KokkosBatched::Mode::TeamVector>) {                                    \
    KOKKOSBATCHED_TEAMVECTOR_TRSV_UPPER_NO_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, MEMBER, DIAG, M, N, ALPHA, A, AS0, AS1, \
                                                                     B, BS);                                           \
  }

#define KOKKOSBATCHED_TRSV_UPPER_TRANSPOSE_INTERNAL_INVOKE(MODETYPE, ALGOTYPE, MEMBER, DIAG, M, N, ALPHA, A, AS0, AS1, \
                                                           B, BS)                                                      \
  if constexpr (std::is_same_v<MODETYPE, KokkosBatched::Mode::Serial>) {                                               \
    KOKKOSBATCHED_SERIAL_TRSV_UPPER_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, DIAG, M, N, ALPHA, A, AS0, AS1, B, BS);        \
  } else if constexpr (std::is_same_v<MODETYPE, KokkosBatched::Mode::Team>) {                                          \
    KOKKOSBATCHED_TEAM_TRSV_UPPER_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, MEMBER, DIAG, M, N, ALPHA, A, AS0, AS1, B, BS);  \
  } else if constexpr (std::is_same_v<MODETYPE, KokkosBatched::Mode::TeamVector>) {                                    \
    KOKKOSBATCHED_TEAMVECTOR_TRSV_UPPER_TRANSPOSE_INTERNAL_INVOKE(ALGOTYPE, MEMBER, DIAG, M, N, ALPHA, A, AS0, AS1, B, \
                                                                  BS);                                                 \
  }

#endif
