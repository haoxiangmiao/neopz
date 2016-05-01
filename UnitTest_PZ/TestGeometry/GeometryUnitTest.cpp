/**
 * @file IntegNumUnitTest.cpp
 * @brief Define a Unit Test using Boost for Numerical integration of the NeoPZ
 *
 */


#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "pzgmesh.h"
#include "pzvec.h"
#include "pzmanvector.h"
#include "pzgeopoint.h"
#include "TPZGeoLinear.h"
#include "pzgeotriangle.h"
#include "pzquad.h"
#include "TPZGeoCube.h"
#include "pzgeotetrahedra.h"
#include "pzgeoprism.h"
#include "pzgeopyramid.h"

#include "TPZCurve.h"
#include "TPZSurface.h"

#include "TPZVTKGeoMesh.h"


#include "pzlog.h"

#ifdef LOG4CXX
static LoggerPtr logger(Logger::getLogger("pz.mesh.testgeom"));
#endif

// Using Unit Test of the Boost Library
#ifdef USING_BOOST

#ifndef WIN32
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MAIN pz geometry_tests tests

#include "boost/test/unit_test.hpp"
#include "boost/test/floating_point_comparison.hpp"
#include "boost/test/output_test_stream.hpp"

#endif

std::string dirname = PZSOURCEDIR;
using namespace pzgeom;

/** 
 * @name Generate a geometric mesh with all topology of elements
 *
 * @{ 
 */

template<class T>
void AddElement(TPZGeoMesh &mesh, TPZVec<REAL> &lowercorner, TPZVec<REAL> &size)
{
    int matid = mesh.NElements()+1;
    T::InsertExampleElement(mesh, matid, lowercorner, size);
}

void FillGeometricMesh(TPZGeoMesh &mesh)
{
    TPZManVector<REAL,3> lowercorner(3,0.),size(3,1.); // Setting the first corner as the origin and the max element size is 1.0;

    AddElement<TPZGeoPoint>(mesh,lowercorner,size);
    AddElement<TPZGeoLinear>(mesh,lowercorner,size);
    AddElement<TPZGeoTriangle>(mesh,lowercorner,size);

//    AddElement<TPZGeoQuad>(mesh,lowercorner,size);
//    AddElement<TPZGeoCube>(mesh,lowercorner,size);
//    AddElement<TPZGeoTetrahedra>(mesh,lowercorner,size);
//    AddElement<TPZGeoPrism>(mesh,lowercorner,size);
//    AddElement<TPZGeoPyramid>(mesh,lowercorner,size);
//    AddElement<TPZGeoBlend<TPZGeoLinear> >(mesh,lowercorner,size);
//    AddElement<TPZGeoBlend<TPZGeoQuad> >(mesh,lowercorner,size);
//    AddElement<TPZGeoBlend<TPZGeoTriangle> >(mesh,lowercorner,size);
//    AddElement<TPZGeoBlend<TPZGeoTetrahedra> >(mesh,lowercorner,size);
//    AddElement<TPZGeoBlend<TPZGeoCube> >(mesh,lowercorner,size);
//    AddElement<TPZGeoBlend<TPZGeoPrism> >(mesh,lowercorner,size);
//    AddElement<TPZGeoBlend<TPZGeoPyramid> >(mesh,lowercorner,size);
}

/* @} */

/** 
 * @name Testing the conformity of gradx
 * @{ 
 */


/** @} */


#ifdef USING_BOOST


BOOST_AUTO_TEST_SUITE(geometry_tests)


BOOST_AUTO_TEST_CASE(gradx_tests) {
    
    TPZGeoMesh * gmesh = new TPZGeoMesh;
    FillGeometricMesh(*gmesh);
    std::ofstream file("Geometry.vtk");
    TPZVTKGeoMesh::PrintGMeshVTK(gmesh, file);
    
    
    
//    BOOST_CHECK(fabsl(point[0]-point2[0]) < tol);
//    BOOST_CHECK(fabsl(weight-weight2) < tol);


}





BOOST_AUTO_TEST_SUITE_END()


#endif

