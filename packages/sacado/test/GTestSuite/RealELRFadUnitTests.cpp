// @HEADER
// *****************************************************************************
//                           Sacado Package
//
// Copyright 2006 NTESS and the Sacado contributors.
// SPDX-License-Identifier: LGPL-2.1-or-later
// *****************************************************************************
// @HEADER

#include "FadUnitTests2.hpp"

typedef ::testing::Types<
  Sacado::ELRFad::DFad<double>,
  Sacado::ELRFad::SFad<double,5>,
  Sacado::ELRFad::SLFad<double,5>
  > FadTypes;

INSTANTIATE_TYPED_TEST_SUITE_P(RealELRFad, FadOpsUnitTest2, FadTypes);
INSTANTIATE_TYPED_TEST_SUITE_P(RealELRFad, RealFadOpsUnitTest2, FadTypes);
