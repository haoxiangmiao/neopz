$Id: euler.cpp,v 1.6 2003-10-17 15:49:49 cedric Exp $
//#include "pzmetis.h"
//#include "pztrnsform.h"
#include "TPZGeoCube.h"
#include "pzshapecube.h"
#include "TPZRefCube.h"
#include "pzshapelinear.h"
#include "TPZGeoLinear.h"
#include "TPZRefLinear.h"
#include "pzrefquad.h"
#include "pzshapequad.h"
#include "pzgeoquad.h"
#include "pzshapetriang.h"
#include "pzreftriangle.h"
#include "pzgeotriangle.h"
#include "pzshapeprism.h"
#include "pzrefprism.h"
#include "pzgeoprism.h"
#include "pzshapetetra.h"
#include "pzreftetrahedra.h"
#include "pzgeotetrahedra.h"
#include "pzshapepiram.h"
#include "pzrefpyram.h"
#include "pzgeopyramid.h"
#include "pzrefpoint.h"
#include "pzgeopoint.h"

#include "TPZGeoElement.h"
#include "pzgmesh.h"
#include "pzcmesh.h"
#include "pzfmatrix.h"
#include "pzelgc3d.h"
#include "pzbndcond.h"
#include "pztempmat.h"
#include "pzcompel.h"
#include "pzanalysis.h"
#include "pzfstrmatrix.h"
#include "pzskylstrmatrix.h"
#include "pzskylmat.h"
#include "pzstepsolver.h"
#include "pzgeoel.h"
#include "pzgnode.h"
#include "pzstack.h"
#include "pzvec.h"
#include "pzsolve.h"
#include "pzelgpoint.h"
#include "pzelg1d.h"
#include "pzelgq2d.h"
#include "pzelgt2d.h"
#include "pzelct2d.h"
#include "pzelcc3d.h"
#include "pzelgt3d.h"
#include "pzelct3d.h"
#include "pzelgpi3d.h"
#include "pzelcpi3d.h"
#include "pzelgpr3d.h"
#include "pzelcpr3d.h"
#include "pzelmat.h"
#include "pzelasmat.h"
#include "pzmattest.h"
#include "pzmat1dlin.h"
#include "pzmat2dlin.h"
#include "pzpoisson3d.h"
#include "pzmaterial.h"
#include "TPZConservationLaw.h"
#include "TPZConsLawTest.h"
#include "TPZEulerConsLaw.h"
#include "TPZDiffusionConsLaw.h"
#include "TPZCompElDisc.h"
#include "TPZShapeDisc.h"
#include "TPZInterfaceEl.h"
#include "TPZIterativeAnalysis.h"
#include "TPZExtendGridDimension.h"
#include "TPZFlowCMesh.h"
#include "TPZIterativeAnalysis.h"
#include "TPZFlowCMesh.h"
#include "pzreal.h"
//#include "TPZJacobMat.h"
//#include "TPZRefPattern.h"
//#include "TPZJacobStrMatrix.h"
#include "pzdxmesh.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <ostream>
#include <string.h>
using namespace std;
//                        c = sqrt(gama*p/ro)  velocidade do som

static REAL p1=0.7,p2=0.902026,p3=1.80405,p4=2.96598,p5=3.2,p6=4.12791,p7=0.22;

static double novequads[15][3] = { {0.,0.,0.},{p1+p1/4.,0.,0.},{p3,0.,0.},{p5-p7/1.5,0.,0.},{p6,0.,0.},
				  {0.,p7,0.},{p1,p7,0.},{p2,.5,0.},{p3,.75,0.},{p4,.5,0.},
				  {p5,p7,0.},{p6,p7,0.},{0.,1.,0.},{p3,1.,0.},{p6,1.,0.}};

static double novecubos[30][3] = { {0.,0.,0.},{p1+p1/4.,0.,0.},{p3,0.,0.},{p5-p7/1.5,0.,0.},{p6,0.,0.},
				  {0.,p7,0.},{p1,p7,0.},{p2,.5,0.},{p3,.75,0.},{p4,.5,0.},
				  {p5,p7,0.},{p6,p7,0.},{0.,1.,0.},{p3,1.,0.},{p6,1.,0.},
				   {0.,0.,0.3},{p1+p1/4.,0.,0.3},{p3,0.,0.3},{p5-p7/1.5,0.,0.3},{p6,0.,0.3},
				  {0.,p7,0.3},{p1,p7,0.3},{p2,.5,0.3},{p3,.75,0.3},{p4,.5,0.3},
				   {p5,p7,0.3},{p6,p7,0.3},{0.,1.,0.3},{p3,1.,0.3},{p6,1.,0.3} };

static double quadrilatero[4][3] = { {0.,0.,0.},{4.1,0.,0.},{4.1,1.,0.},{0.,1.,0.} };

static double quadrilatero2[5][3] = { {0.,0.,0.},{1.80405,0.,0.},{4.12791,0.,0.},{0.,1.,0.},{4.12791,1.,0.} };

// static double quadrilatero3[8][3] = { {0.,0.,0.},{0.6,0.,0.},{0.,0.2,0.},{0.6,0.2,0.},
// 				      {3.,0.2,0.},{0.,1.,0.},{0.6,1.,0.},{3.,1.,0.} };

static double tresprismas[10][3] = { {0.,0.,0.},{1.80405,0.,0.},{4.12791,0.,0.},{0.,1.,0.},{4.12791,1.,0.},
				     {0.,0.,1.},{1.80405,0.,1.},{4.12791,0.,1.},{0.,1.,1.},{4.12791,1.,1.} };

static double hexaedro1[8][3] = { {0.,0.,0.},{1.,0.,0.},{1.,1.,0.},{0.,1.,0.},
				 {0.,0.,1.},{1.,0.,1.},{1.,1.,1.},{0.,1.,1.} };

static double hexaedro[8][3] = { {0.,0.,0.},{4.1,0.,0.},{4.1,1.,0.},{0.,1.,0.},
				 {0.,0.,1.},{4.1,0.,1.},{4.1,1.,1.},{0.,1.,1.} };

//static TPZRefPattern hexaedro9("hexaedro9subs.in");
//static TPZRefPattern hexaedro8("hexaedro8.in");//hexaedro mestre 8 sub-elementos
//static TPZRefPattern hexaedro2("hexaedro2.in");
//static TPZRefPattern quad4("quadrilatero4.in");//quadrilatero mestre 4 sub-elementos*/
//static TPZRefPattern linha2("linha2.in");//aresta mestre 2 sub-elementos*/
void AgrupaList(TPZVec<int> &accumlist,int nivel,int &numaggl);
void SetDeltaTime(TPZMaterial *mat,int nstate);
void CriacaoDeNos(int nnodes,double lista[20][3]);
TPZMaterial *NoveCubos(int grau);
TPZMaterial *NoveQuadrilateros(int grau);
TPZMaterial *ProblemaQ2D1El(int grau);
TPZMaterial *ProblemaT2D(int grau);
TPZMaterial *TresTriangulos(int grau);
TPZMaterial *Triangulo(int grau);
TPZMaterial *Quadrilatero(int grau);
TPZMaterial *Hexaedro(int grau);
TPZMaterial *TresPrismas(int grau);
TPZMaterial *FluxConst3D(int grau);
TPZMaterial *FluxConst2D(int grau);
void ContagemDeElementos(TPZMaterial *mat);
void FileNB(TPZGeoMesh &gmesh,ostream &out,int var);
void Function(TPZVec<REAL> &x,TPZVec<REAL> &result);
void PostProcess(TPZGeoMesh &gmesh,ostream &out);
void Divisao (TPZCompMesh *cmesh);
void NivelDivide(TPZCompMesh *cmesh);
void SequenceDivide2();
void SequenceDivide(int fat[100],int numbel);
void TestShapesDescontinous();
static clock_t start,end;//,begin,ttot=0;
void CoutTime(clock_t &start);

static TPZGeoMesh *gmesh = new TPZGeoMesh;
static TPZCompMesh *cmesh = new TPZFlowCompMesh(gmesh);
//static TPZVec<REAL> x0(3,0.);
static int grau = 0;
static int nivel = 0,tipo;
static int problem=0;
//static REAL pi = 2.0*asin(1.0);
static REAL CFL=-1.0;
static REAL gama = 1.4;


//#define NOTDEBUG
#define CEDRICDEBUG

int main() {

  ofstream outgm("mesh.out");

  cout << "\ntipo\n"
       << "\t[0: TresTriangulos]\n"
       << "\t[1: TresPrismas]\n"
       << "\t[2: FluxConst3D]\n"
       << "\t[3: FluxConst2D]\n"
       << "\t[4: FluxConst2D (outra CC)]\n"
       << "\t[5: NoveQuadrilateros]\n"
       << "\t[6: NoveCubos]\n"
       << "\t\t\t";

  cin >> tipo;
  //tipo = 5;
  problem = tipo;
  cout << "\nGrau do espaco de interpolacao -> 0,1,2,3,... ";
  //cin >> grau;
  grau = 0;
  TPZCompElDisc::gDegree = grau;
  //TPZMaterial *mat;
  TPZConservationLaw *mat;

  if(tipo==0) mat = dynamic_cast<TPZConservationLaw *>(TresTriangulos(grau));
  if(tipo==1) mat = dynamic_cast<TPZConservationLaw *>(TresPrismas(grau));
  if(tipo==2) mat = dynamic_cast<TPZConservationLaw *>(FluxConst3D(grau));
  if(tipo==3 || tipo == 4) mat = dynamic_cast<TPZConservationLaw *>(FluxConst2D(grau));
  if(tipo==5){
    mat = dynamic_cast<TPZConservationLaw *>(NoveQuadrilateros(grau));
    problem = 0;
  }
  if(tipo==6) mat = dynamic_cast<TPZConservationLaw *>(NoveCubos(grau));

  if(1){
    cout << "\ndescontinuo.c::main verificando a consistencia da malha de interfaces\t";
    if(TPZInterfaceElement::main(*cmesh)){
      cout << "->\tOK!";
    } else {
      cout << "->\tPROBLEMAS COM INTERFACES\n\n";
      //return 0;
    }
    //ContagemDeElementos();
  }

  if(0){
    cout << "\nmain::Imprime malhas\n";
    gmesh->Print(outgm);
    cmesh->Print(outgm);
    outgm.flush();
  }  

  int numiter,marcha;
  cout << "\nNumero de iteracoes requerida ? : ";
  cin >> numiter;
  //numiter = 100;
  cout << "main:: Parametro marcha : \n";
  cin >> marcha;
  //marcha = 10;
  if(1){
    cout << "main:: entre CFL (si nulo sera calculado) -> ";
    cin >> CFL;
    //CFL = 0.0;
    TPZDiffusionConsLaw::fCFL = CFL;
    cout << "main:: entre delta (si nulo sera calculado) -> ";
    REAL delta;
    cin >> delta;
    //delta = 0.0;
    TPZDiffusionConsLaw::fDelta= delta;
  }

  if(1){
    start = clock();
    if(1) NivelDivide(cmesh);
    if(0) SequenceDivide2();
    CoutTime(start);
    int nstate=-1;
    if(tipo == 0) nstate = 4;
    if(tipo == 1) nstate = 5;
    if(tipo == 2) nstate = 5;
    if(tipo == 3) nstate = 4;
    if(tipo == 4) nstate = 4;
    if(tipo == 5) nstate = 4;
    if(tipo == 6) nstate = 5;
    SetDeltaTime(mat,nstate);
    if(0){
      gmesh->Print(outgm);
      cmesh->Print(outgm);
      outgm.flush();
    }
  }

  if(1){
    cout << "\nmain::Ajuste no contorno e imprime malhas\n";
    cmesh->AdjustBoundaryElements();
    if(1){
      gmesh->Print(outgm);
      cmesh->Print(outgm);
      outgm.flush();
    }
  }
  if(0){
  /////////////////////////////////////////////////////////////////////////////////////////////////
  TPZVec<int> accumlist;
  int nivel,numaggl;
  cout << "main::Entre nivel da nova malha : ";
  cin >> nivel;
  AgrupaList(accumlist,nivel,numaggl);
  int index;
  TPZCompElDisc disc(*cmesh,index);
  TPZCompMesh *cmesh2 = disc.CreateAgglomerateMesh(cmesh,accumlist,numaggl);
  if(cmesh2) cmesh2->Print(outgm);
  cout << "\n\nmain:: FIM DO PROGRAMA\n\n";
  return 0;
  /////////////////////////////////////////////////////////////////////////////////////////////////
    }
  //com matriz n�o sim�trica e ELU 2D e 3D convergen
  if(1){
    TPZIterativeAnalysis an(cmesh,outgm);
    if(1){//Analysis
      cout << "\nmain::Resolve o sistema\n";
      //TPZSkylineStructMatrix stiff(cmesh);//para formula��o LS : matriz sim�trica
      TPZFStructMatrix stiff(cmesh);//n�o sim�trica
      an.SetStructuralMatrix(stiff);
      an.Solution().Zero();
      TPZStepSolver solver;// ECholesky -> sim�trica e positiva definida
      solver.SetDirect(ELU);//    ELU -> matriz n�o singular
      //solver.SetDirect(ELDLt);//  ELDLt -> s� sim�trica
      an.SetSolver(solver);
      if(1){
	REAL tol;
	tol = 1.0e15;// = norma da solu��o inicial + epsilon
	cout << "\nTolerancia ? : " << tol << "\n";
	//cin >> tol;
	//an.SetExact(Solution);
	int resolution=0;
	cout << "main:: Parametro resolution : \n";
	//cin >> resolution;
	resolution = 0;
	cout << resolution << "\n";
	an.IterativeProcess(outgm,tol,numiter,mat,marcha,resolution);
	if(0) PostProcess(*gmesh,outgm);
	if(0){//MALHAS N�O CONFORMES
	  Divisao(cmesh);
	  cmesh->AdjustBoundaryElements();
	  if(1){
	    cout << "\nmain::Imprime malhas depois de divide manual\n";
	    gmesh->Print(outgm);
	    cmesh->Print(outgm);
	    outgm.flush();
	  }
	  TPZVec<char *> scalar(1),vector(0);
	  scalar[0] = "pressure";
	  int dim = mat->Dimension();
	  TPZDXGraphMesh graph(cmesh,dim,mat,scalar,vector);
	  ofstream *dx = new ofstream("ConsLawFinal.dx");
	  cout << "\nmain::Out file : ConsLawFinal.dx\n";
	  graph.SetOutFile(*dx);
	  graph.SetResolution(resolution);
	  graph.DrawMesh(dim);
	  an.LoadSolution();
	  an.SetBlockNumber();
	  an.Solution().Zero();
	  an.Run();
	  REAL time = mat->TimeStep();
	  graph.DrawSolution(0,time);
	  dx->flush();
	  //dx->close();
	}
      }
    }
    ContagemDeElementos(mat);
  }//if(0/1)

  outgm.close();
  if(cmesh) delete cmesh;
  if(gmesh) delete gmesh;
  //AvisoAudioVisual();
  return 0;
}

void CriacaoDeNos(int nnodes,double lista[20][3]){

   gmesh->NodeVec().Resize(nnodes);   
   TPZVec<REAL> coord(3);
   int i;
   for(i=0;i<nnodes;i++){
     coord[0] = lista[i][0];
     coord[1] = lista[i][1];
     coord[2] = lista[i][2];
     gmesh->NodeVec()[i].Initialize(coord,*gmesh);
  }
}

void SetDeltaTime(TPZMaterial *mat,int nstate){

  TPZVec<REAL> x(3,0.0),sol;
  int i;
  x[0] = 0.5;
  x[1] = 0.5;
  Function(x,sol);
  REAL prod = 0.0,maxveloc;
  for(i=1;i<nstate-1;i++) prod += sol[i]*sol[i];//(u�+v�+w�)*ro�
  REAL dens2 = sol[0]*sol[0];
  maxveloc = sqrt(prod/dens2);//velocidade
  TPZEulerConsLaw *law = dynamic_cast<TPZEulerConsLaw *>(mat);//TPZEulerConsLaw *law = (TPZEulerConsLaw *)(mat);
  REAL press = law->Pressure(sol);
  if(press < 0) cout << "main::SetDeltaTime pressao negativa, toma valor absoluto para calculo do som\n";
  REAL sound = sqrt(law->Gamma()*press/sol[0]);
  maxveloc += sound;
  //REAL deltax = cmesh->DeltaX();
  REAL deltax = cmesh->LesserEdgeOfMesh();
  //REAL deltax = cmesh->MaximumRadiusOfMesh();
  REAL deltaT = CFL*deltax/maxveloc;
  cout << "main::SetDeltaTime : " << deltaT << endl;
  law->SetTimeStep(deltaT);

}

void Divisao (TPZCompMesh *cmesh){

  TPZVec<int> csub(0);
  int n1=1;
  while(n1) {
    cout << "\nId do elemento geometrico a dividir ? : ";
    cin >> n1;
    if(n1 < 0) break;
    int nelc = cmesh->ElementVec().NElements();
    int el=0;
    TPZCompEl *cpel=0;
    for(el=0;el<nelc;el++) {
      cpel = cmesh->ElementVec()[el];
      if(cpel && cpel->Reference()->Id() == n1) break;
    }
    if(cpel && el < nelc && cpel->Type() == 16){
      PZError << "main::Divisao elemento interface (nao foi dividido!)\n\n";
      cout << "Elementos divissiveis:\n";
      for(el=0;el<nelc;el++) {
	cpel = cmesh->ElementVec()[el];
	if(cpel && cpel->Type() != 16){
	  TPZGeoEl *gel = cpel->Reference();
	  if(gel) cout << gel->Id() << ",";
	}
      }
    } else {
      if(!el || el < nelc) cmesh->Divide(el,csub,0);
      else {
	cout << "main::Divisao elemento sem referencia\n";
	ContagemDeElementos(0);
      }
      n1 = 1;
    }
  }
}

void TestShapeDescontinous(){

  int nel = cmesh->ElementVec().NElements(),i;
  REAL C[3];
  if(0){
    for(i=0;i<nel;i++){
      //      TPZCompEl *comp = (cmesh->ElementVec()[i]);
      //      TPZCompElDisc *disc = dynamic_cast<TPZCompElDisc *> (comp);
      //      C[i] = disc->Constant();
    }
  }
  C[0] = 1.;
  C[1] = 1.;
  C[2] = 1.;
  TPZVec<REAL> X0(3,0.5),X(3,0.2);
  X[1] = 0.3;
  X[2] = 0.4;
  int degree = 3;
  TPZFMatrix phi,dphi;
  //  int const N=1;
  TPZShapeDisc::Shape1D(C[0],X0,X,degree,phi,dphi);
  phi.Print("Uni-dimensional",cout);
  dphi.Print("Uni-dimensional",cout);
  phi.Resize(0,0);
  dphi.Resize(0,0);
  TPZShapeDisc::Shape2D(C[1],X0,X,degree,phi,dphi);
  phi.Print("Bi-dimensional",cout);
  dphi.Print("Bi-dimensional",cout);
  phi.Resize(0,0);
  dphi.Resize(0,0);
  TPZShapeDisc::Shape3D(C[2],X0,X,degree,phi,dphi);
  phi.Print("Tri-dimensional",cout);
  dphi.Print("Tri-dimensional",cout);
}

void ContagemDeElementos(TPZMaterial *mat){

  int poin=0,line=0,tria=0,quad=0,tetr=0,pira=0,pris=0,hexa=0,disc=0,inte=0;
  int nelem = cmesh->ElementVec().NElements();
  int k,totel=0,bcel=0,niv = 0,nivmax=0;
  for(k=0;k<nelem;k++){
    TPZCompEl *comp = cmesh->ElementVec()[k];
    if(!comp) continue;
    totel++;
    if(comp->Reference()->MaterialId() < 0) bcel++;
    niv = comp->Reference()->Level();
    if(nivmax < niv) nivmax = niv;
/*     if(comp->Type() == 00) poin++; */
/*     if(comp->Type() == 01) line++; */
/*     if(comp->Type() == 02) tria++; */
/*     if(comp->Type() == 03) quad++; */
/*     if(comp->Type() == 04) tetr++; */
/*     if(comp->Type() == 05) pira++; */
/*     if(comp->Type() == 06) pris++; */
/*     if(comp->Type() == 07) hexa++; */
    if(comp->Type() == 15) disc++;
    if(comp->Type() == 16) inte++;
  }
  nelem = gmesh->ElementVec().NElements();
  int total=0,nivmax2=0;
  for(k=0;k<nelem;k++){
    TPZGeoEl *geo = gmesh->ElementVec()[k];
    if(!geo) continue;
    total++;
    niv = geo->Level();
    if(nivmax2 < niv) nivmax2 = niv;
    if(geo->Reference()){
      int nsides = geo->NSides();
      if(nsides ==  1) poin++;
      if(nsides ==  3) line++;
      if(nsides ==  7) tria++;
      if(nsides ==  9) quad++;
      if(nsides == 15) tetr++;
      if(nsides == 19) pira++;
      if(nsides == 21) pris++;
      if(nsides == 27) hexa++;
    }
  }
  cout << "\nTotal de elementos computacionais  : " << totel;
  cout << "\nTotal de elementos de dominio      : " << abs(totel-bcel);
  cout << "\nTotal de elementos de contorno     : " << bcel;
  cout << "\nTotal de elementos ponto           : " << poin;
  cout << "\nTotal de elementos linha           : " << line;
  cout << "\nTotal de elementos triangulo       : " << tria;
  cout << "\nTotal de elementos quadrilatero    : " << quad;
  cout << "\nTotal de elementos tetraedro       : " << tetr;
  cout << "\nTotal de elementos piramide        : " << pira;
  cout << "\nTotal de elementos prisma          : " << pris;
  cout << "\nTotal de elementos hexaedro        : " << hexa;
  cout << "\nTotal do tipo discontinuo          : " << disc;
  cout << "\nTotal de tipo interface            : " << inte;
  cout << "\nTotal de nos                       : " << gmesh->NodeVec().NElements();
  cout << "\nTotal de elementos geometricos     : " << total;
  cout << "\nTamanho do vetor de connects       : " << cmesh->NConnects();
  cout << "\nTamanho do vetor el. comput.       : " << cmesh->NElements();
  cout << "\nGrau do espa�o de interpola��o     : " << grau;
  cout << "\nNivel maximo comput. atingido      : " << nivmax;
  cout << "\nNivel maximo geomet. atingido      : " << nivmax << endl << endl;
  if(mat){
    cout << "\nPropriedades materiais             : ";
    mat->Print();
  }
  cout << "\nDeltaX                             : " <<  cmesh->DeltaX() << endl;
  cout << "\nLesserEdgeOfEl                     : " <<  cmesh->LesserEdgeOfMesh() << endl;
  cout << "\nMaximumRadiusOfEl                  : " <<  cmesh->MaximumRadiusOfMesh() << endl;
}

int Nivel(TPZGeoEl *gel);
void NivelDivide(TPZCompMesh *cmesh){

  TPZVec<int> csub(0);
  //int nivel;
  cout << "\nmain::Divisao todos os elementos da malha serao divididos!\n";
  //cout << "\nmain::Divisao Nivel da malha final ? : ";
  //cin >> nivel;
  cout << "\nNivel da malha a ser atingido = " << nivel << endl;
  int nelc = cmesh->ElementVec().NElements();
  int el,actual;
  TPZCompEl *cpel;
  TPZGeoEl *gel;
  el = -1;
  while(++el<nelc) {
    cpel = cmesh->ElementVec()[el];
    if(!cpel) continue;
    if(cpel->Type() == 16) continue;
    if(cpel->Material()->Id() < 0) continue;
    gel = cpel->Reference();
    actual = Nivel(gel);
    if(actual < nivel){
      cmesh->Divide(el,csub,0);
      nelc = cmesh->ElementVec().NElements();
      el = -1;
      continue;
    }
  }
}

int Nivel(TPZGeoEl *gel){
  //retorna o n�vel do elemento gel
  if(!gel) return -1;
  TPZGeoEl *fat = gel->Father();
  if(!fat) return 0;
  int niv = 0;
  while(fat){
    fat = fat->Father();
    niv++;
  }
  return niv;
}

static REAL point[5][3] = {{-.8,0.,0.},{-.4,.0,.0},{.0,.0,.0},{.4,.0,.0},{.8,0.,0.}};//linha
static REAL quad[5][3] = {{-.5,-.5,0.},{.5,-.5,.0},{.5,.5,.0},{-.5,.5,.0},{.0,.0,0.}};//quadrilatero
static REAL hexa[9][3] = { {-0.8,-0.8,-0.8},{0.8,-0.8,-0.8},{0.8,0.8,-0.8},{-0.8,0.8,-0.8},
			   {-0.8,-0.8,00.8},{0.8,-0.8,00.8},{0.8,0.8,00.8},{-0.8,0.8,00.8},{0.,0.,0.} };//hexaedro
void PostProcess(TPZGeoMesh &gmesh,ostream &out) {

  int nel = gmesh.Reference()->ElementVec().NElements();
  if(nel > 1000){
    cout << "main::PostProcess mas de 10000 elementos -> processa 2000\n";
  }
  int idmax = 0,dim,chega=1,finish=-1;
  for(int iel=0;iel<nel;iel++){//procurando o id mais alto da lista
    if(++finish >= 2000) return;
    TPZCompEl *cel = gmesh.Reference()->ElementVec()[iel];
    if(!cel) continue;
    TPZGeoEl *el = gmesh.ElementVec()[iel];
    if(chega && cel->Type()==15) {dim = el->Dimension(); chega = 0;}
    int id = el->Id();
    if(id > idmax) idmax = id;
  }
  for(int iel=0;iel<nel;iel++) {
    if(!gmesh.Reference()->ElementVec()[iel]) continue;      
    int elemtype = gmesh.Reference()->ElementVec()[iel]->Type();
    if(elemtype==16) continue;
    TPZCompEl *el = gmesh.Reference()->ElementVec()[iel];
    if(el->Material()->Id() < 0) continue;
    TPZGeoEl *gel = el->Reference();
    if(el && gel) {
      TPZGeoElPoint  *el0d=0;
      TPZGeoEl1d  *el1d=0;
      TPZGeoElT2d *elt2d=0;
      TPZGeoElQ2d *elq2d=0;
      TPZGeoElT3d *elt3d=0;
      TPZGeoElPi3d *elpi3d=0;
      TPZGeoElPr3d *elpr3d=0;
      TPZGeoElC3d  *elc3d=0;
      if(elemtype==0) el0d   = (TPZGeoElPoint *) gel;
      if(elemtype==1) el1d   = (TPZGeoEl1d    *) gel;
      if(elemtype==2) elt2d  = (TPZGeoElT2d   *) gel;
      if(elemtype==3) elq2d  = (TPZGeoElQ2d   *) gel;
      if(elemtype==4) elt3d  = (TPZGeoElT3d   *) gel;
      if(elemtype==5) elpi3d = (TPZGeoElPi3d  *) gel;
      if(elemtype==6) elpr3d = (TPZGeoElPr3d  *) gel;
      if(elemtype==7) elc3d  = (TPZGeoElC3d   *) gel;
      int nsides = gel->NSides();
      if(elemtype==15){
	if(nsides==1) el0d   = (TPZGeoElPoint *) gel;
	if(nsides==3) el1d   = (TPZGeoEl1d    *) gel;
	if(nsides==7) elt2d  = (TPZGeoElT2d   *) gel;
	if(nsides==9) elq2d  = (TPZGeoElQ2d   *) gel;
	if(nsides==27) elc3d  = (TPZGeoElC3d   *) gel;
      }
      out << "Elemento " << el->Reference()->Id() << endl;;
      TPZManVector<REAL> sol(1);
      TPZVec<REAL> csi(3,0.),x(3);
      int np = 5;
      if(dim==3) np = 9;
      for(int p=0;p<np;p++) {
	if(dim==1){
	  csi[0] = point[p][0];
	  csi[1] = point[p][1];
	  csi[2] = point[p][2];
	}
	if(dim==2){
	  csi[0] = quad[p][0];
	  csi[1] = quad[p][1];
	  csi[2] = quad[p][2];
	}
	if(dim==3){
	  csi[0] = hexa[p][0];
	  csi[1] = hexa[p][1];
	  csi[2] = hexa[p][2];
	}
	gel->X(csi,x);
	if(elemtype==0) el0d->Reference()->Solution(csi,0,sol);
	if(elemtype==1) el1d->Reference()->Solution(csi,0,sol);
	if(elemtype==2) elt2d->Reference()->Solution(csi,0,sol);
	if(elemtype==3) elq2d->Reference()->Solution(csi,0,sol);
	if(elemtype==4) elt3d->Reference()->Solution(csi,0,sol);
	if(elemtype==5) elpi3d->Reference()->Solution(csi,0,sol);
	if(elemtype==6) elpr3d->Reference()->Solution(csi,0,sol);
	if(elemtype==7) elc3d->Reference()->Solution(csi,0,sol);
	if(elemtype==15){
	  if(nsides==1) el0d->Reference()->Solution(csi,0,sol);
	  if(nsides==3) el1d->Reference()->Solution(csi,0,sol);
	  if(nsides==7) elt2d->Reference()->Solution(csi,0,sol);
	  if(nsides==9) elq2d->Reference()->Solution(csi,0,sol);
	  if(nsides==27) elc3d->Reference()->Solution(csi,0,sol);
	}
	out << "solucao em x    = " << x[0] << ' ' << x[1] << ' ' << x[2] << endl;
	out << "               u = " << sol[0] << endl;	    
      }
    }
  }
}

void Ordena(TPZVec<REAL> &coordx,TPZVec<int> &sort);
void FileNB(TPZGeoMesh &gmesh,ostream &out,int var) {

  //  int numpoints;
  int nel = gmesh.Reference()->ElementVec().NElements();
  int idmax = 0,dim,chega=1,finish=-1;
  for(int iel=0;iel<nel;iel++){//procurando o id mais alto da lista
    if(++finish >= 5000) return;
    TPZCompEl *cel = gmesh.Reference()->ElementVec()[iel];
    if(!cel) continue;
    TPZGeoEl *el = gmesh.ElementVec()[iel];
    if(chega && cel->Type()==15) {dim = el->Dimension(); chega = 0;}
    int id = el->Id();
    if(id > idmax) idmax = id;
  }
  int cap = nel*4;
  TPZVec<REAL> coordx(cap,0.),coordy(cap,0.),coordz(cap,0.);
  int count = -1,capacity=0;
  while(count++<idmax){
    for(int iel=0;iel<nel;iel++) {
      if(!gmesh.Reference()->ElementVec()[iel]) continue;      
      int elemtype = gmesh.Reference()->ElementVec()[iel]->Type();
      if(elemtype==16) continue;//interface
      TPZCompEl *el = gmesh.Reference()->ElementVec()[iel];
      if(el->Material()->Id() < 0) continue;
      //s� elementos de volume
      TPZGeoEl *gel = el->Reference();
      if(el && gel) {
	if(gel->Id()==count){
	  TPZGeoElPoint  *el0d=0;
	  TPZGeoEl1d  *el1d=0;
	  TPZGeoElT2d *elt2d=0;
	  TPZGeoElQ2d *elq2d=0;
	  TPZGeoElT3d *elt3d=0;
	  TPZGeoElPi3d *elpi3d=0;
	  TPZGeoElPr3d *elpr3d=0;
	  TPZGeoElC3d  *elc3d=0;
	  if(elemtype==0) el0d   = (TPZGeoElPoint    *) gel;
	  if(elemtype==1) el1d   = (TPZGeoEl1d    *) gel;
	  if(elemtype==2) elt2d  = (TPZGeoElT2d   *) gel;
	  if(elemtype==3) elq2d  = (TPZGeoElQ2d   *) gel;
	  if(elemtype==4) elt3d  = (TPZGeoElT3d   *) gel;
	  if(elemtype==5) elpi3d = (TPZGeoElPi3d  *) gel;
	  if(elemtype==6) elpr3d = (TPZGeoElPr3d  *) gel;
	  if(elemtype==7) elc3d  = (TPZGeoElC3d   *) gel;
	  int nsides = gel->NSides();
	  if(elemtype==15){
	    if(nsides==1) el0d   = (TPZGeoElPoint *) gel;
	    if(nsides==3) el1d   = (TPZGeoEl1d    *) gel;
	    if(nsides==7) elt2d  = (TPZGeoElT2d   *) gel;
	    if(nsides==9) elq2d  = (TPZGeoElQ2d   *) gel;
	  }
	  TPZManVector<REAL> sol(1);
	  TPZVec<REAL> csi(3,0.),x(3);
	  //int var = 1;//densidade
	  for(int p=0;p<4;p++) {
	    if(dim==1){
	      csi[0] = point[p][0];
	      csi[1] = point[p][1];
	      csi[2] = point[p][2];
	    }
	    if(dim==2){
	      csi[0] = quad[p][0];
	      csi[1] = quad[p][1];
	      csi[2] = quad[p][2];
	    }
	    gel->X(csi,x);
	    if(elemtype==0) el0d->Reference()->Solution(csi,var,sol);
	    if(elemtype==1) el1d->Reference()->Solution(csi,var,sol);
	    if(elemtype==2) elt2d->Reference()->Solution(csi,var,sol);
	    if(elemtype==3) elq2d->Reference()->Solution(csi,var,sol);
	    if(elemtype==4) elt3d->Reference()->Solution(csi,var,sol);
	    if(elemtype==5) elpi3d->Reference()->Solution(csi,var,sol);
	    if(elemtype==6) elpr3d->Reference()->Solution(csi,var,sol);
	    if(elemtype==7) elc3d->Reference()->Solution(csi,var,sol);
	    if(elemtype==15){
	      if(nsides==1) el0d->Reference()->Solution(csi,var,sol);
	      if(nsides==3) el1d->Reference()->Solution(csi,var,sol);
	      if(nsides==7) elt2d->Reference()->Solution(csi,var,sol);
	      if(nsides==9) elq2d->Reference()->Solution(csi,var,sol);
	    }
	    if(dim==1){
	      coordx[capacity] = x[0];
	      coordy[capacity] = sol[0];
	    }
	    if(dim==2){
	      coordx[capacity] = x[0];
	      coordy[capacity] = x[1];
	      coordz[capacity] = sol[0];
	  }
	    capacity++;
	  }
	  //out.close();
	} else {
	  continue;
	}
      }
    }
  }
  if(dim==1){
    //out << "<<Graphics`MultipleListPlot`\n";
    out << "GRAPH = {";
    int k,linha = 0;
    TPZVec<int> sort(capacity);
    Ordena(coordx,sort);
    for(k=0;k<(capacity-1);k++){
      out <<  "{" << coordx[k] << "," <<  coordy[sort[k]] << "},";
      linha++;
      if(linha == 8){
	out << endl;
	linha = 0;
      }
    }
    out <<  "{" << coordx[k] << "," <<  coordy[sort[k]] << "}};";
    out << "\nListPlot[GRAPH,PlotJoined->True]";
  }
  if(dim==2){
    //out << "<<Graphics`MultipleListPlot`\n";
    out << "GRAPH = {";
    int k,linha = 0;
    TPZVec<int> sort1(capacity);//,sort2(capacity);
    Ordena(coordx,sort1);
//     TPZVec<REAL> sort3(coordy),sort4(coordz);
//     for(k=0;k<capacity;k++){
//       coordy[k] = sort3[sort1[k]];
//       coordz[k] = sort4[sort1[k]];
//     }
//    Ordena(coordy,sort2);
    for(k=0;k<(capacity-1);k++){
      out <<  "{" << coordx[k] << "," << coordy[sort1[k]] << "," << coordz[sort1[k]] << "},";
      linha++;
      if(linha == 6){
	out << endl;
	linha = 0;
      }
    }
    out <<  "{" << coordx[k] << "," << coordy[sort1[k]] << "," << coordz[sort1[k]] << "}};";
    out << "\nListPlot3D[GRAPH]";
  }
}

void Ordena(TPZVec<REAL> &coordx,TPZVec<int> &sort){

  int i,j,cap=sort.NElements();
  for(i=0;i<cap;i++) sort[i] = i;
  for(i=0;i<cap;i++){
    REAL x = coordx[i];
    for(j=i+1;j<cap;j++){
      if(coordx[j] < x){
	coordx[i] = coordx[j];
	coordx[j] = x;
	x = coordx[i];
	int aux = sort[i];
	sort[i] = sort[j];
	sort[j] = aux;
      }
    }
  }
}

void CoutTime(clock_t &start){
    end = clock();
    cout << "\nFim da etapa : "  <<  endl;
    clock_t segundos = ((end - start)/CLOCKS_PER_SEC);
    cout << segundos << " segundos" << endl;
    cout << segundos/60.0 << " minutos" << endl << endl;
}

//----------------------------------------------------------------------------------------------
TPZMaterial *Hexaedro(int grau){
  //Problema teste do Cedric <=> problema teste no paper A. Coutinho e paper Zhang, Yu, Chang levado para 3D
  // e teste no paper de Peyrard and Villedieu
  CriacaoDeNos(8,hexaedro);
  //elemento de volume
  TPZVec<int> nodes;
  nodes.Resize(8);
  nodes[0] = 0;
  nodes[1] = 1;
  nodes[2] = 2;
  nodes[3] = 3;
  nodes[4] = 4;
  nodes[5] = 5;
  nodes[6] = 6;
  nodes[7] = 7;
  TPZGeoElC3d *elgc3d = new TPZGeoElC3d(nodes,1,*gmesh);
  //construtor descont�nuo

  int interfdim = 2;
  TPZCompElDisc::gInterfaceDimension = interfdim;
  gmesh->BuildConnectivity();
  int nummat = 1;
  char *artdiff = "LS";
  cout << "\nmain::Divisao Nivel final da malha ? : ";
  cin >> nivel;
  REAL cfl,delta_x,delta_t,delta,gama;//,maxflinha;

  cfl = ( 1./(2.0*(REAL)grau+1.0) );
  delta_x =  ( 1.0 / pow(2.0,(REAL)nivel) );
  delta_t = cfl*delta_x;//delta_t � <= que este valor
  //calculando novos valores
  delta_t = delta_x*cfl;
  delta =  (10./3.)*cfl*cfl - (2./3.)*cfl + 1./10.;
  gama = 1.4;
  cout << "\nDominio [0,1]x[0,1]"
       << "\nMax df/dx (desconhecido) = 1.0"
       << "\nCFL = " << cfl
       << "\ndelta otimo = " << delta
       << "\nDelta x = " << delta_x
       << "\ndelta t = " << delta_t
       << "\ndiffusao = " << artdiff << endl;
  
  int dim = 3;
  TPZMaterial *mat = new TPZEulerConsLaw(nummat,delta_t,gama,dim,artdiff);

  //((TPZConservationLaw *)mat)->SetIntegDegree(grau);
  mat->SetForcingFunction(Function);
  cmesh->InsertMaterialObject(mat);

  //condi��es de contorno  
  TPZBndCond *bc;
  //REAL big = 1.e12;
  TPZFMatrix val1(5,5,0.),val2(5,1,0.);

  //CC FACE 20: parede
  val1.Zero();
  val2.Zero();
  TPZGeoElBC(elgc3d,20,-1,*gmesh);
  bc = mat->CreateBC(-1,5,val1,val2);
  cmesh->InsertMaterialObject(bc); 

  //CC FACE 21: Dirichlet
  val1.Zero();
  val2.Zero();
  REAL ro = 1.7;
  REAL u = 2.61934;
  REAL v = 0.50632;
  REAL w = 0.0;
  REAL p = 1.52819;
  REAL vel2 = u*u + v*v + w*w;
  val2(0,0) = ro;
  val2(1,0) = ro * u;
  val2(2,0) = ro * v;
  val2(3,0) = ro * w;
  val2(4,0) = p/(gama-1.0) + 0.5 * ro * vel2;
  TPZGeoElBC(elgc3d,21,-2,*gmesh);
  bc = mat->CreateBC(-2,3,val1,val2);
  cmesh->InsertMaterialObject(bc); 

  //CC FACE 22  DIREITA : livre
  val1.Zero();
  val2.Zero();
  TPZGeoElBC(elgc3d,22,-3,*gmesh);
  bc = mat->CreateBC(-3,4,val1,val2);
  cmesh->InsertMaterialObject(bc);

  //CC FACE 23: POSTERIOR : PAREDE = wall
  val1.Zero();
  val2.Zero();
  TPZGeoElBC(elgc3d,23,-4,*gmesh);
  bc = mat->CreateBC(-4,5,val1,val2);//CC MISTA
  cmesh->InsertMaterialObject(bc);

  //CC FACE ESQUERDA 24: Dirichlet
  val1.Zero();
  val2.Zero();
  ro = 1.0;
  u = 2.9;
  v = 0.0;
  w = 0.0;
  p = 0.714286;
  vel2 = u*u+v*v+w*w;
  val2(0,0) = ro;
  val2(1,0) = ro * u;
  val2(2,0) = ro * v;
  val2(3,0) = ro * w;
  val2(4,0) = p/(gama-1.0) +  0.5 * ro * vel2;
  TPZGeoElBC(elgc3d,24,-5,*gmesh);
  bc = mat->CreateBC(-5,3,val1,val2);
  cmesh->InsertMaterialObject(bc);

  //CC FACE 25 SUPERIOR : PAREDE
  val1.Zero();
  val2.Zero();
  TPZGeoElBC(elgc3d,25,-6,*gmesh);
  bc = mat->CreateBC(-6,5,val1,val2);//CC MISTA
  cmesh->InsertMaterialObject(bc);

  cout << endl;
  cmesh->AutoBuild();
  return mat;
}

//----------------------------------------------------------------------------------------------
TPZMaterial *ProblemaT2D(int grau){
  //Teste no paper de A. Coutinho e primeiro problema teste na tese de Jorge Calle
  //teste do papern Zhang, Yu, Chang e teste no paper de Peyrard and Villedieu
  CriacaoDeNos(4,quadrilatero);//para formar dois triangulos
  //elemento de volume
  TPZVec<int> nodes;
  nodes.Resize(3);
  nodes[0] = 0;
  nodes[1] = 2;
  nodes[2] = 3;
  TPZGeoElT2d *elgt2d0 = new TPZGeoElT2d(nodes,1,*gmesh);
  nodes[0] = 0;
  nodes[1] = 1;
  nodes[2] = 2;
  TPZGeoElT2d *elgt2d1 = new TPZGeoElT2d(nodes,1,*gmesh);

  int interfdim = 1;
  TPZCompElDisc::gInterfaceDimension = interfdim;
  gmesh->BuildConnectivity();
  int nummat = 1;
  char *artdiff = "LS";
  cout << "\nmain::Divisao Nivel final da malha ? : ";
  cin >> nivel;
  REAL cfl = ( 1./(2.0*(REAL)grau+1.0) );///0.5;
  REAL delta_x =  ( 1.0 / pow(2.0,(REAL)nivel) );//0.5;
  REAL delta_t = cfl*delta_x;//delta_t � <= que este valor
  //calculando novos valores
  delta_t = delta_x*cfl;
  REAL delta =  (10./3.)*cfl*cfl - (2./3.)*cfl + 1./10.;
  gama = 1.4;
  cout << "\nDominio [0,1]x[0,1]"
       << "\nMax df/dx (desconhecido) = 1.0"
       << "\nCFL = " << cfl
       << "\ndelta otimo = " << delta
       << "\nDelta x = " << delta_x
       << "\ndelta t = " << delta_t
       << "\ndiffusao = " << artdiff << endl;

  int dim = 2;
  TPZMaterial *mat = (TPZEulerConsLaw *) new TPZEulerConsLaw(nummat,delta_t,gama,dim,artdiff);

  mat->SetForcingFunction(Function);
  cmesh->InsertMaterialObject(mat);
  //condi��es de contorno  
  TPZBndCond *bc;
  //REAL big = 1.e12;
  TPZFMatrix val1(4,4,0.),val2(4,1,0.);

  //TYPE CC
  //0: Dirichlet
  //1: Neumann
  //2: Mista
  //3: Dirichlet -> c�lculo do fluxo
  //4: livre
  //5: parede

  //CC ARESTA INFERIOR
  val1.Zero();
  val2.Zero();
  REAL ro = 1.7;
  REAL u = 2.61934;
  REAL v = 0.50632;
  REAL p = 1.52819;
  REAL vel2 = u*u + v*v;
  val2(0,0) = ro;
  val2(1,0) = ro * u;
  val2(2,0) = ro * v;
  val2(3,0) = p/(gama-1.0) + 0.5 * ro * vel2;
  TPZGeoElBC(elgt2d1,3,-1,*gmesh);
  bc = mat->CreateBC(-1,3,val1,val2);
  cmesh->InsertMaterialObject(bc);//bc->SetForcingFunction(Function);

  //CC ARESTA DIREITA : LIVRE
  val1.Zero();
  val2.Zero();
  TPZGeoElBC(elgt2d1,4,-2,*gmesh);
  bc = mat->CreateBC(-2,4,val1,val2);
  cmesh->InsertMaterialObject(bc);//bc->SetForcingFunction(Function);

  //CC ARESTA SUPERIOR : PAREDE - WALL
  val1.Zero();
  val2.Zero();
  TPZGeoElBC(elgt2d0,4,-3,*gmesh);
  bc = mat->CreateBC(-3,5,val1,val2);
  cmesh->InsertMaterialObject(bc);//bc->SetForcingFunction(Function); 

  //CC ARESTA ESQUERDA
  val1.Zero();
  val2.Zero();
  ro = 1.0;
  u = 2.9;
  v = 0.0;
  p = 0.714286;
  val2(0,0) = ro;
  val2(1,0) = ro * u;
  val2(2,0) = ro * v;
  vel2 = u*u+v*v;
  val2(3,0) = p/(gama-1.0) +  0.5 * ro * vel2;
  TPZGeoElBC(elgt2d0,5,-4,*gmesh);
  bc = mat->CreateBC(-4,3,val1,val2);
  cmesh->InsertMaterialObject(bc);//bc->SetForcingFunction(Function); 

  cout << endl;
  cmesh->AutoBuild();

  return mat;
}

//----------------------------------------------------------------------------------------------
TPZMaterial *ProblemaQ2D1El(int grau){
  //Teste no paper de A. Coutinho e primeiro problema teste na tese de Jorge Calle
  //teste do papern Zhang, Yu, Chang  e teste no paper de Peyrard and Villedieu
  CriacaoDeNos(4,quadrilatero);
  //elemento de volume
  TPZVec<int> nodes;
  nodes.Resize(4);
  nodes[0] = 0;
  nodes[1] = 1;
  nodes[2] = 2;
  nodes[3] = 3;
  TPZGeoElQ2d *elgq2d = new TPZGeoElQ2d(nodes,1,*gmesh);

  int interfdim = 1;
  TPZCompElDisc::gInterfaceDimension = interfdim;
  gmesh->BuildConnectivity();
  int nummat = 1;
  char *artdiff = "LS";
  cout << "\nmain::Divisao Nivel final da malha ? : ";
  cin >> nivel;
  REAL cfl = ( 1./(2.0*(REAL)grau+1.0) );///0.5;
  REAL delta_x =  ( 1.0 / pow(2.0,(REAL)nivel) );//0.5;
  REAL delta_t = cfl*delta_x;//delta_t � <= que este valor
  //calculando novos valores
  delta_t = delta_x*cfl;
  REAL delta =  (10./3.)*cfl*cfl - (2./3.)*cfl + 1./10.;
  gama = 1.4;
  cout << "\nDominio [0,1]x[0,1]"
       << "\nMax df/dx (desconhecido) = 1.0"
       << "\nCFL = " << cfl
       << "\ndelta otimo = " << delta
       << "\nDelta x = " << delta_x
       << "\ndelta t = " << delta_t
       << "\ndiffusao = " << artdiff
       << "\ndelta aproximado = " << delta << endl;

  int dim = 2;
  TPZMaterial *mat = (TPZEulerConsLaw *) new TPZEulerConsLaw(nummat,delta_t,gama,dim,artdiff);
  mat->SetForcingFunction(Function);
  cmesh->InsertMaterialObject(mat);

  //condi��es de contorno  
  TPZBndCond *bc;
  //REAL big = 1.e12;
  TPZFMatrix val1(4,4),val2(4,1);

  //CC ARESTA INFERIOR
  val1.Zero();
  val2.Zero();
  REAL ro = 1.7;
  REAL u = 2.61934;
  REAL v = 0.50632;
  REAL p = 1.52819;
  REAL vel2 = u*u + v*v;
  val2(0,0) = ro;
  val2(1,0) = ro * u;
  val2(2,0) = ro * v;
  val2(3,0) = p/(gama-1.0) + 0.5 * ro * vel2;
  TPZGeoElBC(elgq2d,4,-1,*gmesh);
  bc = mat->CreateBC(-1,3,val1,val2);//bc->SetForcingFunction(Function);
  cmesh->InsertMaterialObject(bc);

  //CC ARESTA DIREITA : LIVRE
  val1.Zero();
  val2.Zero();
  ro = 1.0;
  u = 2.9;
  v = 0.0;
  p = 0.714286;
  vel2 = u*u+v*v;
  val2(0,0) = ro;
  val2(1,0) = ro * u;
  val2(2,0) = ro * v;
  val2(3,0) = p/(gama-1.0) +  0.5 * ro * vel2;
  TPZGeoElBC(elgq2d,5,-2,*gmesh);
  bc = mat->CreateBC(-2,4,val1,val2);//bc->SetForcingFunction(Function);
  cmesh->InsertMaterialObject(bc);

  //CC ARESTA SUPERIOR : PAREDE - wall
  val1.Zero();
  val2.Zero();
  TPZGeoElBC(elgq2d,6,-3,*gmesh);
  bc = mat->CreateBC(-3,5,val1,val2);//bc->SetForcingFunction(Function);
  cmesh->InsertMaterialObject(bc); 

  //CC ARESTA ESQUERDA
  val1.Zero();
  val2.Zero();
  ro = 1.0;
  u = 2.9;
  v = 0.0;
  p = 0.714286;
  val2(0,0) = ro;
  val2(1,0) = ro * u;
  val2(2,0) = ro * v;
  vel2 = u*u+v*v;
  val2(3,0) = p/(gama-1.0) +  0.5 * ro * vel2;
  TPZGeoElBC(elgq2d,7,-4,*gmesh);
  bc = mat->CreateBC(-4,3,val1,val2);//bc->SetForcingFunction(Function);
  cmesh->InsertMaterialObject(bc); 

  cout << endl;
  cmesh->AutoBuild();

  return mat;
}

//----------------------------------------------------------------------------------------------
TPZMaterial *TresTriangulos(int grau){
  //Teste no paper de A. Coutinho e primeiro problema teste na tese de Jorge Calle
  //teste do papern Zhang, Yu, Chang e teste no paper de Peyrard and Villedieu
  CriacaoDeNos(5,quadrilatero2);//para formar dois triangulos
  //elemento de volume
  TPZVec<int> nodes;
  int index;
  nodes.Resize(3);
  nodes[0] = 0;
  nodes[1] = 1;
  nodes[2] = 3;
  TPZGeoEl *elgt2d0 = gmesh->CreateGeoElement(ETriangle,nodes,1,index);
  nodes[0] = 1;
  nodes[1] = 2;
  nodes[2] = 4;
  TPZGeoEl *elgt2d1 = gmesh->CreateGeoElement(ETriangle,nodes,1,index);
  nodes[0] = 1;
  nodes[1] = 4;
  nodes[2] = 3;
  TPZGeoEl *elgt2d2 = gmesh->CreateGeoElement(ETriangle,nodes,1,index);
  TPZGeoElement<TPZShapeTriang,TPZGeoTriangle,TPZRefTriangle>::SetCreateFunction(TPZCompElDisc::CreateDisc);
  TPZGeoElement<TPZShapeLinear,TPZGeoLinear,TPZRefLinear>::SetCreateFunction(TPZCompElDisc::CreateDisc);
  int interfdim = 1;
  TPZCompElDisc::gInterfaceDimension = interfdim;
  gmesh->BuildConnectivity();
  int nummat = 1;
  char *artdiff = "LS";
  cout << "\nmain::Divisao Nivel final da malha ? : ";
  cin >> nivel;
  REAL cfl = ( 1./(2.0*(REAL)grau+1.0) );///0.5;
  REAL delta_x =  ( 1.0 / pow(2.0,(REAL)nivel) );//0.5;
  REAL delta_t = cfl*delta_x;//delta_t � <= que este valor
  //calculando novos valores
  delta_t = delta_x*cfl;
  REAL delta =  (10./3.)*cfl*cfl - (2./3.)*cfl + 1./10.;
  gama = 1.4;
  cout << "\nDominio [0,1]x[0,1]"
       << "\nMax df/dx (desconhecido) = 1.0"
       << "\nCFL = " << cfl
       << "\ndelta otimo = " << delta
       << "\nDelta x = " << delta_x
       << "\ndelta t = " << delta_t
       << "\ndiffusao = " << artdiff << endl;

  int dim = 2;
  TPZMaterial *mat = (TPZEulerConsLaw *) new TPZEulerConsLaw(nummat,delta_t,gama,dim,artdiff);

  mat->SetForcingFunction(Function);
  cmesh->InsertMaterialObject(mat);
  //condi��es de contorno  
  TPZBndCond *bc;
  TPZFMatrix val1(4,4,0.),val2(4,1,0.);

  //CC ARESTA INFERIOR: PAREDE
  val1.Zero();
  val2.Zero();
  TPZGeoElBC(elgt2d0,3,-1,*gmesh);
  TPZGeoElBC(elgt2d1,3,-1,*gmesh);
  bc = mat->CreateBC(-1,5,val1,val2);
  cmesh->InsertMaterialObject(bc);

  //CC ARESTA DIREITA : LIVRE
  val1.Zero();
  val2.Zero();
  TPZGeoElBC(elgt2d1,4,-2,*gmesh);
  bc = mat->CreateBC(-2,4,val1,val2);
  cmesh->InsertMaterialObject(bc);

  //CC ARESTA SUPERIOR : 
  val1.Zero();
  val2.Zero();
  REAL ro = 1.7;
  REAL u = 2.61934;
  REAL v = -0.50632;
  REAL p = 1.52819;
  REAL vel2 = u*u + v*v;
  val2(0,0) = ro;
  val2(1,0) = ro * u;
  val2(2,0) = ro * v;
  val2(3,0) = p/(gama-1.0) + 0.5 * ro * vel2;
  TPZGeoElBC(elgt2d2,4,-3,*gmesh);
  bc = mat->CreateBC(-3,3,val1,val2);
  cmesh->InsertMaterialObject(bc);

  //CC ARESTA ESQUERDA
  val1.Zero();
  val2.Zero();
  ro = 1.0;
  u = 2.9;
  v = 0.0;
  p = 0.714286;
  val2(0,0) = ro;
  val2(1,0) = ro * u;
  val2(2,0) = ro * v;
  vel2 = u*u+v*v;
  val2(3,0) = p/(gama-1.0) +  0.5 * ro * vel2;
  TPZGeoElBC(elgt2d0,5,-4,*gmesh);
  bc = mat->CreateBC(-4,3,val1,val2);
  cmesh->InsertMaterialObject(bc);

  cout << endl;
  cmesh->AutoBuild();

  return mat;
}

//----------------------------------------------------------------------------------------------
TPZMaterial *TresPrismas(int grau){
  //Problema teste do Cedric <=> problema teste no paper A. Coutinho e paper Zhang, Yu, Chang levado para 3D
  // e teste no paper de Peyrard and Villedieu
  CriacaoDeNos(10,tresprismas);
  //elemento de volume
  TPZVec<int> nodes;
  nodes.Resize(6);
  nodes[0] = 0;
  nodes[1] = 1;
  nodes[2] = 3;
  nodes[3] = 5;
  nodes[4] = 6;
  nodes[5] = 8;
  TPZGeoElPr3d *elg1 = new TPZGeoElPr3d(nodes,1,*gmesh);
  nodes[0] = 1;
  nodes[1] = 2;
  nodes[2] = 4;
  nodes[3] = 6;
  nodes[4] = 7;
  nodes[5] = 9;
  TPZGeoElPr3d *elg2 = new TPZGeoElPr3d(nodes,1,*gmesh);
  nodes[0] = 3;
  nodes[1] = 1;
  nodes[2] = 4;
  nodes[3] = 8;
  nodes[4] = 6;
  nodes[5] = 9;
  TPZGeoElPr3d *elg3 = new TPZGeoElPr3d(nodes,1,*gmesh);

  int interfdim = 2;
  TPZCompElDisc::gInterfaceDimension = interfdim;
  gmesh->BuildConnectivity2();
  int nummat = 1;
  char *artdiff = "LS";
  cout << "\nmain::Divisao Nivel final da malha ? : ";
  cin >> nivel;
  REAL cfl,delta_x,delta_t,delta,gama;//,maxflinha;

  cfl = ( 1./(2.0*(REAL)grau+1.0) );
  delta_x =  ( 1.0 / pow(2.0,(REAL)nivel) );
  delta_t = cfl*delta_x;//delta_t � <= que este valor
  delta =  (10./3.)*cfl*cfl - (2./3.)*cfl + 1./10.;
  gama = 1.4;
  cout << "\nDominio [0,1]x[0,1]"
       << "\nMax df/dx (desconhecido) = 1.0"
       << "\nCFL = " << cfl
       << "\ndelta otimo = " << delta
       << "\nDelta x = " << delta_x
       << "\ndelta t = " << delta_t
       << "\ndiffusao = " << artdiff << endl;
  
  int dim = 3;
  TPZMaterial *mat = (TPZEulerConsLaw *) new TPZEulerConsLaw(nummat,delta_t,gama,dim,artdiff);

  //((TPZConservationLaw *)mat)->SetIntegDegree(grau);
  mat->SetForcingFunction(Function);
  cmesh->InsertMaterialObject(mat);

  //condi��es de contorno  
  TPZBndCond *bc;
  TPZFMatrix val1(5,5,0.),val2(5,1,0.);

  //CC parede
  val1.Zero();
  val2.Zero();
  TPZGeoElBC(elg1,15,-1,*gmesh);
  TPZGeoElBC(elg2,15,-1,*gmesh);
  TPZGeoElBC(elg3,15,-1,*gmesh);
  TPZGeoElBC(elg1,16,-1,*gmesh);
  TPZGeoElBC(elg2,16,-1,*gmesh);
  TPZGeoElBC(elg1,19,-1,*gmesh);
  TPZGeoElBC(elg2,19,-1,*gmesh);
  TPZGeoElBC(elg3,19,-1,*gmesh);
  bc = mat->CreateBC(-1,5,val1,val2);
  cmesh->InsertMaterialObject(bc); 

  //CC Dirichlet
  val1.Zero();
  val2.Zero();
  REAL ro = 1.7;
  REAL u = 2.61934;
  REAL v = -0.50632;
  REAL w = 0.0;
  REAL p = 1.52819;
  REAL vel2 = u*u + v*v + w*w;
  val2(0,0) = ro;
  val2(1,0) = ro * u;
  val2(2,0) = ro * v;
  val2(3,0) = ro * w;
  val2(4,0) = p/(gama-1.0) + 0.5 * ro * vel2;
  TPZGeoElBC(elg3,18,-2,*gmesh);
  bc = mat->CreateBC(-2,3,val1,val2);
  cmesh->InsertMaterialObject(bc);

  //CC Dirichlet
  val1.Zero();
  val2.Zero();
  ro = 1.0;
  u = 2.9;
  v = 0.0;
  w = 0.0;
  p = 0.714286;
  vel2 = u*u+v*v+w*w;
  val2(0,0) = ro;
  val2(1,0) = ro * u;
  val2(2,0) = ro * v;
  val2(3,0) = ro * w;
  val2(4,0) = p/(gama-1.0) +  0.5 * ro * vel2;
  TPZGeoElBC(elg1,18,-3,*gmesh);
  bc = mat->CreateBC(-3,3,val1,val2);
  cmesh->InsertMaterialObject(bc);

  //CC livre
  val1.Zero();
  val2.Zero();
  TPZGeoElBC(elg2,17,-4,*gmesh);
  bc = mat->CreateBC(-4,4,val1,val2);
  cmesh->InsertMaterialObject(bc);

  cout << endl;
  cmesh->AutoBuild();
  return mat;
}

//----------------------------------------------------------------------------------------------
TPZMaterial *FluxConst3D(int grau){
  //Problema teste do Cedric <=> problema teste no paper A. Coutinho e paper Zhang, Yu, Chang levado para 3D
  // e teste no paper de Peyrard and Villedieu
  CriacaoDeNos(8,hexaedro1);
  //elemento de volume
  TPZVec<int> nodes;
  int index;
  nodes.Resize(8);
  nodes[0] = 0;
  nodes[1] = 1;
  nodes[2] = 2;
  nodes[3] = 3;
  nodes[4] = 4;
  nodes[5] = 5;
  nodes[6] = 6;
  nodes[7] = 7;
  TPZGeoEl *elgc3d = gmesh->CreateGeoElement(ECube,nodes,1,index);
  //construtor descont�nuo
  TPZGeoElement<TPZShapeCube,TPZGeoCube,TPZRefCube>::SetCreateFunction(TPZCompElDisc::CreateDisc);
  //TPZGeoElement<TPZShapeTetra,TPZGeoTetrahedra,TPZRefTetrahedra>::SetCreateFunction(TPZCompElDisc::CreateDisc);
  //TPZGeoElement<TPZShapeTriang,TPZGeoTriangle,TPZRefTriangle>::SetCreateFunction(TPZCompElDisc::CreateDisc);
  TPZGeoElement<TPZShapeQuad,TPZGeoQuad,TPZRefQuad>::SetCreateFunction(TPZCompElDisc::CreateDisc);
  //TPZGeoElement<TPZShapeLinear,TPZGeoLinear,TPZRefLinear>::SetCreateFunction(TPZCompElDisc::CreateDisc);
  int interfdim = 2;
  TPZCompElDisc::gInterfaceDimension = interfdim;
  gmesh->BuildConnectivity2();
  int nummat = 1;
  char *artdiff = "LS";
  cout << "\nmain::Divisao Nivel final da malha ? : ";
  cin >> nivel;
  REAL cfl,delta_x,delta_t,delta,gama;//,maxflinha;

  cfl = ( 1./(2.0*(REAL)grau+1.0) );
  delta_x =  ( 1.0 / pow(2.0,(REAL)nivel) );
  delta_t = cfl*delta_x;//delta_t � <= que este valor
  //calculando novos valores
  delta_t = delta_x*cfl;
  delta =  (10./3.)*cfl*cfl - (2./3.)*cfl + 1./10.;
  gama = 1.4;
  cout << "\nDominio [0,1]x[0,1]"
       << "\nMax df/dx (desconhecido) = 1.0"
       << "\nCFL = " << cfl
       << "\ndelta otimo = " << delta
       << "\nDelta x = " << delta_x
       << "\ndelta t = " << delta_t
       << "\ndiffusao = " << artdiff << endl;
  
  int dim = 3;
  TPZMaterial *mat = (TPZEulerConsLaw *) new TPZEulerConsLaw(nummat,delta_t,gama,dim,artdiff);

  //((TPZConservationLaw *)mat)->SetIntegDegree(grau);
  mat->SetForcingFunction(Function);
  cmesh->InsertMaterialObject(mat);

  //condi��es de contorno  
  TPZBndCond *bc;
  TPZFMatrix val1(5,5,0.),val2(5,1,0.);

  //CC FACE: parede
  val1.Zero();
  val2.Zero();
  TPZGeoElBC(elgc3d,20,-1,*gmesh);
  TPZGeoElBC(elgc3d,21,-1,*gmesh);
  TPZGeoElBC(elgc3d,23,-1,*gmesh);
  TPZGeoElBC(elgc3d,25,-1,*gmesh);
  bc = mat->CreateBC(-1,5,val1,val2);
  cmesh->InsertMaterialObject(bc); 

  //CC FACE : Dirichlet
  val1.Zero();
  val2.Zero();
  REAL ro = 1.0;
  REAL u = 2.9;
  REAL v = 0.0;
  REAL w = 0.0;
  REAL p = 0.714286;
  REAL vel2 = u*u+v*v+w*w;
  val2(0,0) = ro;
  val2(1,0) = ro * u;
  val2(2,0) = ro * v;
  val2(3,0) = ro * w;
  val2(4,0) = p/(gama-1.0) +  0.5 * ro * vel2;
  TPZGeoElBC(elgc3d,24,-2,*gmesh);
  //TPZGeoElBC(elgc3d,22,-2,*gmesh);
  bc = mat->CreateBC(-2,3,val1,val2);
  cmesh->InsertMaterialObject(bc); 

  //CC FACE 22  DIREITA : livre
  val1.Zero();
  val2.Zero();
  TPZGeoElBC(elgc3d,22,-3,*gmesh);
  bc = mat->CreateBC(-3,4,val1,val2);
  cmesh->InsertMaterialObject(bc);

  cout << endl;
  cmesh->AutoBuild();
  return mat;
}


//----------------------------------------------------------------------------------------------
TPZMaterial *FluxConst2D(int grau){

  CriacaoDeNos(4,quadrilatero);
  //elemento de volume
  int index;
  TPZVec<int> nodes;
  nodes.Resize(4);
  nodes[0] = 0;
  nodes[1] = 1;
  nodes[2] = 2;
  nodes[3] = 3;
  TPZGeoEl *elgq2d = gmesh->CreateGeoElement(EQuadrilateral,nodes,1,index);
  //construtor descont�nuo
  TPZGeoElement<TPZShapeQuad,TPZGeoQuad,TPZRefQuad>::SetCreateFunction(TPZCompElDisc::CreateDisc);
  TPZGeoElement<TPZShapeLinear,TPZGeoLinear,TPZRefLinear>::SetCreateFunction(TPZCompElDisc::CreateDisc);

  int interfdim = 1;
  TPZCompElDisc::gInterfaceDimension = interfdim;
  gmesh->BuildConnectivity();
  int nummat = 1;
  char *artdiff = "LS";
  cout << "\nmain::Divisao Nivel final da malha ? : ";
  cin >> nivel;
  //nivel = 2;
  REAL cfl = ( 1./(2.0*(REAL)grau+1.0) );///0.5;
  REAL delta_x =  ( 1.0 / pow(2.0,(REAL)nivel) );//0.5;
  REAL delta_t = cfl*delta_x;//delta_t � <= que este valor
  REAL delta =  (10./3.)*cfl*cfl - (2./3.)*cfl + 1./10.;
  gama = 1.4;
  cout << "\nDominio [0,1]x[0,1]"
       << "\nMax df/dx (desconhecido) = 1.0"
       << "\nCFL = " << cfl
       << "\ndelta otimo = " << delta
       << "\nDelta x = " << delta_x
       << "\ndelta t = " << delta_t
       << "\ndiffusao = " << artdiff
       << "\ndelta aproximado = " << delta << endl;

  int dim = 2;
  TPZMaterial *mat = (TPZEulerConsLaw *) new TPZEulerConsLaw(nummat,delta_t,gama,dim,artdiff);
  mat->SetForcingFunction(Function);
  cmesh->InsertMaterialObject(mat);

  //condi��es de contorno  
  TPZBndCond *bc;
  TPZFMatrix val1(4,4),val2(4,1);
  REAL ro,u,v,vel2,p;

  //CC ARESTAS INFERIOR E SUPERIOR
  val1.Zero();
  val2.Zero();
  TPZGeoElBC(elgq2d,4,-1,*gmesh);
  TPZGeoElBC(elgq2d,6,-1,*gmesh);
  bc = mat->CreateBC(-1,5,val1,val2);
  cmesh->InsertMaterialObject(bc);

  //CC ARESTA ESQUERDA
  val1.Zero();
  val2.Zero();
  ro = 1.0;
  u = 2.9;
  v = 0.0;
  p = 0.714286;
  vel2 = u*u+v*v;
  val2(0,0) = ro;
  val2(1,0) = ro * u;
  val2(2,0) = ro * v;
  val2(3,0) = p/(gama-1.0) +  0.5 * ro * vel2;
  TPZGeoElBC(elgq2d,7,-2,*gmesh);
  bc = mat->CreateBC(-2,3,val1,val2);
  cmesh->InsertMaterialObject(bc);

  //CC ARESTA DIREITA
  val1.Zero();
  val2.Zero();
  TPZGeoElBC(elgq2d,5,-3,*gmesh);
  bc = mat->CreateBC(-3,4,val1,val2);
  cmesh->InsertMaterialObject(bc); 

  cout << endl;
  cmesh->AutoBuild();

  return mat;
}

//----------------------------------------------------------------------------------------------
TPZMaterial *NoveQuadrilateros(int grau){
  //Teste no paper de A. Coutinho e primeiro problema teste na tese de Jorge Calle
  //teste do papern Zhang, Yu, Chang  e teste no paper de Peyrard and Villedieu
  CriacaoDeNos(15,novequads);
  //elemento de volume
  TPZVec<int> nodes;
  int INCID[9][4] = {{0,1,6,5},{1,2,7,6},{2,3,10,9},{3,4,11,10},{5,6,7,12},{7,2,9,8},{10,11,14,9},{7,8,13,12},{8,9,14,13}};
  nodes.Resize(4);
  TPZVec<TPZGeoEl *> elem(9);
  elem.Resize(9);
  int i,index;
  for(i=0;i<9;i++){
    nodes[0] = INCID[i][0];
    nodes[1] = INCID[i][1];
    nodes[2] = INCID[i][2];
    nodes[3] = INCID[i][3];
    elem[i] = gmesh->CreateGeoElement(EQuadrilateral,nodes,1,index);
  }
  //construtor descont�nuo
  TPZGeoElement<TPZShapeQuad,TPZGeoQuad,TPZRefQuad>::SetCreateFunction(TPZCompElDisc::CreateDisc);
  TPZGeoElement<TPZShapeLinear,TPZGeoLinear,TPZRefLinear>::SetCreateFunction(TPZCompElDisc::CreateDisc);
  int interfdim = 1;
  TPZCompElDisc::gInterfaceDimension = interfdim;
  gmesh->BuildConnectivity();
  int nummat = 1;
  char *artdiff = "LS";
  cout << "\nmain::Divisao Nivel final da malha ? : ";
  cin >> nivel;
  REAL cfl = ( 1./(2.0*(REAL)grau+1.0) );///0.5;
  REAL delta_x =  ( 1.0 / pow(2.0,(REAL)nivel) );//0.5;
  REAL delta_t = cfl*delta_x;//delta_t � <= que este valor
  //calculando novos valores
  delta_t = delta_x*cfl;
  REAL delta =  (10./3.)*cfl*cfl - (2./3.)*cfl + 1./10.;
  gama = 1.4;
  cout << "\nDominio [0,1]x[0,1]"
       << "\nMax df/dx (desconhecido) = 1.0"
       << "\nCFL = " << cfl
       << "\ndelta otimo = " << delta
       << "\nDelta x = " << delta_x
       << "\ndelta t = " << delta_t
       << "\ndiffusao = " << artdiff
       << "\ndelta aproximado = " << delta << endl;

  int dim = 2;
  TPZMaterial *mat = (TPZEulerConsLaw *) new TPZEulerConsLaw(nummat,delta_t,gama,dim,artdiff);
  mat->SetForcingFunction(Function);
  cmesh->InsertMaterialObject(mat);

  //condi��es de contorno  
  TPZBndCond *bc;
  TPZFMatrix val1(4,4),val2(4,1);

  //CC ARESTA INFERIOR : PAREDE
  val1.Zero();
  val2.Zero();
  TPZGeoElBC((TPZGeoEl  *)elem[0],4,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[1],4,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[2],4,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[3],4,-1,*gmesh);
  bc = mat->CreateBC(-1,5,val1,val2);
  cmesh->InsertMaterialObject(bc);

  //CC ARESTA DIREITA : OUTFLOW
  val1.Zero();
  val2.Zero();
  TPZGeoElBC((TPZGeoEl  *)elem[3],5,-2,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[6],5,-2,*gmesh);
  bc = mat->CreateBC(-2,4,val1,val2);
  cmesh->InsertMaterialObject(bc);

  //CC ARESTA SUPERIOR : DIRICHLET
  val1.Zero();
  val2.Zero();
  REAL ro = 1.7;
  REAL u = 2.61934;
  REAL v = -0.50632;
  REAL p = 1.52819;
  REAL vel2 = u*u + v*v;
  val2(0,0) = ro;
  val2(1,0) = ro * u;
  val2(2,0) = ro * v;
  val2(3,0) = p/(gama-1.0) + 0.5 * ro * vel2;
  TPZGeoElBC((TPZGeoEl  *)elem[7],6,-3,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[8],6,-3,*gmesh);
  bc = mat->CreateBC(-3,3,val1,val2);
  cmesh->InsertMaterialObject(bc); 

  //CC ARESTA ESQUERDA : INFLOW
  val1.Zero();
  val2.Zero();
  ro = 1.0;
  u = 2.9;
  v = 0.0;
  p = 0.714286;
  val2(0,0) = ro;
  val2(1,0) = ro * u;
  val2(2,0) = ro * v;
  vel2 = u*u+v*v;
  val2(3,0) = p/(gama-1.0) +  0.5 * ro * vel2;
  TPZGeoElBC((TPZGeoEl  *)elem[0],7,-4,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[4],7,-4,*gmesh);
  bc = mat->CreateBC(-4,3,val1,val2);
  cmesh->InsertMaterialObject(bc); 

  cout << endl;
  cmesh->AutoBuild();

  return mat;
}
////////////////////////////////////
void Function(TPZVec<REAL> &x,TPZVec<REAL> &result){

  if(problem == 6){
    result.Resize(5);
    //Condi��o inicial t =  0
    REAL ro = 1.0;
    REAL u = 2.9;// 1.0 , 2.9
    REAL v = 0.0;
    REAL w = 0.0;
    REAL p = 0.714286;// 0.246306 , 2.9
    REAL vel2 = u*u+v*v+w*w;
    result[0] = ro;
    result[1] = ro * u;
    result[2] = ro * v;
    result[3] = ro * w;
    result[4] = p/(gama-1.0) +  0.5 * ro * vel2;
    return;
  }

  if(problem == 0){
    result.Resize(4);
    //Condi��o inicial t =  0
    REAL ro = 1.0;
    REAL u = 2.9;
    REAL v = 0.0;
    REAL p = 0.714286;
    REAL vel2 = u*u + v*v;
    result[0] = ro;
    result[1] = ro * u;
    result[2] = ro * v;
    result[3] = p/(gama-1.0) + 0.5 * ro * vel2;
    return;
  }

  if(problem == 1){
    result.Resize(5);
    //Condi��o inicial t =  0
    REAL ro = 1.0;
    REAL u = 2.9;
    REAL v = 0.0;
    REAL w = 0.0;
    REAL p = 0.714286;
    REAL vel2 = u*u+v*v+w*w;
    result[0] = ro;
    result[1] = ro * u;
    result[2] = ro * v;
    result[3] = ro * w;
    result[4] = p/(gama-1.0) +  0.5 * ro * vel2;
    return;
  }
  if(problem == 2){
    result.Resize(5);
    //Condi��o inicial t =  0
    REAL ro = 1.0;
    REAL u = 2.9;// 1.0 , 2.9
    REAL v = 0.0;
    REAL w = 0.0;
    REAL p = 0.714286;// 0.246306 , 2.9
    REAL vel2 = u*u+v*v+w*w;
    result[0] = ro;
    result[1] = ro * u;
    result[2] = ro * v;
    result[3] = ro * w;
    result[4] = p/(gama-1.0) +  0.5 * ro * vel2;
    return;

    if(0){
      REAL ro = 1.0;
      REAL u = 2.9;
      REAL v = 0.0;
      REAL w = 0.0;
      REAL p = 0.714286;
      REAL vel2 = u*u+v*v+w*w;
      result[0] = ro;
      result[1] = ro * u;
      result[2] = ro * v;
      result[3] = ro * w;
      result[4] = p/(gama-1.0) +  0.5 * ro * vel2;
      return;
    }
  }
  if(problem == 3){
    result.Resize(4);
    //Condi��o inicial t =  0
    REAL ro = 1.0;
    REAL u = 2.9;
    REAL v = 0.0;
    REAL p = 0.714286;
    REAL vel2 = u*u+v*v;
    result[0] = ro;
    result[1] = ro * u;
    result[2] = ro * v;
    result[3] = p/(gama-1.0) +  0.5 * ro * vel2;
    return;
  }
  if(problem == 4){
    result.Resize(4);
    //Condi��o inicial t =  0
    REAL ro = 1.0;
    REAL u = 1.0;
    REAL v = 0.0;
    REAL p = 0.246306;
    REAL vel2 = u*u+v*v;
    result[0] = ro;
    result[1] = ro * u;
    result[2] = ro * v;
    result[3] = p/(gama-1.0) +  0.5 * ro * vel2;
    return;
  }
}

//----------------------------------------------------------------------------------------------
TPZMaterial *NoveCubos(int grau){
  //Problema teste do Cedric <=> problema teste no paper A. Coutinho e paper Zhang, Yu, Chang levado para 3D
  // e teste no paper de Peyrard and Villedieu
   CriacaoDeNos(30,novecubos);
  //elemento de volume
  TPZVec<int> nodes;
  int INCID[9][8] = {{0,1,6,5,15,16,21,20},{1,2,7,6,16,17,22,21},{2,3,10,9,17,18,25,24},{3,4,11,10,18,19,26,25},
		     {5,6,7,12,20,21,22,27},{7,2,9,8,22,17,24,23},{10,11,14,9,25,26,29,24},{7,8,13,12,22,23,28,27},
		     {8,9,14,13,23,24,29,28}};
  nodes.Resize(8);
  TPZVec<TPZGeoEl *> elem(9);
  elem.Resize(9);
  int i,index;
  for(i=0;i<9;i++){
    nodes[0] = INCID[i][0];
    nodes[1] = INCID[i][1];
    nodes[2] = INCID[i][2];
    nodes[3] = INCID[i][3];
    nodes[4] = INCID[i][4];
    nodes[5] = INCID[i][5];
    nodes[6] = INCID[i][6];
    nodes[7] = INCID[i][7];
    elem[i] = gmesh->CreateGeoElement(ECube,nodes,1,index);
  }

  //elemento de volume descont�nuo
  TPZGeoElement<TPZShapeCube,TPZGeoCube,TPZRefCube>::SetCreateFunction(TPZCompElDisc::CreateDisc);
  //TPZGeoElement<TPZShapeTetra,TPZGeoTetrahedra,TPZRefTetrahedra>::SetCreateFunction(TPZCompElDisc::CreateDisc);
  //TPZGeoElement<TPZShapeTriang,TPZGeoTriangle,TPZRefTriangle>::SetCreateFunction(TPZCompElDisc::CreateDisc);
  TPZGeoElement<TPZShapeQuad,TPZGeoQuad,TPZRefQuad>::SetCreateFunction(TPZCompElDisc::CreateDisc);
  int interfdim = 2;
  TPZCompElDisc::gInterfaceDimension = interfdim;
  gmesh->BuildConnectivity2();
  int nummat = 1;
  char *artdiff = "LS";
  cout << "\nmain::Divisao Nivel final da malha ? : ";
  cin >> nivel;
  REAL cfl,delta_x,delta_t,delta,gama;//,maxflinha;

  cfl = ( 1./(2.0*(REAL)grau+1.0) );
  delta_x =  ( 1.0 / pow(2.0,(REAL)nivel) );
  delta_t = cfl*delta_x;//delta_t � <= que este valor
  delta =  (10./3.)*cfl*cfl - (2./3.)*cfl + 1./10.;
  gama = 1.4;
  cout << "\nDominio [0,1]x[0,1]"
       << "\nMax df/dx (desconhecido) = 1.0"
       << "\nCFL = " << cfl
       << "\ndelta otimo = " << delta
       << "\nDelta x = " << delta_x
       << "\ndelta t = " << delta_t
       << "\ndiffusao = " << artdiff << endl;
  
  int dim = 3;
  TPZMaterial *mat = (TPZEulerConsLaw *) new TPZEulerConsLaw(nummat,delta_t,gama,dim,artdiff);

  //((TPZConservationLaw *)mat)->SetIntegDegree(grau);
  mat->SetForcingFunction(Function);
  cmesh->InsertMaterialObject(mat);

  //condi��es de contorno  
  TPZBndCond *bc;
  TPZFMatrix val1(5,5,0.),val2(5,1,0.);

  //CC ARESTA INFERIOR : PAREDE
  val1.Zero();
  val2.Zero();
  TPZGeoElBC((TPZGeoEl  *)elem[0],20,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[1],20,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[2],20,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[3],20,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[4],20,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[5],20,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[6],20,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[7],20,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[8],20,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[0],25,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[1],25,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[2],25,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[3],25,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[4],25,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[5],25,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[6],25,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[7],25,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[8],25,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[0],21,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[1],21,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[2],21,-1,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[3],21,-1,*gmesh);
  bc = mat->CreateBC(-1,5,val1,val2);
  cmesh->InsertMaterialObject(bc);

  //CC ARESTA DIREITA : OUTFLOW
  val1.Zero();
  val2.Zero();
  TPZGeoElBC((TPZGeoEl  *)elem[3],22,-2,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[6],22,-2,*gmesh);
  bc = mat->CreateBC(-2,4,val1,val2);
  cmesh->InsertMaterialObject(bc);

  //CC ARESTA SUPERIOR : DIRICHLET
  val1.Zero();
  val2.Zero();
  REAL ro = 1.7;
  REAL u = 2.61934;
  REAL v = -0.50632;
  REAL w = 0.0;
  REAL p = 1.52819;
  REAL vel2 = u*u + v*v + w*w;
  val2(0,0) = ro;
  val2(1,0) = ro * u;
  val2(2,0) = ro * v;
  val2(3,0) = ro * w;
  val2(4,0) = p/(gama-1.0) + 0.5 * ro * vel2;
  TPZGeoElBC((TPZGeoEl  *)elem[7],23,-3,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[8],23,-3,*gmesh);
  bc = mat->CreateBC(-3,3,val1,val2);
  cmesh->InsertMaterialObject(bc); 

  //CC ARESTA ESQUERDA : INFLOW
  val1.Zero();
  val2.Zero();
  ro = 1.0;
  u = 2.9;
  v = 0.0;
  w = 0.0;
  p = 0.714286;
  vel2 = u*u+v*v+w*w;
  val2(0,0) = ro;
  val2(1,0) = ro * u;
  val2(2,0) = ro * v;
  val2(3,0) = ro * w;
  val2(4,0) = p/(gama-1.0) +  0.5 * ro * vel2;
  TPZGeoElBC((TPZGeoEl  *)elem[0],24,-4,*gmesh);
  TPZGeoElBC((TPZGeoEl  *)elem[4],24,-4,*gmesh);
  bc = mat->CreateBC(-4,3,val1,val2);
  cmesh->InsertMaterialObject(bc);

  cout << endl;
  cmesh->AutoBuild();
  return mat;
}

void SequenceDivide(int fat[100],int numbel){

  TPZVec<int> csub(0);
  int i,el;
  for(i=0;i<numbel;i++){
    cout << "\nId do elemento geometrico a dividir -> " << fat[i] << endl;
    cmesh->Divide(fat[i],csub,1);
  }
  cout << "Elementos divissiveis:\n";
  numbel = cmesh->ElementVec().NElements();
  for(el=0;el<numbel;el++) {
    TPZCompEl *cpel = cmesh->ElementVec()[el];
    if(cpel && cpel->Type() != 16){
      TPZGeoEl *gel = cpel->Reference();
      if(gel) cout << gel->Id() << ",";
    }
  }
}

void SequenceDivide2(){

  TPZVec<int> s(0),s2(0);
  cmesh->Divide(1,s,0);
  int niv = 0;
  if(0){
    while(niv++ < 4){
      cmesh->Divide(s[0],s2,0);
      cmesh->Divide(s[1],s2,0);
      cmesh->Divide(s[3],s2,0);
      cmesh->Divide(s[2],s,0);
    }
    niv = 0;
    cmesh->Divide(2,s,0);
    while(niv++ < 4){
      cmesh->Divide(s[0],s2,0);
      cmesh->Divide(s[3],s2,0);
      cmesh->Divide(s[1],s,0);
    }
  }
  if(0){
    int i;//novecubos
    for(i=0;i<9;i++) cmesh->Divide(i,s,0);
  }
  if(1){//1 cubos
    cmesh->Divide(0,s,0);
    cmesh->Divide(s[7],s,0);//6
    cmesh->Divide(s[5],s,0);//4
    cmesh->Divide(s[5],s,0);//4: essa combina��o da problemas
    //cmesh->Divide(s[4],s,0);
  }
}

void AgrupaList(TPZVec<int> &accumlist,int nivel,int &numaggl){
  //todo elemento deve ser agrupado nem que for para ele mesmo
  cout << "\n\nmain::AgrupaList para malha 2D\n\n";
  int nel = cmesh->NElements(),i;
  //n�o todo index � sub-elemento
  accumlist.Resize(nel,-1);
  for(i=0;i<nel;i++){
    TPZCompEl *cel = cmesh->ElementVec()[i];
    if(!cel) continue;
    TPZGeoEl *gel = cel->Reference();
    //agrupando elementos computacionais
    if(cel->Type() == 16) continue;//pula interface: fica -1 na lista
    TPZGeoEl *father = gel->Father();
    if(!father) continue;
    if(Nivel(father) != nivel) continue;
    int fatid = father->Id();
    accumlist[i] = fatid;
  }
  int j;
  //contagem dos elementos
  numaggl = 0;
  //interface tem entrada -1 em list
  TPZVec<int> list(nel,-1);
  for(i=0;i<nel;i++){
    int fat = accumlist[i];
    int num = list[i];
    if(fat > -1 && num == -1){
      list[i] = numaggl;
      for(j=i+1;j<nel;j++){
	int fat2 = accumlist[j];
	if(fat2 == fat)
	  list[j] = numaggl;
      }
      numaggl++;
    }
  }
  accumlist = list;//for(i=0;i<nel;i++) cout << list[i] << "\n";
}


//   TPZVec<int> list2(accumlist);
//   for(i=0;i<nel;i++){
//     for(j=i+1;j<nel;j++){
//       if(list2[i] > list2[j]){
// 	int aux = list2[i];
// 	list2[i] = list2[j];
// 	list2[j] = aux;
//       }
//     }    
//   }
