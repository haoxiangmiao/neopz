/**
 * @file
 * @brief Contains the TPZBandStructMatrix class which implements Banded Structural Matrices.
 */

#ifndef TPZBANDSTRUCTMATRIX_H
#define TPZBANDSTRUCTMATRIX_H

#include "pzstrmatrix.h"

class TPZCompMesh;
template<class TVar>
class TPZFMatrix;
template<class TVar>
class TPZMatrix;

/**
 * @brief Implements Banded Structural Matrices. \ref structural "Structural Matrix"
 * @ingroup structural
 */
class TPZBandStructMatrix : public TPZStructMatrix {
public:    
	
    TPZBandStructMatrix(TPZCompMesh *);
    
    ~TPZBandStructMatrix();
	
    TPZBandStructMatrix(const TPZBandStructMatrix &copy) : TPZStructMatrix(copy)
    {
    }
	
    virtual TPZMatrix<STATE> * Create();
	
    virtual TPZMatrix<STATE> * CreateAssemble(TPZFMatrix<STATE> &rhs);
	
    virtual TPZStructMatrix * Clone();
	
public:
	
};

#endif //TPZBANDSTRUCTMATRIX_H
