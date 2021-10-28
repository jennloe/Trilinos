//@HEADER
// ************************************************************************
//
//                 Belos: Block Linear Solvers Package
//                  Copyright 2004 Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Jennifer A. Loe (jloe@sandia.gov)
//
// ************************************************************************
//@HEADER
//
// This driver reads a matrix from a file, uses the corresponding map to 
// create a set of random multivectors, and then orthogonalizes the 
// multivectors. 
//
//
#include "BelosConfigDefs.hpp"
#include "BelosOutputManager.hpp"

#include "Teuchos_CommandLineProcessor.hpp"
#include "Teuchos_ParameterList.hpp"
#include "Teuchos_StandardCatchMacros.hpp"
#include <Teuchos_StackedTimer.hpp>

#include "BelosTpetraAdapter.hpp"
#include "BelosMultiVecTraits_Tpetra.hpp"
#include "BelosTpetraOperator.hpp"
#include "Tpetra_Core.hpp"
#include "Tpetra_CrsMatrix.hpp"
#include <MatrixMarket_Tpetra.hpp>
#include "BelosOrthoManagerFactory.hpp"
#include "BelosOrthoManager.hpp"

using Teuchos::RCP;
using Teuchos::rcp;
using Teuchos::Array;

//Function Forward Declaration:
template <class ScalarType>
RCP<Teuchos::SerialDenseMatrix<int, ScalarType>> orthogTpMVecs(Tpetra::MultiVector<ScalarType> & inputVecs, std::string orthogType, int blkSize);

int main(int argc, char *argv[]) {
  Tpetra::ScopeGuard tpetraScope(&argc,&argv);
  {
  typedef double                            ScalarType; 
  typedef int                               OT;
  typedef Tpetra::MultiVector<ScalarType>   MV;
  typedef Tpetra::MultiVector<ScalarType>::mag_type   MT;
  typedef Belos::MultiVecTraits<ScalarType,MV>     MVT;
  typedef Teuchos::SerialDenseMatrix<OT,ScalarType> MAT;

  RCP<const Teuchos::Comm<int> > comm = Tpetra::getDefaultComm();
  int MyPID = Teuchos::rank(*comm);
  int numVecs = -1; 

  bool verbose = true;
  bool proc_verbose = ( verbose && (MyPID==0) ); /* Only print on the zero processor */
  int blockSize = 50; 
  std::string filename("orsirr_1.mtx"); // example matrix
  std::string orthoType("ICGS");
  bool use_stacked_timer = false;              

  Teuchos::CommandLineProcessor cmdp(false,true);
  cmdp.setOption("verbose","quiet",&verbose,"Print messages and results.");
  cmdp.setOption("filename",&filename,"Filename for test matrix.  Matrix market format only.");
  cmdp.setOption("ortho", &orthoType, "Type of orthogonalization: ICGS, IMGS, DGKS.");
  cmdp.setOption("blkSize",&blockSize,"Number of vectors to orthogonalize at each step. -1 indicates to use the number of rows of the input matrix."); 
  cmdp.setOption("numVecs",&numVecs,"Total number of vectors for tester to orthogonalize.");
  cmdp.setOption("stacked-timer", "no-stacked-timer", &use_stacked_timer, "Run with or without stacked timer output");

  if (cmdp.parse(argc,argv) != Teuchos::CommandLineProcessor::PARSE_SUCCESSFUL) {
    return -1;
  }

  // Create the timer.
  RCP<std::ostream> outputStream = rcp(&std::cout,false);
  RCP<Belos::OutputManager<ScalarType> > printer_ = rcp( new Belos::OutputManager<ScalarType>(Belos::TimingDetails,outputStream) );
  std::string OrthoLabel = "Total Orthog time:";
#ifdef BELOS_TEUCHOS_TIME_MONITOR
    RCP<Teuchos::Time> timerOrtho_ = Teuchos::TimeMonitor::getNewCounter(OrthoLabel);
#endif

  // Set output stream and stacked timer:
  RCP<Teuchos::FancyOStream> fancy = Teuchos::fancyOStream(Teuchos::rcpFromRef(std::cout));
  Teuchos::FancyOStream& out = *fancy;
  out.setOutputToRootOnly(0);
  // Set up timers
  Teuchos::RCP<Teuchos::StackedTimer> stacked_timer;
  if (use_stacked_timer){
    stacked_timer = rcp(new Teuchos::StackedTimer("Main"));
  }
  Teuchos::TimeMonitor::setStackedTimer(stacked_timer);
  
  //-----------------------------------------------------------
  // This code sets up a random multivec to orthogonalize.
  // ----------------------------------------------------------
  //Read CrsMats into Tpetra::Operator
  RCP<Tpetra::CrsMatrix<ScalarType>> A = Tpetra::MatrixMarket::Reader<Tpetra::CrsMatrix<ScalarType>>::readSparseFile(filename,comm);
  RCP<const Tpetra::Map<> > map = A->getDomainMap();
  int N = A->getGlobalNumRows();
  
  if(numVecs == -1){
    numVecs = N;
  }

  RCP<Tpetra::MultiVector<ScalarType>> X1 = rcp( new Tpetra::MultiVector<ScalarType>(map, numVecs) );
  X1->randomize();
  RCP<Tpetra::MultiVector<ScalarType>> XCopy = rcp(new Tpetra::MultiVector<ScalarType>(*X1, Teuchos::Copy)); //Deep copy of X1.
  //-----------------------------------------------------------
  // End random multivec setup.
  //-----------------------------------------------------------

  RCP<MAT> coeffMat;
  { //scope guard for timer
  #ifdef BELOS_TEUCHOS_TIME_MONITOR
  Teuchos::TimeMonitor orthotimer(*timerOrtho_);
  #endif
  coeffMat = orthogTpMVecs(*X1, orthoType, blockSize);
  }

  /*// DEBUG: //Verify Orthog:
  RCP<MAT> bigDotAns  = rcp( new MAT(N,N));
  MVT::MvTransMv(1.0, *X1, *X1, *bigDotAns);
  std::cout << "Printed dot prod matrix for verification: " << std::endl;
  bigDotAns->print(std::cout);
  std::cout << std::endl << std::endl;*/

  /*// DEBUG: //Verify coefficients: 
  // (Due to a Kokkos bug, this check crashes if numVecs > 180).  
  MV coeffs_mv = makeStaticLocalMultiVector (*X1, numVecs, numVecs);
  Tpetra::deep_copy(coeffs_mv, *coeffMat);
  XCopy->multiply(Teuchos::NO_TRANS, Teuchos::NO_TRANS, 1.0, *X1, coeffs_mv, -1.0);
  std::vector<MT> norms(numVecs);
  Teuchos::ArrayView<MT> normView(norms);
  XCopy->norm2(normView);
  std::cout << "Here are the QR-Orig norms.  Should be zero." << std::endl;
  std::cout << normView << std::endl << std::endl;*/


  //Print final timing details:
  Teuchos::TimeMonitor::summarize( printer_->stream(Belos::TimingDetails) );

    if (use_stacked_timer) {
      stacked_timer->stop("Main");
      Teuchos::StackedTimer::OutputOptions options;
      options.output_fraction = options.output_histogram = options.output_minmax = true;
      stacked_timer->report(out, comm, options);
    }

  }// End Tpetra Scope Guard
  return 0;
}

template <class ScalarType>
RCP<Teuchos::SerialDenseMatrix<int, ScalarType>> orthogTpMVecs(Tpetra::MultiVector<ScalarType> & inputVecs, std::string orthogType, int blkSize){
  typedef int                               OT;
  typedef typename Teuchos::SerialDenseMatrix<OT,ScalarType> MAT;
  typedef Tpetra::MultiVector<ScalarType>   MV;
  typedef Tpetra::Operator<ScalarType>             OP;
  int numVecs = inputVecs.getNumVectors();
  int numRows = inputVecs.getGlobalLength();

  //Default OutputManager is std::cout.
  Teuchos::RCP<Belos::OutputManager<ScalarType> > myOutputMgr = Teuchos::rcp( new Belos::OutputManager<ScalarType>() );

  //Check that block size is not bigger than total num vecs.
  if(blkSize > numVecs){
    blkSize = numVecs;
  }
  int numLoops = numVecs/blkSize;
  int remainder = numVecs % blkSize;

  RCP<MAT> Coeffs = rcp(new MAT(numVecs,numVecs));
  RCP<MAT> coeffDot;
  std::vector<RCP<const MV>> pastVecArray; // Stores vectors after orthogonalized
  Teuchos::ArrayView<RCP<const MV>> pastVecArrayView;  // To hold the above

  // Create the orthogonalization manager:
  Belos::OrthoManagerFactory<ScalarType, MV, OP> factory;
  Teuchos::RCP<Teuchos::ParameterList> paramsOrtho;   // can be null
  const Teuchos::RCP<Belos::OrthoManager<ScalarType,MV>> orthoMgr = 
                    factory.makeOrthoManager (orthogType, Teuchos::null, myOutputMgr, "Tpetra OrthoMgr", paramsOrtho); 
  
  // Get a view of the first block and normalize: (also orthogonalizes these vecs wrt themselves)
  RCP<MV> vecBlock = inputVecs.subViewNonConst(Teuchos::Range1D(0,blkSize-1));
  RCP<MAT> coeffNorm = Teuchos::rcp( new MAT(Teuchos::View, *Coeffs, blkSize, blkSize));
  RCP<MV> pastVecBlock;
  orthoMgr->normalize(*vecBlock, coeffNorm);
  pastVecArray.push_back(vecBlock);
  
  // Loop over remaining blocks:
  for(int k=1; k<numLoops; k++){
    pastVecArrayView = arrayViewFromVector(pastVecArray);
    vecBlock = inputVecs.subViewNonConst(Teuchos::Range1D(k*blkSize,k*blkSize + blkSize - 1));
    coeffNorm = Teuchos::rcp( new MAT(Teuchos::View, *Coeffs, blkSize, blkSize, k*blkSize, k*blkSize)); 
    coeffDot = Teuchos::rcp(new MAT(Teuchos::View, *Coeffs, k*blkSize, blkSize, 0, k*blkSize));
    Array<RCP<MAT>> dotArray; //Tuechos::Array of Matrices for coeffs from dot products
    dotArray.append(coeffDot);
    int rank = orthoMgr->projectAndNormalize(*vecBlock, dotArray, coeffNorm, pastVecArrayView);
    pastVecBlock = inputVecs.subViewNonConst(Teuchos::Range1D(0,k*blkSize + blkSize - 1));
    pastVecArray.front() = pastVecBlock;
  }
  if( remainder > 0){
    pastVecArrayView = arrayViewFromVector(pastVecArray);
    vecBlock = inputVecs.subViewNonConst(Teuchos::Range1D(numVecs-remainder, numVecs-1));
    coeffNorm = Teuchos::rcp( new MAT(Teuchos::View, *Coeffs, remainder, remainder, numLoops*blkSize, numLoops*blkSize)); 
    coeffDot = Teuchos::rcp(new MAT(Teuchos::View, *Coeffs, numLoops*blkSize, remainder, 0, numLoops*blkSize));
    Array<RCP<MAT>> dotArray; //Tuechos::Array of Matrices for coeffs from dot products
    dotArray.append(coeffDot);
    int rank = orthoMgr->projectAndNormalize(*vecBlock, dotArray, coeffNorm, pastVecArrayView);
  }

  return Coeffs;
}
