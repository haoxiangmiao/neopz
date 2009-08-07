// -*- c++ -*-

//$Id: pzstrmatrix.cpp,v 1.27 2009-08-07 19:10:15 phil Exp $

/* Generated by Together */

#include "pzstrmatrix.h"
#include "pzvec.h"
#include "pzfmatrix.h"
#include "pzmanvector.h"
#include "pzadmchunk.h"
#include "pzcmesh.h"
#include "pzgmesh.h"
#include "pzelmat.h"
#include "pzcompel.h"
#include "pzintel.h"

#include "pzgnode.h"
#include "TPZTimer.h"
#include "TPZMTAssemble.h"

using namespace std;

#include "pzlog.h"

#ifdef LOG4CXX
static LoggerPtr logger(Logger::getLogger("pz.strmatrix.tpzstructmatrix"));
static LoggerPtr loggerel(Logger::getLogger("pz.strmatrix.element"));
#endif

TPZStructMatrix::TPZStructMatrix(TPZCompMesh *mesh) : fMinEq(-1), fMaxEq(-1) {
    fMesh = mesh;
}

TPZStructMatrix::~TPZStructMatrix() {}

TPZMatrix *TPZStructMatrix::Create() {
  cout << "TPZStructMatrix::Create should never be called\n";
  return 0;
}

TPZStructMatrix *TPZStructMatrix::Clone() {
  cout << "TPZStructMatrix::Clone should never be called\n";
  return 0;
}

void TPZStructMatrix::Assemble(TPZMatrix & stiffness, TPZFMatrix & rhs){
  if (fMesh){
    TPZStructMatrix::Assemble(stiffness, rhs, *fMesh,fMinEq,fMaxEq, NULL);
  }
}

/** STATIC METHOD **/
#ifdef WIN32
#include "threadExecuteProdIndex.h"

void TPZStructMatrix::Assemble(TPZMatrix & stiffness, TPZFMatrix & rhs, TPZCompMesh &mesh,
															int mineq, int maxeq, std::set<int> *MaterialIds,
															TExecuteProdIndex* myThread){
#else
void TPZStructMatrix::Assemble(TPZMatrix & stiffness, TPZFMatrix & rhs, TPZCompMesh &mesh,
															int mineq, int maxeq, std::set<int> *MaterialIds){
#endif
  int iel;
  int nelem = mesh.NElements();
  TPZElementMatrix ek(&mesh, TPZElementMatrix::EK),ef(&mesh, TPZElementMatrix::EF);

#ifndef WIN32
	TPZTimer calcstiff("Computing the stiffness matrices");
	TPZTimer assemble("Assembling the stiffness matrices");
#endif
  TPZAdmChunkVector<TPZCompEl *> &elementvec = mesh.ElementVec();

  for(iel=0; iel < nelem; iel++) {
    TPZCompEl *el = elementvec[iel];
    if(!el) continue;

    if(MaterialIds){
      TPZAutoPointer<TPZMaterial> mat = el->Material();
      if (!mat) continue;
      int matid = mat->Id();
      if (MaterialIds->find(matid) == MaterialIds->end()) continue;
    }///if

#ifndef WIN32
		calcstiff.start();
#endif

#ifdef WIN32
	if(myThread) if(myThread->AmIKilled()){
		return;
	}
#endif

		el->CalcStiff(ek,ef);

#ifndef WIN32
		calcstiff.stop();
		assemble.start();
#endif

    if(!el->HasDependency()) {
      ek.ComputeDestinationIndices();
      if(mineq != -1 && maxeq != -1)
      {
        FilterEquations(ek.fSourceIndex,ek.fDestinationIndex,mineq,maxeq);
      }
      stiffness.AddKel(ek.fMat,ek.fSourceIndex,ek.fDestinationIndex);
      rhs.AddFel(ef.fMat,ek.fSourceIndex,ek.fDestinationIndex);
#ifdef LOG4CXX
		if(loggerel->isDebugEnabled())
		{
			std::stringstream sout;
			ek.fMat.Print("Element stiffness matrix",sout);
			ef.fMat.Print("Element right hand side", sout);
			LOGPZ_DEBUG(loggerel,sout.str())
		}
#endif
    } else {
      // the element has dependent nodes
      ek.ApplyConstraints();
      ef.ApplyConstraints();
      ek.ComputeDestinationIndices();
      if(mineq != -1 & maxeq != -1)
      {
        FilterEquations(ek.fSourceIndex,ek.fDestinationIndex,mineq,maxeq);
      }
      stiffness.AddKel(ek.fConstrMat,ek.fSourceIndex,ek.fDestinationIndex);
      rhs.AddFel(ef.fConstrMat,ek.fSourceIndex,ek.fDestinationIndex);
		}

#ifndef WIN32
		assemble.stop();
#endif		
  }//fim for iel
#ifdef LOG4CXX
  {
    std::stringstream sout;
    sout << "Number of equations " << stiffness.Rows() << std::endl;
    sout << calcstiff.processName() << " " << calcstiff << std::endl;
    sout << assemble.processName() << " " << assemble;
/*    stiffness.Print("Matriz de Rigidez: ",sout);
    rhs.Print("Vetor de Carga: ",sout);*/
    LOGPZ_DEBUG(logger,sout.str().c_str());
  }
#endif

}

/** STATIC METHOD **/
// ofstream effile("serialEF.txt");
// #define MTCALCSTIFF
void TPZStructMatrix::Assemble(TPZFMatrix & rhs, TPZCompMesh &mesh, int mineq, int maxeq, std::set<int> *MaterialIds){

#ifdef MTCALCSTIFF
  TPZMTAssemble::AssembleMT(rhs, mesh, mineq, maxeq, MaterialIds);
  return;
#endif

  int iel;
  int nelem = mesh.NElements();
//   TPZElementMatrix ef(&mesh, TPZElementMatrix::EF);

#ifndef WIN32
	TPZTimer calcresidual("Computing the residual vector");
	TPZTimer assemble("Assembling the residual vector");
#endif

  TPZAdmChunkVector<TPZCompEl *> &elementvec = mesh.ElementVec();

  for(iel=0; iel < nelem; iel++) {
    TPZCompEl *el = elementvec[iel];
    if(!el) continue;

    if(MaterialIds){
      TPZAutoPointer<TPZMaterial> mat = el->Material();
      if (!mat) continue;
      int matid = mat->Id();
      if (MaterialIds->find(matid) == MaterialIds->end()) continue;
    }///if
    
    TPZElementMatrix ef(&mesh, TPZElementMatrix::EF);

#ifndef WIN32
		calcresidual.start();
#endif

		el->CalcResidual(ef);

#ifndef WIN32
		calcresidual.stop();
#endif
    
//     effile << "\n************** " << el->Reference()->Type() << " **************\n";
//     el->Print(effile);
//     ef.Print(effile);
//     effile.flush();

#ifndef WIN32
		assemble.start();
#endif

    if(!el->HasDependency()) {
      ef.ComputeDestinationIndices();
      if(mineq != -1 & maxeq != -1)
      {
        FilterEquations(ef.fSourceIndex,ef.fDestinationIndex,mineq,maxeq);
      }
      rhs.AddFel(ef.fMat, ef.fSourceIndex, ef.fDestinationIndex);
    } else {
      // the element has dependent nodes
      ef.ApplyConstraints();
      ef.ComputeDestinationIndices();
      if(mineq != -1 & maxeq != -1)
      {
        FilterEquations(ef.fSourceIndex,ef.fDestinationIndex,mineq,maxeq);
      }
      rhs.AddFel(ef.fConstrMat,ef.fSourceIndex,ef.fDestinationIndex);
		}

#ifndef WIN32
		assemble.stop();
#endif

  }//fim for iel
#ifdef LOG4CXX
  {
    std::stringstream sout;
    sout << calcresidual.processName() << " " << calcresidual << std::endl;
    sout << assemble.processName() << " " << assemble;
    LOGPZ_DEBUG(logger,sout.str().c_str());
  }
#endif

}

  /// filter out the equations which are out of the range
void TPZStructMatrix::FilterEquations(TPZVec<int> &origindex, TPZVec<int> &destindex, int mineq, int upeq)
{
  if(mineq == -1 || upeq == -1) return;
  int count = 0;
  int nel = origindex.NElements();
  int i;
  for(i=0; i<nel; i++)
  {
    if(destindex[i] >= mineq && destindex[i] < upeq)
    {
      origindex[count] = origindex[i];
      destindex[count++] = destindex[i]-mineq;
    }
  }
  origindex.Resize(count);
  destindex.Resize(count);
  
}

TPZMatrix * TPZStructMatrix::CreateAssemble(TPZFMatrix &rhs)
{
    TPZMatrix *stiff = Create();
    int neq = stiff->Rows();
    rhs.Redim(neq,1);
    Assemble(*stiff,rhs);
	
#ifdef LOG4CXX
	if(loggerel->isDebugEnabled())
	{
		std::stringstream sout;
		stiff->Print("Stiffness matrix",sout);
		rhs.Print("Right hand side", sout);
		LOGPZ_DEBUG(loggerel,sout.str())
	}
#endif
    return stiff;
	
}
