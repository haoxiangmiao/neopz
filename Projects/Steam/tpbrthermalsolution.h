/*
 *  tpbrthermalsolution.h
 *  IP3D_v5
 *
 *  Created by Philippe Devloo on 1/9/11.
 *  Copyright 2011 UNICAMP. All rights reserved.
 *
 */

#ifndef TPBRTHERMALSOLUTION
#define TPBRTHERMALSOLUTION

#include "pzreal.h"
#include "pzfmatrix.h"

/// Solution associated with a thermal problem in the confining layer
class TPBRThermalSolution
{
private:
	/// Solution vector
	TPZFMatrix fSolution;
	/// Area associated with the thermal problem
	REAL fArea;
	/// Energy associated with the solution
	REAL fEnergy;
	
public:
	/// constructor
	TPBRThermalSolution(REAL area) : fSolution(), fArea(area), fEnergy(0.)
	{
	}
    /// copy constructor
    TPBRThermalSolution(const TPBRThermalSolution &copy) : fSolution(copy.fSolution), fArea(copy.fArea), fEnergy(copy.fEnergy)
    {
        
    }
    
    /// implement an otherwise implicit = operator
    TPBRThermalSolution &operator=(const TPBRThermalSolution &copy)
    {
        fSolution = copy.fSolution;
        fArea = copy.fArea;
        fEnergy = copy.fEnergy;
        return *this;
    }
    
	/// Update the solution and associated energy
	void UpdateSolution(TPZFMatrix &solution, REAL energy)
	{
		fSolution = solution;
		fEnergy = energy;
	}
	/// Access method to the solution
	TPZFMatrix &Solution()
	{
		return fSolution;
	}
	/// Return the energy associated with the solution
	REAL Energy()
	{
		return fEnergy;
	}
	/// Access function for the area
	REAL Area()
	{
		return fArea;
	}
	
};
#endif