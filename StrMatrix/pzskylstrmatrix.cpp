/**
 * @file
 * @brief Contains the implementation of the TPZSkylineStructMatrix methods. 
 */

#include "pzskylstrmatrix.h"
#include "pzskylmat.h"
#include "pzcmesh.h"
#include "pzsubcmesh.h"
#include "pzvec.h"

TPZStructMatrix * TPZSkylineStructMatrix::Clone(){
    return new TPZSkylineStructMatrix(*this);
}

TPZSkylineStructMatrix::TPZSkylineStructMatrix(const TPZSkylineStructMatrix &cp)
:TPZStructMatrix(cp) {
	//nothing here
}

TPZSkylineStructMatrix::TPZSkylineStructMatrix(TPZCompMesh *mesh) : TPZStructMatrix(mesh)
{
}

TPZSkylineStructMatrix::TPZSkylineStructMatrix(TPZAutoPointer<TPZCompMesh> cmesh) : TPZStructMatrix(cmesh)
{
}

TPZMatrix<STATE> * TPZSkylineStructMatrix::Create(){
    TPZVec<int> skyline;
    fMesh->Skyline(skyline);
    fEquationFilter.FilterSkyline(skyline);
    int neq = fEquationFilter.NActiveEquations();	
    return this->ReallyCreate(neq,skyline);//new TPZSkylMatrix<STATE>(neq,skyline);
}


TPZMatrix<STATE> * TPZSkylineStructMatrix::ReallyCreate(int neq, const TPZVec<int> &skyline){
    return new TPZSkylMatrix<STATE>(neq,skyline);
}




TPZSkylineStructMatrix::~TPZSkylineStructMatrix(){
}




