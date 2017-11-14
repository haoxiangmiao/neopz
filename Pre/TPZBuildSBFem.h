//
//  TPZBuildSBFem.hpp
//  PZ
//
//  Created by Philippe Devloo on 06/01/17.
//
//

#ifndef TPZBuildSBFem_hpp
#define TPZBuildSBFem_hpp

#include <stdio.h>
#include "pzmanvector.h"
#include "pzgmesh.h"
#include "pzcmesh.h"
#include "tpzautopointer.h"
#include <map>

class TPZBuildSBFem
{
    /// geometric mesh
    TPZAutoPointer<TPZGeoMesh> fGMesh;
    
    /// The volumetric elements with Mat Id will spawn SBFemVolume elements with MatId
    std::map<int,int> fMatIdTranslation;
    
    /// Material Id associated with the skeleton elements
    int fSkeletonMatId;
    
    /// partition to which each element belongs
    TPZVec<long> fElementPartition;
    
    /// center node id for each partition
    TPZVec<long> fPartitionCenterNode;
    
public:
    
    /// simple constructor
    TPZBuildSBFem(TPZAutoPointer<TPZGeoMesh> gmesh, int skeletonmatid, std::map<int,int> &matidtranslation) : fGMesh(gmesh), fMatIdTranslation(matidtranslation), fSkeletonMatId(skeletonmatid)
    {
        fElementPartition.resize(fGMesh->NElements());
        fElementPartition.Fill(-1);
    }
    
    /// set the matid translation
    void SetMatIdTranslation(const std::map<int,int> &matidtranslation)
    {
        fMatIdTranslation = matidtranslation;
    }
    
    /// standard configuration means each element is a partition and a center node is created
    void StandardConfiguration();

    /// standard configuration means each element is a partition and a center node is created for the indicated elements
    void StandardConfiguration(TPZVec<long> &elementindices);
    
    /// build element groups according to the id of the scaling centers
    void Configure(TPZVec<long> &scalingcenters);
    
    /// add a partition manually
    void AddPartition(TPZVec<long> &elids, long centernodeindex);
    
    /// add the sbfem elements to the computational mesh, the material should exist in cmesh
    void BuildComputationMesh(TPZCompMesh &cmesh);
    
    /// Divide de skeleton elements
    void DivideSkeleton(int nref);
    
private:
    /// create the geometric skeleton elements
    void AddSkeletonElements();
    
    /// create a geometric node at the center of each partition
    void CreateElementCenterNodes(TPZVec<long> &elindices);
    
    /// create geometric volumetric elements
    void CreateVolumetricElements(TPZCompMesh &cmesh);
    
    /// put the sbfem volumetric elements in element groups
    void CreateElementGroups(TPZCompMesh &cmesh);
};

#endif /* TPZBuildSBFem_hpp */
