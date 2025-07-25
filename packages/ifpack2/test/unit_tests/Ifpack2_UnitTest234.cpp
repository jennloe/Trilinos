// @HEADER
// *****************************************************************************
//       Ifpack2: Templated Object-Oriented Algebraic Preconditioner Package
//
// Copyright 2009 NTESS and the Ifpack2 contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

/// \file Ifpack2_UnitTest234.cpp

/// \brief Unit test for Github Issue #234
///
/// This test ensures that the "chebyshev: boost factor" parameter of
/// Ifpack2::Chebyshev may have type either double or magnitude_type,
/// no matter what the Scalar type is.
///
/// Ifpack2::Chebyshev currently only works with real Scalar types.
/// Furthermore, this test only actually exercises the fix for #234
/// when <tt>Teuchos::ScalarTraits<Scalar>::magnitudeType !=
/// double</tt>.  Thus, this test is not actually effective unless
/// Scalar != double.  One such Scalar value suffices; we use float.
/// However, testing Scalar = double is not entirely useless, since it
/// makes sure that Ifpack2::Chebyshev takes a "chebyshev: boost
/// factor" parameter.  Thus, we build the test for both Scalar =
/// float (if enabled) and Scalar = double (if enabled).

#include "Teuchos_UnitTestHarness.hpp"
#include "Ifpack2_Chebyshev.hpp"
#include "Tpetra_CrsMatrix.hpp"
#include "Tpetra_Core.hpp"
#include "Tpetra_Map.hpp"
#include <type_traits>

namespace {  // (anonymous)

// Issue #234 only relates to the Scalar type (SC), so we only need to
// template on that.
TEUCHOS_UNIT_TEST_TEMPLATE_1_DECL(Chebyshev, Issue234, SC) {
  // We are now in a class method declared by the above macro.
  // The method has these input arguments:
  // (Teuchos::FancyOStream& out, bool& success)

  using std::endl;
  using Teuchos::RCP;
  using Teuchos::rcp;
  using Teuchos::tuple;
  typedef Tpetra::Map<>::local_ordinal_type LO;
  typedef Tpetra::Map<>::global_ordinal_type GO;
  typedef Tpetra::Map<>::node_type NT;
  typedef Tpetra::global_size_t GST;
  typedef Tpetra::CrsMatrix<SC, LO, GO, NT> crs_matrix_type;
  typedef Tpetra::RowMatrix<SC, LO, GO, NT> row_matrix_type;
  typedef Tpetra::Map<LO, GO, NT> map_type;
  typedef typename Teuchos::ScalarTraits<SC>::magnitudeType magnitude_type;
  const SC ONE = Teuchos::ScalarTraits<SC>::one();

  static_assert(!Teuchos::ScalarTraits<SC>::isComplex,
                "This test only makes sense when the Scalar type SC is real.");

  out << "Test fix for #234 (\"chebyshev: boost factor\" parameter)" << endl;
  Teuchos::OSTab tab0(out);

  out << "Create test matrix A" << endl;

  RCP<const Teuchos::Comm<int> > comm = Tpetra::getDefaultComm();

  // Create a nonzero diagonal matrix with a single row per process.
  // We won't actually do anything with it; we just need to give the
  // ContainerFactory a nontrivial matrix.

  const LO lclNumRows = 3;
  const GO gblNumRows = comm->getSize() * lclNumRows;
  const GO indexBase  = 0;

  RCP<const map_type> rowMap =
      rcp(new map_type(static_cast<GST>(gblNumRows),
                       static_cast<size_t>(lclNumRows),
                       indexBase, comm));
  RCP<const map_type> colMap = rowMap;
  RCP<const map_type> domMap = rowMap;
  RCP<const map_type> ranMap = rowMap;

  // For a diagonal matrix, we can use the row Map as the column Map.
  const size_t maxNumEntPerRow = 1;
  RCP<crs_matrix_type> A =
      rcp(new crs_matrix_type(rowMap, colMap, maxNumEntPerRow));
  for (LO lclRow = 0; lclRow < lclNumRows; ++lclRow) {
    A->insertLocalValues(lclRow, tuple(lclRow), tuple(ONE));
  }
  A->fillComplete(domMap, ranMap);

  // 2.1 is not a reasonable value of this parameter.  We just want
  // to use a value here that could not reasonably be a default
  // value.
  const double boostFactor = 2.1;

  // Create the Chebyshev instance with a matrix, and set the
  // "chebyshev: boost factor" parameter, using double as the
  // parameter's type.  Setting the parameter should not throw.
  if (!std::is_same<double, magnitude_type>::value) {
    Ifpack2::Chebyshev<row_matrix_type> prec(A);
    Teuchos::ParameterList params;

    params.set("chebyshev: boost factor", static_cast<double>(boostFactor));
    TEST_NOTHROW(prec.setParameters(params));
  }

  // Create the Chebyshev instance with a matrix, and set the
  // "chebyshev: boost factor" parameter, using magnitude_type as the
  // parameter's type.  Setting the parameter should not throw.
  {
    Ifpack2::Chebyshev<row_matrix_type> prec(A);
    Teuchos::ParameterList params;

    params.set("chebyshev: boost factor", static_cast<magnitude_type>(boostFactor));
    TEST_NOTHROW(prec.setParameters(params));
  }
}

// Define the set of unit tests to instantiate in this file.
#define UNIT_TEST_GROUP_SC(SC) \
  TEUCHOS_UNIT_TEST_TEMPLATE_1_INSTANT(Chebyshev, Issue234, SC)

#include "Ifpack2_ETIHelperMacros.h"

IFPACK2_ETI_MANGLING_TYPEDEFS()

// See discussion in file comments above to understand why we exercise
// only these two Scalar types.

#ifdef HAVE_TPETRA_INST_FLOAT
UNIT_TEST_GROUP_SC(float)
#endif  // HAVE_TPETRA_FLOAT

#ifdef HAVE_TPETRA_INST_DOUBLE
UNIT_TEST_GROUP_SC(double)
#endif  // HAVE_TPETRA_DOUBLE

}  // namespace
