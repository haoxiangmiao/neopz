//
// Created by Francisco Teixeira Orlandini on 12/13/17.
//

#ifndef PZ_TPZSLEPCHANDLER_H
#define PZ_TPZSLEPCHANDLER_H

#include <pzysmp.h>

template<class TVar>
class SPZAlwaysComplex;

template<class TVar>
class TPZSlepcHandler {
  friend class TPZFYsmpMatrix<TVar>;
  /*******************
  *    TPZFYSMPMATRIX    *
  *******************/
  static int SolveEigenProblem(TPZFYsmpMatrix<TVar> &A, TPZVec < typename SPZAlwaysComplex<TVar>::type > &w, TPZFMatrix < typename SPZAlwaysComplex<TVar>::type  > &eigenVectors);

  /** @brief Solves the Ax=w*x eigenvalue problem and does NOT calculates the eigenvectors
   * @param w Stores the eigenvalues
   */
  static int SolveEigenProblem(TPZFYsmpMatrix<TVar> &A, TPZVec < typename SPZAlwaysComplex<TVar>::type > &w);

  /** @brief Solves the generalised Ax=w*B*x eigenvalue problem and calculates the eigenvectors
   * @param w Stores the eigenvalues
   * @param Stores the correspondent eigenvectors
   */
  static int SolveGeneralisedEigenProblem(TPZFYsmpMatrix<TVar> &A, TPZFYsmpMatrix< TVar > &B , TPZVec < typename SPZAlwaysComplex<TVar>::type > &w, TPZFMatrix < typename SPZAlwaysComplex<TVar>::type > &eigenVectors);
  /** @brief Solves the generalised Ax=w*B*x eigenvalue problem and does NOT calculates the eigenvectors
   * @param w Stores the eigenvalues
   */
  static int SolveGeneralisedEigenProblem(TPZFYsmpMatrix<TVar> &A, TPZFYsmpMatrix< TVar > &B , TPZVec < typename SPZAlwaysComplex<TVar>::type > &w);

};


#endif //PZ_TPZSLEPCHANDLER_H
