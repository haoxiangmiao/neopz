/**
 * @file
 * @brief Contains the implementation of the TPZStructMatrix methods. 
 */

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
#include "pzsubcmesh.h"
#include "pzanalysis.h"

#include "pzgnode.h"
#include "TPZTimer.h"
#include "TPZMTAssemble.h"

#include "pzcheckconsistency.h"
#include "pzmaterial.h"

using namespace std;

#include "pzlog.h"

#ifdef LOG4CXX
//static LoggerPtr logger(Logger::getLogger("pz.strmatrix.tpzstructmatrix"));
static LoggerPtr logger(Logger::getLogger("pz.strmatrix"));
static LoggerPtr loggerel(Logger::getLogger("pz.strmatrix.element"));
static LoggerPtr loggerel2(Logger::getLogger("pz.strmatrix.elementinterface"));
static LoggerPtr loggerelmat(Logger::getLogger("pz.strmatrix.elementmat"));
static LoggerPtr loggerCheck(Logger::getLogger("pz.checkconsistency"));
#endif

#ifdef CHECKCONSISTENCY
static TPZCheckConsistency stiffconsist("ElementStiff");
#endif

TPZStructMatrix::TPZStructMatrix(TPZCompMesh *mesh) : fMinEq(-1), fMaxEq(-1) {
	fMesh = mesh;
	TPZSubCompMesh *submesh = dynamic_cast<TPZSubCompMesh *> (mesh);
	if (submesh) {
		fOnlyInternal = true;
	}
	else {
		fOnlyInternal = false;
	}
	this->SetNumThreads(0);
}

TPZStructMatrix::TPZStructMatrix(TPZAutoPointer<TPZCompMesh> cmesh) : fCompMesh(cmesh), fMinEq(-1), fMaxEq(-1) {
	fMesh = cmesh.operator->();
	TPZSubCompMesh *submesh = dynamic_cast<TPZSubCompMesh *> (fMesh);
	if (submesh) {
		fOnlyInternal = true;
	}
	else {
		fOnlyInternal = false;
	}
	this->SetNumThreads(0);
}

TPZStructMatrix::TPZStructMatrix(const TPZStructMatrix &copy){
	fMesh = copy.fMesh;
	fMinEq = copy.fMinEq;
	fMaxEq = copy.fMaxEq;
	fMaterialIds = copy.fMaterialIds;
	fNumThreads = copy.fNumThreads;
	fOnlyInternal = copy.fOnlyInternal;
}

TPZStructMatrix::~TPZStructMatrix() {}

TPZMatrix<STATE> *TPZStructMatrix::Create() {
	cout << "TPZStructMatrix::Create should never be called\n";
	return 0;
}

TPZStructMatrix *TPZStructMatrix::Clone() {
	cout << "TPZStructMatrix::Clone should never be called\n";
	return 0;
}

void TPZStructMatrix::Assemble(TPZMatrix<STATE> & stiffness, TPZFMatrix<STATE> & rhs,TPZAutoPointer<TPZGuiInterface> guiInterface){
//	if(this->fNumThreads){//caravagio
//		this->MultiThread_Assemble(stiffness,rhs,guiInterface);
//	}
//	else{
		this->Serial_Assemble(stiffness,rhs,guiInterface);
//	}
}

void TPZStructMatrix::Assemble(TPZFMatrix<STATE> & rhs,TPZAutoPointer<TPZGuiInterface> guiInterface){
	if(this->fNumThreads){
		this->MultiThread_Assemble(rhs,guiInterface);
	}
	else{
		this->Serial_Assemble(rhs,guiInterface);
	}
}

void TPZStructMatrix::Serial_Assemble(TPZMatrix<STATE> & stiffness, TPZFMatrix<STATE> & rhs, TPZAutoPointer<TPZGuiInterface> guiInterface ){
	
	if(!fMesh){
		LOGPZ_ERROR(logger,"pthread_Assemble called without mesh")
		DebugStop();
	}
#ifdef LOG4CXX
	if(dynamic_cast<TPZSubCompMesh * >(fMesh))
	{
		std::stringstream sout;
		sout << "AllEig = {};";
		LOGPZ_DEBUG(loggerelmat,sout.str())
		
	}
#endif
	
	int iel;
	int nelem = fMesh->NElements();
	TPZElementMatrix ek(fMesh, TPZElementMatrix::EK),ef(fMesh, TPZElementMatrix::EF);
	bool globalresult = true;
	bool writereadresult = true;
	
	TPZTimer calcstiff("Computing the stiffness matrices");
	TPZTimer assemble("Assembling the stiffness matrices");
	TPZAdmChunkVector<TPZCompEl *> &elementvec = fMesh->ElementVec();
	
		TPZFileStream ElemMatrix, ElemRightSide;
//		ElemMatrix.OpenWrite("eksJoao620.dat");
//		ElemRightSide.OpenWrite("efsJoao620.dat");

	int count = 0;
	for(iel=0; iel < nelem; iel++) {
		TPZCompEl *el = elementvec[iel];
		if(!el) continue;
		int matidsize = fMaterialIds.size();
		if(matidsize){
			TPZMaterial * mat = el->Material();
			TPZSubCompMesh *submesh = dynamic_cast<TPZSubCompMesh *> (el);
			if (!mat)
			{
				if (!submesh) {
					continue;
				}
				else if(submesh->NeedsComputing(fMaterialIds) == false) continue;
			}
			else
			{
				int matid = mat->Id();
//                #ifdef LOG4CXX
//                {
//                    std::stringstream sout;
//                    sout << "matid = "<< matid << endl;
//                    LOGPZ_DEBUG(logger,sout.str())
//                }
//                #endif

				if (this->ShouldCompute(matid) == false) continue;//caravagio
			}
		}//if
//#ifdef LOG4CXX
//        {
//            std::stringstream sout;
//            sout << " saiu!!! " << endl;
//            LOGPZ_DEBUG(logger,sout.str())
//        }
//#endif

		
		count++;
		if(!(count%20))
		{
			std::cout << '*';
			std::cout.flush();
		}
		if(!(count%500))
		{
			std::cout << "\n";
		}
		calcstiff.start();
		
		el->CalcStiff(ek,ef);
		
		if(guiInterface) if(guiInterface->AmIKilled()){
			return;
		}
		
//#ifdef LOG4CXX
//		if(dynamic_cast<TPZSubCompMesh * >(fMesh))
//		{
//			std::stringstream objname;
//			objname << "Element" << iel;
//			std::string name = objname.str();
//			objname << " = ";
//			std::stringstream sout;
//			ek.fMat.Print(objname.str().c_str(),sout,EMathematicaInput);
//			sout << "AppendTo[AllEig,Eigenvalues[" << name << "]];";
//			
//			LOGPZ_DEBUG(loggerelmat,sout.str())
//			/*		  if(iel == 133)
//			 {
//			 std::stringstream sout2;
//			 el->Reference()->Print(sout2);
//			 el->Print(sout2);
//			 LOGPZ_DEBUG(logger,sout2.str())
//			 }
//			 */
//		}
//		
//#endif
		
//#ifdef CHECKCONSISTENCY
//		//extern TPZCheckConsistency stiffconsist("ElementStiff");
//		stiffconsist.SetOverWrite(true);
//		bool result;
//		result = stiffconsist.CheckObject(ek.fMat);
//		if(!result)
//		{
//			globalresult = false;
//			std::stringstream sout;
//			sout << "element " << iel << " computed differently";
//			LOGPZ_ERROR(loggerCheck,sout.str())
//		}
//		
//#endif
		
		calcstiff.stop();
		assemble.start();
		
		if(!el->HasDependency()) {
			ek.ComputeDestinationIndices();
			if(fMinEq != -1 || fMaxEq != -1)
			{
				FilterEquations(ek.fSourceIndex,ek.fDestinationIndex,fMinEq,fMaxEq);
			}
			stiffness.AddKel(ek.fMat,ek.fSourceIndex,ek.fDestinationIndex);
			rhs.AddFel(ef.fMat,ek.fSourceIndex,ek.fDestinationIndex);
//#ifdef LOG4CXX
//			if(loggerel->isDebugEnabled() && ! dynamic_cast<TPZSubCompMesh *>(fMesh))
//			{
//				std::stringstream sout;
//				sout << "Element index " << iel << std::endl;
//                TPZGeoEl *gel = el->Reference();
//                int nsides = gel->NSides();
//                TPZManVector<REAL> ksi(gel->Dimension()), co(3);
//                gel->CenterPoint(nsides-1, ksi);
//                gel->X(ksi, co);
//                sout << "center point " << co << std::endl;
////				sout << "Element stiffness matrix\n";
////				ek.fMat.Print("Element Stiffness Matrix", sout);
//				ek.Print(sout);
//				ef.Print(sout);
//				LOGPZ_DEBUG(loggerel,sout.str())
//			}
//#endif
		} else {
			// the element has dependent nodes
			ek.ApplyConstraints();
			ef.ApplyConstraints();
			ek.ComputeDestinationIndices();
			if(fMinEq != -1 || fMaxEq != -1)
			{
				FilterEquations(ek.fSourceIndex,ek.fDestinationIndex,fMinEq,fMaxEq);
			}
			stiffness.AddKel(ek.fConstrMat,ek.fSourceIndex,ek.fDestinationIndex);
			rhs.AddFel(ef.fConstrMat,ek.fSourceIndex,ek.fDestinationIndex);
//#ifdef LOG4CXX
//			if(loggerel->isDebugEnabled() && ! dynamic_cast<TPZSubCompMesh *>(fMesh))
//			{
//				std::stringstream sout;
//				ek.Print(sout);
//				ef.Print(sout);
//				LOGPZ_DEBUG(loggerel,sout.str())
//			}
//#endif
		}
		
		assemble.stop();
	}//fim for iel
	
	
	if(count > 20) std::cout << std::endl;
    
//#ifdef LOG4CXX
//    if(loggerCheck->isDebugEnabled())
//	{
//		std::stringstream sout;
//		sout << "The comparaison results are : consistency check " << globalresult << " write read check " << writereadresult;
//		stiffness.Print("Matriz de Rigidez: ",sout);
//		rhs.Print("Right Handside", sout);
//		LOGPZ_DEBUG(loggerCheck,sout.str())
//	}
//	
//#endif
	
}

void TPZStructMatrix::Serial_Assemble(TPZFMatrix<STATE> & rhs, TPZAutoPointer<TPZGuiInterface> guiInterface){
	
	int iel;
	int nelem = fMesh->NElements();
	
	TPZTimer calcresidual("Computing the residual vector");
	TPZTimer assemble("Assembling the residual vector");
	
	TPZAdmChunkVector<TPZCompEl *> &elementvec = fMesh->ElementVec();
	
	for(iel=0; iel < nelem; iel++) {
		TPZCompEl *el = elementvec[iel];
		if(!el) continue;
		
		TPZMaterial * mat = el->Material();
		if (!mat) continue;
		int matid = mat->Id();
		if (this->ShouldCompute(matid) == false) continue;
		
		TPZElementMatrix ef(fMesh, TPZElementMatrix::EF);
		
		calcresidual.start();
		
		el->CalcResidual(ef);
		
		calcresidual.stop();
		
		assemble.start();
		
		if(!el->HasDependency()) {
			ef.ComputeDestinationIndices();
			if(fMinEq != -1 & fMaxEq != -1)
			{
				FilterEquations(ef.fSourceIndex,ef.fDestinationIndex,fMinEq,fMaxEq);
			}
			rhs.AddFel(ef.fMat, ef.fSourceIndex, ef.fDestinationIndex);
		} else {
			// the element has dependent nodes
			ef.ApplyConstraints();
			ef.ComputeDestinationIndices();
			if(fMinEq != -1 & fMaxEq != -1)
			{
				FilterEquations(ef.fSourceIndex,ef.fDestinationIndex,fMinEq,fMaxEq);
			}
			rhs.AddFel(ef.fConstrMat,ef.fSourceIndex,ef.fDestinationIndex);
		}
		
		assemble.stop();
		
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

void TPZStructMatrix::MultiThread_Assemble(TPZMatrix<STATE> & mat, TPZFMatrix<STATE> & rhs, TPZAutoPointer<TPZGuiInterface> guiInterface)
{
	ThreadData threaddata(*fMesh,mat,rhs,fMinEq,fMaxEq,fMaterialIds,guiInterface);
	const int numthreads = this->fNumThreads;
	TPZVec<pthread_t> allthreads(numthreads);
	int itr;
	if(guiInterface){
		if(guiInterface->AmIKilled()){
			return;
		}
	}
	for(itr=0; itr<numthreads; itr++)
	{
		pthread_create(&allthreads[itr], NULL,ThreadData::ThreadWork, &threaddata);
	}
	
	ThreadData::ThreadAssembly(&threaddata);
	
	for(itr=0; itr<numthreads; itr++)
	{
		pthread_join(allthreads[itr],NULL);
	}
	
}

void TPZStructMatrix::MultiThread_Assemble(TPZFMatrix<STATE> & rhs,TPZAutoPointer<TPZGuiInterface> guiInterface)
{
	//please implement me
	this->Serial_Assemble(rhs, guiInterface);
}

/// filter out the equations which are out of the range
void TPZStructMatrix::FilterEquations(TPZVec<int> &origindex, TPZVec<int> &destindex, int fMinEq, int upeq)
{
	if(fMinEq == -1 || upeq == -1) return;
	int count = 0;
	int nel = origindex.NElements();
	int i;
	for(i=0; i<nel; i++)
	{
		if(destindex[i] >= fMinEq && destindex[i] < upeq)
		{
			origindex[count] = origindex[i];
			destindex[count++] = destindex[i]-fMinEq;
		}
	}
	origindex.Resize(count);
	destindex.Resize(count);
	
}

TPZMatrix<STATE> * TPZStructMatrix::CreateAssemble(TPZFMatrix<STATE> &rhs, TPZAutoPointer<TPZGuiInterface> guiInterface)
{
	TPZMatrix<STATE> *stiff = Create();
	int neq = stiff->Rows();
	rhs.Redim(neq,1);
    Assemble(*stiff,rhs,guiInterface);
	
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

TPZStructMatrix::ThreadData::ThreadData(TPZCompMesh &mesh, TPZMatrix<STATE> &mat,
										TPZFMatrix<STATE> &rhs, int mineq, int maxeq,
										std::set<int> &MaterialIds,
										TPZAutoPointer<TPZGuiInterface> guiInterface)
: fMesh(&mesh),
fGuiInterface(guiInterface),
fGlobMatrix(&mat), fGlobRhs(&rhs),
fMinEq(mineq), fMaxEq(maxeq),
fNextElement(0)
{
	
	fMaterialIds = MaterialIds;
	pthread_mutex_init(&fAccessElement,NULL);
	/*	sem_t *sem_open( ... );
	 int sem_close(sem_t *sem);
	 int sem_unlink(const char *name);
	 */	
/*
#ifdef MACOSX
	std::stringstream sout;
	static int counter = 0;
	sout << "AssemblySem" << counter++;
	fAssembly = sem_open(sout.str().c_str(), O_CREAT,777,1);
	if(fAssembly == SEM_FAILED)
	{
		std::cout << __PRETTY_FUNCTION__ << " could not open the semaphore\n";
        DebugStop();
	}
#else
	int sem_result = sem_init(&fAssembly,0,0);
	if(sem_result != 0)
	{
		std::cout << __PRETTY_FUNCTION__ << " could not open the semaphore\n";
	}
#endif
 */
}

TPZStructMatrix::ThreadData::~ThreadData()
{
	pthread_mutex_destroy(&fAccessElement);
/*
#ifdef MACOSX
	sem_close(fAssembly);
#else
	sem_destroy(&fAssembly);
#endif
 */
}

void *TPZStructMatrix::ThreadData::ThreadWork(void *datavoid)
{
	ThreadData *data = (ThreadData *) datavoid;
	// compute the next element (this method is threadsafe)
	int iel = data->NextElement();
	TPZCompMesh *cmesh = data->fMesh;
	TPZAutoPointer<TPZGuiInterface> guiInterface = data->fGuiInterface;
	int nel = cmesh->NElements();
	while(iel < nel)
	{
		
		TPZAutoPointer<TPZElementMatrix> ek = new TPZElementMatrix(cmesh,TPZElementMatrix::EK);
		TPZAutoPointer<TPZElementMatrix> ef = new TPZElementMatrix(cmesh,TPZElementMatrix::EF);
		
		TPZCompEl *el = cmesh->ElementVec()[iel];
		TPZElementMatrix *ekp = ek.operator->();
		TPZElementMatrix *efp = ef.operator->();
		TPZElementMatrix &ekr = *ekp;
		TPZElementMatrix &efr = *efp;
		el->CalcStiff(ekr,efr);
		
		if(guiInterface) if(guiInterface->AmIKilled()){
			break;
		}
		
		if(!el->HasDependency()) {
			ek->ComputeDestinationIndices();
			
			if(data->fMinEq != -1 || data->fMaxEq != -1)
			{
				FilterEquations(ek->fSourceIndex,ek->fDestinationIndex,data->fMinEq,data->fMaxEq);
			}
#ifdef LOG4CXX
			if(loggerel->isDebugEnabled())
			{
				std::stringstream sout;
                sout << "Element index " << iel << std::endl;
				ek->fMat.Print("Element stiffness matrix",sout);
				ef->fMat.Print("Element right hand side", sout);
				LOGPZ_DEBUG(loggerel,sout.str())
			}
#endif
		} else {
			// the element has dependent nodes
			ek->ApplyConstraints();
			ef->ApplyConstraints();
			ek->ComputeDestinationIndices();
			if(data->fMinEq != -1 || data->fMaxEq != -1)
			{
				FilterEquations(ek->fSourceIndex,ek->fDestinationIndex,data->fMinEq,data->fMaxEq);
			}
#ifdef LOG4CXX
			if(loggerel2->isDebugEnabled() && el->Reference() &&  el->Reference()->MaterialId() == 1 && el->IsInterface())
			{
				std::stringstream sout;
				el->Reference()->Print(sout);
				el->Print(sout);
				ek->Print(sout);
				//			ef->Print(sout);
				LOGPZ_DEBUG(loggerel2,sout.str())
			}
#endif
#ifdef LOG4CXX
			if(loggerel->isDebugEnabled())
			{
				std::stringstream sout;
                sout << "Element index " << iel << std::endl;
				ek->fConstrMat.Print("Element stiffness matrix",sout);
				ef->fConstrMat.Print("Element right hand side", sout);
				LOGPZ_DEBUG(loggerel,sout.str())
			}
#endif
		}
		
		
		// put the elementmatrices on the stack to be assembled (threadsafe)
		data->ComputedElementMatrix(iel,ek,ef);
		// compute the next element (this method is threadsafe)
		iel = data->NextElement();
	}
	pthread_mutex_lock(&data->fAccessElement);
    data->fAssembly.Post();
    /*
#ifdef MACOSX
	sem_post(data->fAssembly);
#else
	sem_post(&data->fAssembly);
#endif
     */
	pthread_mutex_unlock(&data->fAccessElement);	
	
	return 0;
}

// The function which will compute the assembly
void *TPZStructMatrix::ThreadData::ThreadAssembly(void *threaddata)
{
	ThreadData *data = (ThreadData *) threaddata;
	TPZCompMesh *cmesh = data->fMesh;
	TPZAutoPointer<TPZGuiInterface> guiInterface = data->fGuiInterface;
	int nel = cmesh->NElements();
	pthread_mutex_lock(&(data->fAccessElement));
	int nextel = data->fNextElement;
	int numprocessed = data->fProcessed.size();
	bool globalresult = true;
	while(nextel < nel || numprocessed)
	{
		if(guiInterface) if(guiInterface->AmIKilled()){
			break;
		}
		std::map<int, std::pair< TPZAutoPointer<TPZElementMatrix>, TPZAutoPointer<TPZElementMatrix> > >::iterator itavail;
		std::set<int>::iterator itprocess;
		bool keeplooking = false;
		if(data->fSubmitted.size() && data->fProcessed.size())
		{
			itavail = data->fSubmitted.begin();
			itprocess = data->fProcessed.begin();
			if(itavail->first == *itprocess)
			{
				// make sure we come back to look for one more element
				keeplooking = true;
				// Get a hold of the data
				int iel = *itprocess;
				data->fProcessed.erase(itprocess);
				TPZAutoPointer<TPZElementMatrix> ek = itavail->second.first;
				TPZAutoPointer<TPZElementMatrix> ef = itavail->second.second;
				data->fSubmitted.erase(itavail);
#ifdef LOG4CXX
				std::stringstream sout;
				sout << "Assembling element " << iel;
				LOGPZ_DEBUG(logger,sout.str())
#endif
#ifdef CHECKCONSISTENCY
				//static TPZCheckConsistency stiffconsist("ElementStiff");
				stiffconsist.SetOverWrite(true);
				bool result;
				result = stiffconsist.CheckObject(ek->fMat);
				if(!result)
				{
					globalresult = false;
					std::stringstream sout;
					sout << "element " << iel << " computed differently";
					LOGPZ_ERROR(loggerCheck,sout.str())
				}
#endif
				
				// Release the mutex
				pthread_mutex_unlock(&data->fAccessElement);
				// Assemble the matrix
				if(!ek->HasDependency())
				{
					data->fGlobMatrix->AddKel(ek->fMat,ek->fSourceIndex,ek->fDestinationIndex);
					data->fGlobRhs->AddFel(ef->fMat,ek->fSourceIndex,ek->fDestinationIndex);				
				}
				else
				{
					data->fGlobMatrix->AddKel(ek->fConstrMat,ek->fSourceIndex,ek->fDestinationIndex);
					data->fGlobRhs->AddFel(ef->fConstrMat,ek->fSourceIndex,ek->fDestinationIndex);				
				}
				// acquire the mutex
				pthread_mutex_lock(&data->fAccessElement);
			}
		}
		if(!keeplooking)
		{
			pthread_mutex_unlock(&data->fAccessElement);
			LOGPZ_DEBUG(logger,"Going to sleep within assembly")
			// wait for a signal
            data->fAssembly.Wait();
            /*
#ifdef MACOSX
			sem_wait(data->fAssembly);
#else
			sem_wait(&data->fAssembly);
#endif
             */
			LOGPZ_DEBUG(logger,"Waking up for assembly")
			pthread_mutex_lock(&data->fAccessElement);
		}
		nextel = data->fNextElement;
		numprocessed = data->fProcessed.size();
		
	}
	//	std::cout << std::endl;
	{
		std::stringstream sout;
		sout << "nextel = " << nextel << " numprocessed = " << numprocessed << " submitted " << data->fSubmitted.size() << std::endl;
		sout << "The comparaison results are : consistency check " << globalresult;
		LOGPZ_DEBUG(loggerCheck,sout.str())
	}
	pthread_mutex_unlock(&data->fAccessElement);
	return 0;	
}		

int TPZStructMatrix::ThreadData::NextElement()
{
	pthread_mutex_lock(&fAccessElement);
	int iel;
	int nextel = fNextElement;
	TPZCompMesh *cmesh = fMesh;
	TPZAdmChunkVector<TPZCompEl *> &elementvec = cmesh->ElementVec();
	int nel = elementvec.NElements();
	for(iel=fNextElement; iel < nel; iel++)
	{
		TPZCompEl *el = elementvec[iel];
		if(!el) continue;
		if(fMaterialIds.size() == 0) break;
		TPZMaterial * mat = el->Material();
		TPZSubCompMesh *submesh = dynamic_cast<TPZSubCompMesh *> (el);
		if(!mat)
		{
			if(!submesh)
			{
				continue;
			}
			else if(submesh->NeedsComputing(fMaterialIds) == false) continue;
		}
		else 
		{
			int matid = mat->Id();
			if(this->ShouldCompute(matid) == false) continue;
		}
		break;
	}
	fNextElement = iel+1;
	nextel = iel;
	if(iel<nel) fProcessed.insert(iel);
	pthread_mutex_unlock(&fAccessElement);
#ifdef LOG4CXX
	{
		std::stringstream sout;
		sout << __PRETTY_FUNCTION__ << " returning " << nextel << " fNextElement " << fNextElement;
		LOGPZ_DEBUG(logger,sout.str())
	}
#endif
	return nextel;
}

// put the computed element matrices in the map
void TPZStructMatrix::ThreadData::ComputedElementMatrix(int iel, TPZAutoPointer<TPZElementMatrix> &ek, TPZAutoPointer<TPZElementMatrix> &ef)
{
	pthread_mutex_lock(&fAccessElement);
	std::pair< TPZAutoPointer<TPZElementMatrix>, TPZAutoPointer<TPZElementMatrix> > el(ek,ef);
	fSubmitted[iel] = el;
    fAssembly.Post();
    /*
#ifdef MACOSX
	sem_post(fAssembly);
#else
	sem_post(&fAssembly);
#endif
     */
	pthread_mutex_unlock(&fAccessElement);	
	
}

/// Set the set of material ids which will be considered when assembling the system
void TPZStructMatrix::SetMaterialIds(const std::set<int> &materialids)
{
	fMaterialIds = materialids;
#ifdef LOG4CXX
	{
		std::set<int>::const_iterator it;
		std::stringstream sout;
		sout << "setting input material ids ";
		for(it=materialids.begin(); it!= materialids.end(); it++)
		{
			sout << *it << " ";
		}
		LOGPZ_DEBUG(logger,sout.str())
	}
#endif
	if(!fMesh)
	{
		LOGPZ_WARN(logger,"SetMaterialIds called without mesh")
		return;
	}
	int iel;
	TPZAdmChunkVector<TPZCompEl*> &elvec = fMesh->ElementVec();
	int nel = elvec.NElements();
	for(iel=0; iel<nel; iel++)
	{
		TPZCompEl *cel = elvec[iel];
		if(!cel) continue;
		TPZSubCompMesh *subcmesh = dynamic_cast<TPZSubCompMesh *> (cel);
		if(!subcmesh) continue;
		TPZAutoPointer<TPZAnalysis> anal = subcmesh->Analysis();
		if(!anal)
		{
			LOGPZ_ERROR(logger,"SetMaterialIds called for substructure without analysis object")
			DebugStop();
		}
		TPZAutoPointer<TPZStructMatrix> str = anal->StructMatrix();
		if(!str)
		{
			LOGPZ_WARN(logger,"SetMaterialIds called for substructure without structural matrix")
			continue;
		}
		str->SetMaterialIds(materialids);
	}
}

void UniformRefine(int num, TPZGeoMesh &m){
	
	int ref;
	for(ref=0; ref< num; ref++) {
        cout << "Refinement " << ref << endl;
        cout.flush();
		int nelem = m.ElementVec().NElements();
		TPZVec<TPZGeoEl*> subel;
		int iel;
		cout << "Element ";
		for(iel=0; iel<nelem; iel++) {
			if(iel%500==0){
				cout << iel << " ";
				cout.flush();
			}
			if(iel%5000==0 && iel){
				cout << endl;
				cout.flush();
			}
			TPZGeoEl *gel = m.ElementVec()[iel];
			if(!gel) continue;
			gel->Divide(subel);
		}
        cout << endl;
	}
}

