//
//  TPZSBFemVolume.cpp
//  PZ
//
//  Created by Philippe Devloo on 4/4/16.
//
//

#include "TPZSBFemVolume.h"
#include "pzintel.h"
#include "pzmaterial.h"
#include "pzelmat.h"
#include "pzgraphelq2dd.h"


TPZSBFemVolume::TPZSBFemVolume(TPZCompMesh &mesh, TPZGeoEl *gel,long &index) : TPZCompEl(mesh,gel,index), fElementGroupIndex(-1), fSkeleton(-1), fDensity(1.)
{
    
}



/// Compute the K matrices
void TPZSBFemVolume::ComputeKMatrices(TPZElementMatrix &E0, TPZElementMatrix &E1, TPZElementMatrix &E2, TPZElementMatrix &M0)
{
    // do all the computations here
    
    TPZElementMatrix efmat(Mesh(),TPZElementMatrix::EF);
    
    TPZGeoEl *Ref2D = Reference();
    TPZGeoMesh *gmesh = Ref2D->Mesh();
    
    TPZCompMesh *cmesh = Mesh();
    
    TPZInterpolatedElement *CSkeleton = dynamic_cast<TPZInterpolatedElement *>(cmesh->Element(fSkeleton));
    
    CSkeleton->InitializeElementMatrix(E0, efmat);
    CSkeleton->InitializeElementMatrix(E1, efmat);
    CSkeleton->InitializeElementMatrix(E2, efmat);
    CSkeleton->InitializeElementMatrix(M0, efmat);
    
    TPZGeoEl *Ref1D = CSkeleton->Reference();
    
    int matid = Ref2D->MaterialId();
    int dimension = Ref2D->Dimension();
    
    // find the first face side
    int nsides = Ref2D->NSides();
    int is;
    for (is=nsides-1; is>0; is--) {
        if (Ref2D->SideDimension(is) < dimension-1) {
            break;
        }
    }
    int faceside = is+1;
    
    TPZGeoElSide thisside(Ref2D,faceside);
    
    
    TPZMaterial *mat2d = cmesh->FindMaterial(matid);
    
    if(!mat2d) DebugStop();
    
    int nstate = mat2d->NStateVariables();
    
    TPZGeoElSide SkeletonSide(Ref1D,Ref1D->NSides()-1);
    
    TPZTransform tr(2, 1);
    tr = SkeletonSide.NeighbourSideTransform(thisside);
    TPZTransform t2 = Ref2D->SideToSideTransform(thisside.Side(), Ref2D->NSides()-1);
    tr = t2.Multiply(tr);
    // create a one-d integration rule
    TPZIntPoints &intpoints = CSkeleton->GetIntegrationRule();
    
    TPZMaterialData data1d;
    TPZMaterialData data2d;
    CSkeleton->InitMaterialData(data1d);
    CSkeleton->InitMaterialData(data2d);
    int nshape = data2d.phi.Rows();
    data2d.phi.Redim(nshape*2, 1);
    data2d.dphi.Redim(2, 2*nshape);
    data2d.dphix.Redim(2, 2*nshape);
    data2d.dsol[0].Redim(2,2);
    
    TPZFNMatrix<200,STATE> ek(nshape*nstate*2,nshape*nstate*2,0.), ef(nshape*nstate*2,1,0.);
    int npoint = intpoints.NPoints();
    for (int ip = 0; ip<npoint; ip++)
    {
        TPZManVector<REAL,3> xi(1), xiquad(2);
        REAL weight;
        intpoints.Point(ip, xi, weight);
        tr.Apply(xi, xiquad);
        TPZFNMatrix<9,REAL> jacobian(1,1),axes(1,3),jacinv(1,1);
        REAL detjac;
        Ref1D->Jacobian(xi,jacobian,axes,detjac,jacinv);
        Ref2D->Jacobian(xiquad, data2d.jacobian, data2d.axes, data2d.detjac, data2d.jacinv);
        CSkeleton->ComputeRequiredData(data1d, xi);
        ExtendShapeFunctions(data1d,data2d);
        
        for (int i=0; i<nshape; i++) {
            for (int j=0; j<nshape; j++) {
                for (int st=0; st<nstate; st++) {
                    M0.fMat(i*nstate+st,j*nstate+st) += weight*data1d.phi(i,0)*data1d.phi(j,0)*fDensity;
                }
            }
        }
        weight *= fabs(data2d.detjac)*2.;
        // compute the contributions to K11 K12 and K22
        mat2d->Contribute(data2d,weight,ek,ef);
    }
    for (int i=0; i<nstate*nshape; i++) {
        for (int j=0; j<nstate*nshape; j++) {
            E0.fMat(i,j) = ek(i,j);
            E1.fMat(j,i) = ek(i,j+nstate*nshape);
            E2.fMat(i,j) = ek(i+nstate*nshape,j+nstate*nshape);
        }
    }
}

/// extend the border shape functions for SBFem computations
void TPZSBFemVolume::ExtendShapeFunctions(TPZMaterialData &data1d, TPZMaterialData &data2d)
{
    int dim = Reference()->Dimension();
    long nshape = data2d.phi.Rows()/2;
    for (int ish=0; ish<nshape; ish++) {
        data2d.phi(ish+nshape,0) = data1d.phi(ish,0);
        for (int d=0; d<dim-1; d++) {
            data2d.dphi(d,ish+nshape) = data1d.dphi(d,ish);
            data2d.dphi(d,ish) = 0.;
        }
        data2d.dphi(dim-1,ish) = -data1d.phi(ish)/2.;
        data2d.dphi(dim-1,ish+nshape) = 0.;
    }
    TPZInterpolationSpace::Convert2Axes(data2d.dphi, data2d.jacinv, data2d.dphix);

}

TPZCompEl * CreateSBFemCompEl(TPZGeoEl *gel,TPZCompMesh &mesh,long &index)
{
    return new TPZSBFemVolume(mesh,gel,index);
}

/// initialize the data structures of the eigenvectors and eigenvalues associated with this volume element
void TPZSBFemVolume::SetPhiEigVal(TPZFMatrix<STATE> &phi, TPZManVector<STATE> &eigval)
{
    fEigenvalues = eigval;
    int nrow = fLocalIndices.size();
    fPhi.Resize(nrow, phi.Cols());
    for (int i=0; i<nrow; i++) {
        for (int j=0; j<phi.Cols(); j++) {
            fPhi(i,j) = phi(fLocalIndices[i],j);
        }
    }
}

/** @brief Loads the solution within the internal data structure of the element */
/**
 * Is used to initialize the solution of connect objects with dependency. \n
 * Is also used to load the solution within SuperElements
 */
void TPZSBFemVolume::LoadCoef(TPZFMatrix<STATE> &coef)
{
    fCoeficients = coef;
}

/**
 * @brief Computes solution and its derivatives in the local coordinate qsi.
 * @param qsi master element coordinate
 * @param sol finite element solution
 * @param dsol solution derivatives
 * @param axes axes associated with the derivative of the solution
 */
void TPZSBFemVolume::ComputeSolution(TPZVec<REAL> &qsi,
                             TPZSolVec &sol, TPZGradSolVec &dsol,TPZFMatrix<REAL> &axes)
{
    TPZCompMesh *cmesh = Mesh();
    sol.Resize(fCoeficients.Cols());
    dsol.Resize(fCoeficients.Cols());
    TPZGeoEl *Ref2D = Reference();
    int matid = Ref2D->MaterialId();
    TPZMaterial *mat2d = cmesh->FindMaterial(matid);

    int dim = Ref2D->Dimension();
    REAL sbfemparam = (1.-qsi[dim-1])/2.;
    if (sbfemparam<0.) {
        std::cout << "sbfemparam " << sbfemparam << std::endl;
        sbfemparam = 0.;
    }
    if (IsZero(sbfemparam)) {
        sbfemparam = 1.e-6;
        qsi[0] = 0.;
        qsi[1] = 1.-2.e-6;
    }
    TPZInterpolatedElement *CSkeleton = dynamic_cast<TPZInterpolatedElement *>(cmesh->Element(fSkeleton));
    TPZMaterialData data1d,data2d;
    // compute the lower dimensional shape functions
    TPZManVector<REAL,3> qsilow(qsi);
    qsilow.Resize(dim-1);
    CSkeleton->InitMaterialData(data1d);
    TPZGeoEl *Ref1D = CSkeleton->Reference();

    Ref1D->Jacobian(qsilow,data1d.jacobian,data1d.axes,data1d.detjac,data1d.jacinv);
    Ref2D->Jacobian(qsi, data2d.jacobian, data2d.axes, data2d.detjac, data2d.jacinv);
    axes = data2d.axes;
    CSkeleton->ComputeRequiredData(data1d, qsilow);

    int nshape = data1d.phi.Rows();
    int nstate = mat2d->NStateVariables();
#ifdef PZDEBUG
    if (fPhi.Cols() != fCoeficients.Rows()) {
        DebugStop();
    }
#endif
    for (int s=0; s<sol.size(); s++)
    {
        TPZManVector<STATE,10> uh_xi(fPhi.Rows(),0.), Duh_xi(fPhi.Rows(),0.);
        int nphixi = fPhi.Rows();
        int numeig = fPhi.Cols();
        for (int c=0; c<numeig; c++) {
            REAL xiexp;
            REAL xiexpm1;
            if(IsZero(fEigenvalues[c]))
            {
                xiexp = 1;
                xiexpm1 = 0;
            }
            else if(IsZero(fEigenvalues[c]+1.))
            {
                xiexp = sbfemparam;
                xiexpm1 = 1;
            }
            else
            {
                xiexp = pow(sbfemparam,-fEigenvalues[c]);
                xiexpm1 = pow(sbfemparam,-fEigenvalues[c]-1.);
            }
            for (int i=0; i<nphixi; i++) {
                uh_xi[i] += fCoeficients(c,s)*xiexp*fPhi(i,c);
                Duh_xi[i] += -fCoeficients(c,s)*fEigenvalues[c]*xiexpm1*fPhi(i,c);
            }
        }
        std::cout << "uh_xi " << uh_xi << std::endl;
        std::cout << "Duh_xi " << Duh_xi << std::endl;
        sol[s].Resize(nstate);
        sol[s].Fill(0.);
        TPZFNMatrix<9,STATE> dsollow(dim-1,nstate,0.), dsolxieta(dim,nstate,0.);
        TPZManVector<STATE,3> dsolxi(nstate,0.);
        for (int ishape=0; ishape<nshape; ishape++) {
            for (int istate=0; istate<nstate; istate++) {
                sol[s][istate] += data1d.phi(ishape)*uh_xi[ishape*nstate+istate];
                dsolxi[istate] += data1d.phi(ishape)*Duh_xi[ishape*nstate+istate];
                for (int d=0; d<dim-1; d++) {
                    dsollow(d,istate) += data1d.dphi(d,ishape)*uh_xi[ishape*nstate+istate];
                }
            }
        }
        for (int istate = 0; istate<nstate; istate++) {
            for (int d=0; d<dim-1; d++) {
                dsolxieta(d,istate) = dsollow(d,istate);
            }
            dsolxieta(dim-1,istate) = -dsolxi[istate]/2.;
        }
        dsol[s].Resize(dim, nstate);
        dsol[s].Zero();
        for (int istate = 0; istate<nstate; istate++)
        {
            for (int d1=0; d1<dim; d1++) {
                for (int d2=0; d2<dim; d2++) {
                    dsol[s](d1,istate) += data2d.jacinv(d2,d1)*dsolxieta(d2,istate);
                }
            }
        }
    }
    
    // tototototot
    std::cout << "Solution " << sol[0] << std::endl;
    dsol[0].Print("DSol",std::cout);
}


/**
 * @brief Calculates the solution - sol - for the variable var
 * at point qsi, where qsi is expressed in terms of the
 * master element coordinates
 * @param qsi master element coordinate
 * @param var variable name
 * @param sol vetor for the solution
 */
void TPZSBFemVolume::Solution(TPZVec<REAL> &qsi,int var,TPZVec<STATE> &sol)
{
    TPZGeoEl *Ref2D = Reference();
    int matid = Ref2D->MaterialId();
    TPZCompMesh *cmesh = Mesh();

    TPZMaterial *mat2d = cmesh->FindMaterial(matid);
    TPZMaterialData data2d;

    ComputeSolution(qsi, data2d.sol, data2d.dsol, data2d.axes);
    mat2d->Solution(data2d, var, sol);
    
}

void TPZSBFemVolume::CreateGraphicalElement(TPZGraphMesh &graphmesh, int dimension) {

    if(dimension ==2)
    {
        new TPZGraphElQ2dd(this,&graphmesh);
    }
}
