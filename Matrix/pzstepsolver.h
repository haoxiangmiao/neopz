/* Generated by Together */

#ifndef TPZSTEPSOLVER_H
#define TPZSTEPSOLVER_H
#include "pzsolve.h"

#include "pzstream.h"

#include <list>
class TPZFMatrix;

/**
   Defines step solvers class
   @ingroup solvers
*/
class TPZStepSolver : public TPZMatrixSolver {
public:  
  TPZStepSolver(TPZAutoPointer<TPZMatrix> refmat=0);

  TPZStepSolver(const TPZStepSolver & copy);

    virtual ~TPZStepSolver();

    void SetSOR(const int numiterations,const REAL overrelax,const REAL tol,const int FromCurrent);

    void SetSSOR(const int numiterations,const REAL overrelax,const REAL tol,const int FromCurrent);

    void SetJacobi(const int numiterations,const REAL tol,const int FromCurrent);

    void SetCG(const int numiterations,const TPZMatrixSolver &pre,const REAL tol,const int FromCurrent);

    void SetGMRES(const int numiterations,const int numvectors, const TPZMatrixSolver &pre,const REAL tol,const int FromCurrent);
	
    void SetBiCGStab(const int numiterations, const TPZMatrixSolver &pre,const REAL tol,const int FromCurrent);

    void SetDirect (const DecomposeType decomp);

  void SetMultiply();

    virtual TPZSolver *Clone() const { return new TPZStepSolver(*this);}

  void SetTolerance(REAL tol) { fTol = tol; }

    void ResetSolver();
    
    /**
     * return the equations for which the equations had zero pivot
     */
    std::list<int> &Singular()
    {
      return fSingular;
    }

  /**
  This method will reset the matrix associated with the solver
  This is useful when the matrix needs to be recomputed in a non linear problem
  */
  virtual void ResetMatrix();
  /**
     Sets a matrix to the current object
     @param RefMat Sets reference matrix to RefMat     
  */
/*virtual  void SetMatrix(TPZMatrix *Refmat)
{
  TPZMatrixSolver::SetMatrix(Refmat);
}
 */

  /**
  This method gives a preconditioner to share a matrix with the referring solver object
  */
//  virtual void SetMatrix(TPZMatrixSolver *solver);
  /**
  * Updates the values of the current matrix based on the values of the matrix
  */
virtual void UpdateFrom(TPZMatrix *matrix)
  {
    if(fPrecond) fPrecond->UpdateFrom(matrix);
    TPZMatrixSolver::UpdateFrom(matrix);
  }
  
    void Solve(const TPZFMatrix &F, TPZFMatrix &result, TPZFMatrix *residual = 0);
    void SetPreconditioner(TPZSolver &solve);
	/**
	 * Serialization methods
	 */
	void Write(TPZStream & buf);
	void Read(TPZStream & buf);

private:  
    MSolver fSolver;
    DecomposeType fDecompose;
    int fNumIterations;
  int fNumVectors;
    REAL fTol;
    REAL fOverRelax;

    /**
     * @supplierCardinality 1 
     */
    TPZSolver *fPrecond;
    int fFromCurrent;
    
    std::list<int> fSingular;
};
#endif //TPZSTEPSOLVER_H
