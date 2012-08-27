/**
 * @file
 * @brief Contains the implementation of the TPZMatRed methods.
 */

#include <stdlib.h>
#include <stdio.h>
#include <fstream>
using namespace std;


#include "pzmatred.h"
#include "pzfmatrix.h"
#include "pzstepsolver.h"

#include <sstream>
#include "pzlog.h"
#ifdef LOG4CXX
static LoggerPtr logger(Logger::getLogger("pz.matrix.tpzmatred"));
#endif

/*************************** Public ***************************/

/******************/
/*** Construtor ***/

template<class TVar, class TSideMatrix>
TPZMatRed<TVar,  TSideMatrix>::TPZMatRed () : TPZMatrix<TVar>( 0, 0 ), fK11(0,0),fK01(0,0),fK10(0,0),fF0(0,0),fF1(0,0), fMaxRigidBodyModes(0), fNumberRigidBodyModes(0)
{
	fDim0=0;
	fDim1=0;
	fF0IsComputed=0;
    fK00IsDecomposed = 0;
	fK11IsReduced=0;
	fK01IsComputed = 0;
	fF1IsReduced=0;
	fIsReduced = 0;
}

template<class TVar, class TSideMatrix>
TPZMatRed<TVar, TSideMatrix>::TPZMatRed( int dim, int dim00 ):TPZMatrix<TVar>( dim,dim ), fK11(dim-dim00,dim-dim00,0.), fK01(dim00,dim-dim00,0.), 
fK10(dim-dim00,dim00,0.), fF0(dim00,1,0.),fF1(dim-dim00,1,0.), fMaxRigidBodyModes(0), fNumberRigidBodyModes(0)
{
	if(dim<dim00) TPZMatrix<TVar>::Error(__PRETTY_FUNCTION__,"dim k00> dim");
	fDim0=dim00;
	fDim1=dim-dim00;
	fF0IsComputed=0;
    fK00IsDecomposed = 0;
	fK11IsReduced=0;
	fK01IsComputed = 0;
	fF1IsReduced=0;
	fIsReduced = 0;
}

template<class TVar, class TSideMatrix>
TPZMatRed<TVar, TSideMatrix >::~TPZMatRed(){
}

template<class TVar, class TSideMatrix>
int TPZMatRed<TVar, TSideMatrix>::IsSimetric() const {
	if(fK00) return this->fK00->IsSimetric();
	return 0;
}

template<class TVar, class TSideMatrix>
void TPZMatRed<TVar, TSideMatrix>::Simetrize() {
	// considering fK00 is simetric, only half of the object is assembled.
	// this method simetrizes the matrix object
	
	if(!fK00 || !this->fK00->IsSimetric()) return;
	fK01.Transpose(&fK10);
	int row,col;
	for(row=0; row<fDim1; row++) {
		for(col=row+1; col<fDim1; col++) {
			(fK11)(col,row) = (fK11)(row,col);
		}
	}
}

template<class TVar, class TSideMatrix>
int
TPZMatRed<TVar, TSideMatrix>::PutVal(const int r,const int c,const TVar& value ){
	int row(r),col(c);
	if (IsSimetric() && row > col ) Swap( &row, &col );
	if (row<fDim0 &&  col<fDim0)  fK00->PutVal(row,col,value);
	if (row<fDim0 &&  col>=fDim0)  fK01.PutVal(row,col-fDim0,(TVar)value);
	if (row>=fDim0 &&  col<fDim0)  fK10.PutVal(row-fDim0,col,(TVar)value);
	if (row>=fDim0 &&  col>=fDim0)  fK11.PutVal(row-fDim0,col-fDim0,(TVar)value);

	return( 1 );
}

template<class TVar, class TSideMatrix>
const TVar&
TPZMatRed<TVar,TSideMatrix>::GetVal(const int r,const int c ) const {
	int row(r),col(c);
	
	if (IsSimetric() && row > col ) Swap( &row, &col );
	if (row<fDim0 &&  col<fDim0)  return ( fK00->GetVal(row,col) );
	if (row<fDim0 &&  col>=fDim0)  return ( fK01.GetVal(row,col-fDim0) );
	if (row>=fDim0 &&  col<fDim0)  return ( fK10.GetVal(row-fDim0,col) );
	return (fK11.GetVal(row-fDim0,col-fDim0) );
	
}

template<class TVar, class TSideMatrix>
TVar& TPZMatRed<TVar,TSideMatrix>::s(const int r,const int c ) {
	int row(r),col(c);
	
	if (r < fDim0 && IsSimetric() && row > col ) Swap( &row, &col );
	if (row<fDim0 &&  col<fDim0)  return ( fK00->s(row,col) );
	if (row<fDim0 &&  col>=fDim0)  return ( (TVar &)fK01.s(row,col-fDim0) );
	if (row>=fDim0 &&  col<fDim0)  return ( (TVar &)(fK10.s(row-fDim0,col)) );
	return ((TVar &)(fK11.s(row-fDim0,col-fDim0)) );
	
}

template<class TVar, class TSideMatrix>
void TPZMatRed<TVar,TSideMatrix>::SetSolver(TPZAutoPointer<TPZMatrixSolver<TVar> > solver)
{
	fK00=solver->Matrix();
	fSolver = solver;
}


template<class TVar,class TSideMatrix>
void
TPZMatRed<TVar, TSideMatrix>::SetK00(TPZAutoPointer<TPZMatrix<TVar> > K00)
{
	fK00=K00;
}

template<class TVar, class TSideMatrix>
void TPZMatRed<TVar,TSideMatrix>::SetF(const TPZFMatrix<TVar> & F)
{
	
	int FCols=F.Cols(),c,r,r1;
	
	fF0.Redim(fDim0,FCols);
	fF1.Redim(fDim1,FCols);
	
	for(c=0; c<FCols; c++){
		r1=0;
		for(r=0; r<fDim0; r++){
			fF0.PutVal( r,c,F.GetVal(r,c) ) ;
		}
		//aqui r=fDim0
		for( ;r<this->Rows(); r++){
			fF1.PutVal( r1++,c,F.GetVal(r,c) );
		}
	}
	fF1IsReduced = 0;
	fF0IsComputed = 0;
}

template<class TVar, class TSideMatrix>
void TPZMatRed<TVar,TSideMatrix>::GetF( TPZFMatrix<TVar> & F)
{
	
	int FCols=fF0.Cols(),c,r,r1;
	
	F.Redim(fDim0+fDim1,FCols);
	
	for(c=0; c<FCols; c++){
		r1=0;
		for(r=0; r<fDim0; r++){
			F.PutVal( r,c,fF0.GetVal(r,c) ) ;
		}
		//aqui r=fDim0
		for( ;r< fDim0+fDim1; r++){
			F.PutVal( r,c,fF1.GetVal(r1++,c) );
		}
	}

}

template<class TVar, class TSideMatrix>
const TPZFMatrix<TVar>& TPZMatRed<TVar, TSideMatrix>::F1Red()
{
	if (!fDim0 || fF1IsReduced)  return (fF1);
	if(! fF0IsComputed)
	{
        DecomposeK00();
		fSolver->Solve(fF0,fF0);
		fF0IsComputed = 1;
	}
	
	//make [F1]=[F1]-[K10][K0]
	fK10.MultAdd((fF0),(fF1),(fF1),-1,1);
	fF1IsReduced=1;
	return (fF1);
}

template<class TVar, class TSideMatrix>
const TPZFMatrix<TVar>& TPZMatRed<TVar,TSideMatrix>::K11Red()
{
	if (!fDim0 || fK11IsReduced)  {
		//Simetrize();
		return (fK11);
	}
	
	if(!fK01IsComputed)
	{
        DecomposeK00();
		Simetrize();
		fSolver->Solve(fK01,fK01);
        TPZStepSolver<TVar> *step = dynamic_cast<TPZStepSolver<TVar> *>(fSolver.operator->());
        std::cout << "Address " << (void *) step << " Number of singular modes " << step->Singular().size() << std::endl;
		fK01IsComputed = 1;
	}
	
	fK10.MultAdd(fK01,(fK11),(fK11),-1,1);
	fK11IsReduced=1;
	return (fK11);
}

#include "tpzverysparsematrix.h"

template<>
const TPZFMatrix<double>& TPZMatRed<double, TPZVerySparseMatrix<double> >::K11Red()
{
	std::cout << __PRETTY_FUNCTION__ << " should never be called\n";	
	static TPZFMatrix<double> temp;
	return temp;
}

template<>
const TPZFMatrix<float>& TPZMatRed<float, TPZVerySparseMatrix<float> >::K11Red()
{
	std::cout << __PRETTY_FUNCTION__ << " should never be called\n";	
	static TPZFMatrix<float> temp;
	return temp;
}

template<>
const TPZFMatrix<long double>& TPZMatRed<long double, TPZVerySparseMatrix<long double> >::K11Red()
{
	std::cout << __PRETTY_FUNCTION__ << " should never be called\n";	
	static TPZFMatrix<long double> temp;
	return temp;
}

template<>
const TPZFMatrix<std::complex<double> >& TPZMatRed<std::complex<double>, TPZVerySparseMatrix<std::complex<double> > >::K11Red()
{
	std::cout << __PRETTY_FUNCTION__ << " should never be called\n";	
	static TPZFMatrix<std::complex<double> > temp;
	return temp;
}


template<class TVar ,class TSideMatrix>
void TPZMatRed<TVar,TSideMatrix>::U1(TPZFMatrix<TVar> & F)
{
	
	K11Red();
	F1Red();
	F=(fF1);
	fK11.SolveDirect( F ,ELU);
	
	
}

template<>
void TPZMatRed<REAL, TPZVerySparseMatrix<REAL> >::UGlobal(const TPZFMatrix<REAL> & U1, TPZFMatrix<REAL> & result)
{
	//[u0]=[A00^-1][F0]-[A00^-1][A01]
    if( !fF0IsComputed ){
		//compute [F0]=[A00^-1][F0]
        DecomposeK00();
		fSolver->Solve(fF0,fF0);
		fF0IsComputed=1;
	}
	
	if(!fK01IsComputed)
	{
		TPZFMatrix<REAL> k01(fK01);
        DecomposeK00();
		fSolver->Solve(k01,k01);
		fK01 = k01;
		fK01IsComputed = 1;
	}
	
	//make [u0]=[F0]-[U1]
	TPZFMatrix<REAL> u0( fF0.Rows() , fF0.Cols() );
	fK01.MultAdd(U1,(fF0),u0,-1,0);
	
	result.Redim( Rows(),fF0.Cols() );
	int c,r,r1;
	
	for(c=0; c<fF0.Cols(); c++)
	{
		r1=0;
		for(r=0; r<fDim0; r++)
		{
			result.PutVal( r,c,u0.GetVal(r,c) ) ;
		}
		//aqui r=fDim0
		for( ;r<Rows(); r++)
		{
			result.PutVal( r,c,U1.GetVal(r1++,c) );
		}
	}
}

template<class TVar, class TSideMatrix>
void TPZMatRed<TVar, TSideMatrix >::UGlobal(const TPZFMatrix<TVar> & U1, TPZFMatrix<TVar> & result)
{
	TPZFMatrix<TVar> u0( fF0.Rows() , fF0.Cols() );
	
	if(fK01IsComputed)
	{
		//[u0]=[A00^-1][F0]-[A00^-1][A01]
		if( !fF0IsComputed ){
			//compute [F0]=[A00^-1][F0]
            DecomposeK00();
			fSolver->Solve(fF0,fF0);
			fF0IsComputed=1;
		}		
		//make [u0]=[F0]-[U1]
		fK01.MultAdd(U1,(fF0),u0,-1,1);
	} else {
		if (fF0IsComputed) {
			TPZFMatrix<TVar> K01U1(fK01.Rows(),U1.Cols(),0.);
			fK01.MultAdd(U1,U1,K01U1,-1.);
            DecomposeK00();
			fSolver->Solve(K01U1, u0);
			u0 += fF0;
		}
		else {
			TPZFMatrix<TVar> K01U1(fK01.Rows(),U1.Cols(),0.);
			fK01.MultAdd(U1,fF0,K01U1,-1.,1.);
            DecomposeK00();
			fSolver->Solve(K01U1, u0);
		}
	}
	
	//compute result
#ifdef LOG4CXX
	if(logger->isDebugEnabled())
	{
		std::stringstream sout;
		fF0.Print("fF0 ",sout);
		u0.Print("u0 " ,sout);
		LOGPZ_DEBUG(logger,sout.str())   
		
	}
#endif
	
	result.Redim( this->Rows(),fF0.Cols() );
	int c,r,r1;
	
	for(c=0; c<fF0.Cols(); c++)
	{
		r1=0;
		for(r=0; r<fDim0; r++)
		{
			result.PutVal( r,c,u0.GetVal(r,c) ) ;
		}
		//aqui r=fDim0
		for( ;r<this->Rows(); r++)
		{
			result.PutVal( r,c,U1.GetVal(r1++,c) );
		}
	}
}

template<class TVar, class TSideMatrix>
void TPZMatRed<TVar, TSideMatrix>::UGlobal2(TPZFMatrix<TVar> & U1, TPZFMatrix<TVar> & result)
{
	TPZFMatrix<TVar> u0( fF0.Rows() , fF0.Cols() );
	
	if(fK01IsComputed)
	{
		//[u0]=[A00^-1][F0]-[A00^-1][A01][u1]
		if( !fF0IsComputed ){
			//compute [F0]=[A00^-1][F0]
            DecomposeK00();
			fSolver->Solve(fF0,fF0);
			fF0IsComputed=1;
		}		
		//make [u0]=[F0]-[U1]
		fK01.MultAdd(U1,(fF0),u0,-1,1);
	} else {
		if (fF0IsComputed) {
			TPZFMatrix<TVar> K01U1(fK01.Rows(),U1.Cols(),0.);
			fK01.MultAdd(U1,U1,K01U1,-1.);
            DecomposeK00();
			fSolver->Solve(K01U1, u0);
			u0 += fF0;
		}
		else {
			TPZFMatrix<TVar> K01U1(fK01.Rows(),U1.Cols(),0.);
			fK01.MultAdd(U1,fF0,K01U1,-1.,1.);
            DecomposeK00();
			fSolver->Solve(K01U1, u0);
		}
	}
	
	//compute result
#ifdef LOG4CXX
	if(logger->isDebugEnabled())
	{
		std::stringstream sout;
		fF0.Print("fF0 ",sout);
		u0.Print("u0 " ,sout);
		LOGPZ_DEBUG(logger,sout.str())   
		
	}
#endif
	
	result.Redim( fDim0+fDim1,fF0.Cols() );
    int nglob = fDim0+fDim1;
	
	int c,r,r1;
	
	for(c=0; c<fF0.Cols(); c++)
	{
		r1=0;
		for(r=0; r<fDim0; r++)
		{
			result(r,c) = u0(r,c) ;
		}
		//aqui r=fDim0
		for( ;r<nglob; r++)
		{
			result(r,c) = U1(r1++,c);
		}
	}
}


template<class TVar,class TSideMatrix>
void TPZMatRed<TVar, TSideMatrix>::Print(const char *name , std::ostream &out ,const MatrixOutputFormat form ) const
{	
	if(form != EInputFormat) {
		out << "Writing matrix 'TPZMatRed<TSideMatrix>::" << name;
		out << "' (" << this->Dim() << " x " << this->Dim() << "):\n";
		
		fK00->Print("K(0,0)",out,form);
		fK01.Print("K(0,1)",out,form);
		fK10.Print("K(1,0)",out,form);
		fK11.Print("K(1,1)",out,form);
		
		
		fF0.Print("F(0)",out,form);
		fF1.Print("F(1)",out,form);
		
		out << "\n\n";
	} else {
		TPZMatrix<TVar>::Print(name,out,form);
	}	
}

template<class TVar, class TSideMatrix>
int TPZMatRed<TVar,TSideMatrix>::Redim(int dim, int dim00){
	if(dim<dim00) TPZMatrix<TVar>::Error(__PRETTY_FUNCTION__,"dim k00> dim");
	if(fK00) fK00->Redim(dim00,dim00);
	if(fF0)  delete fF0;
	if(fF1)  delete fF1;
	
	fDim0=dim00;
	fDim1=dim-dim00;
	fF0IsComputed=0;
    fK00IsDecomposed = 0;
	fK11IsReduced=0;
	fF1IsReduced=0;
	
	fK01.Redim(fDim0,fDim1);
	fK10.Redim(fDim1,fDim0);
	fK11.Redim(fDim1,fDim1);
	
	fF0=(TVar)NULL;
	fF1=(TVar)NULL;
	this->fRow = dim;
	this->fCol = dim;
	return 0;
}


template<class TVar, class TSideMatrix>
int TPZMatRed<TVar, TSideMatrix>::Zero(){
	if(fK00) fK00->Zero();
	fK01.Zero();
	fK10.Zero();
	fK11.Zero();
	fF0.Zero();
	fF1.Zero();
	fF0IsComputed=0;
    fK00IsDecomposed=0;
	fK11IsReduced=0;
	fF1IsReduced=0;
	return 0;
}


template<class TVar, class TSideMatrix>
void TPZMatRed<TVar, TSideMatrix>::MultAdd(const TPZFMatrix<TVar> &x,
										   const TPZFMatrix<TVar> &y, TPZFMatrix<TVar> &z,
										   const TVar alpha,const TVar beta,
										   const int opt,const int stride) const
{
	// #warning Not functional yet. Still need to Identify all the variables	
	if(!fIsReduced)
	{
		LOGPZ_WARN(logger,"TPZMatRed not reduced, expect trouble")
		TPZMatrix<TVar>::MultAdd(x,y,z,alpha,beta,opt,stride);
		return;
	}
	
	this->PrepareZ(y,z,beta,opt,stride);
	
	if(!opt)
	{
		if(fK01IsComputed)
		{
			DebugStop();
		}
		
		TPZFMatrix<TVar> l_Res(fK01.Rows(), x.Cols(), 0);
		fK01.Multiply(x,l_Res,0,1);
		fSolver->Solve(l_Res,l_Res);
#ifdef LOG4CXX
		if(logger->isDebugEnabled())
		{
			std::stringstream sout;
			l_Res.Print("Internal solution",sout);
			LOGPZ_DEBUG(logger,sout.str())
		}
#endif
		TPZFMatrix<TVar> l_ResFinal(fK11.Rows(), x.Cols(), 0);
		fK11.Multiply(x,l_ResFinal,0,1);
#ifdef LOG4CXX
		if(logger->isDebugEnabled())
		{
			std::stringstream sout;
			l_ResFinal.Print("Intermediate product l_ResFinal",sout);
			LOGPZ_DEBUG(logger,sout.str())
		}
#endif
		fK10.MultAdd(l_Res,l_ResFinal,z,-alpha,alpha,opt,stride);
#ifdef LOG4CXX
		if(logger->isDebugEnabled())
		{
			std::stringstream sout;
			z.Print("Final result z ",sout);
			LOGPZ_DEBUG(logger,sout.str())
		}
#endif
	}
	else
	{
		DebugStop();
	}
}

/** @brief Decompose K00 and adjust K01 and K10 to reflect rigid body modes */
template<class TVar, class TSideMatrix>
void TPZMatRed<TVar, TSideMatrix>::DecomposeK00()
{
    if(fK00IsDecomposed)
    {
        return;
    }
    TPZStepSolver<TVar> *stepsolve = dynamic_cast<TPZStepSolver<TVar> *>(fSolver.operator->());
    TPZStepSolver<TVar> *directsolve(0);
    if(!stepsolve)
    {
        DebugStop();
    }
    if(stepsolve->Solver() == TPZMatrixSolver<TVar>::EDirect)
    {
        directsolve = stepsolve;
    }
    if(!directsolve)
    {
        TPZSolver<TVar> *presolve = stepsolve->PreConditioner();
        TPZStepSolver<TVar> *prestep = dynamic_cast<TPZStepSolver<TVar> *>(presolve);
        if(prestep->Solver() == TPZMatrixSolver<TVar>::EDirect)
        {
            prestep->UpdateFrom(stepsolve->Matrix());
            directsolve = prestep;
        }
    }
    if (directsolve)
    {
        directsolve->Decompose();
        std::list<int> &singular = directsolve->Singular();
        std::list<int>::iterator it;
        int nsing = singular.size();
        if(nsing > fMaxRigidBodyModes-fNumberRigidBodyModes)
        {
            std::cout << "Number of rigid body modes larger than provision\n";
            std::cout << "Number of singular modes " << nsing << std::endl;
            std::cout << "Number of rigid body modes reserved " << fMaxRigidBodyModes << std::endl;
            std::cout << "Rigid body modes ";
            for (it=singular.begin(); it != singular.end(); it++) {
                std::cout << " " << *it;
            }
            std::cout << std::endl;
            //DebugStop();
        }
        for (it=singular.begin(); it != singular.end(); it++) {
            if(fNumberRigidBodyModes < fMaxRigidBodyModes)
            {
                fK01(*it,fDim1-fMaxRigidBodyModes+fNumberRigidBodyModes) = -1.;
                fK10(fDim1-fMaxRigidBodyModes+fNumberRigidBodyModes,*it) = -1.;
                fK11(fDim1-fMaxRigidBodyModes+fNumberRigidBodyModes,fDim1-fMaxRigidBodyModes+fNumberRigidBodyModes) = 1.;
                if(stepsolve != directsolve)
                {
                    TVar diag = stepsolve->Matrix()->GetVal(*it, *it)+ (TVar)1.;
                    stepsolve->Matrix()->PutVal(*it, *it, diag);
                }
            }
            fNumberRigidBodyModes++;
        }
        fK00IsDecomposed = true;
    }
    else
    {
        DebugStop();
    }
}

template<class TVar,class TSideMatrix>
void TPZMatRed<TVar, TSideMatrix>::Write(TPZStream &buf, int withclassid)
{
	TPZMatrix<TVar>::Write(buf, withclassid);
	{//Ints
		buf.Write(&this->fDim0, 1);
		buf.Write(&this->fDim1, 1);
	}
	{//chars
		buf.Write(&this->fDefPositive, 1);
		buf.Write(&this->fF0IsComputed, 1);
        buf.Write(&this->fK00IsDecomposed,1);
		buf.Write(&this->fF1IsReduced, 1);
		buf.Write(&this->fIsReduced, 1);
		buf.Write(&this->fK01IsComputed, 1);
		buf.Write(&this->fK11IsReduced, 1);
		buf.Write(&this->fMaxRigidBodyModes, 1);
		buf.Write(&this->fNumberRigidBodyModes, 1);
	}
	{//Aggregates
		this->fF0.Write(buf, 0);
		this->fF1.Write(buf, 0);
		if(this->fK00)
		{
			this->fK00->Write(buf, 1);
		}
		else
		{
			int flag = 0;
			buf.Write(&flag, 1);
		}
		this->fK01.Write(buf, 0);
		this->fK10.Write(buf, 0);
		this->fK11.Write(buf, 0);
		if(fSolver)
		{
			if(fSolver->Matrix() != fK00)
			{
				std::cout << "Error\n";
			}
			else
			{
				fSolver->Write(buf, 1);
				//TODO Enviar o solver, atenção com a Matrix do Solver;
			}
			
		}
		else
		{
			int flag = -1;
			buf.Write(&flag, 1);
		}
		
	}
	
}

template<class TVar, class TSideMatrix>
void TPZMatRed<TVar, TSideMatrix>::Read(TPZStream &buf, void *context)
{
	TPZMatrix<TVar>::Read(buf, context);
	{//Ints
		buf.Read(&this->fDim0, 1);
		buf.Read(&this->fDim1, 1);
	}
	{//chars
		buf.Read(&this->fDefPositive, 1);
		buf.Read(&this->fF0IsComputed, 1);
        buf.Read(&this->fK00IsDecomposed,1);
		buf.Read(&this->fF1IsReduced, 1);
		buf.Read(&this->fIsReduced, 1);
		buf.Read(&this->fK01IsComputed, 1);
		buf.Read(&this->fK11IsReduced, 1);
		buf.Read(&this->fMaxRigidBodyModes, 1);
		buf.Read(&this->fNumberRigidBodyModes, 1);
	}
	{//Aggregates
		this->fF0.Read(buf, 0);
		this->fF1.Read(buf, 0);
        TPZSaveable *sav = TPZSaveable::Restore(buf, 0);
        TPZMatrix<TVar> *mat = dynamic_cast<TPZMatrix<TVar> *>(sav);
        if(sav && !mat)
        {
            DebugStop();
        }
        fK00 = mat;
		this->fK01.Read(buf, 0);
		this->fK10.Read(buf, 0);
		this->fK11.Read(buf, 0);
        sav = TPZSaveable::Restore(buf, 0);
        TPZMatrixSolver<TVar> *matsolv = dynamic_cast<TPZMatrixSolver<TVar> *>(sav);
        if (sav && !matsolv) {
            DebugStop();
        }
        if(matsolv)
        {
            fSolver = matsolv;
        }
	}
}

#include "tpzverysparsematrix.h"

template <>
int TPZMatRed<REAL, TPZVerySparseMatrix<REAL> >::ClassId() const
{
    return TPZMATRED_VERYSPARSE_ID;
}
template <>
int TPZMatRed<REAL, TPZFMatrix<REAL> >::ClassId() const
{
    return TPZMATRED_FMATRIX_ID;
}

template<class TVar, class TSideMatrix>
int TPZMatRed<TVar,TSideMatrix>::ClassId() const
{
    DebugStop();
    return -1;
}

template class TPZMatRed<float, TPZVerySparseMatrix<float> >;
template class TPZMatRed<float, TPZFMatrix<float> >;

template class TPZMatRed<double, TPZVerySparseMatrix<double> >;
template class TPZMatRed<double, TPZFMatrix<double> >;

template class TPZMatRed<long double, TPZVerySparseMatrix<long double> >;
template class TPZMatRed<long double, TPZFMatrix<long double> >;

template class TPZMatRed<std::complex<double>, TPZVerySparseMatrix<std::complex<double> > >;

template class TPZMatRed<std::complex<float>, TPZFMatrix<std::complex<float> > >;
template class TPZMatRed<std::complex<double>, TPZFMatrix<std::complex<double> > >;
template class TPZMatRed<std::complex<long double>, TPZFMatrix<std::complex<long double> > >;

template class TPZRestoreClass<TPZMatRed<REAL,TPZVerySparseMatrix<REAL> >, TPZMATRED_VERYSPARSE_ID>;
template class TPZRestoreClass<TPZMatRed<REAL, TPZFMatrix<REAL> >, TPZMATRED_FMATRIX_ID>;
