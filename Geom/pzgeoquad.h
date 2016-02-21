/**
 * @file TPZGeoQuad
 * @brief Implements the geometry of a quadrilateral element.
 */

#ifndef TPZGEOQUADH
#define TPZGEOQUADH

#include "pznoderep.h"
#include "pzvec.h"
#include "pzeltype.h"
#include "tpzquadrilateral.h"
#include "pzfmatrix.h"

#include <string>

class TPZGeoEl;
class TPZGeoMesh;

namespace pzgeom {
	
	/**
	 * @ingroup geometry
	 * @brief Implements the geometry of a quadrilateral element. \ref geometry "Geometry"
	 */
	class TPZGeoQuad  : public TPZNodeRep<4, pztopology::TPZQuadrilateral>
	{
	public:
		/** @brief Number of corner nodes */
		enum {NNodes = 4};

		/** @brief Constructor with list of nodes */
		TPZGeoQuad(TPZVec<long> &nodeindexes) : TPZNodeRep<NNodes,pztopology::TPZQuadrilateral>(nodeindexes)
		{
		}
		
		/** @brief Empty constructor */
		TPZGeoQuad() : TPZNodeRep<NNodes,pztopology::TPZQuadrilateral>()
		{
		}
		
		/** @brief Constructor with node map */
		TPZGeoQuad(const TPZGeoQuad &cp,
				   std::map<long,long> & gl2lcNdMap) : TPZNodeRep<NNodes,pztopology::TPZQuadrilateral>(cp,gl2lcNdMap)
		{
		}
		
		/** @brief Copy constructor */
		TPZGeoQuad(const TPZGeoQuad &cp) : TPZNodeRep<NNodes,pztopology::TPZQuadrilateral>(cp)
		{
		}
		
		/** @brief Copy constructor */
		TPZGeoQuad(const TPZGeoQuad &cp, TPZGeoMesh &) : TPZNodeRep<NNodes,pztopology::TPZQuadrilateral>(cp)
		{
		}
		
        static bool IsLinearMapping(int side)
        {
            return true;
        }
        
		/** @brief Returns the type name of the element */
		static std::string TypeName() { return "Quad";}
        
        /** @brief Compute the shape being used to construct the x mapping from local parametric coordinates  */
        static void Shape(TPZVec<REAL> &loc,TPZFMatrix<REAL> &phi,TPZFMatrix<REAL> &dphi){
            TShape(loc, phi, dphi);
        }
        
        /* @brief Compute x mapping from local parametric coordinates */
        void X(const TPZGeoEl &gel,TPZVec<REAL> &loc,TPZVec<REAL> &x) const
        {
            TPZFNMatrix<3*NNodes> coord(3,NNodes);
            CornerCoordinates(gel, coord);
            X(coord,loc,x);
        }
        
        /** @brief Compute gradient of x mapping from local parametric coordinates */
        template<class T>
        void GradX(const TPZGeoEl &gel, TPZVec<T> &loc, TPZFMatrix<T> &gradx) const
        {
            TPZFNMatrix<3*NNodes> coord(3,NNodes);
            CornerCoordinates(gel, coord);
            int nrow = coord.Rows();
            int ncol = coord.Cols();
            TPZFMatrix<T> nodes(nrow,ncol);
            for(int i = 0; i < nrow; i++)
            {
                for(int j = 0; j < ncol; j++)
                {
                    nodes(i,j) = coord(i,j);
                }
            }
            
            GradX(nodes,loc,gradx);
        }
        
        /* @brief Computes the jacobian of the map between the master element and deformed element */
        void Jacobian(const TPZGeoEl &gel,TPZVec<REAL> &param,TPZFMatrix<REAL> &jacobian,TPZFMatrix<REAL> &axes,REAL &detjac,TPZFMatrix<REAL> &jacinv) const
        {
            TPZFNMatrix<3*NNodes> coord(3,NNodes);
            CornerCoordinates(gel, coord);
            Jacobian(coord, param, jacobian, axes, detjac, jacinv);
        }
        
        /** @brief Compute x mapping from element nodes and local parametric coordinates */
        static void X(const TPZFMatrix<REAL> &nodes,TPZVec<REAL> &loc,TPZVec<REAL> &x);
        
        /** @brief Compute gradient of x mapping from element nodes and local parametric coordinates */
        template<class T>
        static void GradX(const TPZFMatrix<T> &nodes,TPZVec<T> &loc, TPZFMatrix<T> &gradx);
        
        /** @brief Compute the shape being used to construct the x mapping from local parametric coordinates  */
        template<class T>
        static void TShape(TPZVec<T> &loc,TPZFMatrix<T> &phi,TPZFMatrix<T> &dphi);
        
        /** @brief Compute the jacoabina associated to the x mapping from local parametric coordinates  */
        static void Jacobian(const TPZFMatrix<REAL> &nodes,TPZVec<REAL> &param,TPZFMatrix<REAL> &jacobian,
                             TPZFMatrix<REAL> &axes,REAL &detjac,TPZFMatrix<REAL> &jacinv);
		
        /**
         * @brief Method which creates a geometric boundary condition
         * element based on the current geometric element, \n
         * a side and a boundary condition number
         */
        static  TPZGeoEl * CreateBCGeoEl(TPZGeoEl *orig,int side,int bc);
        
        
		/** @brief Implementation of normal vector to Hdiv space*/
		/** 
		 construct the normal vector for element Hdiv
		 */
		static void VecHdiv(TPZFMatrix<REAL> & coord,TPZFMatrix<REAL> &NormalVec,TPZVec<int> & VectorSide);
		/** @brief Computes the vecorial product of the two vectors*/ 
		static void VectorialProduct(TPZVec<REAL> &v1, TPZVec<REAL> &v2,TPZVec<REAL> &result);
		/** @brief Computes normal vector to plane determinated by three points */
		static void ComputeNormal(TPZVec<REAL> &p1, TPZVec<REAL> &p2,TPZVec<REAL> &p3,TPZVec<REAL> &result);
        
		/* brief compute the coordinate of a point given in parameter space */
        void VecHdiv(const TPZGeoEl &gel,TPZFMatrix<REAL> &NormalVec,TPZVec<int> & VectorSide) const
        {
            TPZFNMatrix<3*NNodes> coord(3,NNodes);
            CornerCoordinates(gel, coord);
            VecHdiv(coord,NormalVec,VectorSide);
        }

		
	public:
		/** @brief Creates a geometric element according to the type of the father element */
		static TPZGeoEl *CreateGeoElement(TPZGeoMesh &mesh, MElementType type,
										  TPZVec<long>& nodeindexes,
										  int matid,
										  long& index);
	};
    
    template<class T>
    inline void TPZGeoQuad::TShape(TPZVec<T> &loc,TPZFMatrix<T> &phi,TPZFMatrix<T> &dphi) {
        T qsi = loc[0], eta = loc[1];
        
        phi(0,0) = .25*(1.-qsi)*(1.-eta);
        phi(1,0) = .25*(1.+qsi)*(1.-eta);
        phi(2,0) = .25*(1.+qsi)*(1.+eta);
        phi(3,0) = .25*(1.-qsi)*(1.+eta);
        
        dphi(0,0) = .25*(eta-1.);
        dphi(1,0) = .25*(qsi-1.);
        
        dphi(0,1) = .25*(1.-eta);
        dphi(1,1) =-.25*(1.+qsi);
        
        dphi(0,2) = .25*(1.+eta);
        dphi(1,2) = .25*(1.+qsi);
        
        dphi(0,3) =-.25*(1.+eta);
        dphi(1,3) = .25*(1.-qsi);
        
        
    }
    
    inline void TPZGeoQuad::X(const TPZFMatrix<REAL> &nodes,TPZVec<REAL> &loc,TPZVec<REAL> &x){
        
        REAL spacephi[4],spacedphi[8];
        TPZFMatrix<REAL> phi(4,1,spacephi,4);
        TPZFMatrix<REAL> dphi(2,4,spacedphi,8);
        Shape(loc,phi,dphi);
        int space = nodes.Rows();
        
        for(int i = 0; i < space; i++) {
            x[i] = 0.0;
            for(int j = 0; j < 4; j++) {
                x[i] += phi(j,0)*nodes.GetVal(i,j);
            }
        }
        
    }
    
    template<class T>
    inline void TPZGeoQuad::GradX(const TPZFMatrix<T> &nodes,TPZVec<T> &loc, TPZFMatrix<T> &gradx){
        
        gradx.Resize(3,2);
        gradx.Zero();
        int nrow = nodes.Rows();
        int ncol = nodes.Cols();
#ifdef PZDEBUG
        if(nrow != 3 && ncol  != 4){
            std::cout << "Objects of incompatible lengths, gradient cannot be computed." << std::endl;
            std::cout << "nodes matrix must be 3x4." << std::endl;
            DebugStop();
        }
        
#endif
        TPZFNMatrix<3,T> phi(4,1);
        TPZFNMatrix<6,T> dphi(2,4);
        TShape(loc,phi,dphi);
        for(int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 3; j++)
            {
                gradx(j,0) += nodes.GetVal(j,i)*dphi(0,i);
                gradx(j,1) += nodes.GetVal(j,i)*dphi(1,i);
            }
        }
        
    }
    
	
};

#endif 
