// @HEADER
// *****************************************************************************
//        MueLu: A package for multigrid based preconditioning
//
// Copyright 2012 NTESS and the MueLu contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

#ifndef MUELU_AMALGAMATIONFACTORY_DEF_HPP
#define MUELU_AMALGAMATIONFACTORY_DEF_HPP

#include <Xpetra_Matrix.hpp>

#include "MueLu_AmalgamationFactory_decl.hpp"

#include "MueLu_Level.hpp"
#include "MueLu_AmalgamationInfo.hpp"
#include "MueLu_Monitor.hpp"

namespace MueLu {

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
AmalgamationFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::AmalgamationFactory() = default;

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
AmalgamationFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::~AmalgamationFactory() = default;

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
RCP<const ParameterList> AmalgamationFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::GetValidParameterList() const {
  RCP<ParameterList> validParamList = rcp(new ParameterList());
  validParamList->set<RCP<const FactoryBase> >("A", Teuchos::null, "Generating factory of the matrix A");
  return validParamList;
}

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
void AmalgamationFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::DeclareInput(Level& currentLevel) const {
  Input(currentLevel, "A");  // sub-block from blocked A
}

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
void AmalgamationFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::Build(Level& currentLevel) const {
  FactoryMonitor m(*this, "Build", currentLevel);

  RCP<Matrix> A = Get<RCP<Matrix> >(currentLevel, "A");

  /* NOTE: storageblocksize (from GetStorageBlockSize()) is the size of a block in the chosen storage scheme.
     fullblocksize is the number of storage blocks that must kept together during the amalgamation process.

     Both of these quantities may be different than numPDEs (from GetFixedBlockSize()), but the following must always hold:

     numPDEs = fullblocksize * storageblocksize.

     If numPDEs==1
       Matrix is point storage (classical CRS storage).  storageblocksize=1 and fullblocksize=1
       No other values makes sense.

     If numPDEs>1
       If matrix uses point storage, then storageblocksize=1  and fullblockssize=numPDEs.
       If matrix uses block storage, with block size of n, then storageblocksize=n, and fullblocksize=numPDEs/n.
       Thus far, only storageblocksize=numPDEs and fullblocksize=1 has been tested.
  */

  LO fullblocksize    = 1;              // block dim for fixed size blocks
  GO offset           = 0;              // global offset of dof gids
  LO blockid          = -1;             // block id in strided map
  LO nStridedOffset   = 0;              // DOF offset for strided block id "blockid" (default = 0)
  LO stridedblocksize = fullblocksize;  // size of strided block id "blockid" (default = fullblocksize, only if blockid!=-1 stridedblocksize <= fullblocksize)
  LO storageblocksize = A->GetStorageBlockSize();
  // GO indexBase        = A->getRowMap()->getIndexBase();  // index base for maps (unused)

  // 1) check for blocking/striding information

  if (A->IsView("stridedMaps") && Teuchos::rcp_dynamic_cast<const StridedMap>(A->getRowMap("stridedMaps")) != Teuchos::null) {
    Xpetra::viewLabel_t oldView         = A->SwitchToView("stridedMaps");  // NOTE: "stridedMaps are always non-overlapping (correspond to range and domain maps!)
    RCP<const StridedMap> stridedRowMap = Teuchos::rcp_dynamic_cast<const StridedMap>(A->getRowMap());
    TEUCHOS_TEST_FOR_EXCEPTION(stridedRowMap == Teuchos::null, Exceptions::BadCast, "MueLu::CoalesceFactory::Build: cast to strided row map failed.");
    fullblocksize = stridedRowMap->getFixedBlockSize();
    offset        = DOFGidOffset(stridedRowMap);
    blockid       = stridedRowMap->getStridedBlockId();

    if (blockid > -1) {
      std::vector<size_t> stridingInfo = stridedRowMap->getStridingData();
      for (size_t j = 0; j < Teuchos::as<size_t>(blockid); j++)
        nStridedOffset += stridingInfo[j];
      stridedblocksize = Teuchos::as<LocalOrdinal>(stridingInfo[blockid]);

    } else {
      stridedblocksize = fullblocksize;
    }
    // Correct for the storageblocksize
    // NOTE:  Before this point fullblocksize is actually numPDEs
    TEUCHOS_TEST_FOR_EXCEPTION(fullblocksize % storageblocksize != 0, Exceptions::RuntimeError, "AmalgamationFactory: fullblocksize needs to be a multiple of A->GetStorageBlockSize()");
    fullblocksize /= storageblocksize;
    stridedblocksize /= storageblocksize;

    oldView = A->SwitchToView(oldView);
    GetOStream(Runtime1) << "AmalagamationFactory::Build():"
                         << " found fullblocksize=" << fullblocksize << " and stridedblocksize=" << stridedblocksize << " from strided maps. offset=" << offset << std::endl;

  } else {
    GetOStream(Warnings0) << "AmalagamationFactory::Build(): no striding information available. Use blockdim=1 with offset=0" << std::endl;
  }

  // build node row map (uniqueMap) and node column map (nonUniqueMap)
  // the arrays rowTranslation and colTranslation contain the local node id
  // given a local dof id. They are only necessary for the CoalesceDropFactory if
  // fullblocksize > 1
  RCP<const Map> uniqueMap, nonUniqueMap;
  RCP<AmalgamationInfo> amalgamationData;
  RCP<Array<LO> > rowTranslation = Teuchos::null;
  RCP<Array<LO> > colTranslation = Teuchos::null;

  if (fullblocksize > 1) {
    // mfh 14 Apr 2015: These need to have different names than
    // rowTranslation and colTranslation, in order to avoid
    // shadowing warnings (-Wshadow with GCC).  Alternately, it
    // looks like you could just assign to the existing variables in
    // this scope, rather than creating new ones.
    RCP<Array<LO> > theRowTranslation = rcp(new Array<LO>);
    RCP<Array<LO> > theColTranslation = rcp(new Array<LO>);
    AmalgamateMap(*(A->getRowMap()), *A, uniqueMap, *theRowTranslation);
    AmalgamateMap(*(A->getColMap()), *A, nonUniqueMap, *theColTranslation);

    amalgamationData = rcp(new AmalgamationInfo(theRowTranslation,
                                                theColTranslation,
                                                uniqueMap,
                                                nonUniqueMap,
                                                A->getColMap(),
                                                fullblocksize,
                                                offset,
                                                blockid,
                                                nStridedOffset,
                                                stridedblocksize));
  } else {
    amalgamationData = rcp(new AmalgamationInfo(rowTranslation,  // Teuchos::null
                                                colTranslation,  // Teuchos::null
                                                A->getRowMap(),  // unique map of graph
                                                A->getColMap(),  // non-unique map of graph
                                                A->getColMap(),  // column map of A
                                                fullblocksize,
                                                offset,
                                                blockid,
                                                nStridedOffset,
                                                stridedblocksize));
  }

  // store (un)amalgamation information on current level
  Set(currentLevel, "UnAmalgamationInfo", amalgamationData);
}

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
void AmalgamationFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::AmalgamateMap(const Map& sourceMap, const Matrix& A, RCP<const Map>& amalgamatedMap, Array<LO>& translation) {
  typedef typename ArrayView<const GO>::size_type size_type;
  typedef std::unordered_map<GO, size_type> container;

  GO indexBase                     = sourceMap.getIndexBase();
  ArrayView<const GO> elementAList = sourceMap.getLocalElementList();
  size_type numElements            = elementAList.size();
  container filter;

  GO offset  = 0;
  LO blkSize = A.GetFixedBlockSize() / A.GetStorageBlockSize();
  if (A.IsView("stridedMaps") == true) {
    Teuchos::RCP<const Map> myMap         = A.getRowMap("stridedMaps");
    Teuchos::RCP<const StridedMap> strMap = Teuchos::rcp_dynamic_cast<const StridedMap>(myMap);
    TEUCHOS_TEST_FOR_EXCEPTION(strMap == null, Exceptions::RuntimeError, "Map is not of type StridedMap");
    offset  = DOFGidOffset(strMap);
    blkSize = Teuchos::as<const LO>(strMap->getFixedBlockSize());
  }

  Array<GO> elementList(numElements);
  translation.resize(numElements);

  size_type numRows = 0;
  for (size_type id = 0; id < numElements; id++) {
    GO dofID  = elementAList[id];
    GO nodeID = AmalgamationFactory::DOFGid2NodeId(dofID, blkSize, offset, indexBase);

    typename container::iterator it = filter.find(nodeID);
    if (it == filter.end()) {
      filter[nodeID] = numRows;

      translation[id]      = numRows;
      elementList[numRows] = nodeID;

      numRows++;

    } else {
      translation[id] = it->second;
    }
  }
  elementList.resize(numRows);

  amalgamatedMap = MapFactory::Build(sourceMap.lib(), Teuchos::OrdinalTraits<Xpetra::global_size_t>::invalid(), elementList, indexBase, sourceMap.getComm());
}

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
void AmalgamationFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::AmalgamateMap(RCP<const StridedMap> sourceMap, RCP<const Map>& amalgamatedMap) {
  // NOTE: There could be further optimizations here where we detect contiguous maps and then
  // create a contiguous amalgamated maps, which bypasses the expense of the getMyGlobalIndicesDevice()
  // call (which is free for non-contiguous maps, but costs something if the map is contiguous).
  using range_policy = Kokkos::RangePolicy<typename Node::execution_space>;
  using array_type   = typename Map::global_indices_array_device_type;

  array_type elementAList = sourceMap->getMyGlobalIndicesDevice();
  GO indexBase            = sourceMap->getIndexBase();
  LO blkSize              = sourceMap->getFixedBlockSize();
  auto numElements        = elementAList.size() / blkSize;
  typename array_type::non_const_type elementList_nc("elementList", numElements);

  // Amalgamate the map
  Kokkos::parallel_for(
      "Amalgamate Element List", range_policy(0, numElements), KOKKOS_LAMBDA(LO i) {
        elementList_nc[i] = (elementAList[i * blkSize] - indexBase) / blkSize + indexBase;
      });
  array_type elementList = elementList_nc;

  amalgamatedMap = MapFactory::Build(sourceMap->lib(), Teuchos::OrdinalTraits<Xpetra::global_size_t>::invalid(),
                                     elementList, indexBase, sourceMap->getComm());
}

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
const GlobalOrdinal AmalgamationFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::DOFGid2NodeId(GlobalOrdinal gid, LocalOrdinal blockSize,
                                                                                                  const GlobalOrdinal offset, const GlobalOrdinal indexBase) {
  GlobalOrdinal globalblockid = ((GlobalOrdinal)gid - offset - indexBase) / blockSize + indexBase;
  return globalblockid;
}

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
const GlobalOrdinal AmalgamationFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::DOFGidOffset(RCP<const StridedMap> stridedRowMap) {
  GlobalOrdinal offset = stridedRowMap->getMinAllGlobalIndex();

  size_t nStridedOffset = 0;
  for (int j = 0; j < stridedRowMap->getStridedBlockId(); j++) {
    nStridedOffset += stridedRowMap->getStridingData()[j];
  }
  const GO goStridedOffset = Teuchos::as<const GO>(nStridedOffset);

  offset -= goStridedOffset + stridedRowMap->getIndexBase();

  return offset;
}

}  // namespace MueLu

#endif /* MUELU_SUBBLOCKUNAMALGAMATIONFACTORY_DEF_HPP */
