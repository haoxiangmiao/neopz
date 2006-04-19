
//
// Author: PHILIPPE DEVLOO
//
// File:   pzblockdiag.c
//
// Class:  TPZBlockDiagonal
//
// Obs.:   Implements block diagonal matrices
//
// Versao: 01 - 2001
//


#include <string>
#include <math.h>
#include "pzfmatrix.h"
#include "pzblockdiag.h"

//#include "pzerror.h"
#include <stdlib.h>
#include <stdio.h>

#include <sstream>
#include "pzlog.h"
#ifdef LOG4CXX
static LoggerPtr logger(Logger::getLogger("pz.matrix.tpzblockdiagonal"));
#endif

#define IsZero( a )   ( (a) < 1.e-10 && (a) > -1.e-10 )
#define Max( a, b )   ( (a) > (b) ? (a) : (b) )
#define Min( a, b )   ( (a) < (b) ? (a) : (b) )

using namespace std;
static REAL zero =0.;

void TPZBlockDiagonal::AddBlock(int i, TPZFMatrix &block){
  // ::cout << "Iniciando inserc�o de bloco na posic�o\t" ;
  int firstpos = fBlockPos[i];
  //  ::cout << firstpos << "\n";
  int bsize = fBlockSize[i];
  //  ::cout << "Dimens�o do bloco a ser inserido\t" << bsize << "\n";
  
  int r,c;
  for(r=0; r<bsize; r++) {
    for(c=0; c<bsize; c++) {
      //  ::cout << " inserindo elemento: \t local[" << r <<"," << c
      //     << "]\t global [" << firstpos+r+bsize*c << "]= \t"
      //     << block(r,c) << "\t totalizando \t" <<fStorage[firstpos+r+bsize*c] + block(r,c)
      //     << "\n";
      fStorage[firstpos+r+bsize*c] += block(r,c);
    }
  }
}

void TPZBlockDiagonal::SetBlock(int i, TPZFMatrix &block){
  // ::cout << "Iniciando inserc�o de bloco na posic�o\t" ;
  int firstpos = fBlockPos[i];
  //  ::cout << firstpos << "\n";
  int bsize = fBlockSize[i];
  //  ::cout << "Dimens�o do bloco a ser inserido\t" << bsize << "\n";
  
  int r,c;
  for(r=0; r<bsize; r++) {
    for(c=0; c<bsize; c++) {
      //  ::cout << " inserindo elemento: \t local[" << r <<"," << c
      //     << "]\t global [" << firstpos+r+bsize*c << "]= \t"
      //     << block(r,c) << "\t totalizando \t" <<fStorage[firstpos+r+bsize*c] + block(r,c)
      //     << "\n";
      fStorage[firstpos+r+bsize*c] = block(r,c);
    }
  }
}

void TPZBlockDiagonal::GetBlock(int i, TPZFMatrix &block){
    int firstpos = fBlockPos[i];
    int bsize = fBlockSize[i];
    block.Redim(bsize,bsize);
    int r,c;
    for(r=0; r<bsize; r++) {
        for(c=0; c<bsize; c++) {
            block(r,c) = fStorage[firstpos+r+bsize*c];
        }
    }
}
void TPZBlockDiagonal::Initialize(const TPZVec<int> &blocksize){
  int nblock = blocksize.NElements();
#ifdef LOG4CXX
  {
    std::stringstream sout;
    sout << "N�mero de blocos \t" << nblock;
    LOGPZ_DEBUG(logger,sout.str());
  }
#endif
  //  ::cout << "N�mero de blocos \t" << nblock << "\n";
  fBlockSize = blocksize;
  fBlockPos.Resize(nblock+1,0); 
  int b;
  int ndata = 0;
  int neq = 0;
  int bsize;
  for(b=0; b<nblock; b++) {
    bsize = blocksize[b];
    //    ::cout << "Dimens�o do bloco [" << b << "]\t" <<bsize <<"\n";
    fBlockPos[b+1] = fBlockPos[b]+bsize*bsize;
    ndata += bsize*bsize;
    neq += bsize;
    //    ::cout << "N�mero de equac�es\t" << neq << "\n";
  }
#ifdef LOG4CXX
  {
    std::stringstream sout;
    sout << "N�mero de dados da Matriz diagonal\t" << ndata;
    LOGPZ_DEBUG(logger,sout.str());
  }
#endif
  //  ::cout <<"N�mero de dados da Matriz diagonal\t" <<ndata <<"\n";
  fStorage.Fill(0.,0);
  fStorage.Resize(ndata,0.);
  fDecomposed = 0;
  fRow = neq;
  fCol = neq;

  //fStorage.Print(std::cout);
}

void TPZBlockDiagonal::BuildFromMatrix(TPZMatrix &mat) {
  if(mat.Rows() != Rows()) {
    cout << "TPZBlockDiagonal::BuildFromMatrix wrong data structure\n";
    return;
  }
  int nblock = fBlockSize.NElements();
  int b,eq=0;
  for(b=0; b<nblock; b++) {
    int r,c,bsize = fBlockSize[b];
    int pos = fBlockPos[b];
    for(r=0; r<bsize; r++){
      for(c=0; c<bsize; c++) {
	fStorage[pos+r+c*bsize] = mat.GetVal(eq+r,eq+c);
      }
    }
    eq += bsize;
  }
}

/*******************/
/*** Constructor ***/

TPZBlockDiagonal::TPZBlockDiagonal()
  : TPZMatrix(), fStorage(), fBlockPos(1,0),fBlockSize()
{
  
}

TPZBlockDiagonal::TPZBlockDiagonal(const TPZVec<int> &blocksize)
  : TPZMatrix(), fStorage(), fBlockPos(1,0),fBlockSize()
{
  Initialize(blocksize);
}



/********************/
/*** Constructors ***/

TPZBlockDiagonal::TPZBlockDiagonal(const TPZVec<int> &blocksizes, const TPZFMatrix &glob)
  : TPZMatrix(), fBlockSize(blocksizes)
{
  int nblock = blocksizes.NElements();
  fBlockPos.Resize(nblock+1,0);
  int b;
  int ndata = 0;
  int neq = 0;
  int bsize;
  for(b=0; b<nblock; b++) {
    bsize = blocksizes[b];
    fBlockPos[b+1] = fBlockPos[b]+bsize*bsize;
    ndata += bsize*bsize;
    neq += bsize;
  }
  fStorage.Resize(ndata,0.);
  fRow = neq;
  fCol = neq;
  int pos;
  int eq = 0, r, c;
  for(b=0; b<nblock; b++) {
    bsize = fBlockSize[b];
    pos = fBlockPos[b];
    for(r=0; r<bsize; r++) {
      for(c=0; c<bsize; c++) {
	fStorage[pos+r+bsize*c]= glob.GetVal(eq+r,eq+c);
      }
    }
    eq += bsize;
  }

}



/*********************************/
/*** Constructor( TPZBlockDiagonal& ) ***/

TPZBlockDiagonal::TPZBlockDiagonal (const TPZBlockDiagonal & A)
  : TPZMatrix( A.Dim(), A.Dim() ), fStorage(A.fStorage),
    fBlockPos(A.fBlockPos), fBlockSize(A.fBlockSize)
{
}



/******************/
/*** Destructor ***/

TPZBlockDiagonal::~TPZBlockDiagonal ()
{
}



/***********/
/*** Put ***/

int
TPZBlockDiagonal::Put(const int row,const int col,const REAL& value )
{
//  cout << "TPZBlockDiagonal.Put should not be called\n";
  if ( (row >= Dim()) || (col >= Dim()) || row<0 || col<0)
    {
      cout << "TPZBlockDiagonal::Put: " << row << "," << col << "," << Dim();
      cout << "\n";
      return( 0 );
    }

  return( PutVal( row, col, value ) );
}

/***********/
/*** PutVal ***/

int
TPZBlockDiagonal::PutVal(const int row,const int col,const REAL& value )
{
//  cout << "TPZBlockDiagonal.PutVal should not be called\n";

  int b = 0;
  int nb = fBlockSize.NElements();
  if(nb==0) {
    cout << "TPZBlockDiagonal::PutVal called with parameters out of range\n";
    return -1;
  }
  int eq=0;
  int bsize = fBlockSize[b];
  while(eq+bsize <= row && b < nb) {
    eq+=bsize;
    b++;
    bsize = fBlockSize[b];
  }
  if(b==nb) {
    	cout << "TPZBlockDiagonal::PutVal wrong data structure\n";
	return -1;
  }
  if(col < eq || col >= eq+bsize) {
	if(value != 0.) {
    		cout << "TPZBlockDiagonal::PutVal, indices row col out of range\n";
		return -1;
	} else {
		return 0;
	}
  }
  fStorage[fBlockPos[b]+row-eq+bsize*(col-eq)] = value;
  return 0;
}



/***********/
/*** Get ***/

const REAL&
TPZBlockDiagonal::Get(const int row,const int col ) const
{
//  cout << "TPZBlockDiagonal::Get should never be called\n";
  if ( (row >= Dim()) || (col >= Dim()) )
    TPZMatrix::Error(__PRETTY_FUNCTION__, "TPZBlockDiagonal::Get <indices out of band matrix range>" );

  return( GetVal( row, col ) );
}


REAL &
TPZBlockDiagonal::operator()(const int row, const int col) {
//  cout << "TPZBlockDiagonal.operator() should not be called\n";

  int b = 0;
  int nb = fBlockSize.NElements();
  if(nb==0) {
    cout << "TPZBlockDiagonal::operator() called with parameters out of range\n";
    zero = 0.;
    return zero;
  }
  int eq=0;
  int bsize = fBlockSize[b];
  while(eq+bsize <= row && b < nb) {
    eq+=bsize;
    b++;
    bsize = fBlockSize[b];
  }
  if(b==nb) {
    cout << "TPZBlockDiagonal::operator() wrong data structure\n";
    zero = 0.;
    return zero;
  }
  if(col < eq || col >= eq+bsize) {
    cout << "TPZBlockDiagonal::operator(), indices row col out of range\n";
    zero = 0.;
    return zero;
  }
  return fStorage[fBlockPos[b]+row-eq+bsize*(col-eq)];
}

/***********/
/*** GetVal ***/

const REAL &
TPZBlockDiagonal::GetVal(const int row,const int col ) const
{
 // cout << "TPZBlockDiagonal.GetVal should not be called\n";

  int b = 0;
  int nb = fBlockSize.NElements();
  if(nb==0) {
    cout << "TPZBlockDiagonal::GetVal called with parameters out of range\n";
  }
  int eq=0;
  int bsize = fBlockSize[b];
  while(eq+bsize <= row && b < nb) {
    eq+=bsize;
    b++;
    bsize = fBlockSize[b];
  }
  if(b==nb) {
    cout << "TPZBlockDiagonal::GetVal wrong data structure\n";
  }
  if(col < eq || col >= eq+bsize) {
    //cout << "TPZBlockDiagonal::GetVal, indices row col out of range\n";
    zero = 0.;
    return zero;
  }
  return fStorage[fBlockPos[b]+row-eq+bsize*(col-eq)];
}




/******** Operacoes com MATRIZES GENERICAS ********/


/*******************/
/*** MultiplyAdd ***/
//
//  perform a multiply add operation to be used by iterative solvers
//

void TPZBlockDiagonal::MultAdd(const TPZFMatrix &x,const TPZFMatrix &y, TPZFMatrix &z,
			  const REAL alpha,const REAL beta ,const int opt,const int stride )  {
  // Computes z = beta * y + alpha * opt(this)*x
  //          z and x cannot overlap in memory

  if ((!opt && Cols()*stride != x.Rows()) || Rows()*stride != x.Rows())
    TPZMatrix::Error(__PRETTY_FUNCTION__, "TPZBlockDiagonal::MultAdd <matrixs with incompatible dimensions>" );
  if(x.Cols() != y.Cols() || x.Cols() != z.Cols() || x.Rows() != y.Rows() || x.Rows() != z.Rows()) {
    TPZMatrix::Error(__PRETTY_FUNCTION__,"TPZBlockDiagonal::MultAdd incompatible dimensions\n");
  }

  PrepareZ(y,z,beta,opt,stride);
//  int rows = Rows();
  int xcols = x.Cols();
  int nb= fBlockSize.NElements();
  int ic, b, bsize, eq=0, r, c;
  if(opt == 0) {
    for (ic = 0; ic < xcols; ic++) {
      eq=0;
      for(b=0; b<nb; b++) {
	bsize = fBlockSize[b];
	int pos = fBlockPos[b];
	for(r=0; r<bsize; r++) {
	  for(c=0; c<bsize; c++) {
	    z(eq+r,ic) += alpha*fStorage[pos+r+bsize*c]*x.GetVal((eq+c)*stride,ic);
	  }
	}
	eq += bsize;
      }
    }
  } else {
    cout << "xcols \t" << xcols << "\n";
    for (ic = 0; ic < xcols; ic++) {
      eq=0;
      for(b=0; b<nb; b++) {
	bsize = fBlockSize[b];
	int pos = fBlockPos[b];
	for(r=0; r<bsize; r++) {
	  for(c=0; c<bsize; c++) {
	    z(eq+r,ic) += alpha*fStorage[pos+r+bsize*c]*x.GetVal((eq+c)*stride,ic);
	    //   ::cout << "Z[" << (eq+r) <<"," << ic <<"] = " <<z(eq+r,ic) <<"\n";  
	  }
	}
	eq+=bsize;
      }
    }
  }
}







/***************/
/**** Zero ****/
int
TPZBlockDiagonal::Zero()
{

  fStorage.Fill(0.,0);
  fDecomposed = 0;

  return( 1 );
}



/********************/
/*** Transpose () ***/
void
TPZBlockDiagonal::Transpose (TPZMatrix *const T) const
{
  T->Resize( Dim(), Dim() );

  int b, bsize, eq = 0, pos;
  int nb = fBlockSize.NElements(), r, c;
  for ( b=0; b<nb; b++) {
    pos= fBlockPos[b];
    bsize = fBlockSize[b];
    for(r=0; r<bsize; r++) {
      for(c=0; c<bsize; c++) {
	T->PutVal(eq+r,eq+c,fStorage[pos+c+r*bsize]);
      }
    }
    eq += bsize;
  }
}


/*****************/
/*** Decompose_LU ***/
//fElem[ fBand * (2*row + 1) + col ]
int
TPZBlockDiagonal::Decompose_LU()
{
  if (  fDecomposed && fDecomposed == ELU) {
    return ELU;
  } else if(fDecomposed) {
    TPZMatrix::Error(__PRETTY_FUNCTION__,"TPZBlockDiagonal::Decompose_LU is already decomposed with other scheme");
  }

  int b,nb,pos,bsize;
  nb = fBlockSize.NElements();
  for(b=0;b<nb; b++) {
    pos = fBlockPos[b];
    bsize = fBlockSize[b];
    if(!bsize) continue;
    TPZFMatrix temp(bsize,bsize,&fStorage[pos],bsize*bsize);
    temp.Decompose_LU();
  }
  fDecomposed = ELU;
  return 1;
}

int
TPZBlockDiagonal::Substitution( TPZFMatrix *B) const
{
  if(fDecomposed != ELU) {
    TPZMatrix::Error(__PRETTY_FUNCTION__,"TPZBlockDiagonal::Decompose_LU is decomposed with other scheme");
  }

  int b,nb,pos,bsize,eq=0;
  nb = fBlockSize.NElements();
  int c, nc = B->Cols();
  for(c=0; c<nc; c++) {
    eq = 0;
    for(b=0;b<nb; b++) {
      pos = fBlockPos[b];
      bsize = fBlockSize[b];
      if(!bsize) continue;
//      TPZFMatrix temp(bsize,bsize,&fStorage[pos],bsize*bsize);
//      temp.SetIsDecomposed(ELU);
      TPZFMatrix BTemp(bsize,1,&(B->operator()(eq,c)),bsize);
      TPZFMatrix::Substitution(fStorage+pos,bsize,&BTemp);
      eq+= bsize;
    }
  }
  return 1;
}




/************************** Private **************************/

/*************/
/*** Error ***/

/*int
TPZBlockDiagonal::Error(const char *msg1,const char *msg2 ) 
{
  ostringstream out;
  out << "TPZBlockDiagonal::" << msg1 << msg2 << ".\n";
  LOGPZ_ERROR (logger, out.str().c_str());
  // pzerror.Show();
  //exit( 1 );
  return 0;
}*/



/*************/
/*** Clear ***/

int
TPZBlockDiagonal::Clear()
{
  fStorage.Resize(0);
  fBlockPos.Resize(0);
  fBlockSize.Resize(0);
  fRow = 0;
  fCol = 0;
  fDecomposed = 0;
  return( 1 );
}

int TPZBlockDiagonal::main() {

  cout << "Entering the main program\n";
  cout.flush();
  TPZFMatrix ref(7,7,0.);
  int r,c;
  for(r=0; r<7; r++) {
    for(c=0; c<7; c++) {
      ref(r,c) = 5+r*c+3*r;
    }
    ref(r,r) += 1000;
  }
  TPZVec<int> blocksize(3);
  blocksize[0] = 2;
  blocksize[1] = 4;
  blocksize[2] = 1;
  TPZBlockDiagonal bd1(blocksize,ref);
  TPZBlockDiagonal bd2(bd1);
  ref.Print("original matrix");
  bd1.Solve_LU(&ref);
  bd1.Solve_LU(&ref);
  ref.Print("after inverting the diagonal");
  TPZFMatrix ref2;
  bd2.Multiply(ref,ref2);
  bd2.Multiply(ref2,ref);
  ref.Print("restoring the original matrix");
  return 1;

}

void TPZBlockDiagonal::Print(char *msg, std::ostream &out) {

  out << "TPZBlockDiagonal matrix ";
  if(msg) out << msg;
  out  << std::endl;

  int nblock = fBlockSize.NElements();
  out << "Number of blocks " << nblock << std::endl; 
  int b,bsize,pos;
  for(b=0; b<nblock; b++) {
    bsize = fBlockSize[b];
    out << "block number " << b << " size : " << bsize << std::endl;
    int r,c;
    pos = fBlockPos[b];
    for(c=0; c<bsize; c++) {
      for(r=0; r<bsize ; r++) {
	out << fStorage[pos+r+bsize*c] << ' ';
      }
      out << std::endl;
    }
  }
}

   /**
   * Updates the values of the matrix based on the values of the matrix
   */
void TPZBlockDiagonal::UpdateFrom(TPZMatrix *mat)
{
  if(!mat) 
  {
    cout << "TPZBlockDiagonal::UpdateFrom" << " called with zero argument\n";
    return;
  }
  this->fDecomposed = ENoDecompose;
  int nblock = fBlockSize.NElements();
  int b,bsize,pos,firsteq = 0;
  for(b=0; b<nblock; b++) {
    bsize = fBlockSize[b];
//    int r,c;
    pos = fBlockPos[b];
    TPZFMatrix block(bsize,bsize,&fStorage[pos],bsize*bsize);
    mat->GetSub(firsteq,firsteq,bsize,bsize,block);
    firsteq += bsize;
  }
}
