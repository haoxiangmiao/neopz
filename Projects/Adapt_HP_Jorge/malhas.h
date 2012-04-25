//$Id: malhas.h,v 1.2 2009-08-28 22:59:11 fortiago Exp $

class TPZGeoMesh;
class TPZCompMesh;
class TPZGeoEl;
#include <set>
#include <pzvec.h>

template<class TVar>
class TPZFMatrix;

TPZCompMesh *CreateMeshLaxAndSod(int L,REAL &timeStep);
void InitialSolutionLaxAndSod(TPZFMatrix<REAL> &InitialSol, TPZCompMesh * cmesh);

TPZCompMesh *CreateMeshLax2D(int L,REAL &timeStep);
void InitialSolutionLax2D(TPZFMatrix<REAL> &InitialSol, TPZCompMesh * cmesh);

/** For this to work PZ must be compiled with #define LinearConvection in file
 * pzeuler.h
 */
TPZCompMesh *CreateMeshLinearConvection(int L, REAL &timeStep);
void InitialSolutionLinearConvection(TPZFMatrix<REAL> &InitialSol, TPZCompMesh * cmesh);

