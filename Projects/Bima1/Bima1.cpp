#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <cstdlib>
#include "pzgengrid.h"
#include "pzgmesh.h"
#include "pzgeoelbc.h"
#include "pzcmesh.h"
#include "tpzcompmeshreferred.h"
#include "pzcompel.h"
#include "pzpoisson3d.h"
#include "pzbndcond.h"
#include "pzanalysiserror.h"
#include "pzanalysis.h"
#include "pzcmesh.h"
#include "pzstepsolver.h"
#include "TPZParFrontStructMatrix.h"
#include "pzmatrix.h"
#include "TPZCompElDisc.h"
#include "pzfstrmatrix.h"
#include "pzinterpolationspace.h"
#include "pzsubcmesh.h"
#include "pzlog.h"
#include "pzelctemp.h"
#include "pzelchdiv.h"
#include "pzshapequad.h"
#include "pzshapetriang.h"
#include "pzgeoquad.h"
#include "pzgeotriangle.h"
#include "pzfstrmatrix.h"
#include "pzgengrid.h"
#include "pzbndcond.h"
#include "pzmaterial.h"
#include "pzelmat.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "pzlog.h"
#include <cmath>
#include "pzhdivpressure.h"
#include "TPZSkylineNSymStructMatrix.h"

#include "TPZRefPattern.h"

#include "TPZMatDualHybridPoisson.h"

#ifdef LOG4CXX

static LoggerPtr logger(Logger::getLogger("Steklov.main"));

#endif
static void SolExataSteklov(const TPZVec<REAL> &loc, TPZVec<STATE> &u, TPZFMatrix<STATE> &du){
    
    const REAL n = 0;
    
    const REAL x = loc[0];
    const REAL y = loc[1];
    const REAL r = sqrt(x*x+y*y);
    const REAL t = atan2(y,x);
    const REAL sol = pow((REAL)2,0.25 + n/2.)*pow(r,0.5 + n)*cos((0.5 + n)*t);
    u[0] = sol;
    
    du(0,0) = pow((REAL)2,-0.75 + n/2.)*(1 + 2*n)*pow(pow(x,2) + pow(y,2),-0.75 + n/2.)*(x*cos((0.5 + n)*atan2(y,x)) + y*sin((0.5 + n)*atan2(y,x)));
    du(1,0) = pow((REAL)2,-0.75 + n/2.)*(1 + 2*n)*pow(pow(x,2) + pow(y,2),-0.75 + n/2.)*(y*cos((0.5 + n)*atan2(y,x)) - x*sin((0.5 + n)*atan2(y,x)));
    
}


static void Dirichlet(const TPZVec<REAL> &loc, TPZVec<STATE> &result){
    TPZFNMatrix<10,REAL> fake(2,1);
    SolExataSteklov(loc,result,fake);
}
static void Dirichlet2(const TPZVec<REAL> &loc, TPZVec<STATE> &result){
    TPZFNMatrix<10,REAL> fake(2,1);
    result[0] = loc[0]*loc[0];
}

TPZGeoMesh * MalhaGeo(const int h);
/** Resolver o problema do tipo
 * -Laplac(u) = 0
 * du/dn = lambda u em todo contorno
 */

using namespace std;

TPZCompMesh *CreateHybridCompMesh(TPZGeoMesh &gmesh,int porder);

int main()
{
    
    std::ofstream myerrorfile("erros.txt");
	
#ifdef LOG4CXX
    if (logger->isDebugEnabled())
	{
		InitializePZLOG();
		std::stringstream sout;
		sout<< "Problema Hibrido do Abimael"<<endl;
		LOGPZ_DEBUG(logger, sout.str());
	}
#endif
	
	for (int porder= 1; porder<4; porder++) {
		
		for(int h=0;h<4;h++){
			
			
			TPZGeoMesh *gmesh = MalhaGeo(h);//malha geometrica
			
			
			TPZCompMesh *cmesh = CreateHybridCompMesh(*gmesh,porder);//malha computacional
			
			
			
			cmesh->LoadReferences();//mapeia para a malha geometrica lo
			
			TPZAnalysis analysis(cmesh);
            
            TPZSkylineNSymStructMatrix str(cmesh);
            
            analysis.SetStructuralMatrix(str);
            
            TPZStepSolver<STATE> step;
            step.SetDirect(ELU);
            analysis.SetSolver(step);
            
            analysis.Run();
            TPZVec<std::string> scalnames(1),vecnames(0);
            scalnames[0] = "Solution";
            analysis.DefineGraphMesh(2,scalnames,vecnames,"bima.vtk");
            
            analysis.PostProcess(3);
            
            analysis.SetExact(SolExataSteklov);
            TPZVec<REAL> erros(3);
            analysis.PostProcessError(erros);
            myerrorfile << "h = "<< h << " p = " << porder << "\n";
            myerrorfile << "H1 = " << erros[0];
            myerrorfile << " L2 = " << erros[1];
            myerrorfile << " semi H1 = " << erros[2] << std::endl;
            
				
            cmesh->SetName("Malha depois de Analysis-----");
#ifdef LOG4CXX
            if (logger->isDebugEnabled())
			{
				std::stringstream sout;
				cmesh->Print(sout);
			//	submesh->Block().Print("Block",sout);
				LOGPZ_DEBUG(logger,sout.str())
			}
#endif
            delete cmesh;
            delete gmesh;
	
        }
    }
	return 0;
}

TPZCompMesh *CreateHybridCompMesh(TPZGeoMesh &gmesh,int porder){
	TPZCompEl::SetgOrder(porder);
	TPZCompMesh *comp = new TPZCompMesh(&gmesh);
	
//    comp->ApproxSpace().CreateDisconnectedElements();
    comp->ApproxSpace().SetAllCreateFunctionsDiscontinuous();
	
	
	// Criar e inserir os materiais na malha
	TPZMatDualHybridPoisson *mat = new TPZMatDualHybridPoisson(1,0.,0.01);
	TPZMaterial * automat(mat);
	comp->InsertMaterialObject(automat);
	
	
	// Condicoes de contorno
	TPZFMatrix<STATE> val1(1,1,1.),val2(1,1,0.);
	
	TPZMaterial *bnd = automat->CreateBC (automat,-1,2,val1,val2);//misto tbem
    bnd->SetForcingFunction(Dirichlet2);
	comp->InsertMaterialObject(bnd);
	
	// Mixed
	val1(0,0) = 1.;
	val2(0,0)=0.;
	bnd = automat->CreateBC (automat,-2,0,val1,val2);
	TPZBndCond *bndcond = dynamic_cast<TPZBndCond *> (bnd);
    bnd->SetForcingFunction(Dirichlet);
//	bndcond->SetValFunction(ValFunction);
	comp->InsertMaterialObject(bnd);
	
	// Mixed
	val1(0,0) = 1.;
	val2(0,0)=0.;
	bnd = automat->CreateBC (automat,-3,0,val1,val2);
    bnd->SetForcingFunction(Dirichlet);
	comp->InsertMaterialObject(bnd);
	
	// Mixed
	val1(0,0) = 1.;
	val2(0,0)=0.;
	bnd = automat->CreateBC (automat,-4,0,val1,val2);
    bnd->SetForcingFunction(Dirichlet);
	comp->InsertMaterialObject(bnd);
	
	// Mixed
	val1(0,0) = 1.;
	val2(0,0)=0.;
	bnd = automat->CreateBC (automat,-5,0,val1,val2);
    bnd->SetForcingFunction(Dirichlet);
	comp->InsertMaterialObject(bnd);
	
	// Mixed
	val1(0,0) = 1.;
	val2(0,0)=0.;
	bnd = automat->CreateBC (automat,-6,0,val1,val2);
    bnd->SetForcingFunction(Dirichlet);
	comp->InsertMaterialObject(bnd);
	
	// Ajuste da estrutura de dados computacional
	comp->AutoBuild();
	comp->AdjustBoundaryElements();//ajusta as condicoes de contorno
	comp->CleanUpUnconnectedNodes();//deleta os nos que nao tem elemntos conectados
    comp->ApproxSpace().CreateInterfaceElements(comp);
	
#ifdef LOG4CXX
    if (logger->isDebugEnabled())
	{
		std::stringstream sout;
		comp->Print(sout);
		LOGPZ_DEBUG(logger,sout.str())
	}
#endif
	

    std::set<int> matids;
    matids.insert(1);
    for (int i=2; i<=6; i++) {
        matids.insert(-i);
    }

    
    
    comp->ApproxSpace().Hybridize(*comp, matids);
    
    
	
	comp->SetName("Malha Computacional Original");
	
#ifdef LOG4CXX
    if (logger->isDebugEnabled())
	{
		std::stringstream sout;
		comp->Print(sout);
		LOGPZ_DEBUG(logger,sout.str())
	}
#endif
	
	
	
    return comp;
	
}


void ValFunction(TPZVec<REAL> &loc, TPZFMatrix<STATE> &Val1, TPZVec<STATE> &Val2, int &BCType)
{
	BCType = 2;
	Val1.Redim(1, 1);
	Val1(0,0) = 1.;
	Val2[0] = loc[0];
}


TPZGeoMesh * MalhaGeo(const int ndiv){//malha quadrilatera
	
	///malha geometrica
    TPZGeoMesh * gmesh = new TPZGeoMesh();
    
	
    
    
    ///Criando nós
    const int nnodes = 6;
    double coord[nnodes][2] = {{-0.5,0},{0,0},{0.,0.5},{-0.5,0.5},{0.5,0},{0.5,0.5}};
    for(int i = 0; i < nnodes; i++) {
        int nodind = gmesh->NodeVec().AllocateNewElement();
        TPZManVector<REAL,3> nodeCoord(3);
        nodeCoord[0] = coord[i][0];
        nodeCoord[1] = coord[i][1];
        nodeCoord[2] = 0.;
        gmesh->NodeVec()[nodind].Initialize(i,nodeCoord,*gmesh);
    }
    
    ///Criando elementos
    const int nel = 2;
    int els[nel][4] = {{0,1,2,3},{1,4,5,2}};
    for(int iel = 0; iel < nel; iel++){
        TPZManVector<long,4> nodind(4);
        long index;
        nodind[0] = els[iel][0];
        nodind[1] = els[iel][1];
        nodind[2] = els[iel][2];
        nodind[3] = els[iel][3];
        gmesh->CreateGeoElement(EQuadrilateral,nodind,1,index);
    }
    
    ///Criando elementos de contorno
    const int nelbc = 6;
    int bcels[nelbc][3] = {{0,1,-3},{1,4,-2},{4,5,-4},{5,2,-6},{2,3,-6},{3,0,-5}};
    for(int iel = 0; iel < nelbc; iel++){
        TPZManVector<long,4> nodind(2);
        long index;
        nodind[0] = bcels[iel][0];
        nodind[1] = bcels[iel][1];
        int matid = bcels[iel][2];
        gmesh->CreateGeoElement(EOned,nodind,matid,index);
    }
    
    ///Construindo conectividade da malha
	gmesh->BuildConnectivity();
    
	{
		std::ofstream myfile("geoMinima.txt");
		gmesh->Print(myfile);
	}
	
    ///Refinamento uniforme da malha
	
	///Inicializando padrões de refinamento uniforme
    gRefDBase.InitializeUniformRefPattern(EOned);
	gRefDBase.InitializeUniformRefPattern(EQuadrilateral);
    
	for (int i = 0; i < ndiv; i++){
        int nel = gmesh->NElements();
		for (int iel = 0; iel < nel; iel++){
			TPZGeoEl * gel = gmesh->ElementVec()[iel];
			if (!gel) continue;
			if (gel->HasSubElement()) continue;//para nao dividir elementos ja divididos
            TPZVec<TPZGeoEl*> filhos;
			gel->Divide(filhos);
        }///iel
    }///i
    
    {
		std::ofstream myfile("geoRefinado.txt");
		gmesh->Print(myfile);
	}
    
	
#ifdef LOG4CXX
    if (logger->isDebugEnabled())
	{
		std::stringstream sout;
		gmesh->Print(sout);
		LOGPZ_DEBUG(logger, sout.str());
	}
#endif 		 		 
	return gmesh;
} 