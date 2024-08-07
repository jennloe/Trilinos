// @HEADER
// *****************************************************************************
//           Panzer: A partial differential equation assembly
//       engine for strongly coupled complex multiphysics systems
//
// Copyright 2011 NTESS and the Panzer contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

#ifndef PANZER_STK_SCATTER_FIELDS_IMPL_HPP
#define PANZER_STK_SCATTER_FIELDS_IMPL_HPP

#include "Teuchos_Assert.hpp"

#include "Phalanx_config.hpp"
#include "Phalanx_Evaluator_Macros.hpp"
#include "Phalanx_MDField.hpp"
#include "Phalanx_DataLayout.hpp"
#include "Phalanx_DataLayout_MDALayout.hpp"

#include "Panzer_BasisIRLayout.hpp"
#include "Panzer_Traits.hpp"

#include "Teuchos_FancyOStream.hpp"

namespace panzer_stk {

template <typename EvalT,typename TraitsT>
ScatterFields<EvalT,TraitsT>::
ScatterFields(const std::string & scatterName,
              const Teuchos::RCP<STK_Interface> mesh,
              const Teuchos::RCP<const panzer::PureBasis> & basis,
              const std::vector<std::string> & names)
{
  std::vector<double> scaling; // empty

  initialize(scatterName,mesh,basis,names,scaling);
}

template <typename EvalT,typename TraitsT>
ScatterFields<EvalT,TraitsT>::
ScatterFields(const std::string & scatterName,
              const Teuchos::RCP<STK_Interface> mesh,
              const Teuchos::RCP<const panzer::PureBasis> & basis,
              const std::vector<std::string> & names,
              const std::vector<double> & scaling)
{
  initialize(scatterName,mesh,basis,names,scaling);
}

template <typename EvalT,typename TraitsT>
void ScatterFields<EvalT,TraitsT>::
initialize(const std::string & scatterName,
              const Teuchos::RCP<STK_Interface> mesh,
              const Teuchos::RCP<const panzer::PureBasis> & basis,
              const std::vector<std::string> & names,
              const std::vector<double> & scaling)
{
  using panzer::Cell;
  using panzer::NODE;

  mesh_ = mesh;
  scaling_ = scaling;

  bool correctScaling = (names.size()==scaling.size()) || (scaling.size()==0);
  TEUCHOS_TEST_FOR_EXCEPTION(!correctScaling,std::invalid_argument,
     "panzer_stk::ScatterFields evaluator requites a consistent number of scaling parameters (equal to the number of field names) "
     "or an empty \"Field Scaling\" vector");

  // build dependent fields
  scatterFields_.resize(names.size());
  for (std::size_t fd = 0; fd < names.size(); ++fd) {
    scatterFields_[fd] =
      PHX::MDField<const ScalarT,Cell,NODE>(names[fd],basis->functional);
    this->addDependentField(scatterFields_[fd]);
  }

  // determine if this is a cell field or not
  cellFields_ = basis->getElementSpace()==panzer::PureBasis::CONST;

  // setup a dummy field to evaluate
  PHX::Tag<ScalarT> scatterHolder(scatterName,Teuchos::rcp(new PHX::MDALayout<panzer::Dummy>(0)));
  this->addEvaluatedField(scatterHolder);

  this->setName(scatterName+": STK-Scatter Fields");
}

template <typename EvalT,typename TraitsT>
void ScatterFields<EvalT,TraitsT>::
postRegistrationSetup(typename TraitsT::SetupData /* d */,
                      PHX::FieldManager<TraitsT>& /* fm */)
{
  for (std::size_t fd = 0; fd < scatterFields_.size(); ++fd)
    std::string fieldName = scatterFields_[fd].fieldTag().name();
}

template <typename EvalT,typename TraitsT>
void ScatterFields<EvalT,TraitsT>::
evaluateFields(typename TraitsT::EvalData /* d */)
{
   TEUCHOS_ASSERT(false);
}

template < >
void ScatterFields<panzer::Traits::Residual,panzer::Traits>::
evaluateFields(panzer::Traits::EvalData workset)
{
   // for convenience pull out some objects from workset
   const std::vector<std::size_t> & localCellIds = this->wda(workset).cell_local_ids;
   std::string blockId = this->wda(workset).block_id;

   for(std::size_t fieldIndex=0; fieldIndex<scatterFields_.size();fieldIndex++) {
      // scaline field value only if the scaling parameter is specified, otherwise use 1.0
      double scaling = (scaling_.size()>0) ? scaling_[fieldIndex] : 1.0;

      // write field to the STK mesh object
      if(!cellFields_)
        mesh_->setSolutionFieldData(scatterFields_[fieldIndex].fieldTag().name(),blockId,
                                    localCellIds,scatterFields_[fieldIndex].get_view(),scaling);
      else
        mesh_->setCellFieldData(scatterFields_[fieldIndex].fieldTag().name(),blockId,
                                localCellIds,scatterFields_[fieldIndex].get_view(),scaling);
   }
}

} // end panzer_stk

#endif
