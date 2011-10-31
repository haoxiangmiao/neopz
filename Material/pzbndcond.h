/**
 * \file
 * @brief Contains the TPZBndCond class which implements a boundary condition for TPZMaterial objects.
 */

//$Id: pzbndcond.h,v 1.30 2011-05-26 03:28:57 phil Exp $

#ifndef BNDCONDHPP
#define BNDCONDHPP

#include <iostream>

#include "pzreal.h"
#include "pzdiscgal.h"
#include "pzvec.h"
#include "pzmanvector.h"
#include "pzfmatrix.h"
#include "pzmaterialid.h"
#include "tpzautopointer.h"

//#ifdef _AUTODIFF
//#include "fadType.h"
//#endif

template <class T, int N>
class TPZManVector;

/**
 * @ingroup material
 * @brief This class defines the boundary condition for TPZMaterial objects
 */
/**
 * This class redirects the call to Contribute to calls ContributeBC of the referring material object
 */
class TPZBndCond : public TPZDiscontinuousGalerkin {
	
	friend class TPZMaterial;
protected:
	/** @brief boundary condition type */
	int 		fType;
	/** @brief first value of boundary condition */
 	TPZFMatrix	fBCVal1;
	/** @brief second value of boundary condition */
	TPZFMatrix	fBCVal2;
	/** @brief pointer to material which created bc */
	TPZAutoPointer<TPZMaterial> fMaterial;
	
	/** @brief Function to allow fBCVal1 to be variable */
	void (*fValFunction)(TPZVec<REAL> &loc, TPZFMatrix &Val1, TPZVec<REAL> &Val2, int &BCType);
	
	public :
	/** @brief Copy constructor */
	TPZBndCond(TPZBndCond & bc) : TPZDiscontinuousGalerkin(bc), fBCVal1(bc.fBCVal1),
    fBCVal2(bc.fBCVal2), fValFunction(NULL){
		fMaterial = bc.fMaterial;
		fType = bc.fType;
	}
	/** @brief Default constructor */
	TPZBndCond() : TPZDiscontinuousGalerkin(0), fBCVal1(),
    fBCVal2(), fValFunction(NULL){
	}
	/** @brief Default destructor */
    ~TPZBndCond(){}
	
	TPZBndCond(TPZAutoPointer<TPZMaterial> &material,int id,int type,TPZFMatrix &val1,TPZFMatrix &val2) :
    TPZDiscontinuousGalerkin(id), fBCVal1(val1), fBCVal2(val2), fValFunction(NULL) {
		//creates a new material
		if(!material)
		{
			std::cout << __PRETTY_FUNCTION__ << " Creating boundary condition with NULL material" << std::endl;
		}
		fMaterial = material;
		fType = type;
		
	}
	
	TPZBndCond(TPZBndCond &copy, TPZAutoPointer<TPZMaterial> ref) : TPZDiscontinuousGalerkin(copy), fType(copy.fType),
	fBCVal1(copy.fBCVal1), fBCVal2(copy.fBCVal2), fMaterial(ref), fValFunction(copy.fValFunction) {}
	
	
	void SetValFunction(void (*fp)(TPZVec<REAL> &loc, TPZFMatrix &Val1, TPZVec<REAL> &Val2, int &BCType)){
		fValFunction = fp;
	}
	
	void SetMaterial(TPZAutoPointer<TPZMaterial> mat) { fMaterial = mat;}
	
	/** @brief Returns the integrable dimension of the material*/
	int Dimension() { return fMaterial->Dimension(); }
	
	virtual int NFluxes(){ return fMaterial->NFluxes(); }
	
	int NStateVariables() { return fMaterial->NStateVariables(); }
	
	/** @brief Returns the number of norm errors. Default is 3: energy, L2 and H1. */
	virtual int NEvalErrors() {return fMaterial->NEvalErrors();}
	
	int Type() { return fType; }
	
	void SetType(int type){ this->fType = type; }
	
	TPZFMatrix &Val1() { return fBCVal1; }
	
	TPZFMatrix &Val2() { return fBCVal2; }
  
  void SetVal1( TPZFMatrix v1 ) { fBCVal1 = v1; }
	
	void SetVal2( TPZFMatrix v2 ) { fBCVal2 = v2; }
	
	TPZAutoPointer<TPZMaterial> Material() { return fMaterial; }
	
	/** @brief Computes the value of the flux function to be used by ZZ error estimator */
	void Flux(TPZVec<REAL> &x, TPZVec<REAL> &Sol, TPZFMatrix &DSol, TPZFMatrix &axes, TPZVec<REAL> &flux){
		flux.Fill(0.);
	}
	
	void Print(std::ostream & out = std::cout) {
		out << " Boundary condition number = " << Id() << "\n";
		out << " boundary condition type = " << fType << "\n";
		out << " val1 = \n"; fBCVal1.Print("fBCVal1",out);
		out << " val2 = \n"; fBCVal2.Print("fBCVal2",out);
		if (HasForcingFunction()) out << " has forcing function\n";
		else out << "has no forcing function\n";
	}
	
	void UpdataBCValues(TPZMaterialData &data);
	
	/**
	 * @brief It computes a contribution to the stiffness matrix and load vector at one integration point.
	 * @param data [in] stores all input data
	 * @param weight [in] is the weight of the integration rule
	 * @param ek [out] is the stiffness matrix
	 * @param ef [out] is the load vector
	 * @since April 16, 2007
	 */
	virtual void Contribute(TPZMaterialData &data, REAL weight, TPZFMatrix &ek, TPZFMatrix &ef);
	
	/**
	 * @brief It computes a contribution to the stiffness matrix and load vector at one BC integration point.
	 * @param data [in] stores all input data
	 * @param weight [in] is the weight of the integration rule
	 * @param ek [out] is the stiffness matrix
	 * @param ef [out] is the load vector
	 * @param bc [in] is the boundary condition material
	 * @since April 16, 2007
	 */
	virtual void ContributeBC(TPZMaterialData &data, REAL weight, TPZFMatrix &ek, TPZFMatrix &ef, TPZBndCond &bc);
	
	/**
	 * @brief It computes a contribution to the residual vector at one integration point.
	 * @param data [in] stores all input data
	 * @param weight [in] is the weight of the integration rule
	 * @param ef [out] is the residual vector
	 * @since April 16, 2007
	 */
	virtual void Contribute(TPZMaterialData &data, REAL weight, TPZFMatrix &ef);
	
	/**
	 * @brief It computes a contribution to the stiffness matrix and load vector at one BC integration point.
	 * @param data [in] stores all input data
	 * @param weight [in] is the weight of the integration rule
	 * @param ef [out] is the load vector
	 * @param bc [in] is the boundary condition material
	 * @since April 16, 2007
	 */
	virtual void ContributeBC(TPZMaterialData &data, REAL weight, TPZFMatrix &ef, TPZBndCond &bc);
	
	/**
	 * @brief It computes a contribution to stiffness matrix and load vector at one integration point
	 * @param data [in]
	 * @param weight [in]
	 * @param ek [out] is the stiffness matrix
	 * @param ef [out] is the load vector
	 * @since April 16, 2007
	 */
	virtual void ContributeInterface(TPZMaterialData &data, REAL weight, TPZFMatrix &ek, TPZFMatrix &ef);
	
	/**
	 * @brief It computes a contribution to residual vector at one integration point
	 * @param data [in]
	 * @param weight [in]
	 * @param ef [out] is the load vector
	 * @since April 16, 2007
	 */
	virtual void ContributeInterface(TPZMaterialData &data, REAL weight, TPZFMatrix &ef);
	
	/**
	 * @brief It computes a contribution to stiffness matrix and load vector at one BC integration point
	 * @param data [in]
	 * @param weight [in]
	 * @param ek [out] is the stiffness matrix
	 * @param ef [out] is the load vector
	 * @param bc [in] is the boundary condition object
	 * @since April 16, 2007
	 */
	virtual void ContributeBCInterface(TPZMaterialData &data, REAL weight, TPZFMatrix &ek,TPZFMatrix &ef,TPZBndCond &bc);
	
	/**
	 * @brief It computes a contribution to residual vector at one BC integration point
	 * @param data [in]
	 * @param weight [in]
	 * @param ef [out] is the load vector
	 * @param bc [in] is the boundary condition object
	 * @since April 16, 2007
	 */
	virtual void ContributeBCInterface(TPZMaterialData &data, REAL weight, TPZFMatrix &ef,TPZBndCond &bc);
	
	//#ifdef _AUTODIFF
	
	//  void ContributeEnergy(TPZVec<REAL> &x,
	//			      TPZVec<FADFADREAL> &sol,
	//			      TPZVec<FADFADREAL> &dsol,
	//			      FADFADREAL &U,
	//			      REAL weight);
	
	//#endif
	
	
	void Errors(TPZVec<REAL> &x,TPZVec<REAL> &sol,TPZFMatrix &dsol, TPZFMatrix &axes, TPZVec<REAL> &flux,
				TPZVec<REAL> &uexact,TPZFMatrix &duexact,TPZVec<REAL> &val){
		val.Fill(0.);
	}
	
	virtual void Clone(std::map<int, TPZAutoPointer<TPZMaterial> > &matvec);
	
	/** 
	 * @brief Compute interface jumps
	 * @since Feb 14, 2006
	 */
	/**
	 * \f$ values[1] = (solleft - solright)^2 \f$ \n
	 * \f$ values[2] = (dsolleft - dsolright)^2 \f$ \n
	 * \f$ values[0] = values[1] + values[2] \f$
	 */
	virtual void InterfaceJump(TPZVec<REAL> &x, TPZVec<REAL> &leftu,TPZVec<REAL> &rightu,TPZVec<REAL> &jump);
	
	/** @brief Returns the unique identifier for reading/writing objects to streams */
	virtual int ClassId() const;
	/** @brief Saves the element data to a stream */
	virtual void Write(TPZStream &buf, int withclassid);
	
	/** @brief Reads the element data from a stream */
	virtual void Read(TPZStream &buf, void *context);
	
	void ContributeInterfaceErrors(TPZMaterialData &data,
								   REAL weight,
								   TPZVec<REAL> &nkL,
								   TPZVec<REAL> &nkR,
								   int &errorid);
	
	void  ContributeErrors(TPZMaterialData &data,// aqui
						   REAL weight,
						   TPZVec<REAL> &nk,
						   int &errorid){
		//nothing to be done here
	};
	
	
	virtual void ContributeInterfaceBCErrors(TPZMaterialData &data,
											 REAL weight,
											 TPZVec<REAL> &nk,
											 TPZBndCond &bc,
											 int &errorid
											 ){
		//nothing to be done here
	}
	
	/** @brief Calls the aggregate material correspondent function */
	virtual void FillDataRequirements(TPZMaterialData &data);
	
};

#endif
