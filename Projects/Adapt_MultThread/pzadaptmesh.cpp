/* Generated by Together */

#include "pzadaptmesh.h"
#include "pzgclonemesh.h"
#include "pzcclonemesh.h"
#include "pzgeoel.h"
#include "pzcompel.h"
#include "pzfstrmatrix.h"
#include "pzstepsolver.h"
#include "pzintel.h"
#include "pzquad.h"
#include "pzonedref.h"
#include "pzcheckmesh.h"
//multithread -->>
#include <pthread.h>
#include <signal.h>

int TPZAdaptMesh::fThreads_in_use = 0;
int TPZAdaptMesh::fNClones_to_Analyse = 0;
pthread_cond_t TPZAdaptMesh::fSignal_free = PTHREAD_COND_INITIALIZER;
pthread_mutex_t TPZAdaptMesh::fLock_clindex = PTHREAD_MUTEX_INITIALIZER;

TPZAdaptMesh::TPZAdaptMesh(){
  fReference = 0;
  fGeoRef.Resize(0);
  fPatch.Resize(0);
  fPatchIndex.Resize(0);
  fElementError.Resize(0);
  fCloneMesh.Resize(0);
  fFineCloneMesh.Resize(0);
  fMaxP = 10;

  //changed to use multi-threading
  fTrueErrorVec.Resize(0);
  fElementError.Resize(0);
  fExact = 0;
  fClonestoAnalyse.Resize(0);
}

TPZAdaptMesh::~TPZAdaptMesh(){
  CleanUp();
}

void TPZAdaptMesh::SetCompMesh(TPZCompMesh * mesh){
  if(!mesh){
    cout <<"TPZAdaptMesh::Error:\n computational reference mesh must not be NULL!\n";
    return;
  }
  CleanUp();
  //  fReference = mesh;
  fReference = mesh;
  int nel = fReference->ElementVec().NElements();
  fElementError.Resize(nel);
}

void TPZAdaptMesh::SetMaxP(int maxp){
  if (maxp < 1) {
    cout << "TPZAdaptMesh::Error : SetMaxP - maximum p order must be greter than 0... trying to set maximum p to " 
	 << maxp << endl;
    return;
  }
  fMaxP = maxp;
  TPZOneDRef::gMaxP = fMaxP;
}

void TPZAdaptMesh::CleanUp(){
  int i;
  for (i=0;i<fCloneMesh.NElements();i++){
    TPZGeoCloneMesh *gmesh = dynamic_cast<TPZGeoCloneMesh *> (fCloneMesh[i]->Reference());
    gmesh->ResetReference();
    //Cesar July 2003 ->
    //If some reference element is not used to analyse error its fine clone mesh is not created!
    if (fFineCloneMesh[i]){
      fFineCloneMesh[i]->LoadReferences();
      RemoveCloneBC(fFineCloneMesh[i]);
      DeleteElements(fFineCloneMesh[i]);
      delete fFineCloneMesh[i];
    }
    //<-
    fCloneMesh[i]->LoadReferences();
    RemoveCloneBC(fCloneMesh[i]);
    DeleteElements(fCloneMesh[i]);
    delete fCloneMesh[i];
    delete gmesh;
  }
  fCloneMesh.Resize(0);
  fFineCloneMesh.Resize(0);
  fTrueErrorVec.Resize(0);
  fElementError.Resize(0);


  /*  int i;
  for (i=0;i<fCloneMesh.NElements();i++){
    TPZGeoMesh *gmesh =  fCloneMesh[i]->Reference();
	gmesh->ResetReference();
	fFineCloneMesh[i]->LoadReferences();
	this->RemoveCloneBC(fFineCloneMesh[i]);
	DeleteElements(fFineCloneMesh[i]);
	delete fFineCloneMesh[i];
	gmesh->ResetReference();
	fCloneMesh[i]->LoadReferences();
	RemoveCloneBC(fCloneMesh[i]);
	DeleteElements(fCloneMesh[i]);
    delete fCloneMesh[i];
    delete gmesh;
  }
  fCloneMesh.Resize(0);
  fFineCloneMesh.Resize(0);
  */
}

TPZCompMesh * TPZAdaptMesh::GetAdaptedMesh(REAL &error, REAL & truerror, TPZVec<REAL> &ervec, 
					   void(*f)(TPZVec<REAL> &loc, TPZVec<REAL> &val, TPZFMatrix &deriv),
					   TPZVec<REAL> &truervec, 
					   TPZVec<REAL> &effect,int use_trueerror){
  //gets the geometric reference elements that will generate the patch
  GetReferenceElements();
  //  int ngrp = fGeoRef.NElements();
  int i;

  //Generates the patch
  BuildReferencePatch();

  //Creates the patch clones;
  fReference->ComputeNodElCon();

  int printing = 0;
  if(printing) {
	  ofstream test("test.txt",ios::app);
	  fReference->Print(test);
	  fReference->Reference()->Print(test);
  }
  CreateClones();

  //clone analysis
  int cliter;
  int ncl = fCloneMesh.NElements();
  fFineCloneMesh.Resize(ncl);
  int nelmesh = fReference->ElementVec().NElements();
  fElementError.Resize(nelmesh);
  fTrueErrorVec.Resize(nelmesh);
  
  effect.Resize(nelmesh);
  truervec.Resize(nelmesh);
  ervec.Resize(nelmesh);
  ervec.Fill(0.);
  truervec.Fill(0.);
  effect.Fill(0.);
  fElementError.Fill(0.);
  fTrueErrorVec.Fill(0.);
  if (f) fExact = f;
  else fExact = 0;
  //Used to evaluate the error when the true solution is provided======
  TPZVec<int> perm(nelmesh,0);
  TPZVec<REAL> auxerrorvec(nelmesh,0.);
  REAL minerror = 0.;
  //===================================================================
  if (use_trueerror && f){
    for (i=0;i<fReference->ElementVec().NElements();i++){
      TPZInterpolatedElement *el = dynamic_cast<TPZInterpolatedElement *> (fReference->ElementVec()[i]);
      if (el) 
	auxerrorvec[i] = 0.;//UseTrueError (el,f);
      else
	auxerrorvec[i] = 0.;
    }
    minerror = 0.;//SortMinError(auxerrorvec,perm,0.65);
    cout << "MinError " << minerror << endl;
  } 

  //Creates an uniformly refined mesh and evaluates the error
  for (cliter = ncl-1; cliter>=0; cliter--){
    //Análise dos Clones
      //    if (gcmesh->ReferenceElement(0)->MaterialId() <  0 || (use_trueerror && f && !HasTrueError(cliter,minerror,auxerrorvec))){
      
    //    fFineCloneMesh [cliter] = fCloneMesh[cliter]->UniformlyRefineMesh();
//     {
//       ofstream out("output.txt");
//       fCloneMesh[cliter]->Print(out);
//       out.close();
//     }
//    fCloneMesh[cliter]->MeshError(fFineCloneMesh[cliter],fElementError,f,truervec);  
    //    fCloneMesh[cliter]->MeshError(fCloneMesh[cliter],fElementError,f,truervec);  
     fClonestoAnalyse.Push(cliter);
}

  int nthreads = 0;
  fNClones_to_Analyse = fClonestoAnalyse.NElements();
  pthread_t *allthreads = new pthread_t[(const int)fNClones_to_Analyse];
  while (fNClones_to_Analyse) {
    if (fThreads_in_use <=  nthreads){
      pthread_mutex_lock(&fLock_clindex);
      int thr_index = fNClones_to_Analyse-1;
      fNClones_to_Analyse --;
      fThreads_in_use ++;
      if (pthread_create(&allthreads[thr_index],NULL,(void*(*)(void*))TPZAdaptMesh::MeshError,this)){
 	cout << "TPZAdaptMesh::GetAdaptMesh : Error on thread creation... exiting\n";
 	exit (-1);
      }
      pthread_cond_wait (&fSignal_free,&fLock_clindex);
      pthread_mutex_unlock(&fLock_clindex);
     }
   }

  for(i=0;i<fClonestoAnalyse.NElements();i++){
    pthread_join(allthreads[i],0);
  }
  delete allthreads;
  truervec = fTrueErrorVec;

  //Ordena o vetor de erros
  int nelem = fElementError.NElements();
  //  TPZVec<int> perm(nelem,0);
  for(i=0; i<nelem; i++) {
    perm[i] = i;
    ervec[i]=fElementError[i];
  }
  Sort(fElementError,perm);
  
  //  REAL totalerror = 0.;
  //  REAL totaltruerror = 0.;
  //somatório dos componentes do vetor de erro
  for(i=0; i<nelmesh; i++) {
    error += fElementError[i];
    cout << fElementError[i] <<  "\t";
  }
  cout << "\nTPZAdaptMesh::GetAdaptMesh : Error = " << error << endl;

  REAL ninetyfivepercent,auxerror = 0.;
  for(i=0;i<nelem;i++){
    auxerror += fElementError[perm[i]];
    if (auxerror >= 0.65*error){
      ninetyfivepercent = fElementError[perm[i]];
      break;
    }
  }

  if(f) {
    for(i=0; i<nelem; i++){
      truerror += truervec[i];
    }
  }
  
  //inicializa effect com o tamanho de trueeerror
  effect.Resize(truervec.NElements());
  effect.Fill(0.);
  if(f) {
    for(i=0; i<nelem; i++) {
      if(truervec[i] >= 1.e-4*truerror && truervec[i] >= 5e-20 ) {
	effect[i] = ervec[i]/truervec[i];
      }
      else {
	effect[i]=0.;
	truervec[i]=0.;
      }
    }
  }
  //  int nstate = fCloneMesh[0]->MaterialVec()[0]->NStateVariables();
  
  TPZStack <TPZGeoEl*> gelstack;
  TPZStack <int> porder;
  int ibc;
  for (ibc=0;ibc<fReference->ElementVec().NElements();ibc++){
    TPZCompEl *cel = fReference->ElementVec()[ibc];
    if(!cel) continue;
    TPZInterpolatedElement *cintel = dynamic_cast<TPZInterpolatedElement *> (cel);
    int matid =cintel->Material()->Id();
    if ( matid < 0 && matid != -1000){
      int cintorder = cintel->PreferredSideOrder(cintel->NConnects() -1);
      gelstack.Push(cintel->Reference());
      porder.Push(cintorder);      
    }
  }
    
  //Analyse clone element error and, if necessary, analyse element and changes its refinement pattern
  for (i=0;i<fCloneMesh.NElements();i++){
    if (!fFineCloneMesh[i]) continue;
    fCloneMesh[i]->ApplyRefPattern(ninetyfivepercent,fElementError,fFineCloneMesh[i],gelstack,porder);
  }
  
/*   int igeo,ngeoel = gelstack.NElements(); */
/*   for (igeo =0; igeo<ngeoel; igeo++){ */
/*     gelstack[igeo]->Print(); */
/*   } */
  
  TPZCompMesh * adapted =   CreateCompMesh(fReference,gelstack,porder);
  return adapted;
}

void * TPZAdaptMesh::MeshError(void *t){
  TPZAdaptMesh *adapt = (TPZAdaptMesh *) (t);
  pthread_mutex_lock(&adapt->fLock_clindex);
  int cliter = adapt->fClonestoAnalyse[adapt->fNClones_to_Analyse];
//   TPZGeoCloneMesh *gcmesh = dynamic_cast<TPZGeoCloneMesh *> (adapt->fCloneMesh[cliter]->Reference());
//  adapt->fFineCloneMesh [cliter] = adapt->fCloneMesh[cliter]->UniformlyRefineMesh();
//   if (gcmesh->ReferenceElement(0)->MaterialId() <  0){
//     pthread_cond_signal (&adapt->fSignal_free);
//     pthread_mutex_unlock(&adapt->fLock_clindex);
//     pthread_mutex_lock(&adapt->fLock_clindex);
//     fThreads_in_use --;
//     pthread_mutex_unlock(&adapt->fLock_clindex);
//     return 0;
//   }
  pthread_cond_signal (&adapt->fSignal_free);
  pthread_mutex_unlock(&adapt->fLock_clindex);
  TPZGeoCloneMesh *gcmesh = dynamic_cast<TPZGeoCloneMesh *> (adapt->fCloneMesh[cliter]->Reference());
  if (gcmesh->ReferenceElement(0)->MaterialId() <  0){
    adapt->fFineCloneMesh[cliter] = 0;
  }
  else {
    adapt->fFineCloneMesh [cliter] = adapt->fCloneMesh[cliter]->UniformlyRefineMesh();
    adapt->fCloneMesh[cliter]->MeshError(adapt->fFineCloneMesh[cliter],
				       adapt->fElementError, 
				       adapt->fExact, 
				       adapt->fTrueErrorVec);
  }
  pthread_mutex_lock(&adapt->fLock_clindex);
  fThreads_in_use --;
  pthread_mutex_unlock(&adapt->fLock_clindex);
  return 0;
}

void TPZAdaptMesh::GetReferenceElements(){
  if (!fReference){
    cout << "TPZAdaptMesh::Error:\n computational mesh must be initialized to call GetReferenceElements!\n";
    return;
  }
  fReference->GetRefPatches(fGeoRef);

/*   int i; */
/*   int nel = fGeoRef.NElements(); */
/*   for (i=0;i<nel; i++){ */
/*     fGeoRef[i]->Print(cout); */
/*   } */

}

void TPZAdaptMesh::BuildReferencePatch(){
  TPZGeoMesh *gmesh = fReference->Reference();
  gmesh->ResetReference();
  TPZCompMesh *tmpcmesh = new TPZCompMesh (gmesh);
  int i,j;
  for (i=0;i<fGeoRef.NElements();i++){
    fGeoRef[i]->CreateCompEl(*tmpcmesh,i);
  } 
  tmpcmesh->CleanUpUnconnectedNodes();
  TPZStack <int> patchelindex;
  TPZStack <TPZGeoEl *> toclonegel;
  TPZVec<int> n2elgraph;
  TPZVec<int> n2elgraphid;
  TPZStack<int> elgraph;
  TPZVec<int> elgraphindex;
  tmpcmesh->GetNodeToElGraph(n2elgraph,n2elgraphid,elgraph,elgraphindex);
  int clnel = tmpcmesh->NElements();
  fPatchIndex.Push(0);
  for (i=0; i<clnel; i++){
    tmpcmesh->GetElementPatch(n2elgraph,n2elgraphid,elgraph,elgraphindex,i,patchelindex);
    for (j=0; j<patchelindex.NElements(); j++){
      TPZGeoEl *gel = tmpcmesh->ElementVec()[patchelindex[j]]->Reference();
      //      int count = 0;
      if(gel) fPatch.Push(gel);
    }
    int sum = fPatch.NElements();
    fPatchIndex.Push(sum);
  }
  gmesh->ResetReference();
  delete tmpcmesh;
  fReference->LoadReferences();
}

void TPZAdaptMesh::CreateClones(){
  fReference->Reference()->ResetReference();
  fReference->LoadReferences();
  TPZGeoMesh *geomesh = fReference->Reference();
  
  TPZStack<TPZGeoEl*> patch;
  
  int clid,elid;
  for (clid=0; clid<fPatchIndex.NElements()-1;clid++){
    TPZGeoCloneMesh *geoclone = new TPZGeoCloneMesh(geomesh);
    TPZStack<TPZGeoEl*> patch;
    for (elid=fPatchIndex[clid];elid<fPatchIndex[clid+1];elid++){
      patch.Push(fPatch[elid]);
    }
    geoclone->SetElements(patch,fGeoRef[clid]);
    TPZVec<TPZGeoEl *> sub;
    //    int ngcel = geoclone->ElementVec().NElements();
    int printing = 0;
    if(printing) {
      ofstream out("test.txt",ios::app);
      geoclone->Print(out);
    }
    TPZCompCloneMesh *clonecompmesh = new TPZCompCloneMesh(geoclone,fReference);
    clonecompmesh->AutoBuild();
    fCloneMesh.Push(clonecompmesh);    
  }
}


void TPZAdaptMesh::Sort(TPZVec<REAL> &vec, TPZVec<int> &perm) {
  int i,j;
  int imin = 0;
  int imax = vec.NElements();
  for(i=imin; i<imax; i++) {
    for(j=i+1; j<imax; j++) {
      if(vec[perm[i]] < vec[perm[j]]) {
	int kp = perm[i];
	perm[i] = perm[j];
	perm[j] = kp;
      }
    }
  }
}

void TPZAdaptMesh::HeapSort(TPZVec<REAL> &sol, TPZVec<int> &perm){
  int nelem = perm.NElements();
  int i,j;
  for(i=0; i<nelem; i++) perm[i] = i;
  
  if(nelem == 1) return;
  int l, ir,ind;
  REAL q;
  l= nelem/2;
  ir = nelem-1;
  while(l>0 && ir>0) {
    if(l> 0) {
      l--;
      ind = perm[l];
      q=sol[ind];
    } else {
      ind = perm[ir];
      q = sol[ind];
      perm[ir] = perm[0];
      ir--;
    }
    i=l;
    j=l+l+1;
    while(j<=ir) {
      if(j<ir && sol[perm[j]] < sol[perm[j+1]]) j++;
      if(q < sol[perm[j]]) {
	perm[i] = perm[j];
	i=j;
	j= i+i+1;
      } else {
	break;
      }
    }
    perm[i] = ind;
  }
}

TPZCompMesh *TPZAdaptMesh::CreateCompMesh (TPZCompMesh *mesh,             //malha a refinar
					   TPZVec<TPZGeoEl *> &gelstack,  //
					   TPZVec<int> &porders) {

  //Cria um ponteiro para a malha geométrica de mesh
  TPZGeoMesh *gmesh = mesh->Reference();
  if(!gmesh) {
    cout << "TPZAdaptMesh::CreateCompMesh encountered no geometric mesh\n";
    return 0;
  }

  //Reseta as referências do ponteiro para a malha geométrica criada
  //e cria uma nova malha computacional baseada nesta malha geométrica
  gmesh->ResetReference();
  TPZCompMesh *cmesh = new TPZCompMesh(gmesh);
  TPZCheckMesh check(cmesh,&cout);
  int nmat = mesh->MaterialVec().NElements();
  int m;

  //Cria um clone do vetor de materiais da malha mesh
  for(m=0; m<nmat; m++) {
    TPZMaterial *mat = mesh->MaterialVec()[m];
    if(!mat) continue;
    mat->Clone(cmesh->MaterialVec());
  }

  //Idenifica o vetor de elementos computacionais de mesh
  //  TPZAdmChunkVector<TPZCompEl *> &elementvec = mesh->ElementVec();
  
  int el,nelem = gelstack.NElements();
  //  cmesh->SetName("Antes PRefine");
  //  cmesh->Print(cout);

  for(el=0; el<nelem; el++) {
    //identifica os elementos geométricos passados em gelstack
    TPZGeoEl *gel = gelstack[el];
    if(!gel) {
      cout << "TPZAdaptMesh::CreateCompMesh encountered an null element\n";
      continue;
    }
    int celindex;
    //Cria um TPZIntel baseado no gel identificado
    int temporder = TPZCompEl::gOrder;
    TPZCompEl::gOrder = porders[el];
    TPZInterpolatedElement *csint;
    csint = dynamic_cast<TPZInterpolatedElement *> (gel->CreateCompEl(*cmesh,celindex));
    TPZCompEl::gOrder = temporder;
    if(check.CheckConnectOrderConsistency() != -1) {
      cout << "TPZAdaptMesh::CreateCompMesh mesh inconsistent\n";
    }
    
    /**PRefine precisar ser verificado!!!
       if(!csint) continue;
       //Refina em p o elemento criado
       //cmesh->SetName("depois criar elemento");
       //	cmesh->Print(cout);
       csint->PRefine(porders[el]);
       //	cmesh->SetName("depois prefine no elemento");
       //	cmesh->Print(cout);
       */
  }

  //Mais einh!!
  //	cmesh->SetName("Antes Adjust");
  //	cmesh->Print(cout);
  cmesh->AdjustBoundaryElements();
  //  cmesh->SetName("Depois");
  //  cmesh->Print(cout);
  if(check.CheckConnectOrderConsistency() != -1) {
    cout << "TPZAdaptMesh::CreateCompMesh mesh inconsistent\n";
  }
  return cmesh;
}

void TPZAdaptMesh::RemoveCloneBC(TPZCompMesh *mesh)
{
  int nelem = mesh->NElements();
  int iel;
  for(iel=0; iel<nelem; iel++) {
    TPZCompEl *cel = mesh->ElementVec()[iel];
    if(!cel) continue;
    int matid = cel->Material()->Id();
    if(matid == -1000) delete cel;
  }
}

void TPZAdaptMesh::DeleteElements(TPZCompMesh *mesh)
{
  int nelem = mesh->NElements();
  int iel;
  for(iel=0; iel<nelem; iel++) {
    TPZCompEl *cel = mesh->ElementVec()[iel];
    if(!cel) continue;
    TPZInterpolatedElement *cint = dynamic_cast<TPZInterpolatedElement *> (cel);
    if(!cint) continue;
    while(cint->HasDependency()) {
      TPZInterpolatedElement *large = LargeElement(cint);
      TPZInterpolatedElement *nextlarge = LargeElement(large);
      while(nextlarge != large) {
	large = nextlarge;
	nextlarge = LargeElement(large);
      }
      large->RemoveSideRestraintsII(TPZInterpolatedElement::EDelete);
      delete large;
    }
    cint->RemoveSideRestraintsII(TPZInterpolatedElement::EDelete);
    delete cint;
  }
}

TPZInterpolatedElement * TPZAdaptMesh::LargeElement(TPZInterpolatedElement *cint)
{
  int nc = cint->NConnects();
  int side;
  TPZInterpolatedElement *result = cint;
  for(side=0; side<nc; side++) {
    if(cint->Connect(side).HasDependency()) {
      TPZCompElSide cintside(cint,side);
      TPZCompElSide large = cintside.LowerLevelElementList(1);
      if(!large.Exists()) {
	cout << "TPZAdaptMesh::DeleteElements I dont understand\n";
	large = cintside.LowerLevelElementList(1);
				return cint;
      }
      result = dynamic_cast<TPZInterpolatedElement *> (large.Element());
      break;
    }
  }
  return result;
}
