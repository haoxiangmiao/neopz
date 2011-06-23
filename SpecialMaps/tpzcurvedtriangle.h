#ifndef TPZCURVEDTRIANGLE_H
#define TPZCURVEDTRIANGLE_H

// #include "pzfmatrix.h"
// #include "pzvec.h"
#include "pzgeotriangle.h"
// #include "pzgmesh.h"
#include "pzgeoel.h"
#include "pznoderep.h"
// #include "tpztriangle.h"

#include <iostream>

  /**
  / Class made by Paulo Cesar de Alvarenga Lucci (Caju)
  / LabMeC - FEC - UNICAMP
  / 2007
 */
namespace pzgeom
{

/// implements a triange with straight sides but with nonlinear map
class TPZCurvedTriangle : public pzgeom::TPZNodeRep<3,pztopology::TPZTriangle> {

public:

    enum {NNodes = 3};

     bool IsLinearMapping() const { 
          return false; 
     }

   ~TPZCurvedTriangle();

    TPZCurvedTriangle(TPZVec<int> &nodeindexes) : pzgeom::TPZNodeRep<NNodes,pztopology::TPZTriangle>(nodeindexes)
    {
    }

    TPZCurvedTriangle() : pzgeom::TPZNodeRep<NNodes,pztopology::TPZTriangle>()
    {
    }

    TPZCurvedTriangle(const TPZCurvedTriangle &cp,std::map<int,int> & gl2lcNdMap) : pzgeom::TPZNodeRep<NNodes,pztopology::TPZTriangle>(cp,gl2lcNdMap)
    {
    }

    TPZCurvedTriangle(const TPZCurvedTriangle &cp) : pzgeom::TPZNodeRep<NNodes,pztopology::TPZTriangle>(cp)
    {
    }

    TPZCurvedTriangle(const TPZCurvedTriangle &cp, TPZGeoMesh &) : pzgeom::TPZNodeRep<NNodes,pztopology::TPZTriangle>(cp)
    {
    }

    /**
    * returns the type name of the element
    */
    static std::string TypeName() { return "Triangle";}

    /**
    * Method which creates a geometric boundary condition 
    * element based on the current geometric element, 
    * a side and a boundary condition number
    */
    static  TPZGeoEl * CreateBCGeoEl(TPZGeoEl *orig,int side,int bc);

    /**
    /Aqui o X engloba a transformacao curva (Caju) e a transformacao linear (PZ 3node element)
   */
    static void X(TPZFMatrix &coord, TPZVec<REAL>& par, TPZVec< REAL > &result);

    static void Jacobian(TPZFMatrix & coord, TPZVec<REAL>& par, TPZFMatrix &jacobian, TPZFMatrix &axes, REAL &detjac, TPZFMatrix &jacinv);
	
public:
	/**
	 * Creates a geometric element according to the type of the father element
	 */
	static TPZGeoEl *CreateGeoElement(TPZGeoMesh &mesh, MElementType type,
									  TPZVec<int>& nodeindexes,
									  int matid,
									  int& index);
	
};

};
#endif
