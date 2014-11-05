
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "pzgmesh.h"
#include "pzcmesh.h"
#include "pzcompel.h"
#include "pzbndcond.h"
#include "TPZInterfaceEl.h"

#include "TPZRefPattern.h"
#include "tpzgeoelrefpattern.h"
#include "pzgeopoint.h"
#include "TPZGeoLinear.h"
#include "TPZGeoCube.h"
#include "tpztriangle.h"
#include "pzgeoquad.h"
#include "pzgeoelside.h"
#include "tpzgeoblend.h"
#include "tpzarc3d.h"
#include "pzgeotetrahedra.h"
#include "pzgeoelrefless.h"

#include "pzvec.h"
#include "pzstack.h"
#include "pzfmatrix.h"
#include "pzfstrmatrix.h"
#include "TPZParSkylineStructMatrix.h"
#include "pzskylstrmatrix.h"
#include "TPBSpStructMatrix.h"
#include "pzbstrmatrix.h"
#include "pzstepsolver.h"
#include "TPZSkylineNSymStructMatrix.h"

#include "pzanalysis.h"

#include "pzmultiphysicselement.h"
#include "pzmultiphysicscompel.h"
#include "TPZMultiphysicsInterfaceEl.h"
#include "pzbuildmultiphysicsmesh.h"

#include "pzpoisson3d.h"
#include "mixedpoisson.h"
#include "TPZReadGIDGrid.h"
#include "pzanalysis.h"

#include "TPZVTKGeoMesh.h"

#include "pzlog.h"

#include "pzhdivfull.h"
#include "pzelchdiv.h"

#include "pzgeopyramid.h"

#include "PZMatPoissonD3.h"

#include "pznumeric.h"

#include "TPZExtendGridDimension.h"
#include "pzelchdivbound2.h"
#include "pzshapequad.h"
#include "pzshapelinear.h"
#include "pzshapetriang.h"

#include "TestHDivMesh.h"
#include "TPZLagrangeMultiplier.h"
#include "pzmatmixedpoisson3d.h"

#include <iostream>
#include <string>
#include <sstream>
#include <math.h>

using namespace std;
using namespace pzshape;

int matId = 1;

int dirichlet = 0;
int neumann = 1;

int bc0 = -1;
int bc1 = -2;
int bc2 = -3;
int bc3 = -4;
int bc4 = -5;
int bc5 = -6;
int matskeleton = -7;

// just for print data
/** @brief Map used norms */
std::map<REAL,REAL> fDebugMapL2, fDebugMapHdiv;


TPZGeoMesh *GMeshArctan(int dimensao, bool ftriang, int ndiv);
TPZGeoMesh *GMeshDeformedArctan();
TPZGeoMesh *CreateOneCuboArctan(int nref=0);

TPZCompMesh *CMeshFluxArctan(TPZGeoMesh *gmesh, int pOrder, int dim);
TPZCompMesh *CMeshPressureArctan(TPZGeoMesh *gmesh, int pOrder, int dim);
TPZCompMesh *CMeshMixedArctan(TPZGeoMesh * gmesh, TPZVec<TPZCompMesh *> meshvec);

void UniformRefineArctan(TPZGeoMesh* gmesh, int nDiv);
void SolveSystArctan(TPZAnalysis &an, TPZCompMesh *fCmesh);
void PosProcessMultphysicsArctan(TPZVec<TPZCompMesh *> meshvec, TPZCompMesh* mphysics, TPZAnalysis &an, std::string plotfile);
void PosProcessArctan(TPZAnalysis &an, std::string plotfile);

void ErrorL2Arctan(TPZCompMesh *l2mesh, std::ostream &out, int p, int ndiv);
void ErrorHDivArctan(TPZCompMesh *hdivmesh, std::ostream &out, int p, int ndiv);

/** @brief Prints debug map in Mathematica style */
void PrintDebugMapForMathematicaArctan(std::string filenameHdiv, std::string filenameL2);


//solucao exata
void SolExataArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &solp, TPZFMatrix<STATE> &flux);

//lado direito da equacao
void ForcingArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp);

//Para condicao de contorno de Dirichlet
void ForcingBC0DArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp);
void ForcingBC1DArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp);
void ForcingBC2DArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp);
void ForcingBC3DArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp);
void ForcingBC4DArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp);
void ForcingBC5DArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp);

//Para condicao de contorno de Neumann
void ForcingBC0NArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp);
void ForcingBC1NArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp);
void ForcingBC2NArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp);
void ForcingBC3NArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp);
void ForcingBC4NArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp);
void ForcingBC5NArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp);

//criar elementos esqueleto
void AddWrap(TPZMultiphysicsElement *mfcel, int matskeleton, TPZStack< TPZStack< TPZMultiphysicsElement *, 7> > &ListGroupEl);

int dim = 3;
REAL aa = 0.0;
REAL bb = 0.0;
REAL cc = 0.0;
REAL Epsilon = 0.4;
// tensor de permutacao
TPZFNMatrix<2,REAL> TP(dim,dim,0.0);
TPZFNMatrix<2,REAL> InvTP(dim,dim,0.0);

REAL const Pi = M_PI;//4.*atan(1.);

// Para dimensao 2
// tipo 1 triangulo
// tipo 2 quadrilatero

bool ftriang = true;//false;//

#ifdef LOG4CXX
static LoggerPtr logdata(Logger::getLogger("pz.material"));
#endif

#include "pztransfer.h"

int main(int argc, char *argv[])
{
#ifdef LOG4CXX
    InitializePZLOG();
#endif
    
    
    int p = 1;
    int ndiv = 0;
    
    // Hard coded
    for (int id = 0; id < dim; id++){
        TP(id,id) = 1.0;
        InvTP(id,id) = 1.0;
    }
    
    ofstream saidaerro("../ErroPoissonHdivMalhaQuad.txt",ios::app);
    //int tipo = 1;
    //ofstream saidaerro("../ErroPoissonHdivMalhaTriang.txt",ios::app);
    
    for(p=1;p<5;p++)
    {
        int pq = p;
        int pp = p;

        
        for (ndiv=0; ndiv<5; ndiv++)
        {
            
            //TPZGeoMesh *gmesh = CreateOneCuboArctan(ndiv);
            
            TPZGeoMesh *gmesh2d = GMeshArctan( dim, ftriang, ndiv);
            REAL layerthickness = 1.;
//
//            {
//                //  Print Geometrical Base Mesh
//                std::ofstream Dummyfile("GeometricMesh2D.vtk");
//                TPZVTKGeoMesh::PrintGMeshVTK(gmesh2d,Dummyfile, true);
//            }
            
            TPZExtendGridDimension extend(gmesh2d,layerthickness);
            TPZGeoMesh *gmesh3d = extend.ExtendedMesh(1,bc0,bc5);
//            ofstream arg("gmesh1.txt");
//            gmesh2d->Print(arg);
////
////            ofstream arq3("gmesh3d.txt");
////            gmesh3d->Print(arq3);
////            TPZFMatrix<STATE> Tr(3,3,0.0);
//
            TPZGeoMesh *gmesh = gmesh3d;
            gmesh->SetDimension(dim);
            
            
            //            ofstream arg1("gmesh.txt");
            //            gmesh->Print(arg1);
            {
                //  Print Geometrical Base Mesh
                std::ofstream Dummyfile("GeometricMesh3D.vtk");
                TPZVTKGeoMesh::PrintGMeshVTK(gmesh,Dummyfile, true);
            }

            
            TPZCompMesh *cmesh2 = CMeshPressureArctan(gmesh, pp, dim);
            TPZCompMesh *cmesh1 = CMeshFluxArctan(gmesh, pq, dim);
            
            
            
            //            ofstream arg1("cmeshflux.txt");
            //            cmesh1->Print(arg1);
            //
            //            ofstream arg2("cmeshpressure.txt");
            //            cmesh2->Print(arg2);
            //
            //            ofstream arg4("gmesh2.txt");
            //            gmesh->Print(arg4);
            
            
            //malha multifisica
            TPZVec<TPZCompMesh *> meshvec(2);
            meshvec[0] = cmesh1;
            meshvec[1] = cmesh2;
            
            
            //            int Prows =cmesh2->Solution().Rows();
            //            TPZFMatrix<STATE> alphaP(Prows,1,0.0);
            //            alphaP(26,0)=1.0;
            //            cmesh2->LoadSolution(alphaP);
            //            TPZAnalysis anP(cmesh2);
            //            std::string plotfile2("AlphasPressure.vtk");
            //            PosProcess(anP, plotfile2);
            
            
            
            TPZCompMesh * mphysics = CMeshMixedArctan(gmesh,meshvec);
            //            ofstream arg5("cmeshmultiphysics.txt");
            //            mphysics->Print(arg5);
            //            TPZCompEl *cel = mphysics->Element(0);
            //            TPZElementMatrix ek,ef;
            //            cel->CalcStiff(ek, ef);
            
            TPZAnalysis an(mphysics);
            
            SolveSystArctan(an, mphysics);
            
            stringstream ref,grau;
            grau << p;
            ref << ndiv;
            string strg = grau.str();
            string strr = ref.str();
            std::string plotname("OurSolutionArctan");
            std::string Grau("P");
            std::string Ref("H");
            std::string VTK(".vtk");
            std::string plotData;
            plotData = plotname+Grau+strg+Ref+strr+VTK;
            std::string plotfile(plotData);
            
            PosProcessMultphysicsArctan(meshvec,  mphysics, an, plotfile);
            
            
            //Calculo do erro
            TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(meshvec, mphysics);
            TPZVec<REAL> erros;
            
            //            TPZManVector<REAL,3> myerrors(3,0.);
            //            an.SetExact(SolExata);
            //            an.PostProcessError(myerrors);
            
            //            saidaerro << "Valor de epsilone " << EPSILON << std::endl;
            //            saidaerro << "Numero de threads " << numthreads << std::endl;
            saidaerro<<" \nErro da simulacao multifisica do fluxo (q)" <<endl;
            ErrorHDivArctan(cmesh1, saidaerro, p, ndiv);
            
            saidaerro<<" Erro da simulacao multifisica da pressao (p)" <<endl;
            ErrorL2Arctan(cmesh2, saidaerro, p, ndiv);
            //
            std::cout << "Postprocessed\n";
            
            //Plot da solucao aproximada
            // para estudo do ero nao precisa gerar dados de saida
            //PosProcessMultphysics(meshvec,  mphysics, an, plotfile);
            
            stringstream ss;
            ss << p;
            string str = ss.str();
            
            std::cout<< " grau  polinomio " << p << " numero de divisoes " << ndiv << std::endl;
            std::string filename("InputDataArctan");
            std::string L2("L2.txt");
            std::string Hdiv("Hdiv.txt");
            std::string HdivData,L2Data;
            HdivData = filename+str+Hdiv;
            L2Data = filename+str+L2;
            
            PrintDebugMapForMathematicaArctan(HdivData,L2Data);
            
        }
        fDebugMapHdiv.clear();
        fDebugMapL2.clear();
    }
    

    std::cout<< " fim " << std::endl;
    
    return EXIT_SUCCESS;
}

TPZGeoMesh *GMeshArctan(int d, bool ftriang, int ndiv)
{
    
    if(dim!=2)
    {
        std::cout << "dimensao errada" << std::endl;
        dim = 2;
        DebugStop();
    }
    
    int Qnodes =  4;
    
    TPZGeoMesh * gmesh = new TPZGeoMesh;
    gmesh->SetMaxNodeId(Qnodes-1);
    gmesh->NodeVec().Resize(Qnodes);
    TPZVec<TPZGeoNode> Node(Qnodes);
    
    gmesh->SetDimension(dim);
    
    TPZVec <long> TopolQuad(4);
    TPZVec <long> TopolTriang(3);
    TPZVec <long> TopolLine(2);
    TPZVec <long> TopolPoint(1);
    
    //indice dos nos
    long id = 0;
    //    REAL valx;
    //    for(int xi = 0; xi < Qnodes/2; xi++)
    //    {
    //        valx = xi*Lx;
    //        Node[id].SetNodeId(id);
    //        Node[id].SetCoord(0 ,valx );//coord X
    //        Node[id].SetCoord(1 ,0. );//coord Y
    //        gmesh->NodeVec()[id] = Node[id];
    //        id++;
    //    }
    //
    //    for(int xi = 0; xi < Qnodes/2; xi++)
    //    {
    //        valx = Lx - xi*Lx;
    //        Node[id].SetNodeId(id);
    //        Node[id].SetCoord(0 ,valx );//coord X
    //        Node[id].SetCoord(1 ,Ly);//coord Y
    //        gmesh->NodeVec()[id] = Node[id];
    //        id++;
    //    }
    //
    TPZManVector<REAL,3> coord(2,0.);
    int in = 0;
    //c0
    coord[0] = 0.0;
    coord[1] = 0.0;
    gmesh->NodeVec()[in].SetCoord(coord);
    gmesh->NodeVec()[in].SetNodeId(in++);
    //c1
    coord[0] =  1.0;
    coord[1] = 0.0;
    gmesh->NodeVec()[in].SetCoord(coord);
    gmesh->NodeVec()[in].SetNodeId(in++);
    //c2
    coord[0] =  0.0;
    coord[1] =  1.0;
    gmesh->NodeVec()[in].SetCoord(coord);
    gmesh->NodeVec()[in].SetNodeId(in++);
    //c3
    coord[0] = 1.0;
    coord[1] =  1.0;
    gmesh->NodeVec()[in].SetCoord(coord);
    gmesh->NodeVec()[in].SetNodeId(in++);
    //indice dos elementos
    id = 0;
    
    if(ftriang==true) // triangulo
    {
        TopolTriang[0] = 0;
        TopolTriang[1] = 1;
        TopolTriang[2] = 3;
        new TPZGeoElRefPattern< pzgeom::TPZGeoTriangle > (id,TopolTriang,matId,*gmesh);
        id++;
        
        TopolTriang[0] = 0;
        TopolTriang[1] = 3;
        TopolTriang[2] = 2;
        new TPZGeoElRefPattern< pzgeom::TPZGeoTriangle> (id,TopolTriang,matId,*gmesh);
        id++;
        
        TopolLine[0] = 0;
        TopolLine[1] = 1;
        new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,bc1,*gmesh);
        id++;
        
        TopolLine[0] = 1;
        TopolLine[1] = 3;
        new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,bc2,*gmesh);
        id++;
        
        TopolLine[0] = 3;
        TopolLine[1] = 2;
        new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,bc3,*gmesh);
        id++;
        
        TopolLine[0] = 2;
        TopolLine[1] = 0;
        new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,bc4,*gmesh);
        id++;
    }
    else{
        
        TopolQuad[0] = 0;
        TopolQuad[1] = 1;
        TopolQuad[2] = 3;
        TopolQuad[3] = 2;
        new TPZGeoElRefPattern< pzgeom::TPZGeoQuad> (id,TopolQuad,matId,*gmesh);
        id++;
        
        TopolLine[0] = 0;
        TopolLine[1] = 1;
        new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,bc1,*gmesh);
        id++;
        
        TopolLine[0] = 1;
        TopolLine[1] = 3;
        new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,bc2,*gmesh);
        id++;
        
        TopolLine[0] = 3;
        TopolLine[1] = 2;
        new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,bc3,*gmesh);
        id++;
        
        TopolLine[0] = 2;
        TopolLine[1] = 0;
        new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (id,TopolLine,bc4,*gmesh);
    }
    
    gmesh->BuildConnectivity();
    
    /// gmesh para aqui
    
    TPZVec<TPZGeoEl *> sons;
    for (int iref = 0; iref < ndiv; iref++) {
        int nel = gmesh->NElements();
        for (int iel = 0; iel < nel; iel++) {
            TPZGeoEl *gel = gmesh->ElementVec()[iel];
            if (gel->HasSubElement()) {
                continue;
            }
            gel->Divide(sons);
        }
    }
    
    
    //#ifdef LOG4CXX
    //	if(logdata->isDebugEnabled())
    //	{
    //        std::stringstream sout;
    //        sout<<"\n\n Malha Geometrica Inicial\n ";
    //        gmesh->Print(sout);
    //        LOGPZ_DEBUG(logdata,sout.str())
    //	}
    //#endif
    
    return gmesh;
}

TPZGeoMesh *CreateOneCuboArctan(int nref)
{
    TPZGeoMesh *gmesh = new TPZGeoMesh;
    int nnodes = 8;
    int idf0=-1;
    int idf1=-2;
    int idf2=-3;
    int idf3=-4;
    int idf4=-5;
    int idf5=-6;
    
    gmesh->SetDimension(3);
    gmesh->NodeVec().Resize(nnodes);
    
    TPZManVector<REAL,3> coord(3,0.);
    int in = 0;
    
    //cubo [0,1]ˆ3
    //    //c0
    //    coord[0] = 0.0;
    //    coord[1] = 0.0;
    //    coord[2] = 0.0;
    //    gmesh->NodeVec()[in].SetCoord(coord);
    //    gmesh->NodeVec()[in].SetNodeId(in++);
    //    //c1
    //    coord[0] =  1.0;
    //    coord[1] = 0.0;
    //    coord[2] = 0.0;
    //    gmesh->NodeVec()[in].SetCoord(coord);
    //    gmesh->NodeVec()[in].SetNodeId(in++);
    //    //c2
    //    coord[0] =  1.0;
    //    coord[1] =  1.0;
    //    coord[2] = 0.0;
    //    gmesh->NodeVec()[in].SetCoord(coord);
    //    gmesh->NodeVec()[in].SetNodeId(in++);
    //    //c3
    //    coord[0] = 0.0;
    //    coord[1] =  1.0;
    //    coord[2] = 0.0;
    //    gmesh->NodeVec()[in].SetCoord(coord);
    //    gmesh->NodeVec()[in].SetNodeId(in++);
    //
    //    //c4
    //    coord[0] = 0.0;
    //    coord[1] = 0.0;
    //    coord[2] =  1.0;
    //    gmesh->NodeVec()[in].SetCoord(coord);
    //    gmesh->NodeVec()[in].SetNodeId(in++);
    //    //c5
    //    coord[0] =  1.0;
    //    coord[1] = 0.0;
    //    coord[2] =  1.0;
    //    gmesh->NodeVec()[in].SetCoord(coord);
    //    gmesh->NodeVec()[in].SetNodeId(in++);
    //    //c6
    //    coord[0] =  1.0;
    //    coord[1] =  1.0;
    //    coord[2] =  1.0;
    //    gmesh->NodeVec()[in].SetCoord(coord);
    //    gmesh->NodeVec()[in].SetNodeId(in++);
    //    //c7
    //    coord[0] = 0.0;
    //    coord[1] =  1.0;
    //    coord[2] =  1.0;
    //    gmesh->NodeVec()[in].SetCoord(coord);
    //    gmesh->NodeVec()[in].SetNodeId(in++);
    
    // cubo [-1,1]^3
    //c0
    coord[0] = -1.0;
    coord[1] = -1.0;
    coord[2] = -1.0;
    gmesh->NodeVec()[in].SetCoord(coord);
    gmesh->NodeVec()[in].SetNodeId(in);
    in++;
    //c1
    coord[0] =  1.0;
    coord[1] = -1.0;
    coord[2] = -1.0;
    gmesh->NodeVec()[in].SetCoord(coord);
    gmesh->NodeVec()[in].SetNodeId(in);
    in++;
    //c2
    coord[0] =  1.0;
    coord[1] =  1.0;
    coord[2] = -1.0;
    gmesh->NodeVec()[in].SetCoord(coord);
    gmesh->NodeVec()[in].SetNodeId(in);
    in++;
    //c3
    coord[0] = -1.0;
    coord[1] =  1.0;
    coord[2] = -1.0;
    gmesh->NodeVec()[in].SetCoord(coord);
    gmesh->NodeVec()[in].SetNodeId(in);
    in++;
    
    //c4
    coord[0] = -1.0;
    coord[1] = -1.0;
    coord[2] =  1.0;
    gmesh->NodeVec()[in].SetCoord(coord);
    gmesh->NodeVec()[in].SetNodeId(in);
    in++;
    //c5
    coord[0] =  1.0;
    coord[1] = -1.0;
    coord[2] =  1.0;
    gmesh->NodeVec()[in].SetCoord(coord);
    gmesh->NodeVec()[in].SetNodeId(in);
    in++;
    //c6
    coord[0] =  1.0;
    coord[1] =  1.0;
    coord[2] =  1.0;
    gmesh->NodeVec()[in].SetCoord(coord);
    gmesh->NodeVec()[in].SetNodeId(in);
    in++;
    //c7
    coord[0] = -1.0;
    coord[1] =  1.0;
    coord[2] =  1.0;
    gmesh->NodeVec()[in].SetCoord(coord);
    gmesh->NodeVec()[in].SetNodeId(in);
    in++;
    
    
    int index = 0;
    
    TPZVec<long> TopologyQuad(4);
    
    // bottom
    TopologyQuad[0]=0;
    TopologyQuad[1]=1;
    TopologyQuad[2]=2;
    TopologyQuad[3]=3;
    new TPZGeoElRefPattern< pzgeom::TPZGeoQuad>(index,TopologyQuad,idf0,*gmesh);
    index++;
    
    // Front
    TopologyQuad[0]=0;
    TopologyQuad[1]=1;
    TopologyQuad[2]=5;
    TopologyQuad[3]=4;
    new TPZGeoElRefPattern< pzgeom::TPZGeoQuad>(index,TopologyQuad,idf1,*gmesh);
    index++;
    
    // Rigth
    TopologyQuad[0]=1;
    TopologyQuad[1]=2;
    TopologyQuad[2]=6;
    TopologyQuad[3]=5;
    new TPZGeoElRefPattern< pzgeom::TPZGeoQuad>(index,TopologyQuad,idf2,*gmesh);
    index++;
    // Back
    TopologyQuad[0]=3;
    TopologyQuad[1]=2;
    TopologyQuad[2]=6;
    TopologyQuad[3]=7;
    new TPZGeoElRefPattern< pzgeom::TPZGeoQuad>(index,TopologyQuad,idf3,*gmesh);
    index++;
    
    // Left
    TopologyQuad[0]=0;
    TopologyQuad[1]=3;
    TopologyQuad[2]=7;
    TopologyQuad[3]=4;
    new TPZGeoElRefPattern< pzgeom::TPZGeoQuad>(index,TopologyQuad,idf4,*gmesh);
    index++;
    
    // Top
    TopologyQuad[0]=4;
    TopologyQuad[1]=5;
    TopologyQuad[2]=6;
    TopologyQuad[3]=7;
    new TPZGeoElRefPattern< pzgeom::TPZGeoQuad>(index,TopologyQuad,idf5,*gmesh);
    index++;
    
    TPZManVector<long,8> TopolCubo(8,0);
    TopolCubo[0] = 0;
    TopolCubo[1] = 1;
    TopolCubo[2] = 2;
    TopolCubo[3] = 3;
    TopolCubo[4] = 4;
    TopolCubo[5] = 5;
    TopolCubo[6] = 6;
    TopolCubo[7] = 7;
    
    
    new TPZGeoElRefPattern< pzgeom::TPZGeoCube> (index, TopolCubo, matId, *gmesh);
    index++;
    
    gmesh->BuildConnectivity();
    
    /// gmesh para aqui
    
    TPZVec<TPZGeoEl *> sons;
    for (int iref = 0; iref < nref; iref++) {
        int nel = gmesh->NElements();
        for (int iel = 0; iel < nel; iel++) {
            TPZGeoEl *gel = gmesh->ElementVec()[iel];
            if (gel->HasSubElement()) {
                continue;
            }
            gel->Divide(sons);
        }
    }
    
    
    std::ofstream out("SingleCubeWithBcs.vtk");
    TPZVTKGeoMesh::PrintGMeshVTK(gmesh, out, true);
    
    return gmesh;
}

void UniformRefineArctan(TPZGeoMesh* gmesh, int nDiv)
{
    for(int D = 0; D < nDiv; D++)
    {
        int nels = gmesh->NElements();
        for(int elem = 0; elem < nels; elem++)
        {
            TPZVec< TPZGeoEl * > filhos;
            TPZGeoEl * gel = gmesh->ElementVec()[elem];
            gel->Divide(filhos);
        }
    }
    //	gmesh->BuildConnectivity();
}


TPZCompMesh *CMeshFluxArctan(TPZGeoMesh *gmesh, int pOrder, int dim)
{
    /// criar materiais
    //TPZMatPoisson3d *material = new TPZMatPoisson3d(matId,dim);
    TPZMatPoisson3d *material = new TPZMatPoisson3d(matId,dim);
    TPZMaterial * mat(material);
    material->NStateVariables();
    
    //  TPZAutoPointer<TPZFunction<STATE> > force1 = new TPZDummyFunction<STATE>(Forcing1);
    //	material->SetForcingFunction(force1);
    
    TPZCompMesh * cmesh = new TPZCompMesh(gmesh);
    
    
    ///Inserir condicao de contorno
    TPZFMatrix<STATE> val1(2,2,0.), val2(2,1,0.);
    TPZMaterial * BCond0 = material->CreateBC(mat, bc0,dirichlet, val1, val2);
    TPZMaterial * BCond1 = material->CreateBC(mat, bc1,dirichlet, val1, val2);
    TPZMaterial * BCond2 = material->CreateBC(mat, bc2,dirichlet, val1, val2);
    TPZMaterial * BCond3 = material->CreateBC(mat, bc3,dirichlet, val1, val2);
    TPZMaterial * BCond4 = material->CreateBC(mat, bc4,dirichlet, val1, val2);
    TPZMaterial * BCond5 = material->CreateBC(mat, bc5,dirichlet, val1, val2);
    
    cmesh->InsertMaterialObject(mat);
    
    cmesh->SetDimModel(dim);
    
    cmesh->SetAllCreateFunctionsHDiv();
    
    
    cmesh->InsertMaterialObject(BCond0);
    cmesh->InsertMaterialObject(BCond1);
    cmesh->InsertMaterialObject(BCond2);
    cmesh->InsertMaterialObject(BCond3);
    cmesh->InsertMaterialObject(BCond4);
    cmesh->InsertMaterialObject(BCond5);
    
    cmesh->SetDefaultOrder(pOrder);
    
    TPZLagrangeMultiplier *matskelet = new TPZLagrangeMultiplier(matskeleton, dim-1, 1);
    TPZMaterial * mat2(matskelet);
    cmesh->InsertMaterialObject(mat2);
    
    
    //Ajuste da estrutura de dados computacional
    cmesh->AutoBuild();
    
    //#ifdef LOG4CXX
    //	if(logdata->isDebugEnabled())
    //	{
    //        std::stringstream sout;
    //        sout<<"\n\n Malha Computacional_1 Fluxo\n ";
    //        cmesh->Print(sout);
    //        LOGPZ_DEBUG(logdata,sout.str())
    //	}
    //#endif
    
    return cmesh;
}

TPZCompMesh *CMeshPressureArctan(TPZGeoMesh *gmesh, int pOrder, int dim)
{
    /// criar materiais
    //TPZMatPoisson3d *material = new TPZMatPoisson3d(matId,dim);
    TPZMatPoisson3d *material = new TPZMatPoisson3d(matId,dim);
    material->NStateVariables();
    
    //    TPZAutoPointer<TPZFunction<STATE> > force1 = new TPZDummyFunction<STATE>(Forcing1);
    //	material->SetForcingFunction(force1);
    
    TPZCompMesh * cmesh = new TPZCompMesh(gmesh);
    cmesh->SetDimModel(dim);
    TPZMaterial * mat(material);
    cmesh->InsertMaterialObject(mat);
    
    ///Inserir condicao de contorno
    TPZFMatrix<STATE> val1(2,2,0.), val2(2,1,0.);
    //    TPZMaterial * BCond0 = material->CreateBC(mat, bc0,dirichlet, val1, val2);
    //    TPZMaterial * BCond1 = material->CreateBC(mat, bc1,dirichlet, val1, val2);
    //    TPZMaterial * BCond2 = material->CreateBC(mat, bc2,dirichlet, val1, val2);
    //    TPZMaterial * BCond3 = material->CreateBC(mat, bc3,dirichlet, val1, val2);
    //    TPZMaterial * BCond4 = material->CreateBC(mat, bc4,dirichlet, val1, val2);
    //    TPZMaterial * BCond5 = material->CreateBC(mat, bc5,dirichlet, val1, val2);
    //
    //    cmesh->InsertMaterialObject(BCond0);
    //    cmesh->InsertMaterialObject(BCond1);
    //    cmesh->InsertMaterialObject(BCond2);
    //    cmesh->InsertMaterialObject(BCond3);
    //    cmesh->InsertMaterialObject(BCond4);
    //    cmesh->InsertMaterialObject(BCond5);
    
    cmesh->SetDefaultOrder(pOrder);
    //cmesh->SetDimModel(dim);
    
    bool h1function = true;//false em triangulo
    if(h1function){
        cmesh->SetAllCreateFunctionsContinuous();
        cmesh->ApproxSpace().CreateDisconnectedElements(true);
    }
    else{
        cmesh->SetAllCreateFunctionsDiscontinuous();
    }
    
    
    //Ajuste da estrutura de dados computacional
    cmesh->AutoBuild();
    
    
    int ncon = cmesh->NConnects();
    for(int i=0; i<ncon; i++)
    {
        TPZConnect &newnod = cmesh->ConnectVec()[i];
        //newnod.SetPressure(true);
        newnod.SetLagrangeMultiplier(1);
    }
    
    if(!h1function)
    {
        
        int nel = cmesh->NElements();
        for(int i=0; i<nel; i++){
            TPZCompEl *cel = cmesh->ElementVec()[i];
            TPZCompElDisc *celdisc = dynamic_cast<TPZCompElDisc *>(cel);
            celdisc->SetConstC(1.);
            celdisc->SetCenterPoint(0, 0.);
            celdisc->SetCenterPoint(1, 0.);
            celdisc->SetCenterPoint(2, 0.);
            celdisc->SetTrueUseQsiEta();
            //celdisc->SetFalseUseQsiEta();
            
            //            TPZVec<REAL> qsi(3,0.);
            //            qsi[0] = 0.5;
            //            qsi[1] = 0.5;
            //            TPZFMatrix<REAL> phi;
            //            TPZFMatrix<REAL> dphi;
            //            celdisc->Shape(qsi, phi,dphi);
            //            phi.Print("phi = ");
            
            
            if(celdisc && celdisc->Reference()->Dimension() == cmesh->Dimension())
            {
                if(ftriang==true) celdisc->SetTotalOrderShape();
                else celdisc->SetTensorialShape();
            }
            
        }
    }
    
#ifdef DEBUG
    int ncel = cmesh->NElements();
    for(int i =0; i<ncel; i++){
        TPZCompEl * compEl = cmesh->ElementVec()[i];
        if(!compEl) continue;
        TPZInterfaceElement * facel = dynamic_cast<TPZInterfaceElement *>(compEl);
        if(facel)DebugStop();
        
    }
#endif
    
    
    
    //#ifdef LOG4CXX
    //	if(logdata->isDebugEnabled())
    //	{
    //        std::stringstream sout;
    //        sout<<"\n\n Malha Computacional_2 pressure\n ";
    //        cmesh->Print(sout);
    //        LOGPZ_DEBUG(logdata,sout.str());
    //	}
    //#endif
    return cmesh;
}

#include "pzcondensedcompel.h"
#include "pzelementgroup.h"
TPZCompMesh *CMeshMixedArctan(TPZGeoMesh * gmesh, TPZVec<TPZCompMesh *> meshvec)
{
    
    //Creating computational mesh for multiphysic elements
    gmesh->ResetReference();
    TPZCompMesh *mphysics = new TPZCompMesh(gmesh);
    
    //criando material
    int dim = gmesh->Dimension();
    
    bool hdivantigo;
    
    TPZMatMixedPoisson3D *material = new TPZMatMixedPoisson3D(matId,dim); hdivantigo = false; // nesse material tem que ser false
    //TPZMixedPoisson *material = new TPZMixedPoisson(matId,dim); hdivantigo = true;; // nesse material tem que ser true
    //incluindo os dados do problema
    if (hdivantigo) {
        TPZFNMatrix<2,REAL> PermTensor(dim,dim,0.);
        TPZFNMatrix<2,REAL> InvPermTensor(dim,dim,0.);
        
        for (int i=0; i<dim; i++)
        {
            PermTensor(i,i) = 1.0;
        }
        InvPermTensor=PermTensor;
        material->SetPermeabilityTensor(PermTensor, InvPermTensor);
    }
    
    //incluindo os dados do problema
    TPZFNMatrix<2,REAL> PermTensor(dim,dim,0.);
    TPZFNMatrix<2,REAL> InvPermTensor(dim,dim,0.);
    
    PermTensor = TP;
    InvPermTensor = InvTP;
    
    
    material->SetPermeabilityTensor(PermTensor, InvPermTensor);
    
    //solucao exata
    TPZAutoPointer<TPZFunction<STATE> > solexata;
    
    solexata = new TPZDummyFunction<STATE>(SolExataArctan);
    material->SetForcingFunctionExact(solexata);
    mphysics->SetDimModel(dim);
    //funcao do lado direito da equacao do problema
    TPZDummyFunction<STATE> *dum = new TPZDummyFunction<STATE>(ForcingArctan);
    TPZAutoPointer<TPZFunction<STATE> > forcef;
    dum->SetPolynomialOrder(10);
    forcef = dum;
    material->SetForcingFunction(forcef);
    
    //inserindo o material na malha computacional
    TPZMaterial *mat(material);
    mphysics->InsertMaterialObject(mat);
    
    
    //Criando condicoes de contorno
    TPZMaterial * BCond0;
    TPZMaterial * BCond1;
    TPZMaterial * BCond2;
    TPZMaterial * BCond3;
    TPZMaterial * BCond4;
    TPZMaterial * BCond5;
    
    TPZFMatrix<STATE> val1(2,2,0.), val2(2,1,0.);
    val2(0,0) = 0.0;
    val2(1,0) = 0.0;
    TPZAutoPointer<TPZFunction<STATE> > FBCond0 = new TPZDummyFunction<STATE>(ForcingBC0DArctan);
    BCond0 = material->CreateBC(mat, bc0,dirichlet, val1, val2);
    BCond0->SetForcingFunction(FBCond0);
    
    val2(0,0) = 0.0;
    val2(1,0) = 0.0;
    TPZAutoPointer<TPZFunction<STATE> > FBCond1 = new TPZDummyFunction<STATE>(ForcingBC1DArctan);
    BCond1 = material->CreateBC(mat, bc1,dirichlet, val1, val2);
    BCond1->SetForcingFunction(FBCond1);
    
    val2(0,0) = 0.0;
    val2(1,0) = 0.0;
    TPZAutoPointer<TPZFunction<STATE> > FBCond2 = new TPZDummyFunction<STATE>(ForcingBC2DArctan);
    BCond2 = material->CreateBC(mat, bc2,dirichlet, val1, val2);
    BCond2->SetForcingFunction(FBCond2);
    
    val2(0,0) = 0.0;
    val2(1,0) = 0.0;
    TPZAutoPointer<TPZFunction<STATE> > FBCond3 = new TPZDummyFunction<STATE>(ForcingBC3DArctan);
    BCond3 = material->CreateBC(mat, bc3,dirichlet, val1, val2);
    BCond3->SetForcingFunction(FBCond3);
    
    val2(0,0) = 0.0;
    val2(1,0) = 0.0;
    TPZAutoPointer<TPZFunction<STATE> > FBCond4 = new TPZDummyFunction<STATE>(ForcingBC4DArctan);
    BCond4 = material->CreateBC(mat, bc4,dirichlet, val1, val2);
    BCond4->SetForcingFunction(FBCond4);

    val2(0,0) = 0.0;
    val2(1,0) = 0.0;
    TPZAutoPointer<TPZFunction<STATE> > FBCond5 = new TPZDummyFunction<STATE>(ForcingBC5DArctan);
    BCond5 = material->CreateBC(mat, bc5,dirichlet, val1, val2);
    BCond5->SetForcingFunction(FBCond5);

    
    
    mphysics->SetAllCreateFunctionsMultiphysicElem();
    mphysics->InsertMaterialObject(BCond0);
    mphysics->InsertMaterialObject(BCond1);
    mphysics->InsertMaterialObject(BCond2);
    mphysics->InsertMaterialObject(BCond3);
    mphysics->InsertMaterialObject(BCond4);
    mphysics->InsertMaterialObject(BCond5);
    
    //Fazendo auto build
    mphysics->AutoBuild();
    mphysics->AdjustBoundaryElements();
    mphysics->CleanUpUnconnectedNodes();
    
    // Creating multiphysic elements into mphysics computational mesh
    if(hdivantigo==true){
        TPZBuildMultiphysicsMesh::AddElements(meshvec, mphysics);
        TPZBuildMultiphysicsMesh::AddConnects(meshvec,mphysics);
        TPZBuildMultiphysicsMesh::TransferFromMeshes(meshvec, mphysics);
    }
    //Creating multiphysic elements containing skeletal elements.
    else
    {
        TPZBuildMultiphysicsMesh::AddElements(meshvec, mphysics);
        
        //        TPZMaterial * skeletonEl = material->CreateBC(mat, matskeleton, 3, val1, val2);
        //        mphysics->InsertMaterialObject(skeletonEl);
        
        //        TPZLagrangeMultiplier *matskelet = new TPZLagrangeMultiplier(matskeleton, dim-1, 1);
        //        TPZMaterial * mat2(matskelet);
        //        mphysics->InsertMaterialObject(mat2);
        
        int nel = mphysics->ElementVec().NElements();
        TPZStack< TPZStack< TPZMultiphysicsElement *,7> > wrapEl;
        for(int el = 0; el < nel; el++)
        {
            TPZMultiphysicsElement *mfcel = dynamic_cast<TPZMultiphysicsElement *>(mphysics->Element(el));
            if(mfcel->Dimension()==dim) TPZBuildMultiphysicsMesh::AddWrap(mfcel, matId, wrapEl);//criei elementos com o mesmo matId interno, portanto nao preciso criar elemento de contorno ou outro material do tipo TPZLagrangeMultiplier
        }
        meshvec[0]->CleanUpUnconnectedNodes();
        TPZBuildMultiphysicsMesh::AddConnects(meshvec,mphysics);
        TPZBuildMultiphysicsMesh::TransferFromMeshes(meshvec, mphysics);
        
        //------- Create and add group elements -------
        long index, nenvel;
        nenvel = wrapEl.NElements();
        for(int ienv=0; ienv<nenvel; ienv++){
            TPZElementGroup *elgr = new TPZElementGroup(*wrapEl[ienv][0]->Mesh(),index);
            nel = wrapEl[ienv].NElements();
            for(int jel=0; jel<nel; jel++){
                elgr->AddElement(wrapEl[ienv][jel]);
            }
        }
    }
    
    return mphysics;
}

void SolveSystArctan(TPZAnalysis &an, TPZCompMesh *fCmesh)
{
    
    bool isdirect = true;
    if (isdirect) {
        //TPZBandStructMatrix full(fCmesh);
        TPZSkylineStructMatrix skylstr(fCmesh); //caso simetrico
        //    TPZSkylineNSymStructMatrix full(fCmesh);
        an.SetStructuralMatrix(skylstr);
        TPZStepSolver<STATE> step;
        step.SetDirect(ELDLt); //caso simetrico
        //	step.SetDirect(ELU);
        an.SetSolver(step);
        //    an.Assemble();
        an.Run();
    }
    else
    {
        TPZSkylineStructMatrix skylstr(fCmesh); //caso simetrico
        skylstr.SetNumThreads(2);
        an.SetStructuralMatrix(skylstr);
        
        TPZAutoPointer<TPZMatrix<STATE> > matbeingcopied = skylstr.Create();
        TPZAutoPointer<TPZMatrix<STATE> > matClone = matbeingcopied->Clone();
        
        TPZStepSolver<STATE> *precond = new TPZStepSolver<STATE>(matClone);
        TPZStepSolver<STATE> *Solver = new TPZStepSolver<STATE>(matbeingcopied);
        precond->SetReferenceMatrix(matbeingcopied);
        precond->SetDirect(ELDLt);
        Solver->SetGMRES(20, 20, *precond, 1.e-18, 0);
        //        Solver->SetCG(10, *precond, 1.0e-10, 0);
        an.SetSolver(*Solver);
        an.Run();
    }
    
    cout <<"Numero de equacoes "<< fCmesh->NEquations()<< endl;
    
    //Saida de Dados: solucao e  grafico no VT
    //	ofstream file("Solutout");
    //	an.Solution().Print("solution", file);    //Solution visualization on Paraview (VTK)
}

void PosProcessArctan(TPZAnalysis &an, std::string plotfile){
    TPZManVector<std::string,10> scalnames(1), vecnames(0);
    scalnames[0]= "Solution";
    //const int dim = 3;
    int div = 2;
    an.DefineGraphMesh(dim,scalnames,vecnames,plotfile);
    an.PostProcess(div,dim);
    //    std::ofstream out("malhaNormal.txt");
    //    an.Print("nothing",out);
}

void PosProcessMultphysicsArctan(TPZVec<TPZCompMesh *> meshvec, TPZCompMesh* mphysics, TPZAnalysis &an, std::string plotfile){
    
    TPZBuildMultiphysicsMesh::TransferFromMultiPhysics(meshvec, mphysics);
    TPZManVector<std::string,10> scalnames(2), vecnames(2);
    vecnames[0]  = "Flux";
    vecnames[1]  = "ExactFlux";
    //    vecnames[1]  = "GradFluxX";
    //    vecnames[2]  = "GradFluxY";
    //    vecnames[2]  = "GradFluxz";
    scalnames[0] = "Pressure";
    scalnames[1] = "ExactPressure";
    
    //const int dim = 3;
    int div =2;
    an.DefineGraphMesh(dim,scalnames,vecnames,plotfile);
    an.PostProcess(div,dim);
    std::ofstream out("malha.txt");
    an.Print("nothing",out);
    
    //    mphysics->Solution().Print("Solucao");
    
}

void SolExataArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &solp, TPZFMatrix<STATE> &flux){
    
    solp.resize(1);
    solp[0]=0.;
    flux.Resize(dim, 1);
    
    flux(0,0)=flux(1,0)=flux(2,0)=0.;
    double x = pt[0];
    double y = pt[1];
    double z = pt[2];
    solp[0] = atan(exp(  (aa + bb + cc - x - y - z)/Epsilon)  );
    //    REAL exp1 = exp((aa+bb+cc+x+y+z)/(Epsilon));
    //    REAL exp2 = exp(2.0*(aa+bb+cc)/(Epsilon));
    //    REAL exp3 = exp(2.0*(x+y+z)/(Epsilon)));
    //    flux(0,0)= (exp1*(TP(0,0)+TP(0,1)+TP(0,2)))/(exp2+exp3)*Epsilon);
    //    flux(1,0)= (exp1*(TP(1,0)+TP(1,1)+TP(1,2)))/(exp2+exp3)*Epsilon);
    //    flux(2,0)= (exp1*(TP(2,0)+TP(2,1)+TP(2,2)))/(exp2+exp3)*Epsilon);
    
    REAL numerador = exp((aa+bb+cc+x+y+z)/(Epsilon));
    REAL denominador = (exp(2.0*(aa+bb+cc)/(Epsilon))+exp(2.0*(x+y+z)/(Epsilon)))*Epsilon;
    
    flux(0,0)= ( numerador *(TP(0,0)+TP(0,1)+TP(0,2)) )/(   denominador   );
    flux(1,0)= ( numerador *(TP(1,0)+TP(1,1)+TP(1,2)) )/(   denominador   ); // (exp(2.0*(aa+bb+cc)/(Epsilon))+exp(2.0*(x+y+z)/(Epsilon)))*Epsilon
    flux(2,0)= ( numerador *(TP(2,0)+TP(2,1)+TP(2,2)) )/(   denominador   ); // (exp(2.0*(aa+bb+cc)/(Epsilon))+exp(2.0*(x+y+z)/(Epsilon)))*Epsilon

    
}

void ForcingArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp){
    // TP e o tensor de pemearbilidade.
    double x = pt[0];
    double y = pt[1];
    double z = pt[2];
    
    //disp[0] = (  exp((aa+bb+cc+x+y+z)/Epsilon)*(  exp((2.0*(aa+bb+cc))/Epsilon)-exp((2.0*(x+y+z))/Epsilon)  )*(TP(0,0)+TP(0,1)+TP(0,2)+TP(1,0)+TP(1,1)+TP(1,2)+TP(2,0)+TP(2,1)+TP(2,2))  )/(   ( (exp((2.0*(aa+bb+cc))/Epsilon)+exp((2.0*(x+y+z))/Epsilon))*(exp((2.0*(aa+bb+cc))/Epsilon)+exp((2.0*(x+y+z))/Epsilon)) )*Epsilon*Epsilon  );
    //disp[0] = 2*x*x*y*y + 2*x*x*z*z + 2*y*y*z*z;
    
    disp[0]=( (TP(0,0)+TP(0,1)+TP(0,2)+TP(1,0)+TP(1,1)+TP(1,2)+TP(2,0)+TP(2,1)+TP(2,2) ) * (1.0/(cosh((aa+bb+cc-x-y-z)/Epsilon))) * tanh((aa+bb+cc-x-y-z)/Epsilon))/(2*Epsilon*Epsilon);

    
    
}

void ForcingBC0DArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp){

    double x = pt[0];
    double y = pt[1];
    double z = pt[2];
    disp[0] = atan(exp(  (aa + bb + cc - x - y - z)/Epsilon)  );
    //    disp[0] = (x*y*z)*(x*y*z)
}

void ForcingBC1DArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp){
    double x = pt[0];
    double y = pt[1];
    double z = pt[2];
    disp[0] = atan(exp(  (aa + bb + cc - x - y - z)/Epsilon)  );

}

void ForcingBC2DArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp){
    double x = pt[0];
    double y = pt[1];
    double z = pt[2];
    disp[0] = atan(exp(  (aa + bb + cc - x - y - z)/Epsilon)  );
}

void ForcingBC3DArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp){
    double x = pt[0];
    double y = pt[1];
    double z = pt[2];
    disp[0] = atan(exp(  (aa + bb + cc - x - y - z)/Epsilon)  );
}

void ForcingBC4DArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp){
    double x = pt[0];
    double y = pt[1];
    double z = pt[2];
    disp[0] = atan(exp(  (aa + bb + cc - x - y - z)/Epsilon)  );

}

void ForcingBC5DArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp){
    double x = pt[0];
    double y = pt[1];
    double z = pt[2];

    disp[0] = atan(exp(  (aa + bb + cc - x - y - z)/Epsilon)  );

}


void ForcingBC0NArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp){
    double x = pt[0];
    double y = pt[1];

    disp[0] =  (  exp((1.+aa+bb+cc+x+y)/Epsilon)*(TP(1,0)+TP(1,1)+TP(1,2))  )/( (exp((2.*(1.+aa+bb+cc))/Epsilon)+exp((2.*(x+y))/Epsilon))*Epsilon );
    
}

void ForcingBC1NArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp){
    double x = pt[0];
    double z = pt[2];

    disp[0] =  (  exp((1.+aa+bb+cc+x+z)/Epsilon)*(TP(0,0)+TP(0,1)+TP(0,2))  )/( (exp((2.*(1.+aa+bb+cc))/Epsilon)+exp((2.*(x+z))/Epsilon))*Epsilon );
    
}

void ForcingBC2NArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp){
    double y = pt[1];
    double z = pt[2];

    disp[0] =  (  exp((1.+aa+bb+cc+y+z)/Epsilon)*(TP(1,0)+TP(1,1)+TP(1,2))  )/( (exp((2.*(aa+bb+cc))/Epsilon)+exp((2.*(1.+y+z))/Epsilon))*Epsilon );

}

void ForcingBC3NArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp){
    double x = pt[0];
    double z = pt[2];

    disp[0] =  (  exp((1.+aa+bb+cc+x+z)/Epsilon)*(TP(0,0)+TP(0,1)+TP(0,2))  )/( (exp((2.*(aa+bb+cc))/Epsilon)+exp((2.*(1.+x+z))/Epsilon))*Epsilon );

}
void ForcingBC4NArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp){
    double y = pt[1];
    double z = pt[2];
    

    disp[0] =  (  exp((1.+aa+bb+cc+y+z)/Epsilon)*(TP(2,0)+TP(2,1)+TP(2,2))  )/( (exp((2.*(1.+aa+bb+cc))/Epsilon)+exp((2.*(z+y))/Epsilon))*Epsilon );

}

void ForcingBC5NArctan(const TPZVec<REAL> &pt, TPZVec<STATE> &disp){
    double x = pt[0];
    double y = pt[1];
    

    disp[0] =  (  exp((1.+aa+bb+cc+x+y)/Epsilon)*(TP(2,0)+TP(2,1)+TP(2,2))  )/( (exp((2.*(aa+bb+cc))/Epsilon)+exp((2.*(1.+x+y))/Epsilon))*Epsilon );

}

void ErrorHDivArctan(TPZCompMesh *hdivmesh, std::ostream &out, int p, int ndiv)
{
    long nel = hdivmesh->NElements();
    int dim = hdivmesh->Dimension();
    TPZManVector<STATE,10> globalerrors(10,0.);
    for (long el=0; el<nel; el++) {
        TPZCompEl *cel = hdivmesh->ElementVec()[el];
        if(cel->Reference()->Dimension()!=dim) continue; // Filtering lower dimension elements
        TPZManVector<STATE,10> elerror(10,0.);
        elerror.Fill(0.);
        cel->EvaluateError(SolExataArctan, elerror, NULL);
        int nerr = elerror.size();
        for (int i=0; i<nerr; i++) {
            globalerrors[i] += elerror[i]*elerror[i];
        }
        
    }
    out << "Errors associated with HDiv space - ordem polinomial = " << p << "- divisoes = " << ndiv << endl;
    out << "L2 Norm for flux - "<< endl; //L2 Norm for divergence - Hdiv Norm for flux " << endl;
    out <<  setw(16) << sqrt(globalerrors[1]) <<endl;// setw(25)  << sqrt(globalerrors[2]) << setw(21)  << sqrt(globalerrors[3]) << endl;
    //
    //    out << "L2 Norm for flux = "    << sqrt(globalerrors[1]) << endl;
    //    out << "L2 Norm for divergence = "    << sqrt(globalerrors[2])  <<endl;
    //    out << "Hdiv Norm for flux = "    << sqrt(globalerrors[3])  <<endl;
    //
    fDebugMapHdiv.insert(std::pair<REAL, REAL> (ndiv,sqrt(globalerrors[1])));
}

void ErrorL2Arctan(TPZCompMesh *l2mesh, std::ostream &out, int p, int ndiv)
{
    long nel = l2mesh->NElements();
    //int dim = l2mesh->Dimension();
    TPZManVector<STATE,10> globalerrors(10,0.);
    for (long el=0; el<nel; el++) {
        TPZCompEl *cel = l2mesh->ElementVec()[el];
        TPZManVector<STATE,10> elerror(10,0.);
        cel->EvaluateError(SolExataArctan, elerror, NULL);
        int nerr = elerror.size();
        globalerrors.resize(nerr);
#ifdef LOG4CXX
        if (logdata->isDebugEnabled()) {
            std::stringstream sout;
            sout << "L2 Error sq of element " << el << elerror[0]*elerror[0];
            LOGPZ_DEBUG(logdata, sout.str())
        }
#endif
        for (int i=0; i<nerr; i++) {
            globalerrors[i] += elerror[i]*elerror[i];
        }
        
    }
    out << "Errors associated with L2 space - ordem polinomial = " << p << "- divisoes = " << ndiv << endl;
    out << "L2 Norm = "    << sqrt(globalerrors[1]) << endl;
    
    fDebugMapL2.insert(std::pair<REAL, REAL> (ndiv,sqrt(globalerrors[1])));
    
    
}


void PrintDebugMapForMathematicaArctan(std::string filenameHdiv, std::string filenameL2)
{
    std::ofstream outHdiv(filenameHdiv.c_str());
    std::ofstream outL2(filenameL2.c_str());
    
    for (std::map<REAL,REAL>::const_iterator it = fDebugMapL2.begin(); it != fDebugMapL2.end(); it++) {
        outL2 << it->first << "   " << it->second << std::endl;
    }
    outL2.close();
    
    
    for (std::map<REAL,REAL>::const_iterator it = fDebugMapHdiv.begin(); it != fDebugMapHdiv.end(); it++) {
        outHdiv <<  it->first << "   " << it->second << std::endl;
    }
    outHdiv.close();
}

TPZGeoMesh *GMeshDeformedArctan(){
    
    // description of Geometry and application
    // 2D Cylindrical Domain boundaries
    int matId = 1;
    int arc1 = -1;
    int arc2 = -2;
    int arc3 = -3;
    int arc4 = -4;
    int Point1 = -11;
    int Point2 = -22;
    int Point3 = -33;
    int Point4 = -44;
    int WellPoint = -55;
    
    int nodenumber = 9;
    REAL ModelRadius = 1.0;
    TPZGeoMesh * gmesh = new TPZGeoMesh;
    gmesh->NodeVec().Resize(nodenumber);
    
    // Setting node coordantes for Arc3D 1
    int id = 0;
    gmesh->NodeVec()[id].SetNodeId(id);
    gmesh->NodeVec()[id].SetCoord(0,0.0 );//coord X
    gmesh->NodeVec()[id].SetCoord(1,0.0);//coord Y
    id++;
    gmesh->NodeVec()[id].SetNodeId(id);
    gmesh->NodeVec()[id].SetCoord(0,ModelRadius );//coord X
    gmesh->NodeVec()[id].SetCoord(1,0.0);//coord Y
    id++;
    gmesh->NodeVec()[id].SetNodeId(id);
    gmesh->NodeVec()[id].SetCoord(0,0.0 );//coord X
    gmesh->NodeVec()[id].SetCoord(1,ModelRadius);//coord Y
    id++;
    gmesh->NodeVec()[id].SetNodeId(id);
    gmesh->NodeVec()[id].SetCoord(0,-ModelRadius );//coord X
    gmesh->NodeVec()[id].SetCoord(1,0.0);//coord Y
    id++;
    gmesh->NodeVec()[id].SetNodeId(id);
    gmesh->NodeVec()[id].SetCoord(0,0.0 );//coord X
    gmesh->NodeVec()[id].SetCoord(1,-ModelRadius);//coord Y
    id++;
    gmesh->NodeVec()[id].SetNodeId(id);
    gmesh->NodeVec()[id].SetCoord(0,sqrt(2)*ModelRadius/2.);//coord X
    gmesh->NodeVec()[id].SetCoord(1,sqrt(2)*ModelRadius/2.);//coord Y
    id++;
    gmesh->NodeVec()[id].SetNodeId(id);
    gmesh->NodeVec()[id].SetCoord(0,-sqrt(2)*ModelRadius/2.);//coord X
    gmesh->NodeVec()[id].SetCoord(1,sqrt(2)*ModelRadius/2.);//coord Y
    id++;
    gmesh->NodeVec()[id].SetNodeId(id);
    gmesh->NodeVec()[id].SetCoord(0,-sqrt(2)*ModelRadius/2.);//coord X
    gmesh->NodeVec()[id].SetCoord(1,-sqrt(2)*ModelRadius/2.);//coord Y
    id++;
    gmesh->NodeVec()[id].SetNodeId(id);
    gmesh->NodeVec()[id].SetCoord(0,sqrt(2)*ModelRadius/2.);//coord X
    gmesh->NodeVec()[id].SetCoord(1,-sqrt(2)*ModelRadius/2.);//coord Y
    id++;
    
    int elementid = 0;
    // Create Geometrical Arc #1
    // Definition of Arc coordenates
    TPZVec < long > nodeindex(3,0.0);
    nodeindex[0] = 1;
    nodeindex[1] = 2;
    nodeindex[2] = 5;
    TPZGeoElRefPattern < pzgeom::TPZArc3D > * elarc1 = new TPZGeoElRefPattern < pzgeom::TPZArc3D > (elementid,nodeindex, arc1,*gmesh);
    elementid++;
    
    // Create Geometrical Arc #2
    nodeindex[0] = 2;
    nodeindex[1] = 3;
    nodeindex[2] = 6;
    TPZGeoElRefPattern < pzgeom::TPZArc3D > * elarc2 = new TPZGeoElRefPattern < pzgeom::TPZArc3D > (elementid,nodeindex, arc2,*gmesh);
    elementid++;
    
    // Create Geometrical Arc #3
    nodeindex[0] = 3;
    nodeindex[1] = 4;
    nodeindex[2] = 7;
    TPZGeoElRefPattern < pzgeom::TPZArc3D > * elarc3 = new TPZGeoElRefPattern < pzgeom::TPZArc3D > (elementid,nodeindex, arc3,*gmesh);
    elementid++;
    
    // Create Geometrical Arc #4
    nodeindex[0] = 4;
    nodeindex[1] = 1;
    nodeindex[2] = 8;
    TPZGeoElRefPattern < pzgeom::TPZArc3D > * elarc4 = new TPZGeoElRefPattern < pzgeom::TPZArc3D > (elementid,nodeindex, arc4,*gmesh);
    elementid++;
    
    // Create Geometrical Point #1
    nodeindex.resize(1);
    nodeindex[0] = 1;
    TPZGeoElRefPattern < pzgeom::TPZGeoPoint > * elpoint1 = new TPZGeoElRefPattern < pzgeom::TPZGeoPoint > (elementid,nodeindex, Point1,*gmesh);
    elementid++;
    
    // Create Geometrical Point #2
    nodeindex.resize(1);
    nodeindex[0] = 3;
    TPZGeoElRefPattern < pzgeom::TPZGeoPoint > * elpoint2 = new TPZGeoElRefPattern < pzgeom::TPZGeoPoint > (elementid,nodeindex, Point2,*gmesh);
    elementid++;
    
    // Create Geometrical Point #3
    nodeindex.resize(1);
    nodeindex[0] = 2;
    TPZGeoElRefPattern < pzgeom::TPZGeoPoint > * elpoint3 = new TPZGeoElRefPattern < pzgeom::TPZGeoPoint > (elementid,nodeindex, Point3,*gmesh);
    elementid++;
    
    // Create Geometrical Point #4
    nodeindex.resize(1);
    nodeindex[0] = 4;
    TPZGeoElRefPattern < pzgeom::TPZGeoPoint > * elpoint4 = new TPZGeoElRefPattern < pzgeom::TPZGeoPoint > (elementid,nodeindex, Point4,*gmesh);
    elementid++;
    
    // Create Geometrical Point for fluid injection or Production #1
    nodeindex.resize(1);
    nodeindex[0] = 0;
    TPZGeoElRefPattern < pzgeom::TPZGeoPoint > * elpoint = new TPZGeoElRefPattern < pzgeom::TPZGeoPoint > (elementid,nodeindex, WellPoint,*gmesh);
    elementid++;
    
    // Create Geometrical Quad #1
    nodeindex.resize(4);
    nodeindex[0] = 1;
    nodeindex[1] = 2;
    nodeindex[2] = 3;
    nodeindex[3] = 4;
    TPZGeoElRefPattern< pzgeom::TPZGeoBlend < pzgeom::TPZGeoQuad > > * Quad1 = new TPZGeoElRefPattern< pzgeom::TPZGeoBlend < pzgeom::TPZGeoQuad > > (elementid,nodeindex, matId,*gmesh);
    elementid++;
    
    //    // Create Geometrical triangle #1
    //    nodeindex.resize(3);
    //    nodeindex[0] = 0;
    //    nodeindex[1] = 1;
    //    nodeindex[2] = 2;
    //    TPZGeoElRefPattern< pzgeom::TPZGeoBlend < pzgeom::TPZGeoTriangle > > *triangle1 = new TPZGeoElRefPattern< pzgeom::TPZGeoBlend < pzgeom::TPZGeoTriangle > > (elementid,nodeindex, matId,*gmesh);
    //    elementid++;
    //
    //    // Create Geometrical triangle #2
    //    nodeindex[0] = 0;
    //    nodeindex[1] = 2;
    //    nodeindex[2] = 3;
    //    TPZGeoElRefPattern<pzgeom::TPZGeoBlend< pzgeom::TPZGeoTriangle > > *triangle2 = new TPZGeoElRefPattern<pzgeom::TPZGeoBlend< pzgeom::TPZGeoTriangle > > (elementid,nodeindex, matId,*gmesh);
    //    elementid++;
    //
    //    // Create Geometrical triangle #3
    //    nodeindex[0] = 0;
    //    nodeindex[1] = 3;
    //    nodeindex[2] = 4;
    //    TPZGeoElRefPattern<pzgeom::TPZGeoBlend< pzgeom::TPZGeoTriangle > > *triangle3 = new TPZGeoElRefPattern< pzgeom::TPZGeoBlend < pzgeom::TPZGeoTriangle > > (elementid,nodeindex, matId,*gmesh);
    //    elementid++;
    //
    //    // Create Geometrical triangle #4
    //    nodeindex[0] = 0;
    //    nodeindex[1] = 4;
    //    nodeindex[2] = 1;
    //    TPZGeoElRefPattern< pzgeom::TPZGeoBlend< pzgeom::TPZGeoTriangle > > *triangle4 = new TPZGeoElRefPattern<pzgeom::TPZGeoBlend< pzgeom::TPZGeoTriangle > > (elementid,nodeindex, matId,*gmesh);
    //    elementid++;
    
    
    
    
    gmesh->BuildConnectivity();
    
    int nref = 0;
    TPZVec<TPZGeoEl *> sons;
    for (int iref = 0; iref < nref; iref++) {
        int nel = gmesh->NElements();
        for (int iel = 0; iel < nel; iel++) {
            TPZGeoEl *gel = gmesh->ElementVec()[iel];
            if (gel->HasSubElement()) {
                continue;
            }
            gel->Divide(sons);
        }
    }
    
    
    std::ofstream out("CurvoDAC.vtk");
    TPZVTKGeoMesh::PrintGMeshVTK(gmesh, out, true);
    
    return gmesh;
}
