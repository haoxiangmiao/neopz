/**
 * @file
 * @brief Contains TPZSubMeshAnalysis class which implements the analysis procedure to computational sub mesh.
 */

#ifndef TPZSUBMESHANALYSIS_H
#define TPZSUBMESHANALYSIS_H


#include "pzanalysis.h"
#include "pzmatred.h"
class TPZSubCompMesh;

#include "pzfmatrix.h"	// Added by ClassView

/** 
 * @brief Analysis procedure to computational sub mesh. \ref analysis "Analysis"
 * @ingroup analysis
 */
class TPZSubMeshAnalysis : public TPZAnalysis  
{
private:
	TPZFMatrix<REAL> fReferenceSolution;
	
	TPZAutoPointer<TPZMatrix<REAL> > fReducableStiff;
	TPZSubCompMesh *fMesh;
	
public:
	virtual void LoadSolution(const TPZFMatrix<REAL> &sol);
	/**
	 * @brief Constructor: create an object analysis from one mesh
	 **/
	TPZSubMeshAnalysis(TPZSubCompMesh *mesh);
	
	/**
	 * @brief Destructor
	 **/
	virtual ~TPZSubMeshAnalysis();
	
	TPZAutoPointer<TPZMatrix<REAL> > Matrix()
	{
		return fReducableStiff;
	}
	
	/**
	 * @brief Run: assemble the stiffness matrix
	 **/
	void Run(std::ostream &out);
	
	/**
	 * @brief CondensedSolution: returns the condensed stiffness
	 *matrix - ek - and the condensed solution vector - ef
	 */
	void CondensedSolution(TPZFMatrix<REAL> &ek, TPZFMatrix<REAL> &ef);
	
	/**
	 * @brief Assemble the global stiffness matrix and put it into the 
	 * reducable stiffness matrix
	 */
	virtual void Assemble();
	
};

#endif 
