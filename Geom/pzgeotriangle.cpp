/**
 * @file
 * @brief Contains the implementation of the TPZGeoTriangle methods. 
 */

#include "pzgeotriangle.h"
#include "pzfmatrix.h"
#include "pzgeoel.h"
#include "pzshapetriang.h"
#include "pzgmesh.h"
#include "tpzgeoelrefpattern.h"

#include "pzlog.h"
#ifdef _AUTODIFF
#include "fad.h"
#endif

#ifdef LOG4CXX
static log4cxx::LoggerPtr logger(Logger::getLogger("pz.geom.pzgeotriangle"));
#endif

using namespace pzshape;
using namespace std;

namespace pzgeom {
	
	const REAL tol = pzgeom_TPZNodeRep_tol;
    template<class T>
    void TPZGeoTriangle::CalcSideInfluence(const int &side, const TPZVec<T> &xi, T &correctionFactor,
            TPZVec<T> &correctionFactorDxi){
#ifdef PZDEBUG
        std::ostringstream sout;
        if(side < NNodes || side >= NSides){
            sout<<"The side\t"<<side<<"is invalid. Aborting..."<<std::endl;
            PZError<<std::endl<<sout.str()<<std::endl;
            DebugStop();
        }

        if(!pztopology::TPZTriangle::IsInParametricDomain(xi,tol)){
            sout<<"The method CalcSideInfluence expects the point xi to correspond to coordinates of a point";
            sout<<" inside the parametric domain. Aborting...";
            PZError<<std::endl<<sout.str()<<std::endl;
            #ifdef LOG4CXX
            LOGPZ_FATAL(logger,sout.str().c_str());
            #endif
            DebugStop();
        }
#endif
        TPZFNMatrix<4,T> phi(NNodes,1);
        TPZFNMatrix<8,T> dphi(Dimension,NNodes);
        TPZGeoTriangle::TShape(xi,phi,dphi);
        correctionFactorDxi.Resize(TPZGeoTriangle::Dimension, (T) 0);
        int i = -1;
        switch(side){
            case 0:
            case 1:
            case 2:
                correctionFactor = 0;
                return;
            case 3:
                i = 0;
                break;
            case 4:
                i = 1;
                break;
            case 5:
                i = 2;
                break;
            case 6:
                correctionFactor = 1;
                return;
        }
        correctionFactor = phi(i,0) + phi((i+1)%NNodes,0);
        correctionFactor *= correctionFactor;
        correctionFactorDxi[0] = 2 * ( phi(i,0) + phi((i+1)%NNodes,0) ) * ( dphi(0,i) + dphi(0,(i+1)%NNodes) );
        correctionFactorDxi[1] = 2 * ( phi(i,0) + phi((i+1)%NNodes,0) ) * ( dphi(1,i) + dphi(1,(i+1)%NNodes) );

    }

	void TPZGeoTriangle::Shape(TPZVec<REAL> &param,TPZFMatrix<REAL> &phi,TPZFMatrix<REAL> &dphi) {
		REAL qsi = param[0], eta = param[1];
		phi(0,0) = 1.-qsi-eta;
		phi(1,0) = qsi;
		phi(2,0) = eta;
		dphi(0,0) = dphi(1,0) = -1.;
		dphi(0,1) = dphi(1,2) =  1.;
		dphi(1,1) = dphi(0,2) =  0.;
	}
	
	void TPZGeoTriangle::Jacobian(const TPZFMatrix<REAL> & coord, TPZVec<REAL> &param,TPZFMatrix<REAL> &jacobian,TPZFMatrix<REAL> &axes,REAL &detjac,TPZFMatrix<REAL> &jacinv){
		
        int spacedim = coord.Rows();
        jacobian.Resize(2,2); axes.Resize(2,3); jacinv.Resize(2,2);
		TPZFNMatrix<3> phi(3,1);
        TPZFNMatrix<6> dphi(2,3),axest(3,2);
		jacobian.Zero();
		Shape(param,phi,dphi);
        TPZFNMatrix<6> VecMatrix(3,2,0.);
        for(int i = 0; i < 3; i++) {
			for(int j = 0; j < spacedim; j++) {
				VecMatrix(j,0) += coord.GetVal(j,i)*dphi(0,i);
				VecMatrix(j,1) += coord.GetVal(j,i)*dphi(1,i);
			}
        }
        VecMatrix.GramSchmidt(axest,jacobian);
        axest.Transpose(&axes);
		detjac = jacobian(0,0)*jacobian(1,1)-jacobian(1,0)*jacobian(0,1);
    REAL maxjac = 0.;
    for (int i=0; i<2; i++) {
      for (int j=0; j<2; j++) {
        maxjac = Max(maxjac,fabs(jacobian(i,j)));
      }
    }
        if(IsZero(maxjac) || IsZero(detjac/(maxjac*maxjac)))
		{
#ifdef PZDEBUG
			std::stringstream sout;
			sout << "Singular Jacobian " << detjac;
			LOGPZ_ERROR(logger, sout.str())
#endif
			detjac = ZeroTolerance();
		}
        
        jacinv(0,0) =  jacobian(1,1)/detjac;
        jacinv(1,1) =  jacobian(0,0)/detjac;
        jacinv(0,1) = -jacobian(0,1)/detjac;
        jacinv(1,0) = -jacobian(1,0)/detjac;
	}

	
	void TPZGeoTriangle::VectorialProduct(TPZVec<REAL> &v1, TPZVec<REAL> &v2,TPZVec<REAL> &result){
		if(v1.NElements()!=3||v2.NElements()!=3)
		{
			cout << " o tamanho do vetores eh diferente de 3"<< endl;
		}
		REAL x1=v1[0], y1=v1[1],z1=v1[2];
		REAL x2=v2[0], y2=v2[1],z2=v2[2];
		result.Resize(v1.NElements());
		result[0]=y1*z2-z1*y2;
		result[1]=z1*x2-x1*z2;	
		result[2]=x1*y2-y1*x2;	
	}
	
	void TPZGeoTriangle::ComputeNormal(TPZVec<REAL> &p1, TPZVec<REAL> &p2,TPZVec<REAL> &p3,TPZVec<REAL> &result){
		TPZVec<REAL> v1(3);
		TPZVec<REAL> v2(3);
		TPZVec<REAL> normal(3);
		v1[0]=p1[0]-p2[0];
		v1[1]=p1[1]-p2[1];
		v1[2]=p1[2]-p2[2];
		v2[0]=p2[0]-p3[0];
		v2[1]=p2[1]-p3[1];
		v2[2]=p2[2]-p3[2];
		VectorialProduct(v1,v2,normal);
		VectorialProduct(v1,normal,result);	
	}
	
	TPZGeoEl *TPZGeoTriangle::CreateBCGeoEl(TPZGeoEl *orig,int side,int bc) {
        if(side==6) {
			TPZManVector<int64_t> nodes(3);
			int i;
			for (i=0;i<3;i++){
				nodes[i] = orig->SideNodeIndex(side,i);
			}
			int64_t index;
			TPZGeoEl *gel = orig->Mesh()->CreateGeoElement(ETriangle,nodes,bc,index);
			int iside;
			for (iside = 0; iside <6; iside++){
				TPZGeoElSide(gel,iside).SetConnectivity(TPZGeoElSide(orig,TPZShapeTriang::ContainedSideLocId(side,iside)));
			}
			TPZGeoElSide(gel,6).SetConnectivity(TPZGeoElSide(orig,side));
			return gel;
		}
		else if(side>-1 && side<3) {
			TPZManVector<int64_t> nodeindexes(1);
			nodeindexes[0] = orig->SideNodeIndex(side,0);
			int64_t index;
			TPZGeoEl *gel = orig->CreateGeoElement(EPoint,nodeindexes,bc,index);
			TPZGeoElSide(gel,0).SetConnectivity(TPZGeoElSide(orig,side));
			return gel;
		}
		else if(side > 2 && side < 6) {
			TPZManVector<int64_t> nodes(2);
			nodes[0] = orig->SideNodeIndex(side,0);
			nodes[1] = orig->SideNodeIndex(side,1);
			int64_t index;
			TPZGeoEl *gel = orig->CreateGeoElement(EOned,nodes,bc,index);
			TPZGeoElSide(gel,0).SetConnectivity(TPZGeoElSide(orig,TPZShapeTriang::ContainedSideLocId(side,0)));
			TPZGeoElSide(gel,1).SetConnectivity(TPZGeoElSide(orig,TPZShapeTriang::ContainedSideLocId(side,1)));
			TPZGeoElSide(gel,2).SetConnectivity(TPZGeoElSide(orig,side));
			return gel;
		}
		else PZError << "TPZGeoTriangle::CreateBCGeoEl has no bc.\n";
		return 0;
	}
	
	void TPZGeoTriangle::FixSingularity(int side, TPZVec<REAL>& OriginalPoint, TPZVec<REAL>& ChangedPoint)
	{
		ChangedPoint.Resize(OriginalPoint.NElements(),0.);
		ChangedPoint = OriginalPoint;
		
		switch(side)
		{
			case 3:
			{
				if(fabs(OriginalPoint[0]) <= tol && fabs(OriginalPoint[1]- 1.) <= tol)
				{
					ChangedPoint[0] = tol;
					ChangedPoint[1] = 1. - 2.*tol;
				}
				break;
			}
				
			case 4:
			{
				if(fabs(OriginalPoint[0]) <= tol && fabs(OriginalPoint[1]) <= tol)
				{
					ChangedPoint[0] = tol;
					ChangedPoint[1] = tol;
				}
				break;
			}
				
			case 5:
			{
				if(fabs(OriginalPoint[0] - 1.) <= tol && fabs(OriginalPoint[1]) <= tol)
				{
					ChangedPoint[0] = 1.-tol;
					ChangedPoint[1] = tol/2.;
				}
				break;
			}
		}
	}
	
	/** Creates a geometric element according to the type of the father element */
	TPZGeoEl *TPZGeoTriangle::CreateGeoElement(TPZGeoMesh &mesh, MElementType type,
											   TPZVec<int64_t>& nodeindexes,
											   int matid,
											   int64_t& index)
	{
		return CreateGeoElementPattern(mesh,type,nodeindexes,matid,index);
	}
    
    /// create an example element based on the topology
    /* @param gmesh mesh in which the element should be inserted
     @param matid material id of the element
     @param lowercorner (in/out) on input lower corner o the cube where the element should be created, on exit position of the next cube
     @param size (in) size of space where the element should be created
     */
    void TPZGeoTriangle::InsertExampleElement(TPZGeoMesh &gmesh, int matid, TPZVec<REAL> &lowercorner, TPZVec<REAL> &size)
    {
        TPZManVector<REAL,3> co(3),shift(3),scale(3);
        TPZManVector<int64_t,3> nodeindexes(3);
        for (int i=0; i<3; i++) {
            scale[i] = size[i]/3.;
            shift[i] = 1./2.+lowercorner[i];
        }
        
        for (int i=0; i<NCornerNodes; i++) {
            ParametricDomainNodeCoord(i, co);
            for (int j=0; j<co.size(); j++) {
                co[j] = shift[j]+scale[j]*co[j]+(rand()*0.2/RAND_MAX)-0.1;
            }
            nodeindexes[i] = gmesh.NodeVec().AllocateNewElement();
            gmesh.NodeVec()[nodeindexes[i]].Initialize(co, gmesh);
        }
        int64_t index;
        CreateGeoElement(gmesh, ETriangle, nodeindexes, matid, index);
    }
    
    int TPZGeoTriangle::ClassId() const{
        return Hash("TPZGeoTriangle") ^ TPZNodeRep<3, pztopology::TPZTriangle>::ClassId() << 1;
    }

    void TPZGeoTriangle::Read(TPZStream& buf, void* context) {
        TPZNodeRep<3, pztopology::TPZTriangle>::Read(buf, context);
    }

    void TPZGeoTriangle::Write(TPZStream& buf, int withclassid) const {
        TPZNodeRep<3, pztopology::TPZTriangle>::Write(buf, withclassid);
    }



    template void TPZGeoTriangle::CalcSideInfluence<REAL>(const int &, const TPZVec<REAL> &, REAL &, TPZVec<REAL> &);

};

#ifdef _AUTODIFF
template<class T=REAL>
class Fad;

template void pzgeom::TPZGeoTriangle::CalcSideInfluence<Fad<REAL>>(const int &, const TPZVec<Fad<REAL>> &, Fad<REAL> &,
        TPZVec<Fad<REAL>> &);
#endif
