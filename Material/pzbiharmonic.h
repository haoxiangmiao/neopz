// -*- c++ -*-
//$Id: pzbiharmonic.h,v 1.9 2007-12-07 13:47:47 cesar Exp $

#ifndef  TPZBIHARMONICHPP
#define TPZBIHARMONICHPP

#include <iostream>
#include "pzdiscgal.h"
#include "pzfmatrix.h"

/**
 * This class implements discontinuous Galerkin formulation for the bi-harmonic equation.
 * @since Nov 27, 2003
 * @author Igor Mozolevski e Paulo Bosing
 */
class TPZBiharmonic : public TPZDiscontinuousGalerkin {

private:
  REAL  fXf;

  //Problem dimention

public :

  static REAL gLambda1, gLambda2, gSigmaA,gSigmaB, gL_alpha, gM_alpha, gL_betta, gM_betta;

  /**
   * Inicialisation of biharmonic material
   */
  TPZBiharmonic(int nummat, REAL f);

  virtual ~TPZBiharmonic();

  /**
   * Returns the number of norm errors. Default is 3: energy, L2,  H1, semi-norm H2 and H2.
   */
  virtual int NEvalErrors() {return 8;}

  void SetMaterial(REAL &xfin){
    fXf = xfin;
  }

  int Dimension() { return 2;}

  // Returns one because of scalar problem
  int NStateVariables(){
    return 1;
  };

  virtual void Print(std::ostream & out);

  virtual std::string Name() { return "TPZBiharmonic"; }

  //Implements integral over  element's volume
  virtual void Contribute(TPZMaterialData &data,
                            REAL weight,
			    TPZFMatrix &ek,
                            TPZFMatrix &ef);
  // Implements boundary conditions for continuous Galerkin
  virtual void ContributeBC(TPZMaterialData &data,
                              REAL weight,
			      TPZFMatrix &ek,
                              TPZFMatrix &ef,
                              TPZBndCond &bc);

  virtual int VariableIndex(char *name);

  virtual int NSolutionVariables(int var);

  virtual int NFluxes(){ return 0;}

  virtual void Solution(TPZVec<REAL> &Sol,TPZFMatrix &DSol,TPZFMatrix &axes,int var,TPZVec<REAL> &Solout);


  /**compute the value of the flux function to be used by ZZ error estimator*/
  virtual void Flux(TPZVec<REAL> &x, TPZVec<REAL> &Sol, TPZFMatrix &DSol, TPZFMatrix &axes, TPZVec<REAL> &flux);


  void Errors(TPZVec<REAL> &x,TPZVec<REAL> &u,
	      TPZFMatrix &dudx, TPZFMatrix &axes, TPZVec<REAL> &flux,
	      TPZVec<REAL> &u_exact,TPZFMatrix &du_exact,TPZVec<REAL> &values);//Cedric


  virtual void ContributeInterface(TPZMaterialData &data,
                                     REAL weight,
                                     TPZFMatrix &ek,
                                     TPZFMatrix &ef);


  virtual void ContributeBCInterface(TPZMaterialData &data,
                                       REAL weight,
                                       TPZFMatrix &ek,
                                       TPZFMatrix &ef,
                                       TPZBndCond &bc);

};

#endif
