//
//  WaterPhase.h
//  PZ
//
//  Created by Omar on 9/30/15.
//
//

#ifndef __PZ__WaterPhase__
#define __PZ__WaterPhase__

#include <stdio.h>
#include "ReducedPVT.h"

#include "tpzautopointer.h"
#include "pzfmatrix.h"
#include <math.h>


class WaterPhase : public ReducedPVT

{
    
private:
    
    
public:
    
    /** @brief Default constructor $ */
    WaterPhase();
    
    /** @brief Default desconstructor $ */
    ~WaterPhase();
    
    /** @brief Density - kg/m3  $\rho$ */
    void Density(TPZManVector<REAL> &rho, TPZManVector<REAL> state_vars);
    
    /** @brief viscosity - Pa s  $\mu$ */
    void Viscosity(TPZManVector<REAL> &mu, TPZManVector<REAL> state_vars);
    
    /** @brief Compressibility - 1/pa $c$ */
    void Compressibility(TPZManVector<REAL> &c, TPZManVector<REAL> state_vars);
    
};



#endif /* defined(__PZ__WaterPhase__) */
