// @HEADER
// *****************************************************************************
//                           Stokhos Package
//
// Copyright 2009 NTESS and the Stokhos contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

#include "Teuchos_UnitTestHarness.hpp"
#include "Teuchos_TestingHelpers.hpp"
#include "Teuchos_UnitTestRepository.hpp"
#include "Teuchos_GlobalMPISession.hpp"

#include "Stokhos.hpp"
#include "Stokhos_UnitTestHelpers.hpp"
#include "Stokhos_HouseTriDiagPCEBasis.hpp"

// Quadrature functor to be passed into quadrature expansion for mapping
// from Lanczos basis back to original PCE
struct lanczos_pce_quad_func {
  lanczos_pce_quad_func(const Stokhos::OrthogPolyApprox<int,double>& pce_,
			  const Stokhos::OrthogPolyBasis<int,double>& basis_) :
    pce(pce_), basis(basis_), vec(1) {}
  
  double operator() (const double& a) const {
    vec[0] = a;
    return pce.evaluate(vec);
  }
  const Stokhos::OrthogPolyApprox<int,double>& pce;
  const Stokhos::OrthogPolyBasis<int,double>& basis;
  mutable Teuchos::Array<double> vec;
};

// Class encapsulating setup of the Lanczos basis for a given PCE
// u = Func(x), where Func is specified by a template-parameter
template <typename Func>
struct Lanczos_PCE_Setup {
  typedef typename Func::OrdinalType OrdinalType;
  typedef typename Func::ValueType ValueType;
  ValueType rtol, atol;
  Func func;
  OrdinalType sz, st_sz;
  Teuchos::RCP<const Stokhos::CompletePolynomialBasis<OrdinalType,ValueType> > basis;
  Teuchos::RCP< Stokhos::QuadOrthogPolyExpansion<OrdinalType,ValueType> > exp;
  Teuchos::RCP<const Stokhos::HouseTriDiagPCEBasis<OrdinalType,ValueType> > st_1d_proj_basis;
  Teuchos::Array< Teuchos::RCP<const Stokhos::OneDOrthogPolyBasis<OrdinalType,ValueType> > > st_bases;
  Teuchos::RCP<const Stokhos::CompletePolynomialBasis<OrdinalType,ValueType> > st_basis;
  Teuchos::RCP<const Stokhos::Quadrature<OrdinalType,ValueType> > st_quad;
  Stokhos::OrthogPolyApprox<OrdinalType,ValueType> u, v, u_st, v_st;
  
  Lanczos_PCE_Setup(bool normalize) : 
    func()
  {
    rtol = 1e-8;
    atol = 1e-12;
    const OrdinalType d = 3;
    const OrdinalType p = 5;
    
    // Create product basis
    Teuchos::Array< Teuchos::RCP<const Stokhos::OneDOrthogPolyBasis<OrdinalType,ValueType> > > bases(d);
    for (OrdinalType i=0; i<d; i++)
      bases[i] = 
	Teuchos::rcp(new Stokhos::LegendreBasis<OrdinalType,ValueType>(p, true));
    basis =
      Teuchos::rcp(new Stokhos::CompletePolynomialBasis<OrdinalType,ValueType>(bases));
    
    // Create approximation
    sz = basis->size();
    Stokhos::OrthogPolyApprox<OrdinalType,ValueType> x(basis);
    for (OrdinalType i=0; i<d; i++)
      x.term(i, 1) = 1.0;
    
    // Tensor product quadrature
    Teuchos::RCP<const Stokhos::Quadrature<OrdinalType,ValueType> > quad = 
      Teuchos::rcp(new Stokhos::TensorProductQuadrature<OrdinalType,ValueType>(basis, 4*p));

    // Triple product tensor
    Teuchos::RCP<Stokhos::Sparse3Tensor<int,double> > Cijk =
      basis->computeTripleProductTensor(basis->size());
    
    // Quadrature expansion
    exp = Teuchos::rcp(new Stokhos::QuadOrthogPolyExpansion<OrdinalType,ValueType>(basis, Cijk, quad));
    
    // Compute PCE via quadrature expansion
    u.reset(basis);
    v.reset(basis);
    func.eval(*exp, x, u);
    exp->times(v,u,u);
    
    // Compute Lanczos basis
    st_bases.resize(1);
    st_1d_proj_basis = 
      Teuchos::rcp(new Stokhos::HouseTriDiagPCEBasis<OrdinalType,ValueType>(
		     p, u, *Cijk));
    st_bases[0] = st_1d_proj_basis;
    
    st_basis = 
      Teuchos::rcp(new Stokhos::CompletePolynomialBasis<OrdinalType,ValueType>(st_bases, 1e-15));
    st_sz = st_basis->size();
    u_st.reset(st_basis);
    v_st.reset(st_basis);
    u_st[0] = st_1d_proj_basis->getNewCoeffs(0);
    u_st[1] = st_1d_proj_basis->getNewCoeffs(1);
    
    // Tensor product quadrature
    st_quad = 
      Teuchos::rcp(new Stokhos::TensorProductQuadrature<OrdinalType,ValueType>(st_basis));

    // Triple product tensor
    Teuchos::RCP<Stokhos::Sparse3Tensor<int,double> > st_Cijk =
      st_basis->computeTripleProductTensor(st_basis->size());
    
    // Quadrature expansion
    Stokhos::QuadOrthogPolyExpansion<OrdinalType,ValueType> st_exp(st_basis, 
								   st_Cijk,
								   st_quad);
    
    st_exp.times(v_st, u_st, u_st);
  }
  
};

#define LANCZOS_UNIT_TESTS(BASENAME, TAG, FUNC, NORMALIZE)		\
namespace BASENAME ## TAG {						\
									\
  Lanczos_PCE_Setup< FUNC<int,double> > setup(NORMALIZE);		\
									\
  TEUCHOS_UNIT_TEST( BASENAME, TAG ## Map ) {				\
   Stokhos::OrthogPolyApprox<int,double> u2(setup.basis);		\
   setup.st_1d_proj_basis->transformCoeffsFromHouse(			\
     setup.u_st.coeff(),						\
     u2.coeff());							\
   success = Stokhos::comparePCEs(setup.u, "u", u2, "u2",		\
				  setup.rtol, setup.atol, out);		\
  }									\
									\
  TEUCHOS_UNIT_TEST( BASENAME, TAG ## Orthog ) {			\
    const Teuchos::Array<double>& norms =				\
      setup.st_bases[0]->norm_squared();				\
    const Teuchos::Array<double>& weights =				\
      setup.st_quad->getQuadWeights();					\
    const Teuchos::Array< Teuchos::Array<double> >& values =		\
      setup.st_quad->getBasisAtQuadPoints();				\
    Teuchos::SerialDenseMatrix<int,double> mat(setup.st_sz,             \
                                               setup.st_sz);            \
    for (int i=0; i<setup.st_sz; i++) {					\
      for (int j=0; j<setup.st_sz; j++) {				\
	for (unsigned int k=0; k<weights.size(); k++)			\
	  mat(i,j) += weights[k]*values[k][i]*values[k][j];		\
	mat(i,j) /= std::sqrt(norms[i]*norms[j]);			\
      }									\
      mat(i,i) -= 1.0;							\
    }									\
    success = mat.normInf() < setup.atol;				\
    if (!success) {							\
      out << "\n Error, mat.normInf() < atol = " << mat.normInf()	\
	  << " < " << setup.atol << ": failed!\n";			\
      out << "mat = " << mat << std::endl;				\
    }									\
  }									\
									\
  TEUCHOS_UNIT_TEST( BASENAME, TAG ## PCE ) {				\
    Stokhos::OrthogPolyApprox<int,double> v2(setup.basis);		\
    lanczos_pce_quad_func quad_func(setup.v_st, *setup.st_basis);	\
    setup.exp->unary_op(quad_func, v2, setup.u);			\
    success = comparePCEs(setup.v, "v", v2, "v2", setup.rtol,		\
			  setup.atol, out);				\
  }									\
									\
  TEUCHOS_UNIT_TEST( BASENAME, TAG ## Mean ) {				\
    success = Teuchos::testRelErr(					\
      "v.mean()", setup.v.mean(),					\
      "v_st.mean()", setup.v_st.mean(),					\
      "rtol", setup.rtol,						\
      "rtol", setup.rtol,						\
      Teuchos::Ptr<std::ostream>(out.getOStream().get()));		\
									\
  }									\
									\
  TEUCHOS_UNIT_TEST( BASENAME, TAG ## StandardDeviation ) {		\
    success = Teuchos::testRelErr(					\
      "v.standard_deviation()",						\
      setup.v.standard_deviation(),					\
      "v_st.standard_devaition()",					\
      setup.v_st.standard_deviation(),					\
      "rtol", 1e-1,							\
      "rtol", 1e-1,							\
      Teuchos::Ptr<std::ostream>(out.getOStream().get()));		\
  }									\
									\
}

//
// Lanczos tests based on expansion of u = cos(x) where x is a U([-1,1])
// random variable
//
template <typename Ordinal_Type, typename Value_Type>
struct Lanczos_Cos_Func {
  typedef Ordinal_Type OrdinalType;
  typedef Value_Type ValueType;
  static const bool is_even = true;
  void 
  eval(Stokhos::QuadOrthogPolyExpansion<OrdinalType,ValueType>& exp,
       const Stokhos::OrthogPolyApprox<OrdinalType,ValueType>& x, 
       Stokhos::OrthogPolyApprox<OrdinalType,ValueType>& u) {
    exp.cos(u,x);
  }
};
LANCZOS_UNIT_TESTS(Stokhos_LanczosPCEBasis_Proj, Cos, Lanczos_Cos_Func, 
		   false)
  
//
// Lanczos tests based on expansion of u = sin(x) where x is a U([-1,1])
// random variable
//
template <typename Ordinal_Type, typename Value_Type>
struct Lanczos_Sin_Func {
  typedef Ordinal_Type OrdinalType;
  typedef Value_Type ValueType;
  static const bool is_even = false;
  void 
  eval(Stokhos::QuadOrthogPolyExpansion<OrdinalType,ValueType>& exp,
       const Stokhos::OrthogPolyApprox<OrdinalType,ValueType>& x, 
       Stokhos::OrthogPolyApprox<OrdinalType,ValueType>& u) {
    exp.sin(u,x);
  }
};
LANCZOS_UNIT_TESTS(Stokhos_LanczosPCEBasis_Proj, Sin, Lanczos_Sin_Func, 
		   false)

//
// Lanczos tests based on expansion of u = exp(x) where x is a U([-1,1])
// random variable.  For this test we don't use the PCE quad points and 
// instead use those generated for the Lanczos basis
//
template <typename Ordinal_Type, typename Value_Type>
struct Lanczos_Exp_Func {
  typedef Ordinal_Type OrdinalType;
  typedef Value_Type ValueType;
  static const bool is_even = false;
  void 
  eval(Stokhos::QuadOrthogPolyExpansion<OrdinalType,ValueType>& exp,
       const Stokhos::OrthogPolyApprox<OrdinalType,ValueType>& x, 
       Stokhos::OrthogPolyApprox<OrdinalType,ValueType>& u) {
    exp.exp(u,x);
  }
};
LANCZOS_UNIT_TESTS(Stokhos_LanczosPCEBasis_Proj, Exp, Lanczos_Exp_Func, 
		   false)

int main( int argc, char* argv[] ) {
  Teuchos::GlobalMPISession mpiSession(&argc, &argv);
  return Teuchos::UnitTestRepository::runUnitTestsFromMain(argc, argv);
}
