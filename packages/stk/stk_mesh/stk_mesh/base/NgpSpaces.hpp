// Copyright 2002 - 2008, 2010, 2011 National Technology Engineering
// Solutions of Sandia, LLC (NTESS). Under the terms of Contract
// DE-NA0003525 with NTESS, the U.S. Government retains certain rights
// in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//
//     * Neither the name of NTESS nor the names of its contributors
//       may be used to endorse or promote products derived from this
//       software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef STK_MESH_NGPSPACES_HPP
#define STK_MESH_NGPSPACES_HPP

#include <Kokkos_Core.hpp>
#include <stk_util/ngp/NgpSpaces.hpp>

#ifdef KOKKOS_ENABLE_OPENMP
#include <Kokkos_OpenMP.hpp>
#endif

namespace stk {
namespace mesh {

using ExecSpace [[deprecated("Please use stk_util/ngp/NgpSpaces.hpp instead of stk_mesh/base/NgpSpaces.hpp")]] = stk::ngp::ExecSpace;    
using HostExecSpace [[deprecated("Please use stk_util/ngp/NgpSpaces.hpp instead of stk_mesh/base/NgpSpaces.hpp")]] = stk::ngp::HostExecSpace;    
using MemSpace [[deprecated("Please use stk_util/ngp/NgpSpaces.hpp instead of stk_mesh/base/NgpSpaces.hpp")]] = stk::ngp::MemSpace;    
using UVMMemSpace [[deprecated("Please use stk_util/ngp/NgpSpaces.hpp instead of stk_mesh/base/NgpSpaces.hpp")]] = stk::ngp::UVMMemSpace;    
using HostPinnedSpace [[deprecated("Please use stk_util/ngp/NgpSpaces.hpp instead of stk_mesh/base/NgpSpaces.hpp")]] = stk::ngp::HostPinnedSpace;    
using ScheduleType [[deprecated("Please use stk_util/ngp/NgpSpaces.hpp instead of stk_mesh/base/NgpSpaces.hpp")]] = stk::ngp::ScheduleType;    

}
}

#endif
