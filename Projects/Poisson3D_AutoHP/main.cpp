 /**
 * @file Poisson 3D in hexahedra with shock problem
 */

#include "tpzgeoelrefpattern.h"
#include "TPZGeoLinear.h"
#include "tpztriangle.h"
#include "pzgeoquad.h"
#include "pzgeopoint.h"
#include "pzgeotetrahedra.h"
#include "TPZGeoCube.h"
#include "pzgeopyramid.h"
#include "pzgeoprism.h"

#include "pzgeoelbc.h"

#include "pzlog.h"
#include "pzvec.h"
#include "pzadmchunk.h"
#include "pzcmesh.h"
#include "pzvec_extras.h"
#include "pzdebug.h"
#include "pzcheckgeom.h"
#include "pzcheckmesh.h"

#include "pzgeoel.h"
#include "pzgnode.h"
#include "pzgeoelside.h"
#include "pzgeoelbc.h"

#include "pzintel.h"
#include "pzcompel.h"

#include "pzmatrix.h"

#include "pzanalysis.h"
#include "pzfstrmatrix.h"
#include "pzskylstrmatrix.h"
#include "TPZSSpStructMatrix.h"
#include "pzbstrmatrix.h"
#include "pzstepsolver.h"
#include "TPZFrontStructMatrix.h"
#include "TPZParFrontStructMatrix.h"

#include "TPZParSkylineStructMatrix.h"
#include "pzsbstrmatrix.h"
#include "pzfstrmatrix.h"

#include "pzmaterial.h"
#include "pzbndcond.h"
#include "pzelasmat.h"
#include "pzplaca.h"
#include "pzpoisson3d.h"
#include "pzmathyperelastic.h"
#include "pzmattest3d.h"
#include "pzmatplaca2.h"

#include "pzfunction.h"

#include "pzgengrid.h"
#include "TPZExtendGridDimension.h"
#include "TPZReadGIDGrid.h"
#include "TPZVTKGeoMesh.h"

#include "pzshapelinear.h"

#include "TPZRefPatternTools.h"

#include <time.h>
#include <stdio.h>
#include <math.h>

#include <fstream>
#include <cmath>


#include "problem.h"
#include "TPZCreateHDivMesh.h"
#include "pzbuildmultiphysicsmesh.h"
#include "pzcondensedcompel.h"

#include "CreateAndRefineMeshes.h"

using namespace std;
using namespace pzshape;
using namespace pzgeom;


/*** Data Types needed ***/
// Simulation Case
struct SimulationCase {
    bool  IsHdivQ;
    int   n_acc_terms;
    int   eltype;
    int   nthreads;
    std::string  dir_name;
    
    SimulationCase() : IsHdivQ(false), n_acc_terms(0), eltype(7), nthreads(0), dir_name("dump") {
        
    }
    SimulationCase(const SimulationCase &other) : IsHdivQ(other.IsHdivQ), n_acc_terms(other.n_acc_terms), eltype(other.eltype), nthreads(other.nthreads), dir_name(other.dir_name) {
        
    }
};


/**  Global variables  */
REAL GlobScale = 1.;
// Maximum number of equations allowed
long MaxEquations = 1500000;
// Input - output
ofstream out("OutPoissonArcTan.txt",ios::app);             // To store output of the console
// ABOUT H P ADAPTIVE
int MaxPOrder = 9;     // Maximum order for p refinement allowed
int MaxHLevel = 7;      // Maximum level for h refinement allowed
int MaxHUsed = 0;
int MaxPUsed = 0;

int ninitialrefs = 4;

// Poisson problem
STATE ValueK = 100000;
STATE F = sqrt(ValueK);
int ModelDimension;
// Circunference with high gradient - data
TPZManVector<REAL,3> CCircle(3,0.5);
REAL RCircle = 0.25;

//**********   Creating computational mesh with materials    *************
TPZCompMesh *CreateComputationalMesh(TPZGeoMesh *gmesh,int dim,int materialId,int hasforcingfunction,int id_bc0,int id_bc1=0,int id_bc2=0);


int DefineDimensionOverElementType(int typeel);
void GetFilenameFromGID(MElementType typeel, std::string &name);

/** PROBLEMS */
bool SolveSymmetricPoissonProblemOnCubeMesh(SimulationCase &sim_case);

/**
 * Get Global L2 Error for solution and the L2 error for each element.
 * Return the maxime L2 error by elements. Also return in MinErrorByElement argument the minime L2 error for all elements of the mesh.
 */
REAL ProcessingError(TPZAnalysis &analysis,TPZVec<REAL> &ervec,TPZVec<REAL> &ervecbyel,TPZVec<REAL> &gradervecbyel,REAL &MinErrorByElement,REAL &);
void LoadSolutionFirstOrder(TPZCompMesh *cmesh, void (*f)(const TPZVec<REAL> &loc, TPZVec<STATE> &result, TPZFMatrix<STATE> &deriv,TPZVec<STATE> &ddsol));

bool ApplyingStrategyHPAdaptiveBasedOnErrorOfSolution(TPZCompMesh *cmesh,TPZVec<REAL> &ervecbyel,TPZVec<REAL> &gradervecbyel,REAL MaxErrorByElement,REAL &MinErrorByElement,REAL &MinGrad,int ref,int itypeel,REAL &factorError);
bool ApplyingStrategyHPAdaptiveBasedOnErrorOfSolutionAndGradient(TPZCompMesh *cmesh,TPZVec<REAL> &ervecbyel,TPZVec<REAL> &gradervecbyel,REAL MaxErrorByElement,REAL &MinErrorByElement,REAL &MinGrad,int ref,int itypeel,TPZVec<REAL> &Tol);
void ApplyingStrategyPAdaptiveBasedOnExactSphereSolution(TPZCompMesh *cmesh,TPZVec<REAL> &ervecbyel,TPZVec<REAL> &gradervecbyel,REAL MaxErrorByElement,REAL &MinErrorByElement,int ref);

// Writing a relation between number of degree of freedom and L2 error.
bool PrintResultsInMathematicaFormat(TPZVec<REAL> &ErrrVec,TPZVec<long> &NEquations,std::ostream &fileerrors);

void AdjustingOrder(TPZCompMesh *cmesh);
int MaxLevelReached(TPZCompMesh *cmesh);


/** Utilitaries Over Date And Time */
void formatTimeInSec(char *strtime,int lenstrtime,int timeinsec);
bool CreateCurrentResultDirectory(SimulationCase &sim);

#ifdef LOG4CXX
static LoggerPtr  logger(Logger::getLogger("pz.refine"));
#endif



// MAIN FUNCTION TO NUMERICAL SOLVE WITH AUTO ADAPTIVE HP REFINEMENTS
/** Laplace equation on square 1D 2D 3D - Volker John article 2000 */

int main(int argc,char *argv[]) {
    
#ifdef LOG4CXX
	InitializePZLOG();
#endif
	
	// Initializing uniform refinements for reference elements
	gRefDBase.InitializeAllUniformRefPatterns();

	// Getting input data
	// 1 -> line		2 -> triangles		3 -> quadrilateral
    // 4 -> tetraedro	5 -> pyramid		6 -> prisma				7 -> cubo
    struct SimulationCase dummied;

	// Type of elements
	int itypeel = 3;
	// number of initial refinements over original mesh
	if (ninitialrefs > 3) {
		if (itypeel < 4)
			ninitialrefs = 2;
		else
			ninitialrefs = 1;
	}

	int count = 0;
	do {
		if(argc > 1)
			itypeel = atoi(argv[count+1]);
		if(itypeel > 7 || itypeel < 2)
			itypeel = 7;
		count++;
		dummied.eltype = itypeel;
		// Solving symmetricPoissonProblem on [0,1]^d with d=1, d=2 and d=3
		if(!SolveSymmetricPoissonProblemOnCubeMesh(dummied))
			return 1;

		itypeel++;
	} while(count < argc-1);
	
	return 0;
}

bool SolveSymmetricPoissonProblemOnCubeMesh(SimulationCase &sim_case) {
    if(CreateCurrentResultDirectory(sim_case))
        return false;
    
    // To compute processing times
    time_t sttime;
    time_t endtime;
    int time_elapsed;
    char * ptime; // = time_formated;

	// Tolerance for applying hp adaptivity
	TPZVec<REAL> Tol(3, 1.e-6);
    Tol[1] = sqrt(Tol[0]); Tol[2] = sqrt(sqrt(Tol[1]));

	int materialId = 1;
	int id_bc0 = -1;
	int id_bc1 = -2;
	// Generic data for problems to solve
	int NRefs = 10;

	// Output files
	std::stringstream sout;
	sout << sim_case.dir_name.c_str() << "/ErrorsHP_Poisson.txt";
	std::ofstream fileerrors(sout.str().c_str());   // To store all errors calculated by TPZAnalysis (PosProcess)

													// Initial message to print computed errors
	time(&sttime);
	ptime = ctime(&sttime);
	fileerrors << "\nApproximation Error in " << ptime << std::endl << "\nType of element: " << sim_case.eltype << endl;

	// Initializing the vectors of errors to store the errors for any iteration
	TPZVec<REAL> ervec, ErrorVec(100,0.0);
	TPZVec<long> NEquations(100,0L);
	TPZVec<REAL> ervecbyel;
	TPZVec<REAL> gradervecbyel;

	/** Solving for type of geometric elements */
	TPZGeoMesh *gmesh;
	gmesh = CreateGeomMesh(sim_case.eltype,materialId,id_bc0,id_bc1);
	ModelDimension = DefineDimensionOverElementType(sim_case.eltype);
	UniformRefinement(ninitialrefs, gmesh, ModelDimension);

	// Printing initial geometric mesh
	sout.clear();
	sout << sim_case.dir_name.c_str() << "/InitialGMesh_" << ModelDimension << "D_E" << sim_case.eltype << ".vtk";
	ofstream arg2(sout.str().c_str());
	TPZVTKGeoMesh::PrintGMeshVTK(gmesh, arg2);

	/** Variable names for post processing */
	TPZStack<std::string> scalnames, vecnames;
	scalnames.Push("POrder");
	scalnames.Push("Pressure");

	// Printing solution on mesh with initial mesh before of adaptive process
//	TPZGeoMesh *gmeshfirst = CreateGeomMesh(typeel,materialId,id_bc0,id_bc1);
	TPZGeoMesh gmeshfirst(*gmesh);
	TPZCompEl::SetgOrder(1);
	TPZCompMesh *cmeshfirst = CreateComputationalMesh(&gmeshfirst,ModelDimension,materialId,1,id_bc0,id_bc1);
	TPZAnalysis an_sol(cmeshfirst,false);
	LoadSolutionFirstOrder(cmeshfirst,ExactSolutionArcTangent);

	sout.clear();
	sout << sim_case.dir_name.c_str() << "/Poisson" << ModelDimension << "D_MESHINIT_E" << sim_case.eltype << "WITHOUTREF" << ".vtk";
	an_sol.DefineGraphMesh(ModelDimension,scalnames,vecnames,sout.str());

	an_sol.PostProcess(3,ModelDimension);
	long countels = 0;
	for(int ii=0;ii<cmeshfirst->NElements();ii++) {
		if(!cmeshfirst->ElementVec()[ii] || cmeshfirst->ElementVec()[ii]->Dimension()!=ModelDimension) continue;
		countels++;
	}
	out << std::endl << "Number of elements 2D: " << countels << std::endl << std::endl ;
	delete cmeshfirst;
	gmeshfirst.CleanUp();
            
	// Creating computational mesh (approximation space and materials)
	int p = 1, pinit;
	MaxPUsed = pinit = p;
	MaxHUsed = 1;
	TPZCompEl::SetgOrder(p);
	TPZCompMesh *cmesh;
	gmesh->SetName("Malha Geometrica original");
    
	//   ?????????????????????????????????????????????????????????????????????????????????????????????????
    int n_meshes = 0;
    if (sim_case.IsHdivQ) {
        n_meshes = 2;
    }
    
    TPZManVector<TPZCompMesh *,2> meshvec(n_meshes,0);

    int hdivplusplus = sim_case.n_acc_terms;
    if(meshvec.size() == 0)
    {
        cmesh = CreateComputationalMesh(gmesh,ModelDimension,materialId,1,id_bc0,id_bc1);     // Forcing function is out 2013_07_25
    }
    else{
        cmesh = CreateHDivMesh(gmesh, meshvec, p, ModelDimension,hdivplusplus);
    }
	// To storing number of equations and errors obtained for all iterations
	ErrorVec.Resize(NRefs);
	ErrorVec.Fill(0.0L);
	NEquations.Resize(NRefs);
	NEquations.Fill(0L);
    if(meshvec.size() == 0)
    {
        AdjustFluxPolynomialOrders(cmesh, hdivplusplus);
    }
    else
    {
        ReconstructHDivMesh(cmesh, meshvec, hdivplusplus);
    }

	int nref = 0;
	REAL MaxErrorByElement = 1.e7;

	// loop solving iteratively
	do {
		out << "\nSolving Poisson problem " << ModelDimension << "D. Refinement: " << nref << " TypeElement: " << sim_case.eltype << endl;
		std::cout << "\nSolving Poisson problem. Type of element: " << sim_case.eltype << std::endl;
				
		// Initializing the generation mesh process
		time(& sttime);
				
		// Introduzing exact solution depending on the case
		// Solving adaptive process
        cmesh->CleanUpUnconnectedNodes();
        
        
		TPZAnalysis an(cmesh,true);
		an.SetExact(ExactSolutionArcTangent);

		sout.clear();
		sout << sim_case.dir_name.c_str() << "/" << "Poisson" << ModelDimension << "D_E" << sim_case.eltype << "H" << std::setprecision(2) << nref << "P" << pinit << ".vtk";
		an.DefineGraphMesh(ModelDimension,scalnames,vecnames,sout.str().c_str());
        
		cmesh->SetName("Malha computacional adaptada");

		// Solve using symmetric matrix then using Cholesky (direct method)
        TPZParFrontStructMatrix<TPZFrontSym<STATE> > strmat(cmesh);
        strmat.SetNumThreads(8);
        strmat.SetDecomposeType(ELDLt);
		
		TPZStepSolver<STATE> *direct = new TPZStepSolver<STATE>;
		direct->SetDirect(ELU);
		an.SetSolver(*direct);
		delete direct;
		direct = 0;
				
		out << "\tRefinement: " << nref << " TypeElement: " << sim_case.eltype << "NEquations " << cmesh->NEquations() << "\n";
		an.Assemble();
        an.Solve();
        
        UnwrapMesh(cmesh);
		
        if(! meshvec.size())
        {
            TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(meshvec, cmesh);
        }
        
		// Post processing
		an.PostProcess(1,ModelDimension);
		std::ofstream out(sout.str().c_str());
		cmesh->LoadReferences();
		TPZVTKGeoMesh::PrintGMeshVTK(cmesh->Reference(),out,false);
                
		// generation mesh process finished
		time(&endtime);
		time_elapsed = endtime - sttime;
		formatTimeInSec(ptime,256,time_elapsed);
		out << "  Time elapsed " << time_elapsed << " <-> " << ptime << "\n\n";
		fileerrors << "  Time elapsed " << time_elapsed << " <-> " << ptime << "\n";
		std::cout << "  Time elapsed " << time_elapsed << " <-> " << ptime << "\n\n";
				
		REAL MinErrorByElement, MinGradErrorByElement;
		ervecbyel.Resize(0);
		gradervecbyel.Resize(0);
		MaxErrorByElement = ProcessingError(an,ervec,ervecbyel,gradervecbyel,MinErrorByElement,MinGradErrorByElement);
		// Printing obtained errors
		if(ervec[1] > 10. || ervec[1] < 0.) {
			std::cout << "L2 Error is wrong (BIG?!) By! \n\n";
//			break;
		}
		ErrorVec[nref] = ervec[1];
		NEquations[nref] = cmesh->NEquations();

		std::cout << "\n NRef " << nref << "\tL2 Error " << ervec[1] << "  NEquations: " << NEquations[nref] << " PUsed " << MaxPUsed << " HMax " << MaxHUsed << std::endl << std::endl;
		out << "\n NRef " << nref << "\tL2 Error " << ervec[1] << "  NEquations: " << NEquations[nref] << " PUsed " << MaxPUsed << " HMax " << MaxHUsed << std::endl << std::endl;
		if(cmesh->NEquations() > MaxEquations) {
			NRefs = nref+1;							// final iteration
			ErrorVec.Resize(NRefs);
			NEquations.Resize(NRefs);
			continue;
		}
		fileerrors.flush();
		out.flush();

		// HP Refinement Process
		if(NRefs > 1 && nref < (NRefs-1)) {
			out << "\n\nApplying Adaptive Methods... step " << nref << "\n";
			std::cout << "\n\nApplying Adaptive Methods... step " << nref << "\n";
			while(!ApplyingStrategyHPAdaptiveBasedOnErrorOfSolutionAndGradient(cmesh,ervecbyel,gradervecbyel,MaxErrorByElement,MinErrorByElement,MinGradErrorByElement,nref, sim_case.eltype,Tol)) {
				out << "\nStrategy based on error over solution and gradient failed.\n " << std::endl;
				nref = NRefs;
				break;
			}
		}
        std::cout << "NElements " << cmesh->NElements() << " NEquations " << cmesh->NEquations() << std::endl;

        // refazer a malha multifisica
        if (meshvec.size() == 0)
            AdjustFluxPolynomialOrders(cmesh, hdivplusplus);
        else
            ReconstructHDivMesh(cmesh, meshvec, hdivplusplus);

		fileerrors.flush();
		out.flush();
		// Sometimes Writing a relation between number of degree of freedom and L2 error.
        if (meshvec.size() == 0) {
            fileerrors << "H1 approximation\n";
            fileerrors << "H1plusplus = " << hdivplusplus << std::endl;
        }
        else
        {
            fileerrors << "HDiv approximation\n";
            fileerrors << "HDivplusplus = " << hdivplusplus << std::endl;
        }
        PrintResultsInMathematicaFormat(ErrorVec,NEquations,fileerrors);
        fileerrors.flush();
        fileerrors << "done\n";

		nref++;
	}while (nref < NRefs && MaxErrorByElement > Tol[0]);

	if(cmesh)
		delete cmesh;
	cmesh = NULL;
	if(gmesh)
		delete gmesh;
	gmesh = NULL;

	// Writing a relation between number of degree of freedom and L2 error.
	sout.clear();
	sout << sim_case.dir_name.c_str() << "/ErrorsHP_Poisson.nb";
	std::ofstream finalerrors(sout.str().c_str());   // To store all errors calculated by TPZAnalysis (PosProcess)
	if(!PrintResultsInMathematicaFormat(ErrorVec,NEquations,finalerrors))
		std::cout << "\nThe errors and nequations values in Mathematica format was not done.\n";
	
	fileerrors << std::endl << "Finished running for element " << sim_case.eltype << std::endl << std::endl;
	fileerrors.close();
	std::cout << std::endl << "\tFinished running for element " << sim_case.eltype << std::endl << std::endl;
	out.close();
	return true;
}

bool ApplyingStrategyHPAdaptiveBasedOnErrorOfSolutionAndGradient(TPZCompMesh *cmesh,TPZVec<REAL> &ervecbyel,TPZVec<REAL> &gradervecbyel,REAL MaxErrorByElement,REAL &MinErrorByElement,REAL &MinGrad,int nref,int itypeel,TPZVec<REAL> &Tol ) {
	if(!cmesh) return false;
	bool result = true;
	long nels = cmesh->NElements();
    
	TPZManVector<long,27> subels;
	TPZManVector<long,27> subsubels;
	TPZInterpolatedElement *el;
	// To see where the computation is doing
	TPZVec<long> counterreftype(50,0);
    
	REAL factorGrad = .5;
	REAL factorSmall = .1;
	REAL factorErrorBig = 0.8;
    
	REAL BigError = factorErrorBig*MaxErrorByElement + (1.-factorErrorBig)*MinErrorByElement;
	REAL SmallError = factorSmall*MaxErrorByElement + (1. - factorSmall)*MinErrorByElement;
	REAL MaxGrad = factorGrad*gradervecbyel[nels] + (1.-factorGrad)*MinGrad;
	REAL SmallGrad = factorSmall*gradervecbyel[nels] + (1.-factorSmall)*MinGrad;

    
	REAL LaplacianVal;
	REAL MaxLaplacianVal, MinLaplacianVal;
    
	REAL factorLap = 0.7;
	ComputingMaxLaplacian(cmesh,MaxLaplacianVal,MinLaplacianVal);

    REAL LimitLaplace = factorLap*MaxLaplacianVal + (1.-factorLap)*MinLaplacianVal;
    REAL MediumError = 0.5*(MaxErrorByElement + MinErrorByElement);

	/* Printing maximum and minimun values of the errors */
    out << "Erro ->   Max " << MaxErrorByElement << "    Min " << MinErrorByElement << "\nGrad ->   Max " << gradervecbyel[nels] << "   Min " << MinGrad << std::endl;
    out << "MaxGrad " << MaxGrad << "  SmallGrad " << SmallGrad << "    BigError " << BigError << "  SError " << SmallError << "  Tolerance " << Tol[0] << std::endl;
    cout << "Erro ->   Max " << MaxErrorByElement << "    Min " << MinErrorByElement << "\nGrad ->   Max " << gradervecbyel[nels] << "   Min " << MinGrad << std::endl;
    cout << "MaxGrad " << MaxGrad << "  SmallGrad " << SmallGrad << "    BigError " << BigError << "  SError " << SmallError << "  Tolerance " << Tol[0] << std::endl;
    
	// Applying hp refinement only for elements with dimension as model dimension
    std::cout << "Refinando malha com " << nels  << " elementos\n";
    TPZCompMesh *refinemesh = cmesh;
	for(long iel=0L;iel<nels;iel++) {
		bool hused = false, pused = false;
		subels.Resize(0);
        TPZInterpolatedElement *derived = NULL;
        TPZCompEl *cel = cmesh->Element(iel);
		el = dynamic_cast<TPZInterpolatedElement* >(cel);
        if(!el)
        {
            
            TPZMultiphysicsElement *mfys = dynamic_cast<TPZMultiphysicsElement *>(cel);
            
            if (mfys) {
                derived = dynamic_cast<TPZInterpolatedElement*>(mfys->Element(0));
                refinemesh = derived->Mesh();
                TPZGeoMesh *gmesh = refinemesh->Reference();
                if (gmesh->Reference() != refinemesh) {
                    gmesh->ResetReference();
                    refinemesh->LoadReferences();
                }
            }
        }
        else
        {
            derived = el;
        }
		if(!derived || derived->Dimension()!=cmesh->Dimension()) {
			counterreftype[0]++;
			continue;
		}
        
		// element data
        TPZGeoEl *gel = derived->Reference();
        if(!gel) DebugStop();
		int pelement = derived->PreferredSideOrder(gel->NSides() - 1);
		pelement++;
		long index = iel;
		int level = derived->Reference()->Level();
        
        if(!LaplacianValue(cel,LaplacianVal)){
			DebugStop();
        }

        if(ervecbyel[index] > BigError && level < MaxHLevel) {
			if(gradervecbyel[index] > MaxGrad) {
				bool flag;
				flag = false;
				counterreftype[1]++;
				if(LaplacianVal > LimitLaplace && pelement<MaxPOrder) {
#ifdef LOG4CXX
                    {
                        std::stringstream sout;
                        sout << derived->Index() << " " << pelement << " " << 0 << " " << refinemesh->NEquations();
                        LOGPZ_DEBUG(logger, sout.str())
                    }
#endif
					derived->PRefine(pelement);
					pused = true;
					counterreftype[2]++;
					flag = true;
				}
#ifdef LOG4CXX
                {
                    std::stringstream sout;
                    sout << derived->Index() << " " << 0 << " " << 1 << " " << refinemesh->NEquations();
                    LOGPZ_DEBUG(logger, sout.str())
                }
#endif
				derived->Divide(derived->Index(),subels);
                cel = cmesh->Element(iel);
                delete cel;
                level++;
                hused = true;
				if(!flag && level < MaxHLevel) {
					counterreftype[3]++;
					for(long isub_el=0;isub_el<subels.NElements();isub_el++) {
                        TPZCompEl * isub_cel = refinemesh->ElementVec()[subels[isub_el]];
						TPZInterpolatedElement *intel = dynamic_cast<TPZInterpolatedElement* >(isub_cel);
#ifdef LOG4CXX
                        {
                            std::stringstream sout;
                            sout << intel->Index() << " " << 0 << " " << 1 << " " << refinemesh->NEquations();
                            LOGPZ_DEBUG(logger, sout.str())
                        }
#endif
						intel->Divide(subels[isub_el],subsubels);
                        cel = cmesh->Element(iel);
                        delete cel;
					}
                    level++;
				}
			}
			else {
                level++;
                hused = true;
#ifdef LOG4CXX
                {
                    std::stringstream sout;
                    sout << derived->Index() << " " << 0 << " " << 1 << " " << refinemesh->NEquations();
                    LOGPZ_DEBUG(logger, sout.str())
                }
#endif
				derived->Divide(derived->Index(),subels);
                cel = cmesh->Element(iel);
                delete cel;
				counterreftype[4]++;
			}
			counterreftype[7]++;
		}
		else if(ervecbyel[index] > MediumError) {
			counterreftype[10]++;
			if((gradervecbyel[index] > MaxGrad) && level < MaxHLevel) {
                level++;
                hused = true;
				counterreftype[11]++;
#ifdef LOG4CXX
                {
                    std::stringstream sout;
                    sout << derived->Index() << " " << 0 << " " << 1 << " " << refinemesh->NEquations();
                    LOGPZ_DEBUG(logger, sout.str())
                }
#endif
				derived->Divide(derived->Index(),subels);
                cel = cmesh->Element(iel);
                delete cel;

			}
			else if(pelement<MaxPOrder) {
#ifdef LOG4CXX
                {
                    std::stringstream sout;
                    sout << derived->Index() << " " << pelement << " " << 0 << " " << refinemesh->NEquations();
                    LOGPZ_DEBUG(logger, sout.str())
                }
#endif
				derived->PRefine(pelement);
				pused = true;
				counterreftype[12]++;
			}
            else {
                counterreftype[13]++;
            }
		}
		else if(gradervecbyel[index] > SmallGrad || ervecbyel[index] > SmallError) {
			counterreftype[20]++;
			if(pelement < MaxPOrder) {
#ifdef LOG4CXX
                {
                    std::stringstream sout;
                    sout << derived->Index() << " " << pelement << " " << 0 << " "  << refinemesh->NEquations();
                    LOGPZ_DEBUG(logger, sout.str())
                }
#endif
				derived->PRefine(pelement);
				pused = true;
				counterreftype[21]++;
			}
            else
				counterreftype[22]++;
		}

		if(!pused && !hused) {
			counterreftype[40]++;
		}
		if(pused)
			MaxPUsed = (pelement > MaxPUsed) ? pelement : MaxPUsed;
		if(hused)
			MaxHUsed = (level > MaxHUsed) ? level : MaxHUsed;
	}
	refinemesh->ExpandSolution();
	if(!counterreftype[10] && !counterreftype[20]) {
		result = false;
	}
    RegularizeMesh(refinemesh->Reference(),refinemesh->Dimension());
    long nel = refinemesh->NElements();
    for (long el=0; el<nel; el++) {
        TPZCompEl *cel = refinemesh->Element(el);
        if (!cel) {
            continue;
        }
        TPZInterpolatedElement *intel = dynamic_cast<TPZInterpolatedElement *>(cel);
        if (!intel ) {
            TPZMultiphysicsElement *mfel = dynamic_cast<TPZMultiphysicsElement *>(cel);
            if (mfel) {
                intel = dynamic_cast<TPZInterpolatedElement *>(mfel->Element(0));
            }
        }
        if (!intel) {
            DebugStop();
        }
        TPZGeoEl *gel = intel->Reference();
        if (gel->HasSubElement()) {
            TPZManVector<long> subs;
#ifdef LOG4CXX
            {
                std::stringstream sout;
                sout << intel->Index() << " " << 0 << " " << 1 << " "  << refinemesh->NEquations();
                LOGPZ_DEBUG(logger, sout.str())
            }
#endif
            intel->Divide(intel->Index(), subs);
        }
    }
    refinemesh->ExpandSolution();
	// Printing information stored
	PrintNRefinementsByType(nref,nels,refinemesh->NElements(),counterreftype,out);
	PrintNRefinementsByType(nref,nels,refinemesh->NElements(),counterreftype);
	return result;
}

bool ApplyingStrategyHPAdaptiveBasedOnErrorOfSolutionAndGradientMorePOnGrad(TPZCompMesh *cmesh,TPZVec<REAL> &ervecbyel,TPZVec<REAL> &gradervecbyel,REAL MaxErrorByElement,REAL &MinErrorByElement,REAL &MinGrad,int nref,int itypeel,REAL &factorError) {
	if(!cmesh) return false;
	bool result = true;
	long nels = cmesh->NElements();

	TPZVec<long> subels;
	TPZVec<long> subsubels;
	int pelement;
	int level;
	TPZInterpolatedElement *el;
	// To see where the computation is doing
	long index = -1;
	TPZVec<long> counterreftype(50,0);
	long i, ii;

	REAL factorGrad = .6;
	REAL factorSGrad = .15;
	REAL factorErrorBig = 0.8;

	REAL BigError = factorErrorBig*MaxErrorByElement + (1.-factorErrorBig)*MinErrorByElement;
	REAL SmallError = factorError*MaxErrorByElement + (1.-factorError)*MinErrorByElement;
	REAL MaxGrad = factorGrad*gradervecbyel[nels] + (1.-factorGrad)*MinGrad;
	REAL SmallGrad = factorSGrad*gradervecbyel[nels] + (1.-factorSGrad)*MinGrad;

	/* Printing maximum and minimun values of the errors */
	out << "\nErro ->   Max " << MaxErrorByElement << "    Min " << MinErrorByElement << "\nGrad ->   Max " << gradervecbyel[nels] << "   Min " << MinGrad;
	out << "\nMaxGrad " << MaxGrad << "  SmallGrad " << SmallGrad << "    BigError " << BigError << "  SError " << SmallError << "  FactorError " << factorError;
	cout << "\nErro ->   Max " << MaxErrorByElement << "    Min " << MinErrorByElement << "\nGrad ->   Max " << gradervecbyel[nels] << "   Min " << MinGrad;
	cout << "\nMaxGrad " << MaxGrad << "  SmallGrad " << SmallGrad << "    BigError " << BigError << "  SError " << SmallError << "  FactorError " << factorError;

	// Applying hp refinement only for elements with dimension as model dimension
	for(i=0L;i<nels;i++) {
		bool hused = false, pused = false;
		subels.Resize(0);
		el = dynamic_cast<TPZInterpolatedElement* >(cmesh->ElementVec()[i]);
		if(!el || el->Dimension()!=cmesh->Dimension()) {
			counterreftype[0]++;
			continue;
		}

		// element data
		pelement = el->PreferredSideOrder(el->NConnects() - 1);
		pelement++;
		index = el->Index();
		level = el->Reference()->Level();

		if(nref < 6 && (gradervecbyel[i] > MaxGrad || ervecbyel[i] > BigError) && level < MaxHLevel) {
			counterreftype[10]++;
			el->Divide(index,subels);
			el = NULL;
			level++;
			hused = true;
			if(nref && pelement < MaxPOrder) {
				counterreftype[12]++;
				for(ii=0;ii<subels.NElements();ii++) {
					dynamic_cast<TPZInterpolatedElement* >(cmesh->ElementVec()[subels[ii]])->PRefine(pelement);
				}
				pused = true;
			}
			if(gradervecbyel[i] > MaxGrad && level < MaxHLevel) {
				counterreftype[11]++;
				for(ii=0;ii<subels.NElements();ii++) {
					cmesh->ElementVec()[subels[ii]]->Divide(subels[ii],subsubels);
					subels[ii]=0;
				}
				level++;
			}
		}
		else if((gradervecbyel[i] > SmallGrad || ervecbyel[i] > SmallError) && pelement < MaxPOrder && nref) {
			counterreftype[20]++;
			el->PRefine(pelement);
			pused = true;
		}
		else {
			counterreftype[30]++;
		}

		if(!pused && !hused) {
			counterreftype[40]++;
		}
		if(pused)
			MaxPUsed = (pelement > MaxPUsed) ? pelement : MaxPUsed;
		if(hused)
			MaxHUsed = (level > MaxHUsed) ? level : MaxHUsed;
	}
	cmesh->ExpandSolution();
	if(!counterreftype[10] && !counterreftype[20]) {
		result = false;
	}

	// Printing information stored
	PrintNRefinementsByType(nref,nels,cmesh->NElements(),counterreftype,out);
	PrintNRefinementsByType(nref,nels,cmesh->NElements(),counterreftype);
	return result;
}

bool ApplyingStrategyHPAdaptiveBasedOnErrorOfSolution(TPZCompMesh *cmesh,TPZVec<REAL> &ervecbyel,TPZVec<REAL> &gradervecbyel,REAL MaxErrorByElement,REAL &MinErrorByElement,REAL &MinGrad,int nref,int itypeel,REAL &factorError) {
	if(!cmesh) return false;
	bool result = true;
	long nels = cmesh->NElements();
	int dim = cmesh->Dimension();

	TPZVec<long> subels;
	TPZVec<long> subsubels;
	int pelement;
	int level;
	TPZInterpolatedElement *el;
	// To see where the computation is doing
	long index = -1;
	TPZVec<long> counterreftype(50,0);
	long i, ii;

	REAL factorGrad = .2;
	if(dim == 2) factorGrad = 1./3.;
	
	REAL SmallError = factorError*MaxErrorByElement + (1.-factorError)*MinErrorByElement;
	REAL MaxGrad = factorGrad*gradervecbyel[nels] + (1.-factorGrad)*MinGrad;

	TPZVec<STATE> Laplacian(1);
	TPZFMatrix<STATE> dLap(3);
	TPZVec<REAL> psi(3,0.);
	TPZVec<REAL> center(3,0.);

	/* Printing maximum and minimun values of the errors */
	out << "\nErro ->   Min " << MinErrorByElement << "    Max " << MaxErrorByElement << std::endl << "Grad ->   Min " << MinGrad << "   Max " << gradervecbyel[nels] << "\t";
	out << "\nMaxGrad " << MaxGrad << " Factor " << factorGrad << "     SError " << SmallError << " Factor " << factorError;

	// Applying hp refinement only for elements with dimension as model dimension
	for(i=0L;i<nels;i++) {
		bool hused = false, pused = false;
		subels.Resize(0);
		el = dynamic_cast<TPZInterpolatedElement* >(cmesh->ElementVec()[i]);
		if(!el || el->Dimension()!=cmesh->Dimension()) {
			counterreftype[0]++;
			continue;
		}

		// Getting center of the element
		el->Reference()->CenterPoint(el->Reference()->NSides()-1,psi);
		el->Reference()->X(psi,center);
		RightTermArcTangentBad(center,Laplacian,dLap);
		Laplacian[0] /= ValueK;

		// element data
		pelement = el->PreferredSideOrder(el->NConnects() - 1);
		pelement++;
		index = el->Index();
		level = el->Reference()->Level();

		// Applying hp refinement depends on high gradient and high laplacian value, and depends on computed error by element
		if(gradervecbyel[i] > MaxGrad && level < MaxHLevel) {
			counterreftype[10]++;
			el->Divide(index,subels);
			el = NULL;
			level++;
			hused = true;
		}
		if((Laplacian[0] > 10. || ervecbyel[i] > SmallError) && pelement < MaxPOrder && nref) {
			counterreftype[20]++;
			if(el)
				el->PRefine(pelement);
			else {
				for(ii=0;ii<subels.NElements();ii++) {
					dynamic_cast<TPZInterpolatedElement* >(cmesh->ElementVec()[subels[ii]])->PRefine(pelement);
				}
			}
			pused = true;
		}
		if(!pused && !hused) {
			counterreftype[30]++;
		}
		if(pused)
			MaxPUsed = (pelement > MaxPUsed) ? pelement : MaxPUsed;
		if(hused)
			MaxHUsed = (level > MaxHUsed) ? level : MaxHUsed;
	}
	cmesh->ExpandSolution();
	if(!counterreftype[10] && !counterreftype[20]) {
		result = false;
	}

	// Printing information stored
	PrintNRefinementsByType(nref,nels,cmesh->NElements(),counterreftype,out);
	PrintNRefinementsByType(nref,nels,cmesh->NElements(),counterreftype);
	return result;
}


/**
 * Get Global L2 Error for solution and the L2 error for each element.
 * Return the maxime L2 error by elements. Also return in MinErrorByElement argument the minime L2 error for all elements of the mesh.
 */

REAL ProcessingError(TPZAnalysis &analysis,TPZVec<REAL> &ervec,TPZVec<REAL> &ervecbyel,TPZVec<REAL> &gradervecbyel,REAL &MinErrorByElement,REAL &MinGradErrorByElement) {
    long neq = analysis.Mesh()->NEquations();
	if(ModelDimension != analysis.Mesh()->Dimension())
		DebugStop();
    TPZVec<REAL> ux(neq);
    TPZVec<REAL> sigx(neq);
    TPZManVector<REAL,10> totalerror(10,0.);
    analysis.Mesh()->LoadSolution(analysis.Solution());

	TPZAdmChunkVector<TPZCompEl *> elvec = analysis.Mesh()->ElementVec();
    TPZManVector<REAL,10> errors(10);
    errors.Fill(0.0);
    long i, nel = elvec.NElements();
	ervecbyel.Resize(nel,0.0);
	// The last position will be store the maxime value of the gradient errors
	gradervecbyel.Resize(nel+1,0.0);
	REAL maxError = 0.0;
	MinErrorByElement = 1000.0;
	MinGradErrorByElement = 10000.0;

	/** Computing error for all elements with same dimension of the model */
    for(i=0L;i<nel;i++) {
        TPZCompEl *el = (TPZCompEl *) elvec[i];
		if(!el || el->Dimension() != ModelDimension) continue;
        if(el) {
            errors.Fill(0.0);
            el->EvaluateError(analysis.fExact, errors, 0);
            int nerrors = errors.NElements();
            totalerror.resize(nerrors);
            for(int ier = 0; ier < nerrors; ier++)
            {
                totalerror[ier] += errors[ier] * errors[ier];
            }
			// L2 error for each element
            if(i%100 == 0)
            {
                std::cout << "Computed " << i << " elements from " << nel << " total error " << sqrt(totalerror[1]) <<  std::endl;
            }
			ervecbyel[i] = sqrt(errors[1]*errors[1]);
			gradervecbyel[i] = sqrt(errors[2]*errors[2]);
			if(gradervecbyel[i] > gradervecbyel[nel])
				gradervecbyel[nel] = gradervecbyel[i];
			if(gradervecbyel[i] < MinGradErrorByElement)
				MinGradErrorByElement = gradervecbyel[i];
			// The computed error by current element is compared with max and min values to return
			if(ervecbyel[i] > maxError)
				maxError = ervecbyel[i];
			else if(ervecbyel[i] < MinErrorByElement)
				MinErrorByElement = ervecbyel[i];
        }
    }
    
    int nerrors = errors.NElements();
	ervec.Resize(nerrors);
	ervec.Fill(-1.0);
    
	// Returns the square of the calculated errors.
	for(i=0;i<nerrors;i++)
		ervec[i] = sqrt(totalerror[i]);
    return maxError;
}
// Writing a relation between number of degree of freedom and L2 error.
bool PrintResultsInMathematicaFormat(TPZVec<REAL> &ErrorVec,TPZVec<long> &NEquations,std::ostream &fileerrors) {
	int nref;
    STATE fact = 1.0e6;
	long NRefs = ErrorVec.NElements();
	// setting format for ostream
	fileerrors << setprecision(20);
	fileerrors.setf(std::ios::fixed, std::ios::floatfield);
	fileerrors << "\n\n NEquations = {";

	// printing number of equations into a list
	for(nref=0;nref<NRefs-1;nref++) {
		fileerrors << NEquations[nref] << ", ";
	}
	fileerrors << NEquations[nref] << "};" << std::endl << "L2Error = {";
	// printing error values into a list
	for(nref=0;nref<NRefs-1;nref++) {
		fileerrors << ErrorVec[nref]*fact << ", ";
	}
	fileerrors << ErrorVec[nref] << "}/1000000.0;";
	// printing lines to create lists of logarithms
	fileerrors << std::endl << "LogNEquations = Table[Log[10,NEquations[[i]]],{i,1,Length[NEquations]}];" << std::endl;
	fileerrors << "LogL2Errors = Table[Log[10,L2Error[[i]]],{i,1,Length[L2Error]}];" << std::endl;
	fileerrors << "ListPlot[{Table[{LogNEquations[[i]],LogL2Errors[[i]]},{i,1,Length[LogNEquations]}]";
	fileerrors << "},Joined->True,PlotRange->All]\n" << std::endl;
	return true;
}



/** Auxiliar functions  */
void ApplyingStrategyPAdaptiveBasedOnExactSphereSolution(TPZCompMesh *cmesh,TPZVec<REAL> &ervecbyel,TPZVec<REAL> &gradervecbyel,REAL MaxErrorByElement,REAL &MinErrorByElement,int nref) {
    
	if(!cmesh) return;
	long nels = cmesh->NElements();
	TPZVec<long> subels;
	int pelement, dp;
	dp = 1;
	TPZVec<long> subsubels;
	TPZInterpolatedElement *el;
	STATE Tol;
	ZeroTolerance(Tol);
    
	// To see where the computation is doing
	long index = -1;
	TPZVec<long> counterreftype(30,0);
	REAL GradError, SolError;
	long i;
	//	REAL IncrementError = MaxErrorByElement-MinErrorByElement;
	REAL factorErrorLower = 0.1;
	REAL LaplacianValue, GradNorm;
	
	REAL MaxGradErrorByElement = gradervecbyel[nels];
	std::cout << "\nErroMax " << MaxErrorByElement << "   GradError " << MaxGradErrorByElement << "\n";
	
	// Applying hp refinement only for elements with dimension as model dimension
	for(i=0L;i<nels;i++) {
		bool pused = false;
		el = dynamic_cast<TPZInterpolatedElement* >(cmesh->ElementVec()[i]);
		if(!el || el->Dimension()!=cmesh->Dimension()) continue;
		pelement = el->PreferredSideOrder(el->NConnects() - 1);
		index = el->Index();
		if(index != i)
			DebugStop();
		// If the element error is little enough do nothing
		if(ervecbyel[index] < (0.1*Tol)) {
			counterreftype[0]++;
			continue;
		}
		
		// If error is small and laplacian value is very little then the order will be minimized
		if(!GradientAndLaplacianOnCorners(el,GradNorm,LaplacianValue))
			DebugStop();
		// Applying hp refinement depends on high gradient and high laplacian value, and depends on computed error by element
        pelement++;
		GradError = gradervecbyel[i];
		SolError = ervecbyel[i];
		if(SolError > factorErrorLower*MaxErrorByElement) {
			if(pelement<MaxPOrder) {
				el->PRefine(pelement);
				pused = true;
				counterreftype[1]++;
			}
		}
		if(pused)
			MaxPUsed = (pelement > MaxPUsed) ? pelement : MaxPUsed;
	}
	cmesh->ExpandSolution();
	// Printing information stored
	PrintNRefinementsByType(nref,nels,cmesh->NElements(),counterreftype,out);
	PrintNRefinementsByType(nref,nels,cmesh->NElements(),counterreftype);
}


void AdjustingOrder(TPZCompMesh *cmesh) {
	if(!cmesh) return;
	long nels = cmesh->NElements();
	TPZInterpolatedElement *el;
	STATE Tol;
	ZeroTolerance(Tol);

	// To see where the computation is doing
	long i;
	int level, level0 = 100;
	// Searching highest level of computational elements
	for(i=0L;i<nels;i++) {
		el = dynamic_cast<TPZInterpolatedElement* >(cmesh->ElementVec()[i]);
		if(!el || el->Dimension()!=cmesh->Dimension()) continue;
		level = el->Reference()->Level();
		level0 = (level < level0) ? level : level0;
	}
	// Applying hp refinement only for elements with dimension as model dimension
	for(i=0L;i<nels;i++) {
		el = dynamic_cast<TPZInterpolatedElement* >(cmesh->ElementVec()[i]);
		if(!el || el->Dimension()!=cmesh->Dimension()) continue;
		level = el->Reference()->Level();
//		pelement = el->PreferredSideOrder(el->NConnects() - 1);
		if(level-level0+1 < MaxPOrder)
			el->PRefine(level-level0+1);
		else
			el->PRefine(MaxPOrder);
	}
	// Adjusting boundary elements
	cmesh->AdjustBoundaryElements();
}


/**
 * Criteria: Given GlobalL2Error = GE and MaxError (ME) over all the elements
 * If ElementError(EE) > 0.75*ME => twice h-refinement and p-2
 * Else if EE > 0.5*ME  =>  h-refinement and p--
 * Else if EE > 0.25*ME  =>  h-refinement
 * in the other hand => p++
 */
void ApplyingStrategyHPAdaptiveBasedOnErrors(TPZAnalysis &analysis,REAL GlobalL2Error,TPZVec<REAL> &ervecbyel) {

	int ninitialrefs = 2;
	TPZCompMesh *cmesh = analysis.Mesh();
	if(!cmesh) return;
	long nels = cmesh->NElements();
	TPZVec<long> subels;
	int j, k, pelement, dp = 1;
	TPZVec<long> subsubels;
	TPZInterpolatedElement *el;
	REAL errorcel = 0.0;
	for(long i=0L;i<nels;i++) {
		el = dynamic_cast<TPZInterpolatedElement* >(cmesh->ElementVec()[i]);
		if(!el) continue;
		errorcel = ervecbyel[i];
		if(errorcel < 1000*ZeroTolerance()) continue;
		// If error is small and laplacian value is very little then the order will be minimized
		if(errorcel < 0.01*GlobalL2Error) {
			REAL LaplacianValue = Laplacian(el);
			if(LaplacianValue < 0.1) {
				pelement = el->PreferredSideOrder(el->NConnects() - 1);
				// Applying p+1 order for all subelements
				if(pelement > 1)
					el->PRefine(pelement-1);
			}
		}
		else if(errorcel > 0.3*GlobalL2Error) {
            STATE GradNorm = GradientNorm(el);
            if(GradNorm > 3) {
				int level = el->Reference()->Level();
                // Dividing element one level
                el->Divide(el->Index(),subels,0);
                // Dividing sub elements one level more
				if(level < ninitialrefs+3) {
	                for(j=0;j<subels.NElements();j++) {
						TPZInterpolatedElement* scel = dynamic_cast<TPZInterpolatedElement* >(cmesh->ElementVec()[subels[j]]);
					    scel->Divide(subels[j],subsubels,0);
				        for(k=0;k<subsubels.NElements();k++) {
							scel = dynamic_cast<TPZInterpolatedElement* >(cmesh->ElementVec()[subsubels[k]]);
							REAL LaplacianValue = Laplacian(scel);
							if(LaplacianValue > 0.25) {
								pelement = scel->PreferredSideOrder(scel->NConnects() - 1);
						        // Applying p+1 order for all subelements
					            if(pelement+dp < MaxPOrder-1)
					                scel->PRefine(pelement+dp);
							}
						}
	                }
				}
            }
            else {
                el->Divide(el->Index(),subels,0);
                // Dividing sub elements one level more
                for(j=0;j<subels.NElements();j++) {
					TPZInterpolatedElement* scel = dynamic_cast<TPZInterpolatedElement* >(cmesh->ElementVec()[subels[j]]);
					REAL LaplacianValue = Laplacian(scel);
					if(LaplacianValue > 0.25) {
						pelement = scel->PreferredSideOrder(scel->NConnects() - 1);
	                    // Applying p+1 order for all subelements
						scel->PRefine(pelement+1);
					}
					else if(IsZero(LaplacianValue)) {
						pelement = scel->PreferredSideOrder(scel->NConnects() - 1);
	                    // Applying p-1 order for all subelements
						if(pelement > 1) scel->PRefine(pelement-1);
					}
				}
            }
		}
	}
}

void LoadSolutionFirstOrder(TPZCompMesh *cmesh, void (*f)(const TPZVec<REAL> &loc, TPZVec<STATE> &result, TPZFMatrix<STATE> &deriv,TPZVec<STATE> &ddsol)) {
	TPZFMatrix<STATE> solution(cmesh->Solution());
	solution.Zero();
	TPZVec<STATE> sol(1);
	TPZFMatrix<STATE> dsol(cmesh->Dimension(),1);
	TPZVec<STATE> ddsol(9,0.0);
	int nels = cmesh->NElements();
	for(int i=0;i<nels;i++) {
		TPZCompEl *el=cmesh->ElementVec()[i];
		if(!el) continue;
		int nconn = el->Reference()->NCornerNodes();
		for(int j=0;j<nconn;j++) {
			TPZConnect &df = el->Connect(j);
			int seqnum = df.SequenceNumber();
			int pos = cmesh->Block().Position(seqnum);
			TPZVec<REAL> coord(3,0.), coordinates(3,0.);
			el->Reference()->CenterPoint(j,coord);
			el->Reference()->X(coord,coordinates);
			f(coordinates,sol,dsol,ddsol);
			cmesh->Solution()(pos,0) = sol[0];
		}
	}
}

int DefineDimensionOverElementType(int typeel) {
	int dim = 0;
	switch(typeel) {
		case EOned:
			dim = 1;
			break;
		case EQuadrilateral:
		case ETriangle:
			dim = 2;
			break;
		case ETetraedro:
		case EPrisma:
		case EPiramide:
		case ECube:
			dim = 3;
			break;
		default:
			break;
	}
	return dim;
}

void GetFilenameFromGID(MElementType typeel, std::string &nombre) {
	switch (typeel) {
		case EOned:
			nombre = "LinhaReta.dump";
			break;
		case EQuadrilateral:
			nombre = "RegionQuadrada.dump";
			break;
		case ETriangle:
			nombre = "RegionQuadradaT.dump";
			break;
		case ETetraedro:
			nombre = "RegionCuboEnTetrahedros.dump";
			break;
		case EPrisma:
			nombre = "RegionCuboEnPrismas.dump";
			break;
		case EPiramide:
			nombre = "RegionCuboEnPiramides.dump";
			break;
		case ECube:
			nombre = "RegionCuboEnHexahedros.dump";
			break;
		default:
			DebugStop();
			break;
	}
}



/** LAPLACE PROBLEM ON L-SHAPE DOMAIN (2D) */
void ExactSolLaplaceBC(const TPZVec<REAL> &x, TPZVec<STATE> &sol) {
	REAL radius = sqrt(x[0]*x[0] + x[1]*x[1]);
    REAL angle = atan2(x[1],x[0]);;
    /*    if(IsZero(x[0])) {
     if(x[1]>0)
     angle = 0.5*M_PI;
     else {
     sol[0] = 0.;
     return;
     }
     }
     else
     angle = atan(x[1]/x[0]);
     if(angle < -0.5*M_PI || angle > M_PI)
     DebugStop();*/
    sol[0] = 0.5*pow(radius,(REAL(1./3.)))*(sqrt(3.)*sin(angle/3.)+cos(angle/3.));
}
void ExactSolLaplace(const TPZVec<REAL> &x, TPZVec<STATE> &sol,TPZFMatrix<STATE> &dsol) {
	REAL radius = sqrt(x[0]*x[0] + x[1]*x[1]);
    REAL angle = 0;
    if(IsZero(x[0])) {
        if(x[1]>0)
            angle = 0.5*M_PI;
        else {
            sol[0] = 0.;
            return;
        }
    }
    else
        angle = atan(x[1]/x[0]);
    sol[0] = 0.5*pow(radius,(REAL(1./3.)))*(sqrt(3.)*sin(angle/3.)+cos(angle/3.));
    dsol.Zero();
}

//************************************************************************
//**********   Creating computational mesh with materials    *************
//************************************************************************
TPZCompMesh *CreateComputationalMesh(TPZGeoMesh *gmesh,int dim,int materialId,int hasforcingfunction,int id_bc0,int id_bc1,int id_bc2) {
    
    TPZCompMesh *cmesh = new TPZCompMesh(gmesh);
    cmesh->SetDefaultOrder(TPZCompEl::GetgOrder());
    cmesh->SetAllCreateFunctionsContinuous();
    
    // Creating Poisson material
    TPZMaterial *mat = new TPZMatPoisson3d(materialId,dim);
    switch(hasforcingfunction) {
        case 1:
        {
            TPZVec<REAL> convd(3,0.);
            ((TPZMatPoisson3d *)mat)->SetParameters(ValueK,0.,convd);
            mat->SetForcingFunction(new TPZDummyFunction<STATE>(RightTermArcTangentBad));
        }
            break;
        case 2:
            break;
        default:
            break;
    }
    cmesh->InsertMaterialObject(mat);
    // Make compatible dimension of the model and the computational mesh
    cmesh->SetDimModel(mat->Dimension());
    
    // Creating four boundary condition
    TPZFMatrix<STATE> val1(dim,dim,0.),val2(dim,1,0.);
    for(int i=0;i<dim;i++)
        val1.PutVal(i,i,1.);
    TPZMaterial *bc = 0, *bc1 = 0;
    switch(hasforcingfunction) {
        case 0:
        case 1:
            // Condicion de Dirichlet fijando la posicion de la placa
            bc = mat->CreateBC(mat,id_bc0,0,val1,val2);
            break;
        case 2:
            // Condicion de Dirichlet fijando la posicion de la placa
            bc = mat->CreateBC(mat,id_bc0,0,val1,val2);
            bc1 = mat->CreateBC(mat,id_bc1,0,val1,val2);
            bc1->SetForcingFunction(new TPZDummyFunction<STATE>(ExactSolLaplaceBC));
            break;
        default:
            break;
    }
    
    if(bc) cmesh->InsertMaterialObject(bc);
    if(bc1) cmesh->InsertMaterialObject(bc1);
    
    cmesh->AutoBuild();
    cmesh->ExpandSolution();
    cmesh->AdjustBoundaryElements();
    cmesh->CleanUpUnconnectedNodes();
    return cmesh;
}

/***  Functions to manipulate and determine time (current) ***///
void formatTimeInSec(char *strtime,int lenstrtime,int timeinsec) {
    if(!strtime) return;
    memset(strtime,0,strlen(strtime));
    
    int anos=0, meses=0, dias=0, horas=0, minutos=0, segundos=0;
    while(1) {
        if(timeinsec < 60) {
            segundos = timeinsec;
            break;
        }
        else {
            timeinsec -= 60;
            minutos++;
            if(minutos > 59) {
                minutos -= 60;
                horas++;
                if(horas > 23) {
                    horas -= 24;
                    dias++;
                    if(dias > 29) {
                        dias -= 30;
                        meses++;
                        if(meses > 11) {
                            meses -= 12;
                            anos++;
                        }
                    }
                }
            }
        }
    }
    // Formating
    if(anos)
#ifdef WIN32
        sprintf_s(strtime,lenstrtime,"%d a, %d m, %d d, %02d:%02d:%02d",anos,meses,dias,horas,minutos,segundos);
#else
    sprintf(strtime,"%d a, %d m, %d d, %02d:%02d:%02d",anos,meses,dias,horas,minutos,segundos);
#endif
    else {
        if(meses)
#ifdef WIN32
            sprintf_s(strtime,lenstrtime,"%d m, %d d, %02d:%02d:%02d",meses,dias,horas,minutos,segundos);
#else
        sprintf(strtime,"%d m, %d d, %02d:%02d:%02d",meses,dias,horas,minutos,segundos);
#endif
        else {
            if(dias)
#ifdef WIN32
                sprintf_s(strtime,lenstrtime,"%d d, %02d:%02d:%02d",dias,horas,minutos,segundos);
#else
            sprintf(strtime,"%d d, %02d:%02d:%02d",dias,horas,minutos,segundos);
#endif
            else
#ifdef WIN32
                sprintf_s(strtime,lenstrtime,"%02d:%02d:%02d",horas,minutos,segundos);
#else
            sprintf(strtime,"%02d:%02d:%02d",horas,minutos,segundos);
#endif
        }
    }
}

bool CreateCurrentResultDirectory(SimulationCase &sim_case) {
    // To compute processing times
    time_t sttime;
    struct tm* tmtime;
    time(&sttime);
    tmtime = localtime(&sttime);
    tmtime->tm_year += 1900;
    tmtime->tm_mon += 1;
    
    char command[512];
    memset(command,0,512);
    snprintf(command,512,"%s_%04d_%02d_%02d_%02d%02d%02d",sim_case.dir_name.c_str(),tmtime->tm_year,tmtime->tm_mon,tmtime->tm_mday,tmtime->tm_hour,tmtime->tm_min,tmtime->tm_sec);
    
	sim_case.dir_name = command;
	snprintf(command, 512, "mkdir %s", sim_case.dir_name.c_str());
    // Creating the directory
    return system(command);
}
