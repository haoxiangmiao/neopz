/*
 *  TPZElasticCriterion
 *
 *  Created by Nathan Shauer on 5/4/14.
 *  Copyright 2014 __LabMeC__. All rights reserved.
 *
 */

#ifndef TPZELASTICCRITERION
#define TPZELASTICCRITERION

#include "TPZPlasticState.h"
#include "TPZElasticResponse.h"
#include "TPZPlasticBase.h"

class TPZElasticCriterion : public TPZPlasticBase {
public:

    enum {
        NYield = 3
    };

public:
    typedef TPZElasticCriterion fNYields;

    /** @brief Plastic State Variables (EpsT, EpsP, Alpha) at the current time step */
    TPZPlasticState<STATE> fPlasticState;

    TPZElasticResponse fElasticResponse;

public:

    /**
     * @brief empty constructor
     */
    TPZElasticCriterion();

    /**
     * @brief Copy Constructor
     */
    TPZElasticCriterion(const TPZElasticCriterion &cp);

    /**
     * @brief Operator =
     */
    TPZElasticCriterion & operator=(const TPZElasticCriterion &cp);

    void Read(TPZStream &buf);

    void Write(TPZStream &buf) const;


    /** @brief Return the number of plastic steps in the last load step. Zero indicates elastic loading. */
    virtual int IntegrationSteps()const;

    /**
     * @brief Name of the class ina string
     */
    virtual const char * Name() const {
        return "TPZElasticCriteria";
    }

    virtual void Print(std::ostream & out) const {
        out << "Classe: " << this->Name();
        fPlasticState.Print(out);
    }

    /**
     * Imposes the specified strain tensor, evaluating the plastic integration if necessary.
     *
     * @param[in] epsTotal Imposed total strain tensor
     */
    virtual void ApplyStrain(const TPZTensor<REAL> &epsTotal);

    /**
     * Imposes the specified strain tensor and returns the correspondent stress state.
     *
     * @param[in] epsTotal Imposed total strain tensor
     * @param[out] sigma Resultant stress
     */
    virtual void ApplyStrainComputeSigma(const TPZTensor<REAL> &epsTotal, TPZTensor<REAL> &sigma);



    //virtual void ApplySigmaComputeStrain(const TPZTensor<REAL> &sigma, TPZTensor<REAL> &epsTotal);

    /**
     * Imposes the specified strain tensor and returns the corresp. stress state and tangent
     * stiffness matrix.
     *
     * @param[in] epsTotal Imposed total strain tensor
     * @param[out] sigma Resultant stress
     * @param[out] Dep Incremental constitutive relation
     */
    virtual void ApplyStrainComputeDep(const TPZTensor<REAL> &epsTotal, TPZTensor<REAL> &sigma, TPZFMatrix<REAL> &Dep);

    /**
     * Attempts to compute an epsTotal value in order to reach an imposed stress state sigma.
     * This method should be used only for test purposes because it isn't fully robust. Some
     * materials, specially those perfectly plastic and with softening, may fail when applying
     * the Newton Method on ProcessLoad.
     *
     * @param[in] sigma stress tensor
     * @param[out] epsTotal deformation tensor
     */
    virtual void ApplyLoad(const TPZTensor<REAL> & sigma, TPZTensor<REAL> &epsTotal);

    virtual TPZPlasticState<REAL> GetState() const;
    /**
     * @brief Return the value of the yield functions for the given strain
     * @param[in] epsTotal strain tensor (total strain)
     * @param[out] phi vector of yield functions
     */
    virtual void Phi(const TPZTensor<REAL> &epsTotal, TPZVec<REAL> &phi) const;

    virtual void SetElasticResponse(TPZElasticResponse &ER) {
        fElasticResponse = ER;
    }

    virtual TPZElasticResponse GetElasticResponse() const {
        return fElasticResponse;
    }
    /**
     * @brief Update the damage values
     * @param[in] state Plastic state proposed
     */
    virtual void SetState(const TPZPlasticState<REAL> &state);


};


#endif //TPZElasticCriterion_H
