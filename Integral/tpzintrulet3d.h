/**
 * @file
 * @brief Contains the TPZIntRuleT3D class which defines integration rule for tetrahedra.
 */

#ifndef TPZINTRULET3D_H
#define TPZINTRULET3D_H

#include "pzreal.h"
#include "pzmanvector.h"

/**
 * @ingroup integral
 * @brief Integration rule for tetrahedra. \ref integral "Numerical Integration"
 * @author Philippe R. B. Devloo <phil@fec.unicamp.br>
 */
class TPZIntRuleT3D {
	
	/** @brief The list can to access the constructor of the current class. */
    friend class TPZIntRuleList;
    
	/** @brief Number of integration points for this object */
    int	   fNumInt;
	/** @brief Location of the integration point Ksi */
    TPZManVector<long double>	fLocationKsi;
	/** @brief Location of the integration point Eta */
    TPZManVector<long double>	fLocationEta;
	/** @brief Location of the integration point Zeta */
    TPZManVector<long double>	fLocationZeta;
	/** @brief Weight of the integration point */
    TPZManVector<long double>	fWeight;
	
	/**
	 * @brief Constructor of integration rule for tetrahedra.
	 * @param i Order of the polinomial will be exactly integrated.
	 */
    TPZIntRuleT3D(int i = 2);
	/** @brief Default destructor. It delete the vector of points and weights. */
    ~TPZIntRuleT3D();

	/**
	 * @brief Computes the cubature rules following the symmetric construction presented at Linbo Zhang article.
	 * @param order Order of the polinomial will be exactly integrated.
	 */
	void ComputingSymmetricCubatureRule(int order);
	
	/**
	 * @brief Transform barycentric coordinates of the point in tetrahedra by cartesian coordinates (3 component).
	 * @param baryvec Vector of barycentric coordinates (4 components) for each point
	 * @param weightvec Vector of weights related for each point in baryvec vector
	 */
	void TransformBarycentricCoordInCartesianCoord(long double baryvec[],long double weightvec[]);

public:
	enum {NRULESTETRAHEDRA_ORDER = 14};
	
 	/** @brief Returns number of integration points */
	int NInt() const { return fNumInt;}
	
	/** @brief Returns location of the ith point */
	void Loc(int i, TPZVec<REAL> &pos) const;
	
	/** @brief Returns weight for the ith point */
	REAL W(int i) const;
};

#endif
