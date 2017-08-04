/* Generated by Together */// $Id: TPZYCMohrCoulomb.h,v 1.9 2010-12-13 19:34:58 diogo Exp $

#ifndef TPZYCMOHRCOULOMB_H
#define TPZYCMOHRCOULOMB_H

#include "TPZTensor.h"
#include "pzlog.h"
#ifndef WIN32
#include <fenv.h>//NAN DETECTOR
#endif

#ifdef LOG4CXX_PLASTICITY
static LoggerPtr logMohr(Logger::getLogger("TPZYCMohrOriginal"));
#endif

class TPZYCMohrCoulomb  {
	
public:
	
 
	enum {NYield=3};
    
    const char * Name() const
    {
		return "TPZYCMohrCoulomb";	
    }
	
    void Print(std::ostream & out) const
    {
		out << Name();
    }
    
	int GetForceYield()
	{
		return 0; // nothing to be done in this yield criterion
	}
	
	void SetForceYield(const int forceYield)
	{
		// nothing to be done in this yield criterion
	}
	
	/**
	 * Checks if the proposed yield state leads to post-peak material behaviour. If so, the material
	 * is forced to behave in post-peak in order to avoid equation switching during Newton's method
	 * in the PlasticLoop routines.
	 * @param [in] sigma stress state
	 * @param [in] A Thermo Force
	 */
	void SetYieldStatusMode(const TPZTensor<REAL> & sigma, const REAL & A)
	{
		// nothing to be done in this yield criterion
	}
	
	
	/**
	 * Setup of material parameters
	 * @param[in] phi Mohr Coulomb's internal friction angle
	 * VERY IMPORTANT!! The ThermoForceA parameters should be set as:
	 *      fk: Herdening slope for the cohesion
	 *      fYield0 : equivalent Mohr Coulomb cohesion C
	 */
	void SetUp(const REAL & phi) 
	{
		fPhi = phi;
	}
	
	
	/**
	 * Evaluate the yield criteria
	 * @param[in] sigma current stress tensor
	 * @param[in] A current thermodynamical force
	 * @param[out] result Derivative
	 * @param[in] checkForcedYield indicates wether to force post-peak failure behavior
	 */
	template < class T>
	void Compute(const TPZTensor<T> & sigma, const T & A, TPZVec<T> &result, int checkForcedYield = 0) const;
	
	/**
	 * Derivative of the yield function
	 * @param[in] sigma current stress tensor
	 * @param[in] A current thermodynamical force
	 * @param[out] Ndir Stress derivative
	 * @param[in] checkForcedYield indicates wether to force post-peak failure behavior
	 */
	template <class T>
	void N(const TPZTensor<T> & sigma,const T & A,  TPZVec<TPZTensor<T> > & Ndir, int checkForcedYield = 0) const;
	
	/**
	 * Derivative of the yield function with respect to the thermodynamical force
	 * @param[in] sigma current stress tensor
	 * @param[in] A current thermodynamical force
	 * @param[out] h Derivative with respect to thermodynamical force
	 * @param[in] checkForcedYield indicates wether to force post-peak failure behavior
	 */
	template <class T>
	void H(const TPZTensor<T> & sigma,const T & A,  TPZVec<T> & h, int checkForcedYield = 0) const;

    /**
     * Multiplicador para o caso onde utilizamos uma variavel de dano modificada
     */
    template <class T>
    void AlphaMultiplier(const T &A, T &multiplier) const
    {
        multiplier = T(1.);
    }

    void Write(TPZStream &buf) const
    {
        buf.Write(&fPhi);
    }
    
    void Read(TPZStream &buf)
    {
        buf.Read(&fPhi);
    }
public:
	
	REAL fPhi;
    
public:
    //////////////////CheckConv related methods/////////////////////
    
    /** @brief Number of types of residuals */
    int NumCases()
    {
        return 3;
    }
    TPZTensor<REAL> gRefTension;
    
    /** @brief LoadState will keep a given state as static variable of the class */
    void LoadState(TPZFMatrix<REAL> &state)
    {
        int i;
        for(i=0; i<6; i++) gRefTension.fData[i] = state(i,0);
    }
    void ComputeTangent(TPZFMatrix<REAL> &tangent, TPZVec<REAL> &coefs, int icase)
    {
        switch(icase)
        {
            case 0:
            {
                TPZVec<TPZTensor<REAL> > Ndir(3);
                REAL yield = 1.e6;
                this->N<REAL>(gRefTension,yield, Ndir, 0);
                tangent.Redim(1,9);
                tangent(0,0)=Ndir[0].XX();tangent(0,1)=Ndir[0].XY();tangent(0,2)=Ndir[0].XZ();
                tangent(1,0)=Ndir[0].XY();tangent(1,1)=Ndir[0].YY();tangent(1,2)=Ndir[0].YZ();
                tangent(2,0)=Ndir[0].XZ();tangent(2,1)=Ndir[0].YZ();tangent(2,2)=Ndir[0].ZZ();
                
                break;
            }
            case 1:
            {
                TPZVec<TPZTensor<REAL> > Ndir(3);
                REAL yield = 1.e6;
                this->N<REAL>(gRefTension,yield, Ndir, 0);
                tangent.Redim(1,9);
                tangent(0,0)=Ndir[0].XX();tangent(0,1)=Ndir[0].XY();tangent(0,2)=Ndir[0].XZ();
                tangent(1,0)=Ndir[0].XY();tangent(1,1)=Ndir[0].YY();tangent(1,2)=Ndir[0].YZ();
                tangent(2,0)=Ndir[0].XZ();tangent(2,1)=Ndir[0].YZ();tangent(2,2)=Ndir[0].ZZ();
                break;
            }
            case 2:
            {
                TPZVec<TPZTensor<REAL> > Ndir(3);
                REAL yield = 1.e6;
                this->N<REAL>(gRefTension,yield, Ndir, 0);
                tangent.Redim(1,9);
                tangent(0,0)=Ndir[0].XX();tangent(0,1)=Ndir[0].XY();tangent(0,2)=Ndir[0].XZ();
                tangent(1,0)=Ndir[0].XY();tangent(1,1)=Ndir[0].YY();tangent(1,2)=Ndir[0].YZ();
                tangent(2,0)=Ndir[0].XZ();tangent(2,1)=Ndir[0].YZ();tangent(2,2)=Ndir[0].ZZ();
                break;
            }

        }
    }
    
    void Residual(TPZFMatrix<REAL> &res,int icase)
    {
        
        res.Redim(1,1);
        TPZTensor<REAL> grad;
        
        switch(icase)
        {

            case 0:
            {
                TPZVec<REAL> phi(3);
                REAL yield = 1.e6;
                this->Compute(gRefTension,yield,phi,0);
                res(0,0) = phi[0];
                break;
            }
            case 1:
            {
                TPZVec<REAL> phi(3);
                REAL yield = 1.e6;
                this->Compute(gRefTension,yield,phi,0);
                res(0,0) = phi[1];
                break;
            }
            case 2:
            {
                TPZVec<REAL> phi(3);
                REAL yield = 1.e6;
                this->Compute(gRefTension,yield,phi,0);
                res(0,0) = phi[2];
                break;
            }
        }
        
    }
    
    


   
	
};

template < class T>
void TPZYCMohrCoulomb::Compute(const TPZTensor<T> & sigma, const T & A,TPZVec<T> &res, int checkForcedYield) const
{
	
	TPZTensor<T> EigenValues,EigenVec1,EigenVec2,EigenVec3;
  //  sigma.EigenSytem(EigenValues,EigenVec1,EigenVec2,EigenVec3);
    T sigma1,sigma2,sigma3;
    sigma1 = EigenValues.XX();
    sigma2 = EigenValues.YY();
    sigma3 = EigenValues.ZZ();

    T res1,res2,res6;

    res1=(sigma1 - sigma3) + (sigma1 + sigma3)*sin(fPhi) - T(2.)*A*cos(fPhi);//1 3
    res6=(sigma1 - sigma2) + (sigma1 + sigma2)*sin(fPhi) - T(2.)*A*cos(fPhi);// 1 2
    res2=(sigma2 - sigma3) + (sigma2 + sigma3)*sin(fPhi) - T(2.)*A*cos(fPhi);// 2 3
    
    res[0]=res1;
    res[1]=res6;
    res[2]=res2;
    
//    cout << "res[0] = " << res[0]<< endl;
//    cout << "res[0] = " << res[1]<< endl;
//    cout << "res[0] = " << res[2]<< endl;

}

template <class T>
void TPZYCMohrCoulomb::N(const TPZTensor<T> & sigma,const T & A,  TPZVec<TPZTensor<T> > & Ndir, int checkForcedYield) const
{
    
    TPZTensor<T> EigenValues, EigenVec1,EigenVec2,EigenVec3;
  //  sigma.EigenSytem(EigenValues,EigenVec1,EigenVec2,EigenVec3);
    
    T umplussin,umminsin;
    TPZTensor<T> e1(EigenVec1),e3(EigenVec3),e1b(EigenVec1),e2b(EigenVec2),e2c(EigenVec2),e3c(EigenVec3);
    umplussin = 1.+sin(fPhi);
    umminsin = 1.-sin(fPhi);
    
    //phi1 = 0
    //( 1 + sin(phi) )* e1 x e1 - ( 1 - sin(phi) )* e3 x e3
    e1*=umplussin;
    e3*=umminsin;
    e1-=e3;
    Ndir[0]=e1;

    //phi6 =0 right
    //( 1 + sin(phi) )* e1 x e1 - ( 1 - sin(phi) )* e2 x e2
    e1b*=umplussin;
    e2b*=umminsin;
    e1b-=e2b;
    Ndir[1]=e1b;
    
    
    //phi2 = 0 left
     //( 1 + sin(phi) )* e2 x e2 - ( 1 - sin(phi) )* e3 x e3
    e2c*=umplussin;
    e3c*=umminsin;
    e2c-=e3c;
    Ndir[2]=e2c;
    
//    cout << "NDIR 0 = "<< Ndir[0]<< endl;
//    cout << "NDIR 1 = "<< Ndir[1]<< endl;
//    cout << "NDIR 2 = "<< Ndir[2]<< endl;

    
}

template <class T>
void TPZYCMohrCoulomb::H(const TPZTensor<T> & sigma,const T & A,  TPZVec<T> & h, int checkForcedYield) const
{
    
    h[0]=2*cos(fPhi);
    h[1]=2*cos(fPhi);
    h[2]=2*cos(fPhi);
 
    
}

#endif