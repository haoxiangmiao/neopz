/**
 * @file
 * @brief Contains the TPZMatRedStructMatrix class. 
 */

#ifndef TPZMATREDSTRUCTMATRIX
#define TPZMATREDSTRUCTMATRIX

#include "pzmatrix.h"
#include "pzstrmatrix.h"
#include "pzfmatrix.h"
#include "pzcmesh.h"
#include "pzsubcmesh.h"


/**
 * @ingroup substructure
 * @brief .. . \ref substructure " Structure"
 */
template<class TStructMatrix, class TSparseMatrix>
class TPZMatRedStructMatrix : public TPZStructMatrix
{
public:
	/** @brief Constructor */
	TPZMatRedStructMatrix(TPZSubCompMesh *mesh);
	/** @brief Destructor */
	virtual ~TPZMatRedStructMatrix();
	/** @brief Copy constructor */
	TPZMatRedStructMatrix(const TPZMatRedStructMatrix &copy);
	
	virtual TPZStructMatrix *Clone();
	
	virtual TPZMatrix<STATE> *Create();
	
private:
	
	int fInternalEqs;
	
};

#endif
