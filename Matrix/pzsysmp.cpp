/**
 * @file
 * @brief Contains the implementation of the TPZSYsmpMatrix methods.
 */

#include <memory.h>

#include "pzsysmp.h"
#include "pzfmatrix.h"

// ****************************************************************************
// 
// Constructors and the destructor
// 
// ****************************************************************************

template<class TVar>
TPZSYsmpMatrix<TVar>::TPZSYsmpMatrix() : TPZMatrix<TVar>() {
#ifdef USING_MKL
    fPardisoControl.SetMatrix(this);
#endif
    
#ifdef CONSTRUCTOR
    cerr << "TPZSYsmpMatrix(int rows,int cols)\n";
#endif
}

template<class TVar>
TPZSYsmpMatrix<TVar>::TPZSYsmpMatrix(const long rows,const long cols ) : TPZMatrix<TVar>(rows,cols) {

#ifdef USING_MKL
    fPardisoControl.SetMatrix(this);
#endif

#ifdef CONSTRUCTOR
	cerr << "TPZSYsmpMatrix(int rows,int cols)\n";
#endif
}

template<class TVar>
TPZSYsmpMatrix<TVar>::~TPZSYsmpMatrix() {
	// Deletes everything associated with a TPZSYsmpMatrix
#ifdef DESTRUCTOR
	cerr << "~TPZSYsmpMatrix()\n";
#endif
}

template<class TVar>
TPZSYsmpMatrix<TVar> &TPZSYsmpMatrix<TVar>::operator=(const TPZSYsmpMatrix<TVar> &copy) 
{
    TPZMatrix<TVar>::operator=(copy);
    fIA =copy.fIA;
    fJA = copy.fJA;
    fA = copy.fA;
    fDiag = copy.fDiag;
#ifdef USING_MKL
    fPardisoControl = copy.fPardisoControl;
    fPardisoControl.SetMatrix(this);
#endif
    return *this;
}


// ****************************************************************************
//
// Find the element of the matrix at (row,col) in the stencil matrix
//
// ****************************************************************************

template<class TVar>
const TVar &TPZSYsmpMatrix<TVar>::GetVal(const long r,const long c ) const {
	// Get the matrix entry at (row,col) without bound checking
    long row(r),col(c);
    if (r > c) {
        long temp = r;
        row = col;
        col = temp;
    }
	for(int ic=fIA[row] ; ic < fIA[row+1]; ic++ ) {
		if ( fJA[ic] == col ) return fA[ic];
	}
	return this->gZero;
}

/** @brief Put values without bounds checking \n
 *  This method is faster than "Put" if DEBUG is defined.
 */
template<class TVar>
int TPZSYsmpMatrix<TVar>::PutVal(const long r,const long c,const TVar & val )
{
    // Get the matrix entry at (row,col) without bound checking
    long row(r),col(c);
    if (r > c) {
        long temp = r;
        row = col;
        col = temp;
    }
    for(int ic=fIA[row] ; ic < fIA[row+1]; ic++ ) {
        if ( fJA[ic] == col )
        {
            fA[ic] = val;
            return;
        }
    }
    if (val != (TVar(0.))) {
        DebugStop();
    }
    return 0;
    
}


// ****************************************************************************
//
// Multiply and Multiply-Add
//
// ****************************************************************************

template<class TVar>
void TPZSYsmpMatrix<TVar>::MultAdd(const TPZFMatrix<TVar> &x,const TPZFMatrix<TVar> &y,
							 TPZFMatrix<TVar> &z,
							 const TVar alpha,const TVar beta,const int opt) const {
	// computes z = beta * y + alpha * opt(this)*x
	//          z and x cannot share storage
	long  ir, ic;
	long  r = (opt) ? this->Cols() : this->Rows();
	
	// Determine how to initialize z
	if(beta != 0) {
        z = y*beta;
	} else {
        z.Zero();
	}
	
	// Compute alpha * A * x
    long ncols = x.Cols();
    for (long col=0; col<ncols; col++)
    {
        for(long ir=0; ir<this->Rows(); ir++) {
            for(long ic=fIA[ir]; ic<fIA[ir+1]; ic++) {
                long jc = fJA[ic];
                z(ir,col) += alpha * fA[ic] * x.g(jc,col);
                if(jc != ir)
                {
                    z(jc,col) += alpha * fA[ic] * x.g(ir,col);
                }
            }
        }
    }
}

// ****************************************************************************
//
// Print the matrix
//
// ****************************************************************************

template<class TVar>
void TPZSYsmpMatrix<TVar>::Print(const char *title, std::ostream &out ,const MatrixOutputFormat form) const {
	// Print the matrix along with a identification title
	if(form != EInputFormat) {
		out << "\nTSYsmpMatrix Print: " << title << '\n'
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
		for(i=this->Rows()+1; i<fIA[this->Rows()]-1; i++) {
			out << i      << "\t\t"
			<< fJA[i] << '\t'
			<< fA[i]  << '\n';
		}
	} else {
		TPZMatrix<TVar>::Print(title,out,form);
	}
}


// ****************************************************************************
//
// Various solvers
//
// ****************************************************************************

template<class TVar>
void TPZSYsmpMatrix<TVar>::ComputeDiagonal() {
	if(!fDiag.size()) fDiag.resize(this->Rows());
	for(int ir=0; ir<this->Rows(); ir++) {
		fDiag[ir] = GetVal(ir,ir);
	}
}

/** @brief Fill matrix storage with randomic values */
/** This method use GetVal and PutVal which are implemented by each type matrices */
template<class TVar>
void TPZSYsmpMatrix<TVar>::AutoFill(long nrow, long ncol, int symmetric)
{
    if (!symmetric || nrow != ncol) {
        DebugStop();
    }
    TPZFMatrix<TVar> orig;
    orig.AutoFill(nrow,ncol,symmetric);
    
    TPZVec<long> IA(nrow+1);
    TPZStack<long> JA;
    TPZStack<TVar> A;
    IA[0] = 0;
    TPZVec<std::set<long> > eqs(nrow);
    for (long row=0; row<nrow; row++) {
        eqs[row].insert(row);
        for (long col = 0; col<ncol; col++) {
            REAL test = rand()*1./RAND_MAX;
            if (test > 0.5) {
                eqs[row].insert(col);
                if (symmetric) {
                    eqs[col].insert(row);
                }
            }
        }
    }
    long pos=0;
    for (long row=0; row< nrow; row++) {
        for (std::set<long>::iterator col = eqs[row].begin(); col != eqs[row].end(); col++) {
            if(*col >= row)
            {
                JA.Push(*col);
                A.Push(orig(row,*col));
            }
        }
        IA[row+1] = JA.size();
    }
    TPZMatrix<TVar>::Resize(nrow,ncol);
    SetData(IA, JA, A);
}

#ifdef USING_MKL

#include "TPZPardisoControl.h"
/**
 * @name Factorization
 * @brief Those member functions perform the matrix factorization
 * @{
 */


/**
 * @brief Decomposes the current matrix using LDLt. \n
 * The current matrix has to be symmetric.
 * "L" is lower triangular with 1.0 in its diagonal and "D" is a Diagonal matrix.
 */
template<class TVar>
int TPZSYsmpMatrix<TVar>::Decompose_LDLt(std::list<long> &singular)
{
    Decompose_LDLt();
    return 1;
}
/** @brief Decomposes the current matrix using LDLt. */
template<class TVar>
int TPZSYsmpMatrix<TVar>::Decompose_LDLt()
{
    if(this->IsDecomposed() == ELDLt) return 1;
    if (this->IsDecomposed() != ENoDecompose) {
        DebugStop();
    }
    fPardisoControl.SetMatrixType(TPZPardisoControl<TVar>::ESymmetric,TPZPardisoControl<TVar>::EIndefinite);
    fPardisoControl.Decompose();
    this->SetIsDecomposed(ELDLt);
    return 1;
    
}

/** @brief Decomposes the current matrix using Cholesky method. The current matrix has to be symmetric. */
template<class TVar>
int TPZSYsmpMatrix<TVar>::Decompose_Cholesky()
{
    if(this->IsDecomposed() == ECholesky) return 1;
    if (this->IsDecomposed() != ENoDecompose) {
        DebugStop();
    }

    fPardisoControl.SetMatrixType(TPZPardisoControl<TVar>::ESymmetric,TPZPardisoControl<TVar>::EPositiveDefinite);
    fPardisoControl.Decompose();

    this->SetIsDecomposed(ECholesky);
    return 1;
}
/**
 * @brief Decomposes the current matrix using Cholesky method.
 * @param singular
 */
template<class TVar>
int TPZSYsmpMatrix<TVar>::Decompose_Cholesky(std::list<long> &singular)
{
    return Decompose_Cholesky();
}



/** @} */


/**
 * @brief Computes B = Y, where A*Y = B, A is lower triangular with A(i,i)=1.
 * @param b right hand side and result after all
 */
template<class TVar>
int TPZSYsmpMatrix<TVar>::Subst_LForward( TPZFMatrix<TVar>* b ) const
{
    TPZFMatrix<TVar> x(*b);
    fPardisoControl.Solve(*b,x);
    *b = x;
    return 1;
}

/**
 * @brief Computes B = Y, where A*Y = B, A is upper triangular with A(i,i)=1.
 * @param b right hand side and result after all
 */
template<class TVar>
int TPZSYsmpMatrix<TVar>::Subst_LBackward( TPZFMatrix<TVar>* b ) const
{
    return 1;
}

/**
 * @brief Computes B = Y, where A*Y = B, A is diagonal matrix.
 * @param b right hand side and result after all
 */
template<class TVar>
int TPZSYsmpMatrix<TVar>::Subst_Diag( TPZFMatrix<TVar>* b ) const
{
    return 1;
}

/**
 * @brief Computes B = Y, where A*Y = B, A is lower triangular.
 * @param b right hand side and result after all
 */
template<class TVar>
int TPZSYsmpMatrix<TVar>::Subst_Forward( TPZFMatrix<TVar>* b ) const
{
    TPZFMatrix<TVar> x(*b);
    fPardisoControl.Solve(*b,x);
    *b = x;
    return 1;
}

/**
 * @brief Computes B = Y, where A*Y = B, A is upper triangular.
 * @param b right hand side and result after all
 */
template<class TVar>
int TPZSYsmpMatrix<TVar>::Subst_Backward( TPZFMatrix<TVar>* b ) const
{
    return 1;
}


#endif



template class TPZSYsmpMatrix<double>;
template class TPZSYsmpMatrix<float>;