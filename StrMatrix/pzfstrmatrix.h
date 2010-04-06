/* Generated by Together */

#ifndef TPZFSTRUCTMATRIX_H
#define TPZFSTRUCTMATRIX_H
#include "pzstrmatrix.h"
class TPZCompMesh;
class TPZFMatrix;
class TPZMatrix;

/**
 * Implements Full Structural Matrices
 * @ingroup structural
 */
class TPZFStructMatrix : public TPZStructMatrix {
public:    

    TPZFStructMatrix(TPZCompMesh *);

    virtual TPZMatrix * Create();

    virtual TPZMatrix * CreateAssemble(TPZFMatrix &rhs, TPZAutoPointer<TPZGuiInterface> guiInterface);

    virtual TPZStructMatrix * Clone();

public:
};
#endif //TPZFSTRUCTMATRIX_H
