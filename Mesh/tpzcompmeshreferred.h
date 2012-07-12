/**
 * @file
 * @brief Contains declaration of TPZCompMeshReferred class which implements the structure to allow one mesh to refer to the solution of another.
 */
//
// C++ Interface: tpzcompmeshreferred
//
// Description: 
//
//
// Author: Philippe R. B. Devloo <phil@fec.unicamp.br>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TPZCOMPMESHREFERRED_H
#define TPZCOMPMESHREFERRED_H

#include "pzcmesh.h"
#include <vector>

/**
 @brief Implements the structure to allow one mesh to refer to the solution of another. \ref geometry "Geometry"
 @ingroup geometry
 @author Philippe R. B. Devloo
 */
class TPZCompMeshReferred : public TPZCompMesh
{
	
	TPZVec<int> fReferredIndices;
	
	TPZCompMesh *fReferred;
	
public:
    TPZCompMeshReferred(TPZGeoMesh *gmesh);
    
    // inserido por Frederico (LNCC) em 02/04.
    TPZCompMeshReferred(const TPZCompMesh &cmesh);
	
    TPZCompMeshReferred(const TPZCompMeshReferred &compmesh);
	
    virtual ~TPZCompMeshReferred();
	
    void LoadReferred(TPZCompMesh *mesh);
	
    void ResetReferred();
	
    TPZCompEl *ReferredEl(int index);
	
    TPZCompMesh *ReferredMesh() const
    {
		return fReferred;
    }
	
	/** @brief Divide computational element recursively over referred elements. */
	static void DivideReferredEl(TPZVec<TPZCompEl *> WhichRefine, TPZCompMesh * cmesh);
	
	/**
	 * @brief Prints mesh data
	 * @param out indicates the device where the data will be printed
	 */
	virtual void Print(std::ostream & out = std::cout) const;
	
};

#endif
