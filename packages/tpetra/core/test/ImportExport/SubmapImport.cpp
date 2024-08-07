// @HEADER
// *****************************************************************************
//          Tpetra: Templated Linear Algebra Services Package
//
// Copyright 2008 NTESS and the Tpetra contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

#include <Tpetra_ConfigDefs.hpp>

#include <Teuchos_UnitTestHarness.hpp>
#include <Teuchos_Tuple.hpp>
#include <Teuchos_RCP.hpp>
#include <Teuchos_DefaultComm.hpp>
#include <Teuchos_VerboseObject.hpp>
#include <Teuchos_FancyOStream.hpp>

#include <Tpetra_Core.hpp>
#include <Tpetra_Map.hpp>
#include <Tpetra_Vector.hpp>
#include <Tpetra_Import.hpp>

#include <iostream>

using Teuchos::ArrayView;
using Teuchos::FancyOStream;
using Teuchos::getFancyOStream;
using Teuchos::RCP;
using Teuchos::rcpFromRef;
using Teuchos::toString;
using Teuchos::tuple;

RCP<Tpetra::Vector<int> >
TestTpetra (const Teuchos::ArrayView<const Tpetra::Map<>::global_ordinal_type>& srcGID,
            const Teuchos::ArrayView<const Tpetra::Map<>::global_ordinal_type>& destGID)
{
  typedef Tpetra::Map<int> map_type;
  typedef map_type::global_ordinal_type GO;
  typedef Tpetra::global_size_t GST;
  using std::endl;

  RCP<FancyOStream> perr = getFancyOStream (rcpFromRef (std::cerr));
  FancyOStream& err = *perr;
  RCP<const Teuchos::Comm<int> > comm = Teuchos::DefaultComm<int>::getComm ();
  const int myRank = comm->getRank ();

  {
    std::ostringstream os;
    os << "Proc " << myRank << ": {srcGID: " << toString (srcGID)
       << ", destGID: " << toString (destGID) << "}" << endl;
    err << os.str ();
  }

  const GO indexBase = 0;
  const GST INV = Teuchos::OrdinalTraits<GST>::invalid ();
  RCP<const map_type> srcMap (new map_type (INV, srcGID (), indexBase, comm));
  {
    std::ostringstream os;
    os << "Proc " << myRank << ": created srcMap" << endl;
    err << os.str ();
  }

  RCP<const map_type> destMap (new map_type (INV, destGID (), indexBase, comm));
  {
    std::ostringstream os;
    os << "Proc " << myRank << ": created destMap" << endl;
    err << os.str ();
  }

  Tpetra::Vector<int> srcVector (srcMap);
  RCP<Tpetra::Vector<int> > destVector (new Tpetra::Vector<int> (destMap));
  {
    std::ostringstream os;
    os << "Proc " << myRank << ": created vectors" << endl;
    err << os.str ();
  }
  destVector->putScalar (-1);

  Tpetra::Import<> importer (srcMap, destMap);
  {
    std::ostringstream os;
    os << "Proc " << myRank << ": created Import" << endl;
    err << os.str ();
  }
  destVector->doImport (srcVector, importer, Tpetra::INSERT);
  {
    std::ostringstream os;
    os << "Proc " << myRank << ": Finished doImport" << endl;
    err << os.str ();
  }

  // Teuchos::FancyOStream out(Teuchos::rcp(&std::cout,false));
  // destVector->describe(out, Teuchos::VERB_EXTREME);

  return destVector;
}


TEUCHOS_UNIT_TEST( DistObject, SubMapImport1 )
{
  typedef Tpetra::Map<>::global_ordinal_type GO;

  RCP<const Teuchos::Comm<int> > comm = Tpetra::getDefaultComm();
  const int MyPid = comm->getRank();
  /**********************************************************************************/
  // Maps of the import operation:
  // -----------------------------
  // SRC Map  Processor 0: Global IDs = 0 1 2 3 4 5 6
  //          Processor 1: Global IDs =                    9 10 11 12 13 14 15
  //
  // DEST Map Processor 0: Global IDs = 0 1 2 3 4 5 6 7 8
  //          Processor 1: Global IDs =               7 8 9 10 11 12 13 14 15
  //
  //
  // Vectors before import operation:
  // --------------------------------
  // srcVector  = [ 0  0  ...  0 ]
  // destVector = [-1 -1  ... -1 ]
  //
  // Expected result:
  // ----------------
  // destVector Processor 0: Values = [ 0 0 0 0 0 0 0 -1 -1 ]
  //            Processor 1: Values =               [ -1 -1 0 0 0 0 0 0 0 ]
  RCP<Tpetra::Vector<int> > destVector;
  if (MyPid == 0) {
    TEST_NOTHROW( destVector = TestTpetra(tuple<GO>(0,1,2,3,4,5,6), tuple<GO>(0,1,2,3,4,5,6,7,8) ) );
    TEST_COMPARE_ARRAYS( tuple<int>(0,0,0,0,0,0,0,-1,-1), destVector->get1dView() )
  }
  else {
    TEST_NOTHROW( destVector = TestTpetra(tuple<GO>(9,10,11,12,13,14,15), tuple<GO>(7,8,9,10,11,12,13,14,15) ) );
    TEST_COMPARE_ARRAYS( tuple<int>(-1,-1,0,0,0,0,0,0,0), destVector->get1dView() )
  }

  // Process 0 is responsible for printing the "SUCCESS" / "PASSED"
  // message, so without the barrier, it's possible for the test to be
  // reported as passing, even if the other processes crashed or hung.
  comm->barrier ();
}

////
TEUCHOS_UNIT_TEST( DistObject, SubMapImport2 )
{
  using GO = Tpetra::Map<>::global_ordinal_type;

  RCP<const Teuchos::Comm<int> > comm = Tpetra::getDefaultComm();
  const int MyPid = comm->getRank();
  /**********************************************************************************/
  // SRC Map  Processor 0: Global IDs =
  //          Processor 1: Global IDs = 0 1
  //
  // DEST Map Processor 0: Global IDs = 0 1 2
  //          Processor 1: Global IDs =   1 2
  //
  // Vectors before import operation:
  // --------------------------------
  // srcVector  = [] [0  0]
  // destVector = [-1 -1 -1] [-1 -1]
  //
  // Expected result:
  // destVector Processor 0: Values = 0 0 -1
  //            Processor 1: Values =   0 -1
  RCP<Tpetra::Vector<int,int> > destVector;
  if (MyPid == 0) {
    TEST_NOTHROW( destVector = TestTpetra(ArrayView<GO>(), tuple<GO>(0,1,2) ) )
    TEST_COMPARE_ARRAYS( tuple<int>(0,0,-1), destVector->get1dView() )
  }
  else {
    TEST_NOTHROW( destVector = TestTpetra(tuple<GO>(0,1), tuple<GO>(1,2) ) )
    TEST_COMPARE_ARRAYS( tuple<int>(0,-1), destVector->get1dView() )
  }

  // Process 0 is responsible for printing the "SUCCESS" / "PASSED"
  // message, so without the barrier, it's possible for the test to be
  // reported as passing, even if the other processes crashed or hung.
  comm->barrier ();
}

////
TEUCHOS_UNIT_TEST( DistObject, SubMapImport3 )
{
  using GO = Tpetra::Map<>::global_ordinal_type;

  RCP<const Teuchos::Comm<int> > comm = Tpetra::getDefaultComm();
  const int MyPid = comm->getRank();
  /**********************************************************************************/
  // SRC Map  Processor 0: Global IDs =
  //          Processor 1: Global IDs =
  //
  // DEST Map Processor 0: Global IDs = 0 1
  //          Processor 1: Global IDs = 0 1
  //
  // Vectors before import operation:
  // --------------------------------
  // srcVector  = []
  // destVector = -1 -1
  //
  // Expected result:
  // destVector Processor 0: Values = -1 -1
  //            Processor 1: Values = -1 -1
  RCP<Tpetra::Vector<int,int> > destVector;
  if (MyPid == 0) {
    TEST_NOTHROW( destVector = TestTpetra(ArrayView<GO>(), tuple<GO>(0,1) ) )
  }
  else {
    TEST_NOTHROW( destVector = TestTpetra(ArrayView<GO>(), tuple<GO>(0,1) ) )
  }
  TEST_COMPARE_ARRAYS( tuple<int>(-1,-1), destVector->get1dView() )

  // Process 0 is responsible for printing the "SUCCESS" / "PASSED"
  // message, so without the barrier, it's possible for the test to be
  // reported as passing, even if the other processes crashed or hung.
  comm->barrier ();
}


