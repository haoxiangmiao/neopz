//
//  pznlfluidstructureData.h
//  PZ
//
//  Created by Cesar Lucci on 05/06/13.
//
//

#ifndef PZ_TPZPlaneFractureData_h
#define PZ_TPZPlaneFractureData_h

#include <set>
#include <math.h>
#include "pzreal.h"
#include "pzerror.h"
#include "pzvec.h"
#include "pzcmesh.h"
#include "pzgeoel.h"
#include "pzcompel.h"
#include "pzintel.h"
#include <map>



const REAL globStressScale = 1.E-7;


class TimeControl
{
public:
    TimeControl()
    {
        fTtot = 0.;
        factTime = 0.;
        
        fDeltaT_left = 0.;
        fDeltaT_right = 0.;
        
        factDeltaT = 0.;
        
        fwasLeftLastTime = true;
        freachTime_right = false;
    }
    
    ~TimeControl()
    {
        
    }
    
    void SetTimeControl(REAL Ttot)
    {
        fTtot = Ttot;
        factTime = 0.;
        
        fDeltaT_left = 10.;
        fDeltaT_right = Ttot + 10.;
        
        fwasLeftLastTime = true;
        freachTime_right = false;
        
        ComputeActDeltaT();
    }
    
    void TimeisOnLeft()
    {
        fwasLeftLastTime = true;
        
        if(freachTime_right == false)
        {
            fDeltaT_left += 5.;
        }
        else
        {
            fDeltaT_left = factDeltaT;
        }
    }
    
    void TimeisOnRight()
    {
        fwasLeftLastTime = false;
        
        if(freachTime_right == false)
        {//1st time reach time on right from KI=KIc
            fDeltaT_left -= 5.;
            fDeltaT_left = MAX(1.,fDeltaT_left);
            freachTime_right = true;
        }
        
        fDeltaT_right = factDeltaT;
    }
    
    void SetDeltaT(REAL deltaT)
    {
        factDeltaT = deltaT;
        fDeltaT_left = 0.99 * deltaT;
    }
    
    void ComputeActDeltaT()
    {
        if(freachTime_right == false)
        {
            factDeltaT = fDeltaT_left;
        }
        else
        {
            factDeltaT = (fDeltaT_left + fDeltaT_right)/2.;
        }
    }
    
    void UpdateActTime()
    {
        factTime += factDeltaT;
    }
    
    REAL Ttot()
    {
        return fTtot;
    }
    
    REAL actTime()
    {
        return factTime;
    }
    
    REAL actDeltaT()
    {
        return factDeltaT;
    }
    
    void RestartBissection()
    {
        freachTime_right = false;
        factDeltaT = 10.;
        fDeltaT_left = factDeltaT;
        fDeltaT_right = fTtot + 10.;
    }
    
    bool ReachEndOftime()
    {
        return (factTime >= fTtot - 0.01);
    }
    
    bool TimeLimitsIsCloseEnough()
    {
        if(fDeltaT_right < fDeltaT_left)
        {
            std::cout << "\n\n\nMetodo da bisseccao no tempo inverteu os limites!!!\n\n\n";
            DebugStop();
        }
        
        bool isCloseEnough = (fDeltaT_right - fDeltaT_left) < 0.1;
        
        if(isCloseEnough && fwasLeftLastTime)
        {
            fDeltaT_left = fDeltaT_right;
            isCloseEnough = false;
        }
        
        return isCloseEnough;
    }
    
    REAL LeftDeltaT()
    {
        return fDeltaT_left;
    }
 
    REAL RightDeltaT()
    {
        return fDeltaT_right;
    }
    
private:
    REAL fTtot;//Tempo total da simulacao
    REAL factTime;//tempo atual (em segundos)
    REAL factDeltaT;//delta T atual
    
    REAL fDeltaT_left;//deltaT cujo factTime+dt nao propagou a fratura (serah utilizado no metodo da bisseccao).
    REAL fDeltaT_right;//deltaT cujo factTime+dt propagou a fratura (serah utilizado no metodo da bisseccao).
    
    bool fwasLeftLastTime;
    bool freachTime_right;
};



class LeakoffStorage
{
public:
    LeakoffStorage()
    {
        this->fGelId_Penetration.clear();
        this->fPressureIndependent = true;
        
        this->fDefaultLeakoffEnabled = true;
        this->fLeakoffEnabled = this->fDefaultLeakoffEnabled;
    }
    ~LeakoffStorage()
    {
        fGelId_Penetration.clear();
    }
    
    void SetPressureIndependent()
    {
        this->fPressureIndependent = true;
    }
    
    void SetPressureDependent()
    {
        this->fPressureIndependent = false;
    }
    
    bool IsPressureIndependent()
    {
        return this->fPressureIndependent;
    }
    
    std::map<int,REAL> & GetLeakoffMap()
    {
        return fGelId_Penetration;
    }
    
    void SetLeakoffMap(std::map<int,REAL> & GelId_Penetration)
    {
        fGelId_Penetration = GelId_Penetration;
    }
    
    void UpdateLeakoff(TPZCompMesh * cmesh, REAL deltaT);
    
    REAL VlFtau(REAL pfrac, REAL tau, REAL Cl, REAL Pe, REAL gradPref, REAL vsp);
    
    REAL FictitiousTime(REAL VlAcum, REAL pfrac, REAL Cl, REAL Pe, REAL gradPref, REAL vsp);
    
    REAL QlFVl(int gelId, REAL pfrac, REAL deltaT, REAL Cl, REAL Pe, REAL gradPref, REAL vsp);
    
    REAL dQlFVl(int gelId, REAL pfrac, REAL deltaT, REAL Cl, REAL Pe, REAL gradPref, REAL vsp);
    
    void Printleakoff(std::ofstream & outf);
    
    void SetDefaultLeakoffEnabled(bool isEnabled)
    {
        this->fDefaultLeakoffEnabled = isEnabled;
        this->fLeakoffEnabled = isEnabled;
    }
    
    void DisableLeakoff()
    {
        fLeakoffEnabled = false;
    }
    
    void RestoreDefaultLeakoff()
    {
        fLeakoffEnabled = fDefaultLeakoffEnabled;
    }
    
protected:
    std::map<int,REAL> fGelId_Penetration;
    
    bool fPressureIndependent;
    
    bool fLeakoffEnabled;
    bool fDefaultLeakoffEnabled;
};




/**
 * To understand what is implemented here, see table (MaterialIds Table.xls) on folder (/NeoPZ/Projects/HydraulicFracturePropagation/PlaneFracture)
 */
class MaterialIdGen
{
public:
    
    MaterialIdGen()
    {
        
    }
    
    ~MaterialIdGen()
    {
        
    }
    
    int Aux1DMatId()
    {
        return -1;
    }
    
    int CrackTipMatId()
    {
        return -2;
    }
    
    int RockMatId(int layer)
    {
#ifdef DEBUG
        if(layer < 0 || layer > 99)
        {
            //Soh pode ter 100 camadas (de 0 a 99)
            DebugStop();
        }
#endif
        
        return (layer+1)*10;
    }
    
    int BulletMatId(int layer)
    {
        return -RockMatId(layer);
    }
    
    int InsideFractMatId(int layer, int stripe)
    {
#ifdef DEBUG
        if(stripe < 0 || stripe > 9)
        {
            //Soh pode ter 10 faixas de pressao (de 0 a 9)
            DebugStop();
        }
#endif
        
        return -(RockMatId(layer) + 1000 + stripe);
    }
    
    int OutSideFractMatId(int layer)
    {
        return -(RockMatId(layer) + 2000);
    }
    
    int FarfieldMatId(int layer)
    {
        return -(RockMatId(layer) + 3000);
    }
    
    int LeftMatId(int layer)
    {
        return -(RockMatId(layer) + 4000);
    }
    
    int RightMatId(int layer)
    {
        return -(RockMatId(layer) + 5000);
    }
    
    int TopMatId()
    {
        return -6010;
    }
    
    int BottomMatId()
    {
        return -7010;
    }
    
    bool IsInsideFractMat(int matId)
    {
        if(matId <= -1010 && matId >= -2009)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    
    int WhatStripe(int matId)
    {
        if(IsInsideFractMat(matId) == false)
        {
            std::cout << "\n\nGiven materialId (" << matId << ") does NOT belong to InsideFracture!!!\n\n";
            DebugStop();
        }
        
        int val = matId/10.;
        int lastDigit = matId - val*10;
        
        return fabs(lastDigit);//Last digit corresponds to the stripe
    }
    
    bool IsOutsideFractMat(int matId)
    {
        if(matId <= -2010 && matId >= -3000)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    
    bool IsBoundaryMaterial(int matId)
    {
        if(matId <= -3010 && matId >= -7010)
        {//is 2D BC between: left, right, farfield, top or bottom
            return true;
        }
        else
        {
            return false;
        }
    }
    
    bool IsBulletMaterial(int matId)
    {
        if(matId <= -10 && matId >= -1000)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    
    bool IsRockMaterial(int matId)
    {
        if(matId >= 10 && matId <= 1000)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    
    int WhatLayerFromInsideFracture(int insideMatId)
    {
#ifdef DEBUG
        if(IsInsideFractMat(insideMatId) == false)
        {//The given materialId IS NOT inside fracture
            DebugStop();
        }
#endif
        
        int inside = fabs(insideMatId);
        int stripe = inside - (inside/10)*10;
        
        int lay = (-1010-insideMatId-stripe)/10;
        
        return lay;
    }
    //------------------------------------------------------------------------------------------------------------
};


class LayerProperties
{
public:
    LayerProperties()
    {
        fYoung = 0.;
        fPoisson = 0.;
        fSigmaMax = 0.;
        fSigmaMin = 0.;
        fSigmaConf = 0.;
        fTVDini = 0.;
        fTVDfin = 0.;
        fKIc = 0.;
        fCl = 0.;
        fPe = 0.;
        fgradPref = 0.;
        fvsp = 0.;
    }
    
    LayerProperties(REAL Young, REAL Poisson, REAL SigMax, REAL SigMin, REAL SigConf, REAL TVDi, REAL TVDf,
                    REAL KIc, REAL Cl, REAL Pe, REAL gradPref, REAL vsp)
    {
#ifdef DEBUG
        REAL tol = 1.E-9;
        if(SigMax > 0. + tol || SigMin > 0. + tol || SigConf > 0. + tol)
        {
            std::cout << "\n\nPre-stress must be null or compressive!\n";
            std::cout << "\n\nSee " << __PRETTY_FUNCTION__ << "\n\n";
            DebugStop();
        }
        if(Pe < 0. - tol)
        {
            std::cout << "\n\nStatic pressure (Pe) must be null or positive!\n";
            std::cout << "\n\nSee " << __PRETTY_FUNCTION__ << "\n\n";
            DebugStop();
        }
        if(gradPref < 0. + tol)
        {
            std::cout << "\n\nReference pressure (gradPref) must be positive!\n";
            std::cout << "\n\nSee " << __PRETTY_FUNCTION__ << "\n\n";
            DebugStop();
        }
#endif
        
        fYoung = Young;
        fPoisson = Poisson;
        fSigmaMax = SigMax;
        fSigmaMin = SigMin;
        fSigmaConf = SigConf;
        fTVDini = TVDi;
        fTVDfin = TVDf;
        fKIc = KIc;
        fCl = Cl;
        fPe = Pe;
        fgradPref = gradPref;
        fvsp = vsp;
    }
    LayerProperties(const LayerProperties & cp)
    {
        fYoung = cp.fYoung;
        fPoisson = cp.fPoisson;
        fSigmaMax = cp.fSigmaMax;
        fSigmaMin = cp.fSigmaMin;
        fSigmaConf = cp.fSigmaConf;
        fTVDini = cp.fTVDini;
        fTVDfin = cp.fTVDfin;
        fKIc = cp.fKIc;
        fCl = cp.fCl;
        fPe = cp.fPe;
        fgradPref = 0.;
        fvsp = cp.fvsp;
    }
    ~LayerProperties()
    {
        fYoung = 0.;
        fPoisson = 0.;
        fSigmaMax = 0.;
        fSigmaMin = 0.;
        fSigmaConf = 0.;
        fTVDini = 0.;
        fTVDfin = 0.;
        fKIc = 0.;
        fCl = 0.;
        fPe = 0.;
        fgradPref = 0.;
        fvsp = 0.;
    }
    
    //Elastic 3D
    REAL fYoung;
    REAL fPoisson;
    REAL fSigmaMax;
    REAL fSigmaMin;
    REAL fSigmaConf;
    
    //TVD limits
    REAL fTVDini;
    REAL fTVDfin;
    
    //SIF
    REAL fKIc;
    
    //leafoff
    REAL fCl;
    REAL fPe;
    REAL fgradPref;
    REAL fvsp;
};


class LayerStruct
{
public:
    LayerStruct()
    {
        this->fActPressureIndex = 1;
        this->f_Npress_Lay_Stripe_solutionRow.clear();
        this->fPrestressYY_layIndex.clear();
        this->f_Npress_solutionRowsTurnedOn.clear();
        //
        this->fLayerVec.Resize(0);
    }
    
    ~LayerStruct()
    {
        this->fActPressureIndex = 0;
        this->f_Npress_Lay_Stripe_solutionRow.clear();
        this->fPrestressYY_layIndex.clear();
        this->f_Npress_solutionRowsTurnedOn.clear();
        //
        this->fLayerVec.Resize(0);
    }
    
    void ResetData()
    {
        this->f_Npress_Lay_Stripe_solutionRow.clear();
        this->fElastReducedSolution.Resize(0,0);
        this->fPrestressYY_layIndex.clear();
        this->f_Npress_solutionRowsTurnedOn.clear();
    }
    
    void SetLayerVec(TPZVec<LayerProperties> & LayerVec)
    {
        this->fLayerVec = LayerVec;
    }
    
    void SetSolutionRow4PressureLessOrEqualThan(REAL prestressYYapplied, int stripe, int row)
    {
        std::map<REAL,std::set<int> >::iterator itappliedprestress, itactprestress;
        int nPress = this->GetNPressuresUnderThisPressure(prestressYYapplied);
        
        for(itactprestress  = this->fPrestressYY_layIndex.begin();
            itactprestress != this->fPrestressYY_layIndex.end();
            itactprestress++)
        {
            REAL tol = 1.*globStressScale;
            if(itactprestress->first < (prestressYYapplied + tol))
            {
                std::set<int>::iterator itWhatLay;
                for(itWhatLay = itactprestress->second.begin();
                    itWhatLay != itactprestress->second.end();
                    itWhatLay++)
                {
                    int layer = *(itWhatLay);
                    this->f_Npress_Lay_Stripe_solutionRow[nPress][layer][stripe] = row;
                }
            }
        }
    }
    
    void SetSolutionRow4PressureGreaterThan(REAL prestressYYapplied, int stripe, int row)
    {
        std::map<REAL,std::set<int> >::iterator itappliedprestress, itactprestress;
        int nPress = this->GetNPressuresUnderThisPressure(prestressYYapplied);
        
        for(itactprestress  = this->fPrestressYY_layIndex.begin();
            itactprestress != this->fPrestressYY_layIndex.end();
            itactprestress++)
        {
            REAL tol = 1.*globStressScale;
            if(itactprestress->first > (prestressYYapplied + tol))
            {
                std::set<int>::iterator itWhatLay;
                for(itWhatLay = itactprestress->second.begin();
                    itWhatLay != itactprestress->second.end();
                    itWhatLay++)
                {
                    int layer = *(itWhatLay);
                    this->f_Npress_Lay_Stripe_solutionRow[nPress][layer][stripe] = row;
                }
            }
        }
    }
    
    void SetActPressureIndex(int actPressureIndex)
    {
        this->fActPressureIndex = actPressureIndex;
    }
    
    void SetElastSolutionMatrix(TPZFMatrix<REAL> & solution)
    {
        this->fElastReducedSolution = solution;
    }
    
    void InsertPrestressYYandLayer(REAL prestressYY, int lay)
    {
        std::map<REAL,std::set<int> >::iterator it = this->fPrestressYY_layIndex.find(-prestressYY);
        
        if(it == this->fPrestressYY_layIndex.end())
        {
            std::set<int> laySet;
            laySet.insert(lay);
            this->fPrestressYY_layIndex[-prestressYY] = laySet;
        }
        else
        {
            it->second.insert(lay);
        }
    }
    
    void InsertActiveEquation(int pressIndex, int row)
    {
        if(this->f_Npress_solutionRowsTurnedOn.find(pressIndex) == this->f_Npress_solutionRowsTurnedOn.end())
        {
            std::set<int> activeEq;
            activeEq.insert(row);
            this->f_Npress_solutionRowsTurnedOn[pressIndex] = activeEq;
        }
        else
        {
            this->f_Npress_solutionRowsTurnedOn.find(pressIndex)->second.insert(row);
        }
    }
    
    bool ThisRowIsTurnedOn(int row)
    {
        bool isOn = false;
        
        std::map< int,std::set<int> >::iterator itPressIndex = this->f_Npress_solutionRowsTurnedOn.find(this->fActPressureIndex);
        if(itPressIndex != this->f_Npress_solutionRowsTurnedOn.end())
        {
            std::set<int>::iterator itRow = itPressIndex->second.find(row);
            if(itRow != itPressIndex->second.end())
            {
                isOn = true;
            }
        }
        
        return isOn;
    }
    
    int GetSolutionRow(int layer, int stripe)
    {
        int row = -1;
        
        std::map< int,std::map< int,std::map<int,int> > >::iterator itNpress = this->f_Npress_Lay_Stripe_solutionRow.find(this->fActPressureIndex);
        if(itNpress != this->f_Npress_Lay_Stripe_solutionRow.end())
        {
            std::map< int,std::map<int,int> >::iterator itLay = itNpress->second.find(layer);
            if(itLay != itNpress->second.end())
            {
                std::map<int,int>::iterator itStripe = itLay->second.find(stripe);
                if(itStripe != itLay->second.end())
                {
                    row = itStripe->second;
                }
            }
        }
        
        if(row < 0)
        {
            std::cout << "\n\nCamada " << layer << " nao apresenta stripe " << stripe << "\n";
            std::cout << "Veja " << __PRETTY_FUNCTION__ << "\n\n\n";
            DebugStop();
        }

        return row;
    }
    
    int GetNPrestressYYonFracture()
    {
        return this->fPrestressYY_layIndex.size();
    }
    
    REAL GetSequencedPrestressYY(int pressindex)
    {
        int count = 0;
        std::map<REAL,std::set<int> >::iterator it;
        
        for(it = this->fPrestressYY_layIndex.begin(); it != this->fPrestressYY_layIndex.end(); it++)
        {
            if(count == pressindex)
            {
                return it->first;
            }
            count ++;
        }
        
        DebugStop();//index > this->fPrestressYY_layIndex.size()
        return 0.;
    }
    
    REAL GetStressAppliedJustForJIntegral(int layer, int stripe)
    {
        //A linha da solucao mais fraca garante que pegarei a equacao que abrange
        //ela mesma e, quando houver, as demais camadas (havera qdo nas demais ocorrer sigYY < appliedP).
        std::map<REAL,std::set<int> >::iterator itLowestSigYY = this->fPrestressYY_layIndex.begin();
        std::set<int>::iterator itAnyLayer = itLowestSigYY->second.begin();
        int weakerLayer = *itAnyLayer;
        int weakerLayerSolutionRow = this->GetSolutionRow(weakerLayer,stripe);
        
        if(f_Npress_solutionRowsTurnedOn.find(this->fActPressureIndex)->second.find(weakerLayerSolutionRow) ==
           f_Npress_solutionRowsTurnedOn.find(this->fActPressureIndex)->second.end())
        {
            std::cout << "\nActPressureIndex = " << this->fActPressureIndex << "\n";
            std::cout << "weakerLayer = " << weakerLayer << "\n";
            std::cout << "weakerLayerSolutionRow = " << weakerLayerSolutionRow << "\n";
            std::cout << "\n\nCamada mais fraca nao estah com equacao ligada!!!\n\n\n";
            DebugStop();
        }
        
        REAL sol = this->fElastReducedSolution(weakerLayerSolutionRow,0);
        int sequencedPrestressIndex = this->fActPressureIndex-1;//aqui as pressoes comecam na posicao 0.
        REAL stressApplied = this->GetSequencedPrestressYY(sequencedPrestressIndex);
        
        //Eh porque a integral-J nao inclui a translacao do pre-stress
        //(ver TPZPlaneFractureMesh::GetFractureCompMeshReferred)
        REAL preStressShift = -this->fLayerVec[layer].fSigmaMin;
        REAL cellStressApllied = sol*stressApplied - preStressShift;
        
        return cellStressApllied;
    }
    
    int GetNPressuresUnderThisPressure(REAL pressure)
    {
        REAL tol = 1.*globStressScale;
        std::map<REAL,std::set<int> >::iterator itappliedprestress = this->fPrestressYY_layIndex.lower_bound(pressure - tol);
        int npress = std::distance(this->fPrestressYY_layIndex.begin(),itappliedprestress) + 1;
        
        return npress;
    }
    
    void GetActiveEquations(std::set<int> & actEq)
    {
        actEq = this->f_Npress_solutionRowsTurnedOn.find(this->fActPressureIndex)->second;
    }
    
    //retorna 1 para equacoes pertinentes aa faixa de pressao, e 0 para as equacoes que nao devem existir nesta simulacao.
    //Serah utilizado como o alpha da solucao multifisica associada a cada equacao (Ver TPZPlaneFractureKernel::ApplyEquationFilter).
    REAL GetEquationAlpha(int row)
    {
        if(row == 0)
        {
            return 1.;
        }
        
        std::map< int,std::map< int,std::map<int,int> > >::iterator itNpress = f_Npress_Lay_Stripe_solutionRow.find(this->fActPressureIndex);
        std::map< int,std::map<int,int> >::iterator itLayer;
        for(itLayer = itNpress->second.begin();
            itLayer != itNpress->second.end();
            itLayer++)
        {
            std::map<int,int>::iterator itStripe;
            for(itStripe = itLayer->second.begin();
                itStripe != itLayer->second.end();
                itStripe++)
            {
                if(itStripe->second == row)
                {
                    if(this->ThisRowIsTurnedOn(row))
                    {
                        //Eu verifiquei que a convergencia eh melhor por cima! Por isso nao eh 1.0
                        return 1.1;
                    }
                    else
                    {
                        //Equacao presente porem travada pelo equation filter
                        return 1.;
                    }
                }
            }
        }
        
        return 0.;
    }
    
    int NLayers()
    {
        return this->fLayerVec.NElements();
    }
    
    const LayerProperties & GetLayer(int index)
    {
        return this->fLayerVec[index];
    }
    
    const LayerProperties & GetLayerFromZcoord(REAL zCoord)
    {
        int whatLayer = this->WhatLayer(zCoord);
        return this->fLayerVec[whatLayer];
    }
    
    int WhatLayer(int zCoord)
    {
        for(int lay = 0; lay < this->fLayerVec.NElements(); lay++)
        {
            if(fabs(zCoord) > this->fLayerVec[lay].fTVDini && fabs(zCoord) < this->fLayerVec[lay].fTVDfin)
            {
                return lay;
            }
        }
        DebugStop();//nao achou o layer
        return -1;
    }
    
    void PrintMe()
    {
        std::cout << "\nEquations structure:\nNpress\tLay\tStripe\trow\n";
        std::map< int,std::map< int,std::map<int,int> > >::iterator itNpress;
        std::map< int,std::map<int,int> >::iterator itLay;
        std::map<int,int>::iterator itStripe;
        
        for(itNpress = this->f_Npress_Lay_Stripe_solutionRow.begin(); itNpress != this->f_Npress_Lay_Stripe_solutionRow.end(); itNpress++)
        {
            for(itLay = itNpress->second.begin(); itLay != itNpress->second.end(); itLay++)
            {
                for(itStripe = itLay->second.begin(); itStripe != itLay->second.end(); itStripe++)
                {
                    bool eqActive = false;
                    if(this->f_Npress_solutionRowsTurnedOn.find(itNpress->first) != this->f_Npress_solutionRowsTurnedOn.end())
                    {
                        if(this->f_Npress_solutionRowsTurnedOn.find(itNpress->first)->second.find(itStripe->second) !=
                           this->f_Npress_solutionRowsTurnedOn.find(itNpress->first)->second.end())
                        {
                            eqActive = true;
                        }
                    }
                    std::cout << itNpress->first << "\t" << itLay->first << "\t" << itStripe->first << "\t" << itStripe->second << "\t";
                    if(eqActive)
                    {
                        std::cout << "on\n";
                    }
                    else
                    {
                        std::cout << "off\n";
                    }
                }
            }
            std::cout << "-------------------------------\n";
        }
        
        std::cout << "\nPrestressYY_layIndex\n\n";
        std::map<REAL,std::set<int> >::iterator itPre;
        for(itPre = this->fPrestressYY_layIndex.begin();
            itPre != this->fPrestressYY_layIndex.end();
            itPre++)
        {
            std::cout << itPre->first << ": ";
            std::set<int>::iterator itPreLayer;
            for(itPreLayer = itPre->second.begin();
                itPreLayer != itPre->second.end();
                itPreLayer++)
            {
                std::cout << *itPreLayer << "\t";
            }
            std::cout << "\n";
        }
    }
    
protected:
    
    /** Guarda a linha da solucao da elastica de espacos reduzidos, baseado na quantidade de pressoes envolvidas, a camada e faixa */
    std::map< int,std::map< int,std::map<int,int> > > f_Npress_Lay_Stripe_solutionRow;
    
    /**
     * Para uma quantidade de camadas envolvidas em uma certa pressao, deve-se ligar as equacoes pertinentes.
     * Desta forma, este mapa guarda as linhas das respectivas equacoes a serem ligadas, em funcao da quantidade de camadas envolvidas
     */
    std::map< int,std::set<int> > f_Npress_solutionRowsTurnedOn;
    
    /** Representa a quantidade de grupos de layers ativos para aproximacao da solucao */
    int fActPressureIndex;
    
    /** Matriz_vetor solucao da elastica de espacos reduzidos */
    TPZFMatrix<REAL> fElastReducedSolution;
    
    /** Vetor de camadas. Posicao 0: camada mais acima. Ultima posicao: camada mais abaixo */
    TPZVec<LayerProperties> fLayerVec;
    
    /** Grarda os layers, indexados pelos respectivos valores do PrestressYY */
    std::map<REAL,std::set<int> > fPrestressYY_layIndex;
};


class Output3DDataStruct
{
public:
    
    Output3DDataStruct();
    ~Output3DDataStruct();

    void SetQinj1wing(REAL Qinj1wing);
    
    int NTimes();
    void InsertTAcumVolW(REAL time, REAL vol);
    void InsertTAcumVolLeakoff(REAL time, REAL vol);
    void InsertTL(REAL time, REAL L);
    void InsertTHsup(REAL time, REAL Hsup);
    void InsertTHinf(REAL time, REAL Hinf);

    void PrintMathematica(std::ofstream & outf);
        
    REAL fQinj1wing;
    
    //maps indexed by time
    std::map<REAL,REAL> fTAcumVolW;
    std::map<REAL,REAL> fTAcumVolLeakoff;
    std::map<REAL,REAL> fTL;
    std::map<REAL,REAL> fTHsup;
    std::map<REAL,REAL> fTHinf;
};

extern TimeControl globTimeControl;

extern LeakoffStorage globLeakoffStorage;

extern MaterialIdGen globMaterialIdGen;

extern Output3DDataStruct globFractOutput3DData;

extern LayerStruct globLayerStruct;

#endif
