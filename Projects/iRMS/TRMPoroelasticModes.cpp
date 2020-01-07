//
//  TRMPoroelasticModes.cpp
//  PZ
//
//  Created by Omar on 5/1/17.
//
//

#include "TRMPoroelasticModes.h"

/** @brief Default constructor */
TRMPoroelasticModes::TRMPoroelasticModes() : TPZMatWithMem<TRMMemory, TPZDiscontinuousGalerkin>()
{
    fdimension = 0;
}

/** @brief Constructor based on a material id */
TRMPoroelasticModes::TRMPoroelasticModes(int matid, int dimension, int nstate) : TPZMatWithMem<TRMMemory, TPZDiscontinuousGalerkin>(matid)
{
    fdimension = dimension;
    fnstate = nstate;
}

/** @brief Default desconstructor */
TRMPoroelasticModes::~TRMPoroelasticModes(){
    
}

/** @brief Copy constructor $ */
TRMPoroelasticModes::TRMPoroelasticModes(const TRMPoroelasticModes& other){
    this->fdimension    = other.fdimension;
    this->fSimulationData    = other.fSimulationData;
}

/** @brief Copy assignemnt operator $ */
TRMPoroelasticModes& TRMPoroelasticModes::operator = (const TRMPoroelasticModes& other){
    
    if (this != & other) // prevent self-assignment
    {
        this->fdimension    = other.fdimension;
        this->fSimulationData    = other.fSimulationData;
    }
    return *this;
}


void TRMPoroelasticModes::FillDataRequirements(TPZVec<TPZMaterialData> &datavec)
{
    int ndata = datavec.size();
    for (int idata=0; idata < ndata ; idata++) {
        datavec[idata].SetAllRequirements(false);
        datavec[idata].fNeedsSol = true;
    }
    
}

void TRMPoroelasticModes::FillBoundaryConditionDataRequirement(int type,TPZVec<TPZMaterialData> &datavec)
{
    int ndata = datavec.size();
    for (int idata=0; idata < ndata ; idata++) {
        datavec[idata].SetAllRequirements(false);
        datavec[idata].fNeedsSol = true;
        datavec[idata].fNeedsNormal = true;
    }
}

void TRMPoroelasticModes::Print(std::ostream &out) {
    out << "\t Base class print:\n";
    out << " name of material : " << this->Name() << "\n";
    TPZMaterial::Print(out);
}

int TRMPoroelasticModes::VariableIndex(const std::string &name) {
    if (!strcmp("u", name.c_str()))   return 0;
    if (!strcmp("p", name.c_str())) return 1;
    if (!strcmp("s_v", name.c_str())) return 4;
    if (!strcmp("id", name.c_str()))  return 5;
    return TPZMatWithMem::VariableIndex(name);
}

int TRMPoroelasticModes::NSolutionVariables(int var) {
    switch(var) {
        case 0:
            return fdimension; // vector
        case 1:
            return 1; // scalar
        case 4:
            return 1; // scalar
        case 5:
            return 1; // integer
    }
    
    DebugStop();
    return TPZMatWithMem::NSolutionVariables(var);
}

void TRMPoroelasticModes::Compute_Sigma(REAL & l, REAL & mu, TPZFMatrix<REAL> & S,TPZFMatrix<REAL> & Grad_u){
    
    
    REAL trace;
    for (int i = 0; i < 3; i++) {
        trace = 0.0;
        for (int j = 0; j < 3; j++) {
            S(i,j) = mu * (Grad_u(i,j) + Grad_u(j,i));
            trace +=  Grad_u(j,j);
        }
        S(i,i) += l * trace;
    }
    
    return;
}

void TRMPoroelasticModes::Contribute(TPZVec<TPZMaterialData> &datavec, REAL weight, TPZFMatrix<STATE> &ek, TPZFMatrix<STATE> &ef){
    
    if (Dimension() == 3) {
        this->Contribute_3D(datavec, weight, ek, ef);
        return;
    }
    
    int u_b = 0;
    int p_b = 1;
    
    TPZFMatrix<REAL> & phi_u        = datavec[u_b].phi;
    TPZFMatrix<REAL> & phi_p        = datavec[p_b].phi;

    
    TPZFNMatrix<20,REAL> grad_phi_u;
    TPZAxesTools<STATE>::Axes2XYZ(datavec[u_b].dphix, grad_phi_u, datavec[u_b].axes);
    
    TPZFNMatrix<3,REAL> grad_u_n;
    TPZAxesTools<STATE>::Axes2XYZ(datavec[u_b].dsol[0], grad_u_n, datavec[u_b].axes);
  
    REAL p_n  = 0.0;
    
    if (this->Id() != 12) {
        p_n = datavec[p_b].sol[0][0];
    }
    
    REAL phi_0  = 0.25;
    
    int nphi_u = phi_u.Rows();
    int nphi_p = phi_p.Rows();
    int first_u = 0;
    int first_p = nphi_u * 2;
    
    TPZFNMatrix<9,REAL> Grad_vx_i(fdimension,1,0.0);
    TPZFNMatrix<9,REAL> Grad_vy_i(fdimension,1,0.0);
    TPZFNMatrix<9,REAL> Grad_vx_j(fdimension,1,0.0);
    TPZFNMatrix<9,REAL> Grad_vy_j(fdimension,1,0.0);
    
    // Getting the space functions from memory
    long global_point_index = datavec[u_b].intGlobPtIndex;
    TRMMemory & memory = GetMemory()[global_point_index];
    
    REAL l_dr   = memory.lambda();
    REAL mu_dr  = memory.mu();
    REAL alpha  = memory.alpha();
    
    REAL rho_r = 2500.0;
    TPZManVector<REAL,3> g = fSimulationData->Gravity();
    TPZManVector<REAL,3> b(3,0.0);
//    b[0] = - rho_r*g[0];
//    b[1] = - rho_r*g[1];
//    b[2] = - rho_r*g[2];
    
    TPZFNMatrix<9,REAL> S_n(3,3);
    grad_u_n.Resize(3, 3);
    Compute_Sigma(l_dr, mu_dr, S_n, grad_u_n);
    
    REAL source = alpha * p_n;
    
    REAL dvxdx, dvxdy;
    REAL dvydx, dvydy;
    
    REAL duxdx, duxdy;
    REAL duydx, duydy;
    
    for (int iu = 0; iu < nphi_u; iu++) {
        
        // Computing Gradient of the test function for each component
        for (int d = 0; d < fdimension; d++) {
            Grad_vx_i(d,0) = grad_phi_u(d,iu);
            Grad_vy_i(d,0) = grad_phi_u(d,iu);
        }
        
        dvxdx = Grad_vx_i(0,0);
        dvxdy = Grad_vx_i(1,0);
        
        dvydx = Grad_vy_i(0,0);
        dvydy = Grad_vy_i(1,0);
        
        ef(2*iu + first_u, 0)   += weight * ((S_n(0,0) - source) * Grad_vx_i(0,0) + S_n(0,1) * Grad_vx_i(1,0) - b[0] * phi_u(iu,0));
        ef(2*iu+1 + first_u, 0)	+= weight * (S_n(1,0) * Grad_vy_i(0,0) + (S_n(1,1) - source) * Grad_vy_i(1,0) - b[1] * phi_u(iu,0));
        
        for (int ju = 0; ju < nphi_u; ju++) {
            
            // Computing Gradient of the test function for each component
            for (int d = 0; d < fdimension; d++) {
                Grad_vx_j(d,0) = grad_phi_u(d,ju);
                Grad_vy_j(d,0) = grad_phi_u(d,ju);
            }
            
            duxdx = Grad_vx_j(0,0);
            duxdy = Grad_vx_j(1,0);
            
            duydx = Grad_vy_j(0,0);
            duydy = Grad_vy_j(1,0);
            
            ek(2*iu + first_u, 2*ju + first_u)      += weight * ( (2.0*mu_dr + l_dr)*duxdx*dvxdx + mu_dr*duxdy*dvxdy);
            ek(2*iu + first_u, 2*ju+1 + first_u)    += weight * ( l_dr*duydy*dvxdx + mu_dr*duydx*dvxdy);
            ek(2*iu+1 + first_u, 2*ju + first_u)	+= weight * ( l_dr*duxdx*dvydy + mu_dr*duxdy*dvydx);
            ek(2*iu+1 + first_u, 2*ju+1 + first_u)	+= weight * ( (2.0*mu_dr + l_dr)*duydy*dvydy + mu_dr*duydx*dvydx);
            
        }
        
    }
    
    if (this->Id() == 12) {
        return;
    }
    
    for (int ip = 0; ip < nphi_p; ip++) {
        
        ef(ip + first_p, 0)		+= weight *  ( 0.0 * phi_p(ip,0));
        
        for (int jp = 0; jp < nphi_p; jp++) {
            
            ek(ip + first_p, jp + first_p)		+= weight * phi_p(jp,0) * phi_p(ip,0);
        }
        
    }
    
    
}

void TRMPoroelasticModes::Contribute(TPZVec<TPZMaterialData> &datavec, REAL weight, TPZFMatrix<STATE> &ef){
    
    TPZFMatrix<STATE>  ek_fake(ef.Rows(),ef.Rows(),0.0);
    this->Contribute(datavec, weight, ek_fake, ef);
    
}

void TRMPoroelasticModes::ContributeBC(TPZVec<TPZMaterialData> &datavec, REAL weight, TPZFMatrix<STATE> &ek, TPZFMatrix<STATE> &ef, TPZBndCond &bc){
    
    if (Dimension() == 3) {
        this->ContributeBC_3D(datavec, weight, ek, ef, bc);
        return;
    }
    
    if (!fSimulationData->IsCurrentStateQ()) {
        return;
    }
    
    
    int u_b = 0;
    
    TPZFMatrix<REAL>  &phiu = datavec[u_b].phi;
    TPZManVector<REAL,10>  &u = datavec[u_b].sol[0];

    int phru = phiu.Rows();
    short in,jn;
    REAL v[2];
    v[0] = bc.Val2()(0,0);	//	Ux displacement
    v[1] = bc.Val2()(1,0);	//	Uy displacement
    
    // Dirichlet in Pressure
    switch (bc.Type())
    {
        case 0 :
        {
            //	Dirichlet condition for each state variable
            //	Elasticity Equation
            for(in = 0 ; in < phru; in++)
            {
                //	Contribution for load Vector
                ef(2*in,0)		+= gBigNumber*(u[0] - v[0])*phiu(in,0)*weight;	// X displacement Value
                ef(2*in+1,0)	+= gBigNumber*(u[1] - v[1])*phiu(in,0)*weight;	// y displacement Value
                
                for (jn = 0 ; jn < phru; jn++)
                {
                    //	Contribution for Stiffness Matrix
                    ek(2*in,2*jn)		+= gBigNumber*phiu(in,0)*phiu(jn,0)*weight;	// X displacement
                    ek(2*in+1,2*jn+1)	+= gBigNumber*phiu(in,0)*phiu(jn,0)*weight;	// Y displacement
                }
            }
            break;
            
        }
            
        case 1 :
        {
            //	Dirichlet condition for each state variable
            //	Elasticity Equation
            for(in = 0 ; in < phru; in++)
            {
                //	Contribution for load Vector
                ef(2*in,0)		+= gBigNumber*(u[0] - v[0])*phiu(in,0)*weight;	// X displacement Value
                
                for (jn = 0 ; jn < phru; jn++)
                {
                    //	Contribution for Stiffness Matrix
                    ek(2*in,2*jn)		+= gBigNumber*phiu(in,0)*phiu(jn,0)*weight;	// X displacement
                }
            }
            break;
            
        }
            
        case 2 :
        {
            //	Dirichlet condition for y each state variable
            //	Elasticity Equation
            for(in = 0 ; in < phru; in++)
            {
                //	Contribution for load Vector
                ef(2*in+1,0)	+= gBigNumber*(u[1] - v[1])*phiu(in,0)*weight;	// y displacement Value
                
                for (jn = 0 ; jn < phru; jn++)
                {
                    //	Contribution for Stiffness Matrix
                    ek(2*in+1,2*jn+1)	+= gBigNumber*phiu(in,0)*phiu(jn,0)*weight;	// Y displacement
                }
            }
            break;
            
        }
            
        case 3 :
        {
            //	Neumann condition for each state variable
            //	Elasticity Equation
            for(in = 0 ; in < phru; in++)
            {
                //	Normal Tension Components on neumman boundary
                ef(2*in,0)		+= -1.0 * weight * v[0] * phiu(in,0);		//	Tnx
                ef(2*in+1,0)	+= -1.0 * weight * v[1] * phiu(in,0);		//	Tny
            }
            
            break;
        }
            
        case 4 :
        {
            //	Neumann condition for each state variable
            //	Elasticity Equation
            for(in = 0 ; in <phru; in++)
            {
                //	Normal Tension Components on neumman boundary
                ef(2*in,0)		+= -1.0* v[0]*phiu(in,0)*weight;		//	Tnx
            }
            
            break;
        }
            
        case 5 :
        {
            //	Neumann condition for each state variable
            //	Elasticity Equation
            for(in = 0 ; in <phru; in++)
            {
                //	Normal Tension Components on neumman boundary
                ef(2*in+1,0)	+= -1.0* v[1]*phiu(in,0)*weight;		//	Tny
            }
            
            break;
        }
            
        case 6 :
        {
            
            //	Neumann condition for each state variable
            //	Elasticity Equation
            
            REAL well_pressure = bc.Val2()(0,0);
            if (bc.HasTimedependentBCForcingFunction()) {
                TPZManVector<STATE,2> f(2);
                TPZFMatrix<double> gradf;
                REAL time = fSimulationData->t();
                bc.TimedependentBCForcingFunction()->Execute(datavec[u_b].x, time, f, gradf);
                well_pressure = f[0];
            }
            
            
            REAL Tn[2];
            TPZManVector<STATE,3> n =  datavec[u_b].normal;
            
            for (int d = 0; d < Dimension(); d++) {
                Tn[d] = well_pressure * n[d];
            }
            
            for(in = 0 ; in < phru; in++)
            {
                //	Normal Tension Components on neumman boundary
                ef(2*in,0)		+= 1.0 * weight * Tn[0] * phiu(in,0);		//	Tnx
                ef(2*in+1,0)	+= 1.0 * weight * Tn[1] * phiu(in,0);		//	Tny
            }
            
            break;
        }
        default:
        {
            DebugStop();
        }
            break;
    }
    
    
}

void TRMPoroelasticModes::Contribute_3D(TPZVec<TPZMaterialData> &datavec, REAL weight, TPZFMatrix<STATE> &ek, TPZFMatrix<STATE> &ef){
    
    int nvars = 4; // {p,sa,sb,t}
    int u_b = 0;
    int p_b = 1;
    
    TPZFMatrix<REAL> & phi_u        = datavec[u_b].phi;
    TPZFMatrix<REAL> & phi_p        = datavec[p_b].phi;
    
    
    TPZFNMatrix<20,REAL> grad_phi_u;
    TPZAxesTools<STATE>::Axes2XYZ(datavec[u_b].dphix, grad_phi_u, datavec[u_b].axes);
    
    TPZFNMatrix<3,REAL> grad_u_n;
    TPZAxesTools<STATE>::Axes2XYZ(datavec[u_b].dsol[0], grad_u_n, datavec[u_b].axes);
    
    REAL p_n  = 0.0;
    
    if (this->Id() != 14) {
        p_n = datavec[p_b].sol[0][0];
    }
    else{
        int aja = 0;
    }
    
    REAL phi_0  = 0.25;
    
    int nphi_u = phi_u.Rows();
    int nphi_p = phi_p.Rows();
    int first_u = 0;
    int first_p = nphi_u * 3;
    
    
    // Getting the space functions from memory
    long global_point_index = datavec[u_b].intGlobPtIndex;
    TRMMemory & memory = GetMemory()[global_point_index];
    
    REAL l_dr   = memory.lambda();
    REAL mu_dr  = memory.mu();
    REAL alpha  = memory.alpha();
    REAL sw     = memory.sa_n();
    
    TPZManVector<REAL,3> b(3,0.0);
    
    TPZFNMatrix<9,REAL> S_n(3,3);
    grad_u_n.Resize(3, 3);
    Compute_Sigma(l_dr, mu_dr, S_n, grad_u_n);
    
    REAL source = alpha * p_n;
    
    
    TPZFNMatrix<9,REAL> Grad_vx_i(fdimension,1,0.0);
    TPZFNMatrix<9,REAL> Grad_vy_i(fdimension,1,0.0);
    TPZFNMatrix<9,REAL> Grad_vz_i(fdimension,1,0.0);
    TPZFNMatrix<9,REAL> Grad_vx_j(fdimension,1,0.0);
    TPZFNMatrix<9,REAL> Grad_vy_j(fdimension,1,0.0);
    TPZFNMatrix<9,REAL> Grad_vz_j(fdimension,1,0.0);
    
    REAL dvxdx, dvxdy, dvxdz;
    REAL dvydx, dvydy, dvydz;
    REAL dvzdx, dvzdy, dvzdz;
    
    REAL duxdx, duxdy, duxdz;
    REAL duydx, duydy, duydz;
    REAL duzdx, duzdy, duzdz;
    
    for (int iu = 0; iu < nphi_u; iu++) {
        
        // Computing Gradient of the test function for each component
        for (int d = 0; d < fdimension; d++) {
            Grad_vx_i(d,0) = grad_phi_u(d,iu);
            Grad_vy_i(d,0) = grad_phi_u(d,iu);
            Grad_vz_i(d,0) = grad_phi_u(d,iu);
        }
        
        ef(3*iu + first_u, 0)   += weight * ((S_n(0,0) - source) * Grad_vx_i(0,0) + S_n(0,1) * Grad_vx_i(1,0) + S_n(0,2) * Grad_vx_i(2,0) - b[0] * phi_u(iu,0));
        ef(3*iu+1 + first_u, 0)	+= weight * (S_n(1,0) * Grad_vy_i(0,0) + (S_n(1,1) - source) * Grad_vy_i(1,0) + S_n(1,2) * Grad_vy_i(2,0) - b[1] * phi_u(iu,0));
        ef(3*iu+2 + first_u, 0)	+= weight * (S_n(2,0) * Grad_vz_i(0,0) + S_n(2,1) * Grad_vz_i(1,0) + (S_n(2,2) - source) * Grad_vz_i(2,0) - b[2] * phi_u(iu,0));
        
        //x
        dvxdx = Grad_vx_i(0,0);
        dvxdy = Grad_vx_i(1,0);
        dvxdz = Grad_vx_i(2,0);
        
        //y
        dvydx = Grad_vy_i(0,0);
        dvydy = Grad_vy_i(1,0);
        dvydz = Grad_vy_i(2,0);
        
        //z
        dvzdx = Grad_vz_i(0,0);
        dvzdy = Grad_vz_i(1,0);
        dvzdz = Grad_vz_i(2,0);
        
        
        for (int ju = 0; ju < nphi_u; ju++) {
            
            // Computing Gradient of the test function for each component
            for (int d = 0; d < fdimension; d++) {
                Grad_vx_j(d,0) = grad_phi_u(d,ju);
                Grad_vy_j(d,0) = grad_phi_u(d,ju);
                Grad_vz_j(d,0) = grad_phi_u(d,ju);
            }
            
            //x
            duxdx = Grad_vx_j(0,0);
            duxdy = Grad_vx_j(1,0);
            duxdz = Grad_vx_j(2,0);
            
            //y
            duydx = Grad_vy_j(0,0);
            duydy = Grad_vy_j(1,0);
            duydz = Grad_vy_j(2,0);
            
            //z
            duzdx = Grad_vz_j(0,0);
            duzdy = Grad_vz_j(1,0);
            duzdz = Grad_vz_j(2,0);
            
            // Gradient 1
            ek(3*iu + first_u, 3*ju + first_u)      += weight * ((l_dr + 2.*mu_dr)*duxdx*dvxdx + mu_dr*duxdy*dvxdy + mu_dr*duxdz*dvxdz);
            ek(3*iu + first_u, 3*ju+1  + first_u)   += weight * (l_dr*duydy*dvxdx + mu_dr*duydx*dvxdy);
            ek(3*iu + first_u, 3*ju+2  + first_u)   += weight * (l_dr*duzdz*dvxdx + mu_dr*duzdx*dvxdz);
            
            // Gradient 2
            ek(3*iu+1 + first_u, 3*ju  + first_u)   += weight * (l_dr*duxdx*dvydy + mu_dr*duxdy*dvydx);
            ek(3*iu+1 + first_u, 3*ju+1  + first_u) += weight * ((l_dr + 2.*mu_dr)*duydy*dvydy + mu_dr*duydx*dvydx + mu_dr*duydz*dvydz);
            ek(3*iu+1 + first_u, 3*ju+2  + first_u) += weight * (l_dr*duzdz*dvydy + mu_dr*duzdy*dvydz);
            
            // Gradient 3
            ek(3*iu+2 + first_u, 3*ju  + first_u)   += weight * (l_dr*duxdx*dvzdz + mu_dr*duxdz*dvzdx);
            ek(3*iu+2 + first_u, 3*ju+1  + first_u) += weight * (l_dr*duydy*dvzdz + mu_dr*duydz*dvzdy);
            ek(3*iu+2 + first_u, 3*ju+2  + first_u) += weight * ((l_dr + 2.*mu_dr)*duzdz*dvzdz + mu_dr*duzdx*dvzdx + mu_dr*duzdy*dvzdy);
            
        }
    }
    
    
    if (this->Id() == 14) {
        return;
    }
    
    for (int ip = 0; ip < nphi_p; ip++) {
        
        ef(ip + first_p, 0)		+= weight *  ( 0.0 * phi_p(ip,0));
        
        for (int jp = 0; jp < nphi_p; jp++) {
            
            ek(ip + first_p, jp + first_p)		+= weight * phi_p(jp,0) * phi_p(ip,0);
        }
        
    }
    
}

void TRMPoroelasticModes::ContributeBC_3D(TPZVec<TPZMaterialData> &datavec, REAL weight, TPZFMatrix<STATE> &ek, TPZFMatrix<STATE> &ef, TPZBndCond &bc){
    
    if (!fSimulationData->IsCurrentStateQ()) {
        return;
    }
    
    
    int u_b = 0;
    
    TPZFMatrix<REAL>  &phiu = datavec[u_b].phi;
    TPZManVector<REAL,10>  &u = datavec[u_b].sol[0];
    
    int phru = phiu.Rows();
    short in,jn;
    REAL v[3];
    v[0] = bc.Val2()(0,0);	//	Ux displacement
    v[1] = bc.Val2()(1,0);	//	Uy displacement
    v[2] = bc.Val2()(2,0);	//	Uz displacement
    
    // Dirichlet in Pressure
    switch (bc.Type())
    {
        case 0 :
        {
            //	Dirichlet condition for each state variable
            //	Elasticity Equation
            for(in = 0 ; in < phru; in++)
            {
                //	Contribution for load Vector
                ef(3*in,0)		+= gBigNumber*(u[0] - v[0])*phiu(in,0)*weight;	// X displacement Value
                ef(3*in+1,0)	+= gBigNumber*(u[1] - v[1])*phiu(in,0)*weight;	// y displacement Value
                ef(3*in+2,0)	+= gBigNumber*(u[2] - v[2])*phiu(in,0)*weight;	// Z displacement Value
                
                for (jn = 0 ; jn < phru; jn++)
                {
                    //	Contribution for Stiffness Matrix
                    ek(3*in,3*jn)		+= gBigNumber*phiu(in,0)*phiu(jn,0)*weight;	// X displacement
                    ek(3*in+1,3*jn+1)	+= gBigNumber*phiu(in,0)*phiu(jn,0)*weight;	// Y displacement
                    ek(3*in+2,3*jn+2)	+= gBigNumber*phiu(in,0)*phiu(jn,0)*weight;	// Z displacement
                }
            }
            break;
            
        }
            
        case 1 :
        {
            
            //	Elasticity Equation
            for(in = 0 ; in < phru; in++)
            {
                //	Contribution for load Vector
                ef(3*in,0)		+= gBigNumber*(u[0] - v[0])*phiu(in,0)*weight;	// X displacement Value
                ef(3*in+1,0)	+= gBigNumber*(u[1] - v[1])*phiu(in,0)*weight;	// y displacement Value
                
                for (jn = 0 ; jn < phru; jn++)
                {
                    //	Contribution for Stiffness Matrix
                    ek(3*in,3*jn)		+= gBigNumber*phiu(in,0)*phiu(jn,0)*weight;	// X displacement
                    ek(3*in+1,3*jn+1)	+= gBigNumber*phiu(in,0)*phiu(jn,0)*weight;	// Y displacement
                }
            }
            break;
            
        }
            
        case 2 :
        {
            DebugStop();
            break;
            
        }
            
        case 3 :
        {
            
            //	Neumann condition for each state variable
            //	Elasticity Equation
            for(in = 0 ; in < phru; in++)
            {
                //	Normal Tension Components on neumman boundary
                ef(3*in,0)		+= -1.0 * weight * v[0] * phiu(in,0);		//	Tnx
                ef(3*in+1,0)	+= -1.0 * weight * v[1] * phiu(in,0);		//	Tny
                ef(3*in+2,0)	+= -1.0 * weight * v[2] * phiu(in,0);		//	Tnz
            }
            
            break;
        }
            
        case 4 :
        {
            DebugStop();
            break;
        }
            
        case 5 :
        {
            DebugStop();
            break;
        }
            
        case 6 :
        {
            
            //	Neumann condition for each state variable
            //	Elasticity Equation
            
            REAL well_pressure = bc.Val2()(0,0);
            if (bc.HasTimedependentBCForcingFunction()) {
                TPZManVector<STATE,2> f(2);
                TPZFMatrix<double> gradf;
                REAL time = fSimulationData->t();
                bc.TimedependentBCForcingFunction()->Execute(datavec[u_b].x, time, f, gradf);
                well_pressure = f[0];
            }
            
            
            REAL Tn[3];
            TPZManVector<STATE,3> n =  datavec[u_b].normal;
            
            for (int d = 0; d < Dimension(); d++) {
                Tn[d] = well_pressure * n[d];
            }
            
            for(in = 0 ; in < phru; in++)
            {
                //	Normal Tension Components on neumman boundary
                ef(2*in,0)		+= 1.0 * weight * Tn[0] * phiu(in,0);		//	Tnx
                ef(2*in+1,0)	+= 1.0 * weight * Tn[1] * phiu(in,0);		//	Tny
                ef(3*in+2,0)	+= 1.0 * weight * Tn[2] * phiu(in,0);		//	Tnz
            }
            
            break;
        }
        default:
        {
            DebugStop();
        }
            break;
    }
}


void TRMPoroelasticModes::ContributeBCInterface(TPZMaterialData &data, TPZVec<TPZMaterialData> &datavecleft, REAL weight, TPZFMatrix<STATE> &ek, TPZFMatrix<STATE> &ef, TPZBndCond &bc){
    DebugStop();
}

void TRMPoroelasticModes::ContributeBCInterface(TPZMaterialData &data, TPZVec<TPZMaterialData> &datavecleft, REAL weight, TPZFMatrix<STATE> &ef, TPZBndCond &bc){
    DebugStop();
}

void TRMPoroelasticModes::ContributeInterface(TPZMaterialData &data, TPZVec<TPZMaterialData> &datavecleft, TPZVec<TPZMaterialData> &datavecright, REAL weight, TPZFMatrix<STATE> &ek,TPZFMatrix<STATE> &ef){
    DebugStop();
}

void TRMPoroelasticModes::ContributeInterface(TPZMaterialData &data, TPZVec<TPZMaterialData> &datavecleft, TPZVec<TPZMaterialData> &datavecright, REAL weight,TPZFMatrix<STATE> &ef){
    DebugStop();
}

void TRMPoroelasticModes::Solution(TPZVec<TPZMaterialData> &datavec, int var, TPZVec<REAL> &Solout) {
    
    Solout.Resize( this->NSolutionVariables(var));
    
    int u_b = 0;
    int p_b = 1;
    
    // Getting the solutions and derivatives
    TPZManVector<REAL,2> u = datavec[u_b].sol[0];
    
    REAL l_dr   = 2.30769e9;
    REAL mu_dr  = 1.53846e9;
    
    TPZFNMatrix <6,REAL> du     = datavec[u_b].dsol[0];
    TPZFNMatrix <9,REAL> axes_u	= datavec[u_b].axes;
    
    // Computing Gradient of the Solution
    TPZFNMatrix<9,REAL> Grad_u(3,3,0.0),S(3,3,0.0);
    TPZAxesTools<STATE>::Axes2XYZ(du, Grad_u, axes_u);
    
    Grad_u.Resize(3, 3);
    this->Compute_Sigma(l_dr, mu_dr, S, Grad_u);
    S *= 1.0e-6;
    
    
    //	Displacements
    if(var == 0){
        for (int d = 0; d < Dimension(); d++) {
            Solout[d] = u[d];
        }
        return;
    }
    
    if(var == 1){
        bool side_burdenQ = this->Id() == 12 || this->Id() == 14;
        if (side_burdenQ) {
            Solout[0] = 0.0;
        }
        else{
            int p = datavec[p_b].sol[0][0];
            Solout[0] = p;
        }
        return;
    }
    
    if(var == 4){
        Solout[0] = (S(0,0)+S(1,1)+S(2,2))/3.0;
        return;
    }
    
    if(var == 5){
        Solout[0] = this->Id();
        return;
    }
    
    std::cout  << " not implemented. " << std::endl;
    DebugStop();
    
}


int TRMPoroelasticModes::ClassId() const {
    return -63786378;
}

// -------------------------------------------------------------------------------------------

void TRMPoroelasticModes::Write(TPZStream &buf, int withclassid) {
    
    TPZDiscontinuousGalerkin::Write(buf, withclassid);
    
}

// -------------------------------------------------------------------------------------------

void TRMPoroelasticModes::Read(TPZStream &buf, void *context) {
    TPZDiscontinuousGalerkin::Read(buf, context);
    
}

// Update element memory by copying the n+1 data to the n data
void TRMPoroelasticModes::UpdateMemory()
{
    DebugStop();
}