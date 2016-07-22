

#include <cmath>
#include <set>

#include <iostream>
#include <fstream>
#include <string>
#include "pzgmesh.h"
#include "pzstack.h"
#include "TPZVTKGeoMesh.h"
#include "pzanalysis.h"
#include "pzbndcond.h"
#include "TPZStokesMaterial.h"
#include <pzgeoel.h>
#include "pzgeoelbc.h"
#include "pzfmatrix.h"
#include "pzbstrmatrix.h"
#include <TPZGeoElement.h>
#include "TPZVTKGeoMesh.h"
#include "pzbuildmultiphysicsmesh.h"
#include "TPZInterfaceEl.h"
#include "TPZMultiPhysicsInterfaceEl.h"
#include "pzmat2dlin.h"
#include "pzfstrmatrix.h"
#include "pzskylstrmatrix.h"
#include "TPZSkylineNSymStructMatrix.h"
#include "pzstepsolver.h"
#include "TPZGeoLinear.h"
#include "tpzgeoelrefpattern.h"


//------------------STOKES Creep Of Concrete------------------------




/**
 * @brief Funcao para criar a malha geometrica do problema a ser simulado
 * @note A malha sera unidimensional formada por nel elementos de tamanho elsize
 * @param uNDiv number of divisions ortogonal to the plates performed on the domain
 * @param vNDiv number of divisions parallel to the plates performed on the domain
 * @param nel numero de elementos
 * @param elsize tamanho dos elementos
 */
TPZGeoMesh *CreateGMesh(int nelx, int nely, double hx, double hy);

/**
 * @brief Funcao para criar a malha computacional da velocidade a ser simulado
 * @note Responsavel pela criacao dos espacos de aproximacao do problema
 * @param gmesh malha geometrica
 * @param pOrder ordem polinomial de aproximacao
 */
TPZCompMesh *CMesh_v(TPZGeoMesh *gmesh, int pOrder);

/**
 * @brief Funcao para criar a malha computacional da pressão a ser simulado
 * @note Responsavel pela criacao dos espacos de aproximacao do problema
 * @param gmesh malha geometrica
 * @param pOrder ordem polinomial de aproximacao
 */
TPZCompMesh *CMesh_p(TPZGeoMesh *gmesh, int pOrder);

/**
 * @brief Funcao para criar a malha computacional multi-fisica ser simulado
 * @note Responsavel pela criacao dos espacos de aproximacao do problema
 * @param gmesh malha geometrica
 * @param pOrder ordem polinomial de aproximacao
 */
TPZCompMesh *CMesh_m(TPZGeoMesh *gmesh, int pOrder);


//Função para criar interface entre elmentos:

TPZCompEl *CreateInterfaceEl(TPZGeoEl *gel,TPZCompMesh &mesh,int &index);


void Error(TPZCompMesh *hdivmesh, std::ostream &out, int p, int ndiv);

//Variáveis globais do problema:

const int dim = 2; //Dimensão do problema
const int matID = 1; //Materia do elemento volumétrico
const int matBCbott = -1, matBCtop = -2, matBCleft = -3, matBCright = -4; //Materiais das condições de contorno
const int matInterface = 4; //Material do elemento de interface
const int matIntBCbott=-11, matIntBCtop=-12, matIntBCleft=-13, matIntBCright=-14; //Materiais das condições de contorno (elementos de interface)
const int matPoint =-5; //Materia de um ponto
const int dirichlet = 0, neumann = 1, mixed = 2, pointtype=3; //Condições de contorno do problema ->default Dirichlet na esquerda e na direita
const REAL visco=1.,theta=-1.; //Coeficientes: viscosidade, fator simetria

const int quadmat1 = 1; //Parte inferior do quadrado
const int quadmat2 = 2; //Parte superior do quadrado
const int quadmat3 = 3; //Material de interface


void AddMultiphysicsInterfaces(TPZCompMesh &cmesh, int matfrom, int mattarget);


using namespace std;

// definition of f
void f_source(const TPZVec<REAL> & x, TPZVec<STATE>& f);

// definition of v analytic
void v_exact(const TPZVec<REAL> & x, TPZVec<STATE>& f);

// definition of sol analytic
void sol_exact(const TPZVec<REAL> & x, TPZVec<STATE>& p, TPZFMatrix<STATE>& v);


//Função principal do programa:

int main(int argc, char *argv[])
{

    TPZMaterial::gBigNumber = 1.e16;
    
#ifdef LOG4CXX
    InitializePZLOG();
#endif
    //Dados do problema:
    
    int h_level = 2;
    
    double hx=1.,hy=1.; //Dimensões em x e y do domínio
    int nelx=h_level, nely=h_level; //Número de elementos em x e y
    int nx=nelx+1 ,ny=nely+1; //Número de nos em x  y
    int pOrder = 2; //Ordem polinomial de aproximação
    //double elsizex=hx/nelx, elsizey=hy/nely; //Tamanho dos elementos
    //int nel = elsizex*elsizey; //Número de elementos a serem utilizados
    
    
    //Gerando malha geométrica:
    
    TPZGeoMesh *gmesh = CreateGMesh(nx, ny, hx, hy); //Função para criar a malha geometrica
    
#ifdef PZDEBUG
    std::ofstream fileg("MalhaGeo.txt"); //Impressão da malha geométrica (formato txt)
    std::ofstream filegvtk("MalhaGeo.vtk"); //Impressão da malha geométrica (formato vtk)
    gmesh->Print(fileg);
    TPZVTKGeoMesh::PrintGMeshVTK(gmesh, filegvtk,true);
#endif
    
    //Gerando malha computacional:
    
    TPZCompMesh *cmesh_v = CMesh_v(gmesh, pOrder); //Função para criar a malha computacional da velocidade
    TPZCompMesh *cmesh_p = CMesh_p(gmesh, pOrder); //Função para criar a malha computacional da pressão
    TPZCompMesh *cmesh_m = CMesh_m(gmesh, pOrder); //Função para criar a malha computacional multifísica

#ifdef PZDEBUG
    {
        std::ofstream filecv("MalhaC_v.txt"); //Impressão da malha computacional da velocidade (formato txt)
        std::ofstream filecp("MalhaC_p.txt"); //Impressão da malha computacional da pressão (formato txt)
        cmesh_v->Print(filecv);
        cmesh_p->Print(filecp);
    }
#endif
    
    TPZManVector<TPZCompMesh *, 2> meshvector(2);
    meshvector[0] = cmesh_v;
    meshvector[1] = cmesh_p;
    TPZBuildMultiphysicsMesh::AddElements(meshvector, cmesh_m);
    TPZBuildMultiphysicsMesh::AddConnects(meshvector, cmesh_m);
    TPZBuildMultiphysicsMesh::TransferFromMeshes(meshvector, cmesh_m);
    cmesh_m->LoadReferences();
    
    AddMultiphysicsInterfaces(*cmesh_m,matInterface,matID);
    AddMultiphysicsInterfaces(*cmesh_m,matIntBCbott,matBCbott);
    AddMultiphysicsInterfaces(*cmesh_m,matIntBCtop,matBCtop);
    AddMultiphysicsInterfaces(*cmesh_m,matIntBCleft,matBCleft);
    AddMultiphysicsInterfaces(*cmesh_m,matIntBCright,matBCright);

#ifdef PZDEBUG
    std::ofstream fileg1("MalhaGeo2.txt"); //Impressão da malha geométrica (formato txt)
    gmesh->Print(fileg1);
    
    std::ofstream filecm("MalhaC_m.txt"); //Impressão da malha computacional multifísica (formato txt)
    cmesh_m->Print(filecm);
#endif
    
    //Resolvendo o Sistema:
    
    bool optimizeBandwidth = true; //Impede a renumeração das equacoes do problema (para obter o mesmo resultado do Oden)
    TPZAnalysis an(cmesh_m, optimizeBandwidth); //Cria objeto de análise que gerenciará a analise do problema
    TPZSkylineNSymStructMatrix matskl(cmesh_m); //caso nao simetrico ***
    matskl.SetNumThreads(2);
    an.SetStructuralMatrix(matskl);
    TPZStepSolver<STATE> step;
    step.SetDirect(ELU);
    an.SetSolver(step);
    an.Assemble();//Assembla a matriz de rigidez (e o vetor de carga) global
    
    
#ifdef PZDEBUG
    //Imprimir Matriz de rigidez Global:
    {
        std::ofstream filestiff("stiffness.txt");
        an.Solver().Matrix()->Print("K1 = ",filestiff,EMathematicaInput);

        std::ofstream filerhs("rhs.txt");
        an.Rhs().Print("R = ",filerhs,EMathematicaInput);
        
        std::ofstream fileAlpha("alpha.txt");
        an.Solution().Print("Alpha = ",fileAlpha,EMathematicaInput);
    }
#endif
    
    
    an.Solve();

    //    REAL a[] = {-4.94792e-18, 4.16667e-18, -4.94792e-18,-3.19038e-33,4.94792e-18,3.14852e-32,4.94792e-18,4.16667e-18, -4.94792e-18, -4.16667e-18, 4.94792e-18,-4.16667e-18,1.54074e-49, 0.216146, -0.216146, 1.0689e-15,-0.208333,-0.0078125, 0.0078125, 0.208333};
//    int size = an.Solution().Rows();
//    for (int i = 0; i < size; i++) {
//        an.Solution()(i,0) = a[i];
//    }
//    an.LoadSolution();

    
#ifdef PZDEBUG
    //Imprimindo vetor solução:
    {
        TPZFMatrix<REAL> solucao=cmesh_m->Solution();//Pegando o vetor de solução, alphaj
        std::ofstream solout("sol.txt");
        solucao.Print("Sol",solout,EMathematicaInput);//Imprime na formatação do Mathematica
    }
#endif
    
    //Calculo do erro
    
    TPZManVector<REAL,3> Errors;
    ofstream ErroOut("Erro.txt");
    an.SetExact(sol_exact);
    an.PostProcessError(Errors,ErroOut);
    
    
    
    //Pós-processamento (paraview):

    std::string plotfile("Stokes.vtk");
    TPZStack<std::string> scalnames, vecnames;
    scalnames.Push("Pressure");
    vecnames.Push("Velocity");
    vecnames.Push("f");
    vecnames.Push("V_exact");
    
    
    int postProcessResolution = 4; //  keep low as possible
    int dim = gmesh->Dimension();
    an.DefineGraphMesh(dim,scalnames,vecnames,plotfile);
    an.PostProcess(postProcessResolution,dim);
    
    std::cout << "FINISHED!" << std::endl;
    
    return 0;
}

// definition of f
void f_source(const TPZVec<REAL> & x, TPZVec<STATE>& f){
    f.resize(2);
    
    STATE xv = x[0];
    STATE yv = x[1];
//    STATE zv = x[2];
    
    STATE f_x = 4.*((xv*xv*xv)*(6. - 12.*yv) + (xv*xv*xv*xv)*(-3. + 6.*yv) +
                    yv*(1. - 3.*yv + 2.*(yv*yv)) - 6.*xv*yv*(1. - 3.*yv + 2.*(yv*yv)) +
                    3.*(xv*xv)*(-1. + 4.*yv - 6.*(yv*yv) + 4.*(yv*yv*yv)));
    STATE f_y = -4.*(-3.*((-1. + yv)*(-1. + yv))*(yv*yv) - 3.*(xv*xv)*(1. - 6.*yv + 6.*(yv*yv)) +
                     2.*(xv*xv*xv)*(1. - 6.*yv + 6.*(yv*yv)) +
                     xv*(1. - 6.*yv + 12.*(yv*yv) - 12.*(yv*yv*yv) + 6.*(yv*yv*yv*yv)));
    
    f[0] = f_x; // x direction
    f[1] = f_y; // y direction
}

// definition of v analytic
void v_exact(const TPZVec<REAL> & x, TPZVec<STATE>& f){
    
    f.resize(2);
    
    STATE xv = x[0];
    STATE yv = x[1];
    
    STATE v_x =  -2.*((-1. + xv)*(-1. + xv))*(xv*xv)*(-1. + yv)*yv*(-1. + 2.*yv);
    STATE v_y =  +2.*(-1. + xv)*xv*(-1. + 2.*xv)*((-1. + yv)*(-1. + yv))*(yv*yv);
    
    f[0] = v_x; // x direction
    f[1] = v_y; // y direction
    
}

// Solução analítica - Artigo
void sol_exact(const TPZVec<REAL> & x, TPZVec<STATE>& sol, TPZFMatrix<STATE>& dsol){
    
    dsol.Resize(2,2);
    sol.Resize(3);
    
    STATE xv = x[0];
    STATE yv = x[1];
    
    STATE v_x =  -2.*((-1. + xv)*(-1. + xv))*(xv*xv)*(-1. + yv)*yv*(-1. + 2.*yv);
    STATE v_y =  +2.*(-1. + xv)*xv*(-1. + 2.*xv)*((-1. + yv)*(-1. + yv))*(yv*yv);
    STATE pressure= 0.;
    
    sol[0]=v_x;
    sol[1]=v_y;
    sol[2]=pressure;

    
//    // old
//    // x direction
//    dsol(0,0)= -4*(-1 + xv)*(-1 + xv)*xv*(-1 + yv)*yv*(-1 + 2*yv) - 4*(-1 + xv)*xv*xv*(-1 + yv)*yv*(-1 + 2*yv);
//    dsol(0,1)=-4*(-1 + xv)*(-1 + xv)*xv*xv*(-1 + yv)*yv - 2*(-1 + xv)*(-1 + xv)*xv*xv*(-1 + yv)*(-1 + 2*yv) - 2*(-1 + xv)*(-1 + xv)*xv*xv*yv*(-1 + 2*yv);
//    
//    // y direction
//    dsol(1,0)= 4*(-1 + xv)*xv*(-1 + yv)*(-1 + yv)*yv*yv + 2*(-1 + xv)*(-1 + 2*xv)*(-1 + yv)*(-1 + yv)*yv*yv + 2*xv*(-1 + 2*xv)*(-1 + yv)*(-1 + yv)*yv*yv;
//    dsol(1,1)= 4*(-1 + xv)*xv*(-1 + 2*xv)*(-1 + yv)*(-1 + yv)*yv + 4*(-1 + xv)*xv*(-1 + 2*xv)*(-1 + yv)*yv*yv;

    
    // x direction
    dsol(0,0)= -4*(-1 + xv)*(-1 + xv)*xv*(-1 + yv)*yv*(-1 + 2*yv) - 4*(-1 + xv)*xv*xv*(-1 + yv)*yv*(-1 + 2*yv);
    dsol(0,1)= 4*(-1 + xv)*xv*(-1 + yv)*(-1 + yv)*yv*yv + 2*(-1 + xv)*(-1 + 2*xv)*(-1 + yv)*(-1 + yv)*yv*yv + 2*xv*(-1 + 2*xv)*(-1 + yv)*(-1 + yv)*yv*yv;
    
    // y direction
    dsol(1,0)= -4*(-1 + xv)*(-1 + xv)*xv*xv*(-1 + yv)*yv - 2*(-1 + xv)*(-1 + xv)*xv*xv*(-1 + yv)*(-1 + 2*yv) - 2*(-1 + xv)*(-1 + xv)*xv*xv*yv*(-1 + 2*yv);
    dsol(1,1)= 4*(-1 + xv)*xv*(-1 + 2*xv)*(-1 + yv)*(-1 + yv)*yv + 4*(-1 + xv)*xv*(-1 + 2*xv)*(-1 + yv)*yv*yv;
    
}

TPZGeoMesh *CreateGMesh(int nx, int ny, double hx, double hy)
{
 
    int i,j;
    long id, index;
    
    
    //Criando malha geométrica, nós e elementos.
    //Inserindo nós e elementos no objeto malha:
    
    TPZGeoMesh *gmesh = new TPZGeoMesh();
    gmesh->SetDimension(2);
    
    //Vetor auxiliar para armazenar coordenadas:
    
    TPZVec <REAL> coord (3,0.);

    
    //Inicialização dos nós:
    
    for(i = 0; i < ny; i++){
        for(j = 0; j < nx; j++){
            id = i*nx + j;
            coord[0] = (j)*hx/(nx - 1);
            coord[1] = (i)*hy/(ny - 1);
            //using the same coordinate x for z
            coord[2] = 0.;
            //cout << coord << endl;
            //Get the index in the mesh nodes vector for the new node
            index = gmesh->NodeVec().AllocateNewElement();
            //Set the value of the node in the mesh nodes vector
            gmesh->NodeVec()[index] = TPZGeoNode(id,coord,*gmesh);
        }
    }
    
    //Ponto 1
    TPZVec<long> pointtopology(1);
    pointtopology[0] = 0;
    
    gmesh->CreateGeoElement(EPoint,pointtopology,matPoint,id);
    
    
    
    //Vetor auxiliar para armazenar as conecções entre elementos:
    
    TPZVec <long> connect(4,0);
    
    
    //Conectividade dos elementos:
    
    for(i = 0; i < (ny - 1); i++){
        for(j = 0; j < (nx - 1); j++){
            index = (i)*(nx - 1)+ (j);
            connect[0] = (i)*ny + (j);
            connect[1] = connect[0]+1;
            connect[2] = connect[1]+(nx);
            connect[3] = connect[0]+(nx);
            gmesh->CreateGeoElement(EQuadrilateral,connect,matID,id);
        }
    }
    
    
    //Gerando informação da vizinhança:
    
    gmesh->BuildConnectivity();
    
    {
        TPZCheckGeom check(gmesh);
        check.CheckUniqueId();
    }
    long el, numelements = gmesh->NElements();
    
    TPZManVector <long> TopolPlate(4);
    
    for (el=0; el<numelements; el++)
    {
        long totalnodes = gmesh->ElementVec()[el]->NNodes();
        TPZGeoEl *plate = gmesh->ElementVec()[el];
        for (int i=0; i<4; i++){
            TopolPlate[i] = plate->NodeIndex(i);
        }
        
        //Colocando as condicoes de contorno:
        TPZManVector <TPZGeoNode> Nodefinder(totalnodes);
        TPZManVector <REAL,3> nodecoord(3);
        
        //Na face x = 1
        TPZVec<long> ncoordzbottVec(0); long sizeOfbottVec = 0;
        TPZVec<long> ncoordztopVec(0); long sizeOftopVec = 0;
        TPZVec<long> ncoordzleftVec(0); long sizeOfleftVec = 0;
        TPZVec<long> ncoordzrightVec(0); long sizeOfrightVec = 0;
        
        for (long i = 0; i < totalnodes; i++)
        {
            Nodefinder[i] = gmesh->NodeVec()[TopolPlate[i]];
            Nodefinder[i].GetCoordinates(nodecoord);
            if (nodecoord[2] == 0. & nodecoord[1] == 0.)
            {
                sizeOfbottVec++;
                ncoordzbottVec.Resize(sizeOfbottVec);
                ncoordzbottVec[sizeOfbottVec-1] = TopolPlate[i];
            }
            if (nodecoord[2] == 0. & nodecoord[1] == hy)
            {
                sizeOftopVec++;
                ncoordztopVec.Resize(sizeOftopVec);
                ncoordztopVec[sizeOftopVec-1] = TopolPlate[i];
            }
            if (nodecoord[2] == 0. & nodecoord[0] == 0.)
            {
                sizeOfleftVec++;
                ncoordzleftVec.Resize(sizeOfleftVec);
                ncoordzleftVec[sizeOfleftVec-1] = TopolPlate[i];
            }
            if (nodecoord[2] == 0. & nodecoord[0] == hx)
            {
                sizeOfrightVec++;
                ncoordzrightVec.Resize(sizeOfrightVec);
                ncoordzrightVec[sizeOfrightVec-1] = TopolPlate[i];
            }
        }
        
        if (sizeOfbottVec == 2) {
            int sidesbott = plate->WhichSide(ncoordzbottVec);
            TPZGeoElSide platesidebott(plate, sidesbott);
            TPZGeoElBC(platesidebott,matBCbott);
            TPZGeoElBC(platesidebott,matIntBCbott);
        }
        
        if (sizeOftopVec == 2) {
            int sidestop = plate->WhichSide(ncoordztopVec);
            TPZGeoElSide platesidetop(plate, sidestop);
            TPZGeoElBC(platesidetop,matBCtop);
            TPZGeoElBC(platesidetop,matIntBCtop);
        }
        
        if (sizeOfleftVec == 2) {
            int sidesleft = plate->WhichSide(ncoordzleftVec);
            TPZGeoElSide platesideleft(plate, sidesleft);
            TPZGeoElBC(platesideleft,matBCleft);
            TPZGeoElBC(platesideleft,matIntBCleft);
        }
        
        if (sizeOfrightVec == 2) {
            int sidesright = plate->WhichSide(ncoordzrightVec);
            TPZGeoElSide platesideright(plate, sidesright);
            TPZGeoElBC(platesideright,matBCright);
            TPZGeoElBC(platesideright,matIntBCright);
        }
        
        
        ncoordzbottVec.Resize(0);
        sizeOfbottVec = 0;
        ncoordztopVec.Resize(0);
        sizeOftopVec = 0;
        ncoordzleftVec.Resize(0);
        sizeOfleftVec = 0;
        ncoordzrightVec.Resize(0);
        sizeOfrightVec = 0;
        
    }
    
    // Criando e inserindo elemento de interfação:
//    TPZVec<long> nodind3(2);
//    
//    nodind3[0]=1;
//    nodind3[1]=4;
//    
//    gmesh->CreateGeoElement(EOned, nodind3, matInterface, index); //Criando elemento de interface (GeoElement)
    
    
    //Criando interface (Geralizado):
    
    TPZVec<long> nodint(2);
    for(i = 0; i < (ny - 1); i++){
        for(j = 0; j < (nx - 1); j++){
            if(j>0&&j<(nx-1)){
                nodint[0]=j+nx*i;
                nodint[1]=j+nx*(i+1);
                gmesh->CreateGeoElement(EOned, nodint, matInterface, index); //Criando elemento de interface (GeoElement)
                
            }
            if(i>0&&j<(ny-1)){
                nodint[0]=j+ny*i;
                nodint[1]=j+ny*i+1;
                gmesh->CreateGeoElement(EOned, nodint, matInterface, index); //Criando elemento de interface (GeoElement)
            
            }
    
        }
    }
    
    
    //new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (nodind3,matInterface,*gmesh); //Criando elemento de interface (RefPattern)
    id++;
    
    gmesh->AddInterfaceMaterial(quadmat1, quadmat2, quadmat3);
    gmesh->AddInterfaceMaterial(quadmat2, quadmat1, quadmat3);
    
    TPZCheckGeom check(gmesh);
    check.CheckUniqueId();
    
    gmesh->BuildConnectivity();
    
    //Impressão da malha geométrica:
    
    ofstream bf("before.vtk");
    TPZVTKGeoMesh::PrintGMeshVTK(gmesh, bf);
    return gmesh;

    
    
}

TPZCompEl *CreateInterfaceEl(TPZGeoEl *gel,TPZCompMesh &mesh,long &index) {
    if(!gel->Reference() && gel->NumInterfaces() == 0)
        return new TPZInterfaceElement(mesh,gel,index);
    
    return NULL;
}


TPZCompMesh *CMesh_v(TPZGeoMesh *gmesh, int pOrder)
{
    
    //Criando malha computacional:
    
    TPZCompMesh * cmesh = new TPZCompMesh(gmesh);
    cmesh->SetDefaultOrder(pOrder);//Insere ordem polimonial de aproximação
    cmesh->SetDimModel(dim);//Insere dimensão do modelo
  
    
    //Definição do espaço de aprximação:
    
    cmesh->SetAllCreateFunctionsContinuous(); //Criando funções H1:
    
    //cmesh->SetAllCreateFunctionsHDiv(); //Criando funções HDIV:
    
    
    //Criando elementos com graus de liberdade differentes para cada elemento (descontínuo):
    
    //cmesh->ApproxSpace().CreateDisconnectedElements(true); //Criando elementos desconectados (descontínuo)
    
    
    //Criando material:
    //Criando material cujo nSTATE = 2 ou seja linear
    
    TPZMat2dLin *material = new TPZMat2dLin(matID); //Criando material que implementa a formulação fraca do problema modelo
    
    cmesh->InsertMaterialObject(material); //Insere material na malha
    
    //Dimensões do material (para H1 e descontinuo):
    TPZFMatrix<STATE> xkin(2,2,0.), xcin(2,2,0.), xfin(2,2,0.);
    material->SetMaterial(xkin, xcin, xfin);
    
    //Dimensões do material (para HDiv):
    //TPZFMatrix<STATE> xkin(1,1,0.), xcin(1,1,0.), xfin(1,1,0.);
    //material->SetMaterial(xkin, xcin, xfin);

    
    //Condições de contorno:

    TPZFMatrix<REAL> val1(1,1,0.), val2(2,1,0.);
    
    TPZMaterial * BCond0 = material->CreateBC(material, matBCbott, dirichlet, val1, val2); //Cria material que implementa a condição de contorno inferior
    cmesh->InsertMaterialObject(BCond0); //Insere material na malha
    
    TPZMaterial * BCond1 = material->CreateBC(material, matBCtop, dirichlet, val1, val2); //Cria material que implementa a condicao de contorno superior
    cmesh->InsertMaterialObject(BCond1); //Insere material na malha
    
    TPZMaterial * BCond2 = material->CreateBC(material, matBCleft, dirichlet, val1, val2); //Cria material que implementa a condicao de contorno esquerda
    cmesh->InsertMaterialObject(BCond2); //Insere material na malha
    
    TPZMaterial * BCond3 = material->CreateBC(material, matBCright, dirichlet, val1, val2); //Cria material que implementa a condicao de contorno direita
    cmesh->InsertMaterialObject(BCond3); //Insere material na malha
    
    
    //Criando elementos computacionais que gerenciarão o espaco de aproximacao da malha:
    
    int ncel = cmesh->NElements();
    for(int i =0; i<ncel; i++){
        TPZCompEl * compEl = cmesh->ElementVec()[i];
        if(!compEl) continue;
        TPZInterfaceElement * facel = dynamic_cast<TPZInterfaceElement *>(compEl);
        if(facel)DebugStop();
        
    }

    
    cmesh->AutoBuild();
    cmesh->AdjustBoundaryElements();
    cmesh->CleanUpUnconnectedNodes();
    

    
    return cmesh;
    
}

TPZCompMesh *CMesh_p(TPZGeoMesh *gmesh, int pOrder)
{
    
    // @omar::
    
    pOrder--; // Space restriction apapapa
    
    //Criando malha computacional:
    
    TPZCompMesh * cmesh = new TPZCompMesh(gmesh);
    cmesh->SetDefaultOrder(pOrder); //Insere ordem polimonial de aproximação
    cmesh->SetDimModel(dim); //Insere dimensão do modelo

    // @omar::
   // cmesh->SetAllCreateFunctionsDiscontinuous();
    
    cmesh->SetAllCreateFunctionsContinuous(); //Criando funções H1
    cmesh->ApproxSpace().CreateDisconnectedElements(true);
    
    
    //Criando material:
    //Criando material cujo nSTATE = 2 ou seja linear
    
    TPZMat2dLin *material = new TPZMat2dLin(matID);//criando material que implementa a formulacao fraca do problema modelo
    
    cmesh->InsertMaterialObject(material); //Insere material na malha
    
    //Dimensões do material (para H1 e descontínuo):
    TPZFMatrix<STATE> xkin(1,1,0.), xcin(1,1,0.), xfin(1,1,0.);
    material->SetMaterial(xkin, xcin, xfin);
    
    //Condições de contorno:
    
    TPZFMatrix<REAL> val1(1,1,0.), val2(1,1,0.);
   
    
    TPZMaterial * BCPoint = material->CreateBC(material, matPoint, pointtype, val1, val2); //Cria material que implementa um ponto para a pressao
    cmesh->InsertMaterialObject(BCPoint); //Insere material na malha


    //Criando elementos computacionais que gerenciarão o espaco de aproximação da malha

    int ncel = cmesh->NElements();
    for(int i =0; i<ncel; i++){
        TPZCompEl * compEl = cmesh->ElementVec()[i];
        if(!compEl) continue;
        TPZInterfaceElement * facel = dynamic_cast<TPZInterfaceElement *>(compEl);
        if(facel)DebugStop();
        
    }
    std::set<int> materialids;
    materialids.insert(matID);
    cmesh->AutoBuild(materialids);
    cmesh->LoadReferences();
    cmesh->ApproxSpace().CreateDisconnectedElements(false);
    cmesh->AutoBuild();
    

    // @omar::
    int ncon = cmesh->NConnects();
    for(int i=0; i<ncon; i++)
    {
        TPZConnect &newnod = cmesh->ConnectVec()[i];
        newnod.SetLagrangeMultiplier(1);
    }
    
//    cmesh->AdjustBoundaryElements();
//    cmesh->CleanUpUnconnectedNodes();
    
    return cmesh;
    
}

TPZCompMesh *CMesh_m(TPZGeoMesh *gmesh, int pOrder)
{

    //Criando malha computacional:
    
    TPZCompMesh * cmesh = new TPZCompMesh(gmesh);
    cmesh->SetDefaultOrder(pOrder); //Insere ordem polimonial de aproximação
    cmesh->SetDimModel(dim); //Insere dimensão do modelo
    cmesh->SetAllCreateFunctionsMultiphysicElem();
    
    
    // Criando material:
    
    TPZStokesMaterial *material = new TPZStokesMaterial(matID,dim,visco,theta);//criando material que implementa a formulacao fraca do problema modelo
    // Inserindo material na malha
    TPZAutoPointer<TPZFunction<STATE> > fp = new TPZDummyFunction<STATE> (f_source);
    TPZAutoPointer<TPZFunction<STATE> > vp = new TPZDummyFunction<STATE> (v_exact);
    //TPZAutoPointer<TPZFunction<STATE> > solp = new TPZDummyFunction<STATE> (sol_exact);
    material->SetForcingFunction(fp);
    material->SetForcingFunctionExact(vp);
    //material->SetForcingFunctionExact(solp);
    cmesh->InsertMaterialObject(material);
    
    
    //Condições de contorno:
    
    TPZFMatrix<REAL> val1(1,1,0.), val2(2,1,0.);
    
    val2(0,0) = 0.0; // vx -> 0
    val2(1,0) = 0.0; // vy -> 0
    
    TPZMaterial * BCond0 = material->CreateBC(material, matBCbott, dirichlet, val1, val2); //Cria material que implementa a condição de contorno inferior
    cmesh->InsertMaterialObject(BCond0); //Insere material na malha
    
    TPZMaterial * BCond1 = material->CreateBC(material, matBCtop, dirichlet, val1, val2); //Cria material que implementa a condicao de contorno superior
    cmesh->InsertMaterialObject(BCond1); //Insere material na malha
    
    TPZMaterial * BCond2 = material->CreateBC(material, matBCleft, dirichlet, val1, val2); //Cria material que implementa a condicao de contorno esquerda
    cmesh->InsertMaterialObject(BCond2); //Insere material na malha
    
    TPZMaterial * BCond3 = material->CreateBC(material, matBCright, dirichlet, val1, val2); //Cria material que implementa a condicao de contorno direita
    cmesh->InsertMaterialObject(BCond3); //Insere material na malha
    
    //Ponto
    
    TPZFMatrix<REAL> val3(1,1,0.), val4(1,1,0.);
    val4(0,0)=0.0;
    
    TPZMaterial * BCPoint = material->CreateBC(material, matPoint, pointtype, val3, val4); //Cria material que implementa um ponto para a pressão
    cmesh->InsertMaterialObject(BCPoint); //Insere material na malha
    
    

    
#ifdef PZDEBUG
    int ncel = cmesh->NElements();
    for(int i =0; i<ncel; i++){
        TPZCompEl * compEl = cmesh->ElementVec()[i];
        if(!compEl) continue;
        TPZInterfaceElement * facel = dynamic_cast<TPZInterfaceElement *>(compEl);
        if(facel)DebugStop();
        
    }
#endif
    
    
    
    //Criando elementos computacionais que gerenciarão o espaco de aproximação da malha:
    
    cmesh->AutoBuild();
    cmesh->AdjustBoundaryElements();
    cmesh->CleanUpUnconnectedNodes();
    
    return cmesh;
    
}


void AddMultiphysicsInterfaces(TPZCompMesh &cmesh, int matfrom, int mattarget)
{
    TPZGeoMesh *gmesh = cmesh.Reference();
    long nel = gmesh->NElements();
    for (long el = 0; el<nel; el++) {
        TPZGeoEl *gel = gmesh->Element(el);
        if (gel->MaterialId() != matfrom) {
            continue;
        }
        
        int nsides= gel->NSides();
        
        TPZGeoElSide gelside(gel,nsides-1);
        TPZStack<TPZCompElSide> celstack;
        gelside.EqualLevelCompElementList(celstack, 0, 0);
        if (celstack.size() != 2) {
            DebugStop();
        }
        gel->SetMaterialId(mattarget);
        long index;
        new TPZMultiphysicsInterfaceElement(cmesh,gel,index,celstack[1],celstack[0]);
    }  
    
}

//Função Erro (Não utilizada)
void Error(TPZCompMesh *cmesh, std::ostream &out, int p, int ndiv)
{
    DebugStop();
    long nel = cmesh->NElements();
    //int dim = cmesh->Dimension();
    TPZManVector<STATE,10> globalerrors(10,0.);
    for (long el=0; el<nel; el++) {
        TPZCompEl *cel = cmesh->ElementVec()[el];
        TPZManVector<STATE,10> elerror(10,0.);
        cel->EvaluateError(sol_exact, elerror, NULL);
        int nerr = elerror.size();
        for (int i=0; i<nerr; i++) {
            globalerrors[i] += elerror[i]*elerror[i];
        }
        
    }
    out << "Errors associated with HDiv space - ordem polinomial = " << p << "- divisoes = " << ndiv << endl;
    out << "L2 Norm for flux - L2 Norm for divergence - Hdiv Norm for flux " << endl;
    out <<  setw(16) << sqrt(globalerrors[1]) << setw(25)  << sqrt(globalerrors[2]) << setw(21)  << sqrt(globalerrors[3]) << endl;
    //
    //    out << "L2 Norm for flux = "    << sqrt(globalerrors[1]) << endl;
    //    out << "L2 Norm for divergence = "    << sqrt(globalerrors[2])  <<endl;
    //    out << "Hdiv Norm for flux = "    << sqrt(globalerrors[3])  <<endl;
    //
}


