/* Generated by Together */

#ifndef TPZPARFRONTSTRUCTMATRIX_H
#define TPZPARFRONTSTRUCTMATRIX_H
#include "TPZFrontStructMatrix.h"
#include "pzstrmatrix.h"
#include "pzcmesh.h" 

#include "TPZFrontMatrix.h"

#include "TPZFrontNonSym.h"
#include "TPZFrontSym.h"

#include "pzelmat.h"


#include <signal.h>
#include <time.h>

//#ifndef PZPAR
#include <pthread.h>
//#endif

class TPZElementMatrix;


class TPZMatrix;
class TPZFMatrix;
class TPZCompMesh;
class TPZFileEqnStorage;

template<class front>
/**
 * TPZParFrontStructMatrix is derived fron TPZFrontStructMatrix. \n
 * Is a Structural matrix with parallel techniques included
 * It uses TPZParFrontMatrix as its FrontalMatrix
 * @ingroup frontal structural
 */
class TPZParFrontStructMatrix : public TPZFrontStructMatrix<front> {
public:     

     /**
      * Sets number of threads to be used in frontal process
      */
     void SetNumberOfThreads(
          int nthreads //! Number of threads to be used
          );

     //Virtual function must return same type
     /**
      * It clones a TPZStructMatrix
      */
     TPZStructMatrix *Clone();
     /**
      * Constructor passing as parameter a TPZCompMesh
      */
     TPZParFrontStructMatrix(
          TPZCompMesh *mesh //! Mesh to refer to
          );
     /**
      * Returns a poniter to TPZMatrix
      */
     TPZMatrix * CreateAssemble(
          TPZFMatrix &rhs //! Load matrix
          );
     
     
     virtual void Assemble(TPZMatrix & mat, TPZFMatrix & rhs);

     /** Used only for testing */
     static int main();
     
     /**
      * It computes element matrices in an independent thread. \n
      * It is passed as a parameter to the  pthread_create() function. \n
      * It is a 'static void *' to be used by pthread_create
      */
     static void *ElementAssemble(void *t);
     /**
      * It assembles element matrices in the global stiffness matrix, it is also executed in an independent thread. \n
      * It is passed as a parameter to the  pthread_create() function. \n
      * It is a 'static void *' to be used by pthread_create
      */
     static void *GlobalAssemble(void *t);
     /**
      * It writes decomposed equations to a binary file on disk. It is executed in an independent thread. \n
      * It is passed as a parameter to the  pthread_create() function. \n
      * It is a 'static void *' to be used by pthread_create
     static void *WriteFile(void *t);*/
     
private:
     /**
      * Number of threads used in the process. \n
      * It needs at least three independet threads to execute:\n
          *ElementAssemble\n
          *GlobalAssemble\n
          *WriteFile\n
      */  
     int fNThreads;
     /**Current computed element*/
     int fCurrentElement;
     /**Current assembled element in the global stiffness matrix*/
     int fCurrentAssembled;
     /**Total number of elements*/
     int fNElements;
     /**
      * Maximum stack size allowed. \n
      * Whenever this value is reached a execution of element computing is suspended
      */
     int fMaxStackSize;
     /**Local pointer to stiffness matrix*/
     TPZMatrix * fStiffness;
     /**Local pointer to load matrix*/
     TPZFMatrix * fRhs;

     /**
      * Stack containing elements to be assembled on Stiffness matrix. \n
      * ElemenAssemble pushes elements on the stack. \n
      * GlobalAssemble pops elements from the stack.
      */
     TPZStack <int> felnum;
     TPZStack <TPZElementMatrix *> fekstack;
     TPZStack <TPZElementMatrix *> fefstack;

};
#endif //TPZPARFRONTSTRUCTMATRIX_H
