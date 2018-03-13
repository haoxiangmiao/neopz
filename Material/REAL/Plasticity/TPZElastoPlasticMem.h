//$Id: pzelastoplasticmem.h,v 1.7 2009-10-04 05:44:22 erick Exp $

#ifndef PZELASTOPLASTICMEM_H
#define PZELASTOPLASTICMEM_H

#include "TPZMaterial.h"
#include "TPZTensor.h"
#include "TPZPlasticState.h"

  /**
   * This class defines the material memory that should be stored at each integration point
   * for the purposes of an elastoplastic material.
   */

class TPZElastoPlasticMem : public TPZSavable
{
public:
    
virtual int ClassId() const;

    
	TPZElastoPlasticMem();
	
	TPZElastoPlasticMem(const TPZElastoPlasticMem & source);
	
	const TPZElastoPlasticMem & operator=(const TPZElastoPlasticMem & source);
	
	virtual ~TPZElastoPlasticMem();
	
	const std::string Name()const;
	
    virtual void Write(TPZStream &buf, int withclassid) const;

    void Read(TPZStream &buf, void *context);

	virtual void Print(std::ostream &out = std::cout)const;
	
	/**
	 * Operator<<
	 */
    friend std::ostream& operator<<( std::ostream& Out, const TPZElastoPlasticMem & s )
	{
		s.Print(Out);
		return Out;
	}
	
	/**
	 * Total (elastic+plastic) stress
	 */
	TPZTensor<REAL> fSigma;
	
	/**
	 * Plastic state vars
	 */
	TPZPlasticState<REAL> fPlasticState;
	
	int fPlasticSteps;
	
	REAL fPhi;
    
    TPZManVector<REAL,3> fDisplacement;
	
};

#endif