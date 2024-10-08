// @HEADER
// *****************************************************************************
//                           Intrepid2 Package
//
// Copyright 2007 NTESS and the Intrepid2 contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

/** \file   Intrepid2_HierarchicalBasis_HDIV_PYR_ETI.cpp
    \brief  \brief ETI instantiation for Intrepid2 HierarchicalBasis_HDIV_PYR class.
    \author Created by N.V. Roberts.
 */

#include "Intrepid2_HierarchicalBasis_HDIV_PYR.hpp"

using DefaultDeviceType = Kokkos::DefaultExecutionSpace::device_type;
template class Intrepid2::HierarchicalBasis_HDIV_PYR<Kokkos::DefaultExecutionSpace::device_type,double,double>;
