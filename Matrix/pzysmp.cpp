/**
 * @file
 * @brief Contains the implementation of the TPZFYsmpMatrix methods.
 */
#include "pzysmp.h"
#include "pzfmatrix.h"
#include "pzvec.h"

#include <memory.h>
#include <string>
#include <map>
#include <pthread.h>

#ifdef USING_BLAS
	double cblas_ddoti(const int N, const double *X, const int *indx,
                   const double *Y);
#endif

using namespace std;

// ****************************************************************************
//
// Constructors and the destructor
//
// ****************************************************************************
template<class TVar>
void TPZFYsmpMatrix<TVar>::InitializeData(){}

template<class TVar>
void TPZFYsmpMatrix<TVar>::MultiplyDummy(TPZFYsmpMatrix<TVar> & B, TPZFYsmpMatrix<TVar> & Res){
    int i,j,k;
    if (B.Rows()!=this->Rows()) return;
    int rows = this->Rows();
    REAL aux=0.;
    for(i=0;i<rows;i++){
        for(j=0;j<rows;j++){
            for(k=0;k<rows;k++){
				// C[i][j] += A[i][k]*B[k][j];
				aux+=GetVal(i,k)*B.GetVal(i,k);
			}
			Res.PutVal(i,j,aux);
			aux=0.;
		}
    }
}

// ****************************************************************************
//
// Constructor
//
// ****************************************************************************

template<class TVar>
TPZFYsmpMatrix<TVar>::TPZFYsmpMatrix(const TPZVerySparseMatrix<TVar> &cp) : TPZMatrix<TVar>
()
{
	fDiag = 0;
	fIA=0;
	fJA=0;
	fA=0;
	*this = cp;
}

template<class TVar>
TPZFYsmpMatrix<TVar> &TPZFYsmpMatrix<TVar>::operator=(const TPZFYsmpMatrix<TVar> &cp) {
	// Deletes everything associated with a TPZFYsmpMatrix
	delete []fDiag;
	fDiag = 0;
	delete []fIA;
	fIA=0;
	delete []fJA;
	fJA=0;
	delete []fA;
	fA=0;
	TPZMatrix<TVar>::operator=(cp);
	int nrows = cp.Rows();
	int count = cp.fIA[nrows];
	fJA = new int[count];
	fA = new TVar[count];
	fIA = new int[nrows+1];
	fIA[0] = 0;
	memcpy(fJA,cp.fJA,count*sizeof(int));
	memcpy(fIA , cp.fIA, (nrows+1)*sizeof(int));
	memcpy(fA, cp.fA, count*sizeof(REAL));
	if(cp.fDiag)
	{
		fDiag = new TVar[nrows];
		memcpy(fDiag, cp.fDiag, nrows*sizeof(REAL));
	}
	return *this;
}

template<class TVar>
TPZFYsmpMatrix<TVar> &TPZFYsmpMatrix<TVar>::operator=(const TPZVerySparseMatrix<TVar> &cp)
{
	// Deletes everything associated with a TPZFYsmpMatrix
	delete []fDiag;
	fDiag = 0;
	delete []fIA;
	fIA=0;
	delete []fJA;
	fJA=0;
	delete []fA;
	fA=0;
	int nrows = cp.Rows();
	
	int count = 0, c = 0, r = 0;
	
	count = cp.fExtraSparseData.size();
	fJA = new int[count];
	fA = new TVar[count];
	fIA = new int[nrows+1];
	fIA[0] = 0;
	
	map< pair<int,int>, REAL>::const_iterator it;
	c = 0;
	r = 0;
	for(it=cp.fExtraSparseData.begin(); it!= cp.fExtraSparseData.end(); it++)
	{
		int row = it->first.first;
		if(r != row)
		{
			r++;
			while(r < row) 
			{
				fIA[r] = c;
				r++;
			}
			fIA[row] = c;
			r = row;
		}
		int col = it->first.second;
		fJA[c] = col;
		TVar val = it->second;
		fA[c] = val;
		c++;
	}
	r++;
	while(r<=nrows)
	{
		fIA[r] = c;
		r++;
	}
	return *this;
}

template<class TVar>
int TPZFYsmpMatrix<TVar>::PutVal(const int row, const int col, const TVar &Value){
    int k;
    int flag=0;
    for(k=fIA[row];k<fIA[row+1];k++){
		if(fJA[k]==col){
			flag=1;
			fA[k]=Value;
			break;
		}
    }
    if(!flag) 
    {
		cout << "TPZFYsmpMatrix::PutVal: Non existing position on sparse matrix: line = " << row << " column " << col << endl;
		DebugStop();
		return 0;
    }
    else
    {
		return 1;
    }
}
template<class TVar>
void TPZFYsmpMatrix<TVar>::AddKel(TPZFMatrix<TVar> & elmat, TPZVec<int> & destinationindex){
    int i,j,k = 0;
    TVar value=0.;
    int ipos,jpos;
    for(i=0;i<elmat.Rows();i++){
        for(j=0;j<elmat.Rows();j++){
            ipos=destinationindex[i];
            jpos=destinationindex[j];
            value=elmat.GetVal(i,j);
            //cout << "j= " << j << endl;
            if(value != 0.){
                //cout << "fIA[ipos] " << fIA[ipos] << "     fIA[ipos+1] " << fIA[ipos+1] << endl;
                int flag = 0;
				k++;
				if(k >= fIA[ipos] && k < fIA[ipos+1] && fJA[k]==jpos)
				{ // OK -> elements in sequence
					fA[k]+=value;
					flag = 1;
				}else
				{
					for(k=fIA[ipos];k<fIA[ipos+1];k++){
						if(fJA[k]==jpos || fJA[k]==-1){
							//cout << "fJA[k] " << fJA[k] << " jpos "<< jpos << "   " << value << endl;
							//cout << "k " << k << "   "<< jpos << "   " << value << endl;
							flag=1;
							if(fJA[k]==-1){
								fJA[k]=jpos;
								fA[k]=value;
								// cout << jpos << "   " << value << endl;
								break;
							}else{
								fA[k]+=value;
								break;
							}
						}
					}
				}
                if(!flag) cout << "TPZFYsmpMatrix::AddKel: Non existing position on sparse matrix: line =" << ipos << " column =" << jpos << endl;         }
        }
    }
}

template<class TVar>
void TPZFYsmpMatrix<TVar>::AddKel(TPZFMatrix<TVar> & elmat, TPZVec<int> & sourceindex, TPZVec<int> & destinationindex){
	int i,j,k = 0;
	TVar value=0.;
	int ipos,jpos;
	for(i=0;i<sourceindex.NElements();i++){
		for(j=0;j<sourceindex.NElements();j++){
			ipos=destinationindex[i];
			jpos=destinationindex[j];
			value=elmat.GetVal(sourceindex[i],sourceindex[j]);
            //cout << "j= " << j << endl;
			if(value != 0.){
                //cout << "fIA[ipos] " << fIA[ipos] << "     fIA[ipos+1] " << fIA[ipos+1] << endl;
				int flag = 0;
				k++;
				if(k >= fIA[ipos] && k < fIA[ipos+1] && fJA[k]==jpos)
				{ // OK -> elements in sequence
					fA[k]+=value;
					flag = 1;
				}else
				{
					for(k=fIA[ipos];k<fIA[ipos+1];k++){
						if(fJA[k]==jpos || fJA[k]==-1){
							//cout << "fJA[k] " << fJA[k] << " jpos "<< jpos << "   " << value << endl;
							//cout << "k " << k << "   "<< jpos << "   " << value << endl;
							flag=1;
							if(fJA[k]==-1){
								fJA[k]=jpos;
								fA[k]=value;
								// cout << jpos << "   " << value << endl;
								break;
							}else{
								fA[k]+=value;
								break;
							}
						}
					}
				}
				if(!flag) cout << "TPZFYsmpMatrix::AddKel: Non existing position on sparse matrix: line =" << ipos << " column =" << jpos << endl;         }
		}
	}
}

template<class TVar>
void TPZFYsmpMatrix<TVar>::AddKelOld(TPZFMatrix<TVar> & elmat, TPZVec < int > & destinationindex){
	int i=0;
	int j=0;
	int ilocal=0;
	//  int jlocal=0;
	int nel = destinationindex.NElements();
	std::multimap<int,int> mapindex;
	std::multimap<int,int>::iterator hint = mapindex.begin();
	for(i=0;i<nel;i++){
		ilocal = destinationindex[i];
		hint = mapindex.insert(hint,std::make_pair(ilocal,i));
		//    mapindex[ilocal] = i;
	}
	for(i=0;i<nel;i++){
		ilocal = destinationindex[i];
		int jfirst = fIA[ilocal];
		int jlast = fIA[ilocal+1];
		int *Aptr = &fJA[jfirst];
		int *AptrLast = &fJA[jlast];
		j=0;
		std::multimap<int,int>::iterator itelmat = mapindex.begin();
		while(j<nel && Aptr != AptrLast)
		{
			if(*Aptr == (*itelmat).first)
			{
				int jel = (*itelmat).second;
				fA[jfirst] += elmat(i,jel);
				itelmat++;
				if(itelmat != mapindex.end() && (*itelmat).second != jel)
				{
					Aptr++;
					jfirst++;
				}
				j++;
			}
			else if(*Aptr < (*itelmat).first)
			{
				Aptr++;
				jfirst++;
			}
			else if(*Aptr > (*itelmat).second)
			{
				std::cout << __PRETTY_FUNCTION__ << " inconsistent\n";
				int *iptr = &fJA[jfirst];
				while(iptr < AptrLast) 
				{
					cout << *iptr << " ";
					iptr++;
				}
				cout << endl;
				std::multimap<int,int>::iterator itelmat2 = mapindex.begin();
				for(;itelmat2 != mapindex.end(); itelmat2++)
				{
					cout << (*itelmat2).first << "/" << (*itelmat2).second << " ";
				}
				cout << endl;
				
			}
		}
		if(j!= nel)
		{
			std::cout << __PRETTY_FUNCTION__ << " inconsistent2 j = " << j << " nel " << nel << "\n";
			int *iptr = &fJA[jfirst];
			while(iptr < AptrLast) 
			{
				cout << *iptr << " ";
				iptr++;
			}
			cout << endl;
			std::multimap<int,int>::iterator itelmat2 = mapindex.begin();
			for(;itelmat2 != mapindex.end(); itelmat2++)
			{
				cout << (*itelmat2).first << "/" << (*itelmat2).second << " ";
			}
			cout << endl;
		}
	}
	
}

/*
 void TPZFYsmpMatrix::AddKelOld(TPZFMatrix<>& elmat, TPZVec < int > & destinationindex){
 int i=0;
 int j=0;
 int ilocal=0;
 int jlocal=0;
 int nel = destinationindex.NElements();
 for(i=0;i<nel;i++){
 ilocal = destinationindex[i];
 for(j=0;j<nel;j++){
 jlocal = destinationindex[j];
 Element(ilocal,jlocal)+=elmat(i,j);
 }
 }
 
 }
 */
template<class TVar>
TPZFYsmpMatrix<TVar>::TPZFYsmpMatrix(const int rows,const int cols ) : TPZMatrix<TVar>(rows,cols) {
	// Constructs an empty TPZFYsmpMatrix
	//    fSolver = -1;
	fSymmetric = 0;
	//    fMaxIterations = 4;
	//    fSORRelaxation = 1.;
	fDiag = 0;
	fA = 0;
	fIA = 0;
	fJA = 0;
#ifdef CONSTRUCTOR
	cerr << "TPZFYsmpMatrix(int rows,int cols)\n";
#endif
}

template<class TVar>
TPZFYsmpMatrix<TVar>::~TPZFYsmpMatrix() {
	// Deletes everything associated with a TPZFYsmpMatrix
	delete []fDiag;
	fDiag = 0;
	delete []fIA;
	fIA=0;
	delete []fJA;
	fJA=0;
	delete []fA;
	fA=0;
	//    fSolver = -1;
#ifdef DESTRUCTOR
	cerr << "~TPZFYsmpMatrix()\n";
#endif
}

// ****************************************************************************
//
// Find the element of the matrix at (row,col) in the stencil matrix
//
// ****************************************************************************

template<class TVar>
const TVar & TPZFYsmpMatrix<TVar>::GetVal(const int row,const int col ) const {
	// Get the matrix entry at (row,col) without bound checking
	
	// Now look through the requested row and see if there is anything
	// in column col
	/*  int loccol = col+1;
	 for(int ic=fIA[row]-1 ; ic < fIA[row+1]-1; ic++ ) {
	 if ( fJA[ic] == loccol ) return fA[ic];
	 }*/
	int loccol = col;
	for(int ic=fIA[row] ; ic < fIA[row+1]; ic++ ) {
		if ( fJA[ic] == loccol && fJA[ic] != -1 ) return fA[ic];
	}
	return this->gZero;
}

// ****************************************************************************
//
// Multiply and Multiply-Add
//
// ****************************************************************************

template<class TVar>
void TPZFYsmpMatrix<TVar>::MultAddMT(const TPZFMatrix<TVar> &x,const TPZFMatrix<TVar> &y,
							   TPZFMatrix<TVar> &z,
							   const TVar alpha,const TVar beta,const int opt,const int stride )  {
	// computes z = beta * y + alpha * opt(this)*x
	//          z and x cannot share storage
	if(x.Cols() != y.Cols() || x.Cols() != z.Cols() || y.Rows() != z.Rows() )
	{
		cout << "\nERROR! in TPZVerySparseMatrix::MultiplyAdd : incompatible dimensions in x, y or z\n";
		return;
	}
	
	int  ir, ic, icol, xcols;
	xcols = x.Cols();
	TVar sum;
	int  r = (opt) ? this->Cols() : this->Rows();
	
	// Determine how to initialize z
	for(ic=0; ic<xcols; ic++) {
		TVar *zp = &(z(0,ic));
		if(beta != 0) {
			const TVar *yp = &(y.g(0,0));
			TVar *zlast = zp+r*stride;
			if(beta != 1. || (&z != &y && stride != 1)) {
				while(zp < zlast) {
					*zp = beta * (*yp);
					zp += stride;
					yp += stride;
				}
			}
			else if(&z != &y) {
				memcpy(zp,yp,r*sizeof(TVar));
			}
		} else {
			TVar *zp = &(z(0,0)), *zlast = zp+r*stride;
			while(zp != zlast) {
				*zp = 0.;
				zp += stride;
			}
		}
	}
	// Compute alpha * A * x
	if(xcols == 1 && stride == 1 && opt == 0)
	{
		if(this->Cols() != x.Rows()*stride || this->Rows() != y.Rows()*stride)
		{
			cout << "\nERROR! in TPZFYsmpMatrix::MultiplyAddMT: incompatible dimensions in opt=false\n";
			return;
		} 
		for(ir=0; ir<r; ir++) {
			int icolmin = fIA[ir];
			int icolmax = fIA[ir+1];
			const TVar *xptr = &(x.g(0,0));
			TVar *Aptr = fA;
			int *JAptr = fJA;
			for(sum = 0.0, icol=icolmin; icol<icolmax; icol++ ) {
				sum += Aptr[icol] * xptr[JAptr[icol]];
			}
			z(ir,0) += alpha * sum;
		}
	}
	else 
	{
		for(ic=0; ic<xcols; ic++) {
			if(opt == 0) {
				
				for(ir=0; ir<this->Rows(); ir++) {
					for(sum = 0.0, icol=fIA[ir]; icol<fIA[ir+1]; icol++ ) {
						sum += fA[icol] * x.g((fJA[icol])*stride,ic);
					}
					z(ir*stride,ic) += alpha * sum;
				}
			}
			
			// Compute alpha * A^T * x
			else 
			{
				if (this->Rows() != x.Rows()*stride || this->Cols() != y.Rows()*stride)
				{
					cout << "\nERROR! in TPZFYsmpMatrix::MultiplyAddMT: incompatible dimensions in opt=true\n";
					return; 
				}
				int jc;
				int icol;
				for(ir=0; ir<this->Rows(); ir++) {
					for(icol=fIA[ir]; icol<fIA[ir+1]; icol++ ) {
						if(fJA[icol]==-1) break; //Checa a exist�cia de dado ou n�
						jc = fJA[icol];
						TVar aval = fA[icol];
						//cout << "FA["<<icol<<"] = "<<aval<< " * x["<< ir<<"] ="<< x.Get(ir,ic)<< endl;
						z(jc*stride,ic) += alpha * aval * x.g(ir*stride,ic);
					}
				}
			}
		}
	}
}

// ****************************************************************************
//
// Multiply and Multiply-Add
//
// ****************************************************************************

template<class TVar>
void TPZFYsmpMatrix<TVar>::MultAdd(const TPZFMatrix<TVar> &x,const TPZFMatrix<TVar> &y,
							 TPZFMatrix<TVar> &z,
							 const TVar alpha,const TVar beta,const int opt,const int stride ) const {
	// computes z = beta * y + alpha * opt(this)*x
	//          z and x cannot share storage
	
	if(x.Cols() != y.Cols() || x.Cols() != z.Cols() || y.Rows() != z.Rows() )
	{
		cout << "\nERROR! em TPZFYsmpMatrix::MultiplyAdd : incompatible dimensions in x, y or z\n";
		return;
	}
	
	int  ic, xcols;
	xcols = x.Cols();
	int  r = (opt) ? this->Cols() : this->Rows();
	
	// Determine how to initialize z
	for(ic=0; ic<xcols; ic++) {
		TVar *zp = &(z(0,ic));
		if(beta != 0) {
			const TVar *yp = &(y.g(0,0));
			TVar *zlast = zp+r*stride;
			if(beta != 1. || (&z != &y && stride != 1)) {
				while(zp < zlast) {
					*zp = beta * (*yp);
					zp += stride;
					yp += stride;
				}
			}
			else if(&z != &y) {
				memcpy(zp,yp,r*sizeof(REAL));
			}
		} else {
			TVar *zp = &(z(0,0)), *zlast = zp+r*stride;
			while(zp != zlast) {
				*zp = 0.;
				zp += stride;
			}
		}
	}
	/*
	 TPZFYsmpMatrix *target;
	 int fFirsteq;
	 int fLasteq;
	 TPZFMatrix<>*fX;
	 TPZFMatrix<>*fZ;
	 REAL fAlpha;
	 int fOpt;
	 int fStride;
	 */
  #warning "Modifiquei nthreads == 0"
	const int numthreads = 0;
	pthread_t allthreads[numthreads+1];
	TPZMThread alldata[numthreads+1];
	int res[numthreads+1];
	int i;
	int firsteq = 0;
  if (numthreads == 0) {
		alldata[0].target = this;
		alldata[0].fFirsteq = 0;
		alldata[0].fLasteq = this->Rows();
		alldata[0].fX = &x;
		alldata[0].fZ = &z;
		alldata[0].fAlpha = alpha;
		alldata[0].fOpt = opt;
		alldata[0].fStride = stride;
    ExecuteMT(&alldata[0]);

  }
  else {
	int eqperthread = r/numthreads;
	for(i=0;i<numthreads;i++) 
	{
		alldata[i].target = this;
		alldata[i].fFirsteq = firsteq;
		alldata[i].fLasteq = firsteq+eqperthread;
		firsteq += eqperthread;
		if(i==numthreads-1) alldata[i].fLasteq = this->Rows();
		alldata[i].fX = &x;
		alldata[i].fZ = &z;
		alldata[i].fAlpha = alpha;
		alldata[i].fOpt = opt;
		alldata[i].fStride = stride;
		res[i] = pthread_create(&allthreads[i],NULL,ExecuteMT, &alldata[i]);
	}
	for(i=0;i<numthreads;i++) pthread_join(allthreads[i], NULL);
  }
	
}

// ****************************************************************************
//
// Print the matrix
//
// ****************************************************************************

template<class TVar>
void TPZFYsmpMatrix<TVar>::Print(const char *title, ostream &out ,const MatrixOutputFormat form) const {
	// Print the matrix along with a identification title
	if(form != EInputFormat) {
		out << "\nTFYsmpMatrix Print: " << title << '\n'
		<< "\tRows    = " << this->Rows()  << '\n'
		<< "\tColumns = " << this->Cols() << '\n';
		int i;
		out << "\tIA\tJA\tA\n"
		<< "\t--\t--\t-\n";
		for(i=0; i<=this->Rows(); i++) {
			out << i      << '\t'
			<< fIA[i] << '\t'
			<< fJA[i] << '\t'
			<< fA[i]  << '\n';
		}
		for(i=this->Rows()+1; i<fIA[this->Rows()]; i++) {
			out << i      << "\t\t"
			<< fJA[i] << '\t'
			<< fA[i]  << '\n';
		}
	}
}


// ****************************************************************************
//
// Various solvers
//
// ****************************************************************************

template<class TVar>
void TPZFYsmpMatrix<TVar>::ComputeDiagonal() {
	if(fDiag) return;
	int rows = this->Rows();
	fDiag = new TVar [rows];
	for(int ir=0; ir<rows; ir++) {
		fDiag[ir] = GetVal(ir,ir);
	}
}

template<class TVar>
void TPZFYsmpMatrix<TVar>::SolveSOR( int &numiterations, const TPZFMatrix<TVar> &rhs, TPZFMatrix<TVar> &x,
							  TPZFMatrix<TVar> *residual, TPZFMatrix<TVar> &/*scratch*/,
							  const TVar overrelax, TVar &tol,
							  const int FromCurrent,const int direction ) {
	
	//    if(!fDiag) ComputeDiagonal();
	int irStart = 0,irLast = this->Rows(),irInc = 1;
	if(direction < 0) {
		irStart = this->Rows()-1;
		irLast = -1;
		irInc = -1;
	}
	if(!FromCurrent) x.Zero();
	TVar eqres = 2.*tol;
	int iteration;
	for(iteration=0; iteration<numiterations && eqres >= tol; iteration++) {
		eqres = 0.;
		int ir=irStart;
		while(ir != irLast) {
			TVar xnewval=rhs.g(ir,0);
			for(int ic=fIA[ir]; ic<fIA[ir+1]; ic++) {
				xnewval -= fA[ic] * x(fJA[ic],0);
			}
			eqres += xnewval*xnewval;
			x(ir,0) += overrelax*(xnewval/fDiag[ir]);
			ir += irInc;
		}
		eqres = sqrt(eqres);
	}
	tol = eqres;
	numiterations = iteration;
	if(residual) this->Residual(x,rhs,*residual);
}

template<class TVar>
int TPZFYsmpMatrix<TVar>::Zero()
{
	int size = fIA[this->fRow] * sizeof(TVar);
	int diagSize = min(this->fRow, this->fCol) * sizeof(TVar);
	memset(fA,'\0',size);
	memset(fDiag,'\0', diagSize);
	return 1;
}


/**
 * Solves the linear system using Jacobi method. \n
 * @param numiterations The number of interations for the process.
 * @param F The right hand side of the system.
 * @param result The solution.
 * @param residual Returns F - A*U which is the solution residual.
 * @param scratch Available manipulation area on memory.
 * @param tol The tolerance value.
 * @param FromCurrent It starts the solution based on FromCurrent. Obtaining solution FromCurrent + 1.
 */
template<class TVar>
void TPZFYsmpMatrix<TVar>::SolveJacobi(int & numiterations, const TPZFMatrix<TVar> & F, TPZFMatrix<TVar> & result, TPZFMatrix<TVar> * residual, TPZFMatrix<TVar> & scratch, REAL & tol, const int FromCurrent) 
{
	if(!fDiag) {
		cout << "TPZSYsmpMatrix::Jacobi cannot be called without diagonal\n";
		numiterations = 0;
		if(residual) {
			this->Residual(result,F,*residual);
			tol = sqrt(Norm(*residual));
		}
		return;
	}
	int c = F.Cols();
	int r = this->Rows();
	int it=0;
	if(FromCurrent) {
		this->Residual(result,F,scratch);
		for(int ic=0; ic<c; ic++) {
			for(int i=0; i<r; i++) {
				result(i,ic) += scratch(i,ic)/(fDiag)[i];
			}
		}
	} else 
	{
		for(int ic=0; ic<c; ic++) {
			for(int i=0; i<r; i++) {
				result(i,ic) = F.GetVal(i,ic)/(fDiag)[i];
			}
		}
	}
	if(it<numiterations)
	{
		this->Residual(result,F,scratch);
		TVar res = Norm(scratch);
		for(int it=1; it<numiterations && res > tol; it++) {
			for(int ic=0; ic<c; ic++) {
				for(int i=0; i<r; i++) {
					result(i,ic) += (scratch)(i,ic)/(fDiag)[i];
				}
			}
			this->Residual(result,F,scratch);
			res = Norm(scratch);
		}
	}
	if(residual) *residual = scratch;
}

template<class TVar>
void *TPZFYsmpMatrix<TVar>::ExecuteMT(void *entrydata)
{
	TPZMThread *data = (TPZMThread *) entrydata;
	const TPZFYsmpMatrix *mat = data->target;
	TVar sum;
	int xcols = data->fX->Cols();
	int ic,ir,icol;
	// Compute alpha * A * x
	if(xcols == 1 && data->fStride == 1 && data->fOpt == 0)
	{
		for(ir=data->fFirsteq; ir<data->fLasteq; ir++) {
			int icolmin = mat->fIA[ir];
			int icolmax = mat->fIA[ir+1];
			const TVar *xptr = &(data->fX->g(0,0));
			TVar *Aptr = mat->fA;
			int *JAptr = mat->fJA;
			for(sum = 0.0, icol=icolmin; icol<icolmax; icol++ ) {
				sum += Aptr[icol] * xptr[JAptr[icol]];
			}
			data->fZ->operator()(ir,0) += data->fAlpha * sum;
		}
	}
	else 
	{
		for(ic=0; ic<xcols; ic++) {
			if(data->fOpt == 0) {
				
				for(ir=data->fFirsteq; ir<data->fLasteq; ir++) {
					for(sum = 0.0, icol=mat->fIA[ir]; icol<mat->fIA[ir+1]; icol++ ) {
						sum += mat->fA[icol] * data->fX->g((mat->fJA[icol])*data->fStride,ic);
					}
					data->fZ->operator()(ir*data->fStride,ic) += data->fAlpha * sum;
				}
			}
			
			// Compute alpha * A^T * x
			else 
			{
				int jc;
				int icol;
				for(ir=data->fFirsteq; ir<data->fLasteq; ir++) 
				{
					for(icol=mat->fIA[ir]; icol<mat->fIA[ir+1]; icol++ ) 
					{
						if(mat->fJA[icol]==-1) break; //Checa a exist�cia de dado ou n�
						jc = mat->fJA[icol];
						data->fZ->operator()(jc*data->fStride,ic) += data->fAlpha * mat->fA[icol] * data->fX->g(ir,ic);			
					}
				}
				
			}
		}
	}
	return 0;
}
static int  FindCol(int *colf, int *coll, int col)
{
	if(col == *colf) return 0;
	int *begin = colf;
	int *end = coll;
	while (begin != end)
	{
		int dist = (end-begin)/2;
		int *mid = begin+dist;
		if(*mid == col) return (mid-colf);
		else if(*mid > col) end=mid;
		else begin = mid;
	}
	return -1;
}

template<class TVar>
int TPZFYsmpMatrix<TVar>::GetSub(const int sRow,const int sCol,const int rowSize,
						   const int colSize, TPZFMatrix<TVar> & A ) const {
	int ir;
	for(ir=0; ir<rowSize; ir++)
	{
		int row = sRow+ir;
		int colfirst = fIA[row];
		int collast = fIA[row+1];
		int iacol = FindCol(fJA+colfirst,fJA+collast-1,sCol);
		int ic;
		for(ic=0; ic<colSize; ic++) A(ir,ic) = fA[iacol+colfirst];
	}
	return 0;
}


template<class TVar>
void TPZFYsmpMatrix<TVar>::GetSub(const TPZVec<int> &indices,TPZFMatrix<TVar> &block) const
{
	std::map<int,int> indord;
	int i,size = indices.NElements();
	for(i=0; i<size; i++)
	{
		indord[indices[i]] = i;
	}
	std::map<int,int>::iterator itset,jtset;
	for(itset = indord.begin(); itset != indord.end(); itset++)
	{
		int *jfirst = fJA+fIA[(*itset).first];
		int *jlast = fJA+fIA[(*itset).first+1]-1;
		//    int row = (*itset).first;
		for(jtset = indord.begin(); jtset != indord.end(); jtset++)
		{
			int col = FindCol(jfirst,jlast,(*jtset).first);
			int dist = jfirst+col-fJA;
			block((*itset).second,(*jtset).second) = fA[dist];
			jfirst += col+1;
		}
	}
}

/*
 * Perform row update of the sparse matrix
 */
template<class TVar>
void TPZFYsmpMatrix<TVar>::RowLUUpdate(int sourcerow, int destrow)
{
	int *sourcefirst = fJA+fIA[sourcerow];
	int *sourcelast = fJA+fIA[sourcerow+1]-1;
	int sourcecol = FindCol(sourcefirst,sourcelast,sourcerow);
	if(sourcecol < 0)
	{
		cout << __PRETTY_FUNCTION__ << " at line " << __LINE__ << " source not found\n";
		return;
	}
	int sourcedist = sourcefirst+sourcecol-fJA;
	int *destfirst = fJA+fIA[destrow];
	int *destlast = fJA+fIA[destrow+1]-1;
	int destcol = FindCol(destfirst,destlast,destrow);
	int destdist = destfirst+destcol-fJA;
	if(destcol < 0)
	{
		cout << __PRETTY_FUNCTION__ << " at line " << __LINE__ << " destrow not found\n";
		return;
	}
	if(fA[sourcedist] < 1.e-15)
	{
		cout << __PRETTY_FUNCTION__ << " at line " << __LINE__ << " small pivot " << fA[sourcedist] << "\n";
		return;
	}
	TVar mult = fA[destdist]/fA[sourcedist];
	if(mult == 0.) return;
	destdist++;
	sourcedist++;
	while(destdist < fIA[destrow+1] && sourcedist < fIA[sourcerow+1])
	{
		if(fJA[destdist] == fJA[sourcedist])
		{
			fA[destdist] -= fA[sourcedist]*mult;
			destdist++;
			sourcedist++;
		}
		else if(fJA[destdist] < fJA[sourcedist])
		{
			destdist++;
		}
		else
		{
			sourcedist++;
		}
	}
	
}

/**
 * Decomposes the current matrix using LU decomposition.
 */
template<class TVar>
int TPZFYsmpMatrix<TVar>::Decompose_LU(std::list<int> &singular)
{
	return Decompose_LU();
}
template<class TVar>
int TPZFYsmpMatrix<TVar>::Decompose_LU()
{
	int row;
	int neq = this->Rows();
	for(row=1; row<neq; row++)
	{
		//    int firstcol = fIA[row];
		int lastcol = fIA[row+1];
		int colind = 0;
		if(fJA[lastcol-1] < row) continue;
		while(fJA[colind] < row)
		{
			RowLUUpdate(fJA[colind],row);
			colind++;
		}
	}
	this->fDecomposed=1;
	return 1;
}

template<class TVar>
int TPZFYsmpMatrix<TVar>::Substitution( TPZFMatrix<TVar> *B ) const
{
	int row;
	int bcol = B->Cols();
	int col;
	int neq = this->Rows();
	
	// forward substitution
	for(row=0; row<neq; row++)
	{
		int firstrow = fIA[row];
		int lastrow = fIA[row+1];
		if(fJA[firstrow] > row || fJA[lastrow-1] < row)
		{
			cout << __PRETTY_FUNCTION__ << " " << __LINE__ << " inconsistent column information for row " << row << endl;
			continue;
		}
		int rowcounter = firstrow;
		while(fJA[rowcounter] < row)
		{
			for(col=0; col<bcol; col++)
			{
				(*B)(row,col) -= fA[rowcounter]*(*B)(fJA[rowcounter],col);
			}
		}
		for(col=0; col<bcol; col++)
		{
			(*B)(row,col) /= fA[rowcounter];
		}
	}
	// backward substitution
	for(row = neq-1; row >= 0; row--)
	{
		int firstrow = fIA[row];
		int lastrow = fIA[row+1];
		int col = FindCol(fJA+firstrow,fJA+lastrow-1,row);
		if(col < 0)
		{
			cout << __PRETTY_FUNCTION__ << " " << __LINE__ << " inconsistent column information for row " << row << endl;
			continue;
		}
		int coldist = firstrow+col+1;
		while(coldist < lastrow)
		{
			for(col=0; col<bcol; col++)
			{
				(*B)(row,col) -= fA[coldist]*(*B)(fJA[coldist],col);
			}
		}
	}
	return 1;
}

template class TPZFYsmpMatrix<long double>;
template class TPZFYsmpMatrix<double>;
template class TPZFYsmpMatrix<float>;
template class TPZFYsmpMatrix<int>;
