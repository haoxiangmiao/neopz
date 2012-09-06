/**
 * @file
 * @brief Contains implementations of the TPZSubMeshAnalysis methods: implementation of the TPZSubMeshAnalysis class.
 */

#include "pzsmanal.h"
#include "pzsubcmesh.h"
#include "pzfmatrix.h"
#include "pzstrmatrix.h"
#include "pzsolve.h"
#include "pzlog.h"
using namespace std;
#ifdef LOG4CXX
static LoggerPtr logger(Logger::getLogger("pz.analysis.subcmeshanalysis"));
#endif
// Construction/Destruction

TPZSubMeshAnalysis::TPZSubMeshAnalysis(TPZSubCompMesh *mesh) : TPZAnalysis(mesh), fReducableStiff(0){
	fMesh = mesh;
    fReferenceSolution.Redim(fCompMesh->NEquations(),1);
}

TPZSubMeshAnalysis::~TPZSubMeshAnalysis()
{
	
}

void TPZSubMeshAnalysis::Assemble(){
	
	std::cout << "Assembling the SubCompMesh index " << fMesh->Index() << std::endl;
	int numeq = fCompMesh->NEquations();
	int numinternal = fMesh->NumInternalEquations();
	fReferenceSolution.Redim(numeq,1);
	fRhs.Redim(numeq,1);
    if(!fReducableStiff) 
    {
        fReducableStiff = new TPZMatRed<STATE, TPZFMatrix<STATE> > ();
    }
	fReducableStiff->Redim(numeq,numinternal);
	TPZMatRed<STATE, TPZFMatrix<STATE> > *matred = dynamic_cast<TPZMatRed<STATE, TPZFMatrix<STATE> > *> (fReducableStiff.operator->());
    if(!fSolver->Matrix())
    {
        fSolver->SetMatrix(fStructMatrix->Create());	
    }
    matred->SetMaxNumberRigidBodyModes(fMesh->NumberRigidBodyModes());
	//	fReducableStiff.SetK00(fSolver->Matrix());
	// this will initialize fK00 too
	matred->SetSolver(dynamic_cast<TPZMatrixSolver<STATE> *>(fSolver->Clone()));
	//	TPZStructMatrix::Assemble(fReducableStiff,fRhs, *fMesh);
	time_t before = time (NULL);
	fStructMatrix->Assemble(fReducableStiff,fRhs,fGuiInterface);
	time_t after = time(NULL);
	double diff = difftime(after, before);
	std::cout << __PRETTY_FUNCTION__ << " tempo " << diff << std::endl;
}

void TPZSubMeshAnalysis::Run(std::ostream &out){
	
	//fReducableStiff.Print("Reducable stiff before assembled");
	fReferenceSolution = fSolution;
	time_t tempo = time(NULL);
	Assemble();
	time_t tempodepois = time(NULL);
	double elapsedtime = difftime(tempodepois, tempo);
	
	std::cout << "Tempo para assemblagem " << elapsedtime << std::endl;
	if (!fReducableStiff) {
        DebugStop();
    }
	TPZMatRed<STATE, TPZFMatrix<STATE> > *matred = dynamic_cast<TPZMatRed<STATE, TPZFMatrix<STATE> > *> (fReducableStiff.operator->());
	matred->SetF(fRhs);
}
void TPZSubMeshAnalysis::CondensedSolution(TPZFMatrix<STATE> &ek, TPZFMatrix<STATE> &ef){
	time_t tempo = time(NULL);
	if (!fReducableStiff) {
        DebugStop();
    }
	TPZMatRed<STATE, TPZFMatrix<STATE> > *matred = dynamic_cast<TPZMatRed<STATE, TPZFMatrix<STATE> > *> (fReducableStiff.operator->());
//	ek = matred->K11Red();
//	ef = matred->F1Red();
////caravagio
	matred->F1Red(ef);
    matred->K11Reduced(ek, ef);
////
	time_t tempodepois = time(NULL);
	double elapsedtime = difftime(tempodepois, tempo);
	
	std::cout << "Tempo para inversao " << elapsedtime << std::endl;
	
}

void TPZSubMeshAnalysis::LoadSolution(const TPZFMatrix<STATE> &sol)
{
#ifdef LOG4CXX
    {
        std::stringstream sout;
        fReferenceSolution.Print("fReferenceSolution", sout);
        LOGPZ_DEBUG(logger,sout.str())
    }
#endif
	//	sol.Print("sol");
	int numinter = fMesh->NumInternalEquations();
	int numeq = fMesh->TPZCompMesh::NEquations();
	TPZFMatrix<STATE> soltemp(numeq-numinter,1,0.);
	int i;
	for(i=0; i<numeq-numinter; i++) {
		soltemp(i,0) = sol.GetVal(numinter+i,0)-fReferenceSolution(numinter+i,0);
	}
	TPZFMatrix<STATE> uglobal(numeq,1,0.);
    if(fReducableStiff)
    {
        TPZMatRed<STATE, TPZFMatrix<STATE> > *matred = dynamic_cast<TPZMatRed<STATE, TPZFMatrix<STATE> > *> (fReducableStiff.operator->());
        matred->UGlobal(soltemp,uglobal);        
        fSolution = fReferenceSolution + uglobal;
    }
#ifdef LOG4CXX
    {
        std::stringstream sout;
        fSolution.Print("fSolution", sout);
        LOGPZ_DEBUG(logger,sout.str())
    }
#endif
	TPZAnalysis::LoadSolution();
}
