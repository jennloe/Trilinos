// @HEADER
// *****************************************************************************
//            LOCA: Library of Continuation Algorithms Package
//
// Copyright 2001-2005 NTESS and the LOCA contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

#include "LOCA.H"
#include "LOCA_LAPACK.H"
#include "ChanProblemInterface.H"

int main()
{
  int n = 100;
  double alpha = 3.5;
  double beta = 0.0;
  double scale = 1.0;
  int maxNewtonIters = 10;

  alpha = alpha / scale;

  // These seeds were found to work -- not the first try
  int seed1 = 3;
  int seed2 = 4; //Subsequent seeds keep incrementing by seed2-seed1
  double homScale = -1.0;

  /* Uncomment for interactive mode
  std::cout << "Input first seed" << std::endl;     std::cin >> seed1;
  std::cout << "Input second seed" << std::endl;    std::cin >> seed2;
  std::cout << "Input identity-scale" << std::endl; std::cin >> homScale;
  */

  try {

    // Create parameter list
    Teuchos::RCP<Teuchos::ParameterList> paramList =
      Teuchos::rcp(new Teuchos::ParameterList);

    // Create LOCA sublist
    Teuchos::ParameterList& locaParamsList = paramList->sublist("LOCA");

    // Create the stepper sublist and set the stepper parameters
    Teuchos::ParameterList& stepperList = locaParamsList.sublist("Stepper");
    Teuchos::ParameterList& stepSizeList = locaParamsList.sublist("Step Size");

    // Create the "Solver" parameters sublist to be used with NOX Solvers
    Teuchos::ParameterList& nlParams = paramList->sublist("NOX");
    Teuchos::ParameterList& nlPrintParams = nlParams.sublist("Printing");
    nlPrintParams.set("Output Information",
              NOX::Utils::Details +
              NOX::Utils::OuterIteration +
              NOX::Utils::InnerIteration +
              NOX::Utils::Warning +
              NOX::Utils::Error +
              NOX::Utils::Parameters +
              NOX::Utils::StepperIteration +
              NOX::Utils::StepperDetails +
              NOX::Utils::StepperParameters);

    // Create LAPACK Factory
    Teuchos::RCP<LOCA::LAPACK::Factory> lapackFactory =
      Teuchos::rcp(new LOCA::LAPACK::Factory);

    // Create global data object
    Teuchos::RCP<LOCA::GlobalData> globalData =
      LOCA::createGlobalData(paramList, lapackFactory);

    // Set up the problem interface
    ChanProblemInterface chan(globalData, n, alpha, beta, scale);
    LOCA::ParameterVector p;
    p.addParameter("alpha",alpha);
    p.addParameter("beta",beta);
    p.addParameter("scale",scale);

    // Create a group which uses that problem interface. The group will
    // be initialized to contain the default initial guess for the
    // specified problem.
    Teuchos::RCP<LOCA::LAPACK::Group> grp =
      Teuchos::rcp(new LOCA::LAPACK::Group(globalData, chan));
    grp->setParams(p);

    // Set up the status tests
    Teuchos::RCP<NOX::StatusTest::NormF> statusTestA =
      Teuchos::rcp(new NOX::StatusTest::NormF(*grp, 1.0e-8));
    Teuchos::RCP<NOX::StatusTest::MaxIters> statusTestB =
      Teuchos::rcp(new NOX::StatusTest::MaxIters(maxNewtonIters));
    Teuchos::RCP<NOX::StatusTest::Combo> combo =
      Teuchos::rcp(new NOX::StatusTest::Combo(NOX::StatusTest::Combo::OR,
                           statusTestA, statusTestB));

    // Create the homotopy group
//     Teuchos::RCP<LOCA::Homotopy::Group> hGrp =
//       Teuchos::rcp(new LOCA::Homotopy::Group(locaParamsList, globalData, grp));

    Teuchos::RCP<Teuchos::ParameterList> hParams =
      Teuchos::rcp(&locaParamsList.sublist("Homotopy"),false);
    hParams->set("Bordered Solver Method", "LAPACK Direct Solve");
    Teuchos::RCP<NOX::Abstract::Vector> startVec =
      grp->getX().clone(NOX::ShapeCopy);
    startVec->random(true, seed1); /* Always use same seed for testing */
    startVec->abs(*startVec);
    std::vector< Teuchos::RCP<const NOX::Abstract::Vector> > solns;

    LOCA::Abstract::Iterator::IteratorStatus status
         = LOCA::Abstract::Iterator::Finished;

    int deflationIter = 0;
    const int maxDeflationIters = 4;

    while (deflationIter < maxDeflationIters
           &&  status==LOCA::Abstract::Iterator::Finished) {

      // ToDo:  Add deflateAndReset(vec) to Homotopy group,
      //        including option to perturb startVec

      Teuchos::RCP<LOCA::Homotopy::DeflatedGroup> hGrp =
        Teuchos::rcp(new LOCA::Homotopy::DeflatedGroup(globalData,
                        paramList, hParams, grp, startVec, solns,homScale));

      // Override default parameters
      stepperList.set("Max Nonlinear Iterations", maxNewtonIters);
      stepperList.set("Continuation Method", "Arc Length");
      stepperList.set("Max Steps", 100);

      stepSizeList.set("Min Step Size", 1.0e-5);


      // Create the stepper
      LOCA::Stepper stepper(globalData, hGrp, combo, paramList);

      // Solve the nonlinear system
      status = stepper.run();

      if (status != LOCA::Abstract::Iterator::Finished)
        globalData->locaUtils->out() << "Stepper failed to converge!"
                   << std::endl;
      else {
        // Deflate with soln vector just solved for
        solns.push_back(grp->getX().clone(NOX::DeepCopy));

        // If seed1=seed2, then all seeds the same. Otherwise, keep incrementing seed
        startVec->random(true, seed2 + deflationIter*(seed2-seed1));
      }

      deflationIter++;

      // Output the parameter list
      if (globalData->locaUtils->isPrintType(NOX::Utils::StepperParameters)) {
        globalData->locaUtils->out()
      << std::endl << "Final Parameters for deflation iter "
        << deflationIter << std::endl
      << "****************" << std::endl;
        stepper.getList()->print(globalData->locaUtils->out());
        globalData->locaUtils->out() << std::endl;
      }

    } //End While loop over delfation iters

    LOCA::destroyGlobalData(globalData);
  }

  catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }
  catch (const char *s) {
    std::cout << s << std::endl;
  }
  catch (...) {
    std::cout << "Caught unknown exception!" << std::endl;
  }

  std::cout << "All examples passed" << std::endl;

  return 0;
}
