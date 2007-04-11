//$Id: pzreferredcompel.h,v 1.6 2007-04-11 14:27:35 tiago Exp $

// -*- c++ -*-
#ifndef PZSPECIAL
#define PZSPECIAL

class TPZElementMatrix;
class TPZCompEl;
class TPZInterpolatedElement;
class TPZCompElDisc;
class TPZGeoEl;
class TPZCompMesh;
class TPZFMatrix;
#include "pzvec.h"
#include "pzmanvector.h"

template<class TCOMPEL>
class TPZReferredCompEl : public TCOMPEL {
  public:

   TPZReferredCompEl(TPZCompMesh &mesh, TPZGeoEl *gel, int &index);

  ~TPZReferredCompEl();

  /** Returns referred element of this
   */
  TPZCompEl * ReferredElement();

 /**
  * Computes solution and its derivatives in local coordinate qsi
  * @param qsi master element coordinate
  * @param phi matrix containing shape functions compute in qsi point
  * @param dphix matrix containing the derivatives of shape functions in the direction of the axes
  * @param axes direction of the derivatives
  * @param sol finite element solution
  * @param dsol solution derivatives
  */
  virtual void ComputeSolution(TPZVec<REAL> &qsi, TPZFMatrix &phi, TPZFMatrix &dphix,
                               const TPZFMatrix &axes, TPZVec<REAL> &sol, TPZFMatrix &dsol);
  
 /**
   * Computes solution and its derivatives in the local coordinate qsi.
   * @param qsi master element coordinate
   * @param sol finite element solution
   * @param dsol solution derivatives
   * @param axes axes associated with the derivative of the solution
  */
  virtual void ComputeSolution(TPZVec<REAL> &qsi,
                               TPZVec<REAL> &sol, TPZFMatrix &dsol,TPZFMatrix &axes);
  
 /**
   * Computes solution and its derivatives in the local coordinate qsi.
   * @param qsi master element coordinate of the interface element
   * @param leftsol finite element solution
   * @param dleftsol solution derivatives
   * @param leftaxes axes associated with the left solution
   * @param rightsol finite element solution
   * @param drightsol solution derivatives
   * @param rightaxes axes associated with the right solution
  */
virtual void ComputeSolution(TPZVec<REAL> &qsi, 
                               TPZVec<REAL> &sol, TPZFMatrix &dsol,TPZFMatrix &axes,
                               TPZVec<REAL> &normal,
                               TPZVec<REAL> &leftsol, TPZFMatrix &dleftsol,TPZFMatrix &leftaxes,
                               TPZVec<REAL> &rightsol, TPZFMatrix &drightsol,TPZFMatrix &rightaxes);
 /**
   * Computes solution and its derivatives in the local coordinate qsi.
   * This method will function for both volumetric and interface elements
   * @param qsi master element coordinate of the interface element
   * @param sol finite element solution
   * @param dsol solution derivatives
   * @param axes axes associated with the derivative of the solution
   * @param leftsol finite element solution
   * @param dleftsol solution derivatives
   * @param leftaxes axes associated with the left solution
   * @param rightsol finite element solution
   * @param drightsol solution derivatives
   * @param rightaxes axes associated with the right solution
  */
// virtual void ComputeSolution(TPZVec<REAL> &qsi,
//                              TPZVec<REAL> &normal,
//                              TPZVec<REAL> &leftsol, TPZFMatrix &dleftsol,TPZFMatrix &leftaxes,
//                              TPZVec<REAL> &rightsol, TPZFMatrix &drightsol,TPZFMatrix &rightaxes);

  /**
   * Prints element data
   * @param out indicates the device where the data will be printed
   */
  virtual void Print(std::ostream & out = std::cout);

  protected:

  /**
   * Append solution of the referred element.
   */
  void AppendOtherSolution(TPZVec<REAL> &qsi, TPZVec<REAL> &sol,
                           TPZFMatrix &dsol,  TPZFMatrix &axes);
  void AppendOtherSolution(TPZVec<REAL> &qsi, TPZVec<REAL> &sol,
                           TPZFMatrix &dsol,  const TPZFMatrix &axes);
  void AppendOtherSolution(TPZVec<REAL> &qsi, TPZVec<REAL> &sol, TPZFMatrix &dsol,
                           TPZFMatrix &axes,
                           TPZVec<REAL> &normal,
                           TPZVec<REAL> &leftsol, TPZFMatrix &dleftsol, TPZFMatrix &leftaxes,
                           TPZVec<REAL> &rightsol, TPZFMatrix &drightsol,TPZFMatrix &rightaxes);
};

/**
 * Adjust the derivatives from one system of axes to the other
 */
void AdjustSolutionDerivatives(TPZFMatrix &dsolfrom, TPZFMatrix &axesfrom,
                               TPZFMatrix &dsolto, const TPZFMatrix &axesto);

  TPZCompEl *CreateReferredDisc(TPZGeoEl *gel,TPZCompMesh &mesh,int &index);
  TPZCompEl *CreateReferredPointEl(TPZGeoEl *gel,TPZCompMesh &mesh,int &index);
  TPZCompEl *CreateReferredLinearEl(TPZGeoEl *gel,TPZCompMesh &mesh,int &index);
  TPZCompEl *CreateReferredQuadEl(TPZGeoEl *gel,TPZCompMesh &mesh,int &index);
  TPZCompEl *CreateReferredTriangleEl(TPZGeoEl *gel,TPZCompMesh &mesh,int &index);
  TPZCompEl *CreateReferredCubeEl(TPZGeoEl *gel,TPZCompMesh &mesh,int &index);
  TPZCompEl *CreateReferredPrismEl(TPZGeoEl *gel,TPZCompMesh &mesh,int &index);
  TPZCompEl *CreateReferredPyramEl(TPZGeoEl *gel,TPZCompMesh &mesh,int &index);
  TPZCompEl *CreateReferredTetraEl(TPZGeoEl *gel,TPZCompMesh &mesh,int &index);

  void Append(TPZVec<REAL> &u1, TPZVec<REAL> &u2, TPZVec<REAL> &u12);
  void Append(TPZFMatrix &u1, TPZFMatrix &u2, TPZFMatrix &u12);
  bool AreEqual(const TPZVec<REAL> &A, const TPZVec<REAL> &B, REAL tol = 1e-10);
#endif
