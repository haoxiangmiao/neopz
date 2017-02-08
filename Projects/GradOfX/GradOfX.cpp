#include "pzlog.h"
#include "tpzautopointer.h"
#include "TPZRefPatternTools.h"


#include "pzgeopoint.h"
#include "TPZGeoLinear.h"
#include "tpztriangle.h"
#include "pzgeoquad.h"

#include "TPZGeoCube.h"
#include "pzgeotetrahedra.h"
#include "pzgeoprism.h"
#include "pzgeopyramid.h"

#include "TPZRefPattern.h"
#include "tpzgeoelrefpattern.h"

#include "pzgmesh.h"
#include "TPZVTKGeoMesh.h"



TPZGeoMesh * OneDimensional();
TPZGeoMesh * TwoDimensionalT();
TPZGeoMesh * TwoDimensionalQ();
TPZGeoMesh * ThreeDimensionalT();
TPZGeoMesh * ThreeDimensionalH();
TPZGeoMesh * ThreeDimensionalPr();
TPZGeoMesh * ThreeDimensionalPy();

void ComputeGradofX(TPZGeoMesh * mesh, std::string file_name, TPZFMatrix<REAL> &triplets);


int main()
{
    
#ifdef LOG4CXX
    InitializePZLOG();
#endif
    std::string file_name;
    TPZFMatrix<REAL> triplets;
    
    
    {
        TPZGeoMesh * mesh = OneDimensional();
        file_name = "x_and_grad_1D.txt";
        triplets.Resize(3,3);
        triplets.Zero();
        triplets(0,0) = -1.0;
        triplets(1,0) = +0.0;
        triplets(2,0) = +1.0;
        ComputeGradofX(mesh,file_name,triplets);
    }
    {
        TPZGeoMesh * mesh = TwoDimensionalT();
        file_name = "x_and_grad_2D_T.txt";
        triplets.Resize(4,3);
        triplets.Zero();
        triplets(0,0) = -0.0;
        triplets(0,1) = +0.0;
        triplets(1,0) = +1.0;
        triplets(1,1) = +0.0;
        triplets(2,0) = -0.0;
        triplets(2,1) = +1.0;
        triplets(3,0) = +0.25;
        triplets(3,1) = +0.25;
        ComputeGradofX(mesh,file_name,triplets);
    }
    
    {
        TPZGeoMesh * mesh = TwoDimensionalQ();
        file_name = "x_and_grad_2D_Q.txt";
        triplets.Resize(4,3);
        triplets.Zero();
        triplets(0,0) = +0.25;
        triplets(0,1) = +0.25;
        triplets(1,0) = -0.25;
        triplets(1,1) = +0.25;
        triplets(2,0) = -0.25;
        triplets(2,1) = -0.25;
        triplets(3,0) = +0.25;
        triplets(3,1) = -0.25;
        ComputeGradofX(mesh,file_name,triplets);
    }
    
    {
        TPZGeoMesh * mesh = ThreeDimensionalT();
        file_name = "x_and_grad_3D_T.txt";
        triplets.Resize(4,3);
        triplets.Zero();
        triplets(0,0) = +0.25;
        triplets(0,1) = +0.25;
        triplets(0,2) = +0.00;
        
        triplets(1,0) = +0.15;
        triplets(1,1) = +0.15;
        triplets(1,2) = +0.25;
        
        triplets(2,0) = +0.05;
        triplets(2,1) = +0.05;
        triplets(2,2) = +0.25;
        
        triplets(3,0) = +0.00;
        triplets(3,1) = +0.00;
        triplets(3,2) = +1.00;
        
        ComputeGradofX(mesh,file_name,triplets);
    }
    
    {
        TPZGeoMesh * mesh = ThreeDimensionalH();
        file_name = "x_and_grad_3D_H.txt";
        triplets.Resize(4,3);
        triplets.Zero();
        triplets(0,0) = +0.25;
        triplets(0,1) = +0.25;
        triplets(0,2) = +0.00;
        
        triplets(1,0) = -0.15;
        triplets(1,1) = -0.15;
        triplets(1,2) = +0.25;
        
        triplets(2,0) = +0.05;
        triplets(2,1) = +0.05;
        triplets(2,2) = -0.25;
        
        triplets(3,0) = +1.00;
        triplets(3,1) = +1.00;
        triplets(3,2) = +1.00;
        
        ComputeGradofX(mesh,file_name,triplets);
    }
    
    {
        TPZGeoMesh * mesh = ThreeDimensionalPr();
        file_name = "x_and_grad_3D_Pr.txt";
        triplets.Resize(4,3);
        triplets.Zero();
        triplets(0,0) = +0.25;
        triplets(0,1) = +0.25;
        triplets(0,2) = +0.00;
        
        triplets(1,0) = +0.15;
        triplets(1,1) = +0.15;
        triplets(1,2) = +0.25;
        
        triplets(2,0) = +0.05;
        triplets(2,1) = +0.05;
        triplets(2,2) = +0.25;
        
        triplets(3,0) = +0.00;
        triplets(3,1) = +0.00;
        triplets(3,2) = +1.00;
        
        ComputeGradofX(mesh,file_name,triplets);
    }
    
    {
        TPZGeoMesh * mesh = ThreeDimensionalPy();
        file_name = "x_and_grad_3D_Py.txt";
        triplets.Resize(4,3);
        triplets.Zero();
        triplets(0,0) = -0.25;
        triplets(0,1) = -0.25;
        triplets(0,2) = +0.00;
        
        triplets(1,0) = +0.15;
        triplets(1,1) = +0.15;
        triplets(1,2) = +0.25;
        
        triplets(2,0) = -0.05;
        triplets(2,1) = -0.05;
        triplets(2,2) = +0.25;
        
        triplets(3,0) = +0.00;
        triplets(3,1) = +0.00;
        triplets(3,2) = +0.99999;
        
        ComputeGradofX(mesh,file_name,triplets);
    }

    return 0;
}

void ComputeGradofX(TPZGeoMesh * mesh, std::string file_name, TPZFMatrix<REAL> &triplets){
    
    std::ofstream file(file_name.c_str());
    
    TPZManVector<REAL,3> triplet_xi_eta_zeta(3,0);
    TPZManVector<REAL,3> x(3,0);
    TPZFMatrix<REAL> gradx(3,3,0.0);
    TPZFMatrix<REAL> coordinates;
    TPZFMatrix<REAL> jac;
    TPZFMatrix<REAL> axes;
    REAL detjac;
    TPZFMatrix<REAL> jacinv;
    
    int nplot_points = triplets.Rows();

    
    int nel = mesh->NElements();
    for (int iel = 0; iel < nel; iel++) {
        TPZGeoEl * gel  = mesh->Element(iel);
        if (!gel) {
            std::cout << "not geometric element found." << std::endl;
            DebugStop();
        }
        
        gel->NodesCoordinates(coordinates);
        
        file << " ------------------------------------------------------ " << std::endl;
        file << " Geometric Element identifier = " << gel->Id() << std::endl;
        
        for (int ip = 0; ip <  nplot_points; ip++) {
            triplet_xi_eta_zeta[0] = triplets(ip,0);
            triplet_xi_eta_zeta[1] = triplets(ip,1);
            triplet_xi_eta_zeta[2] = triplets(ip,2);
            file << " triplet_xi_eta_zeta coordinate = " << triplet_xi_eta_zeta[0] << " ,  " << triplet_xi_eta_zeta[1] << " ,  " << triplet_xi_eta_zeta[2] << std::endl;
            gel->X(triplet_xi_eta_zeta, x);
            file << " Mapped x coordinate = " << x[0] << " ,  " << x[1] << " ,  " << x[2] << std::endl;
            gel->GradX(triplet_xi_eta_zeta, gradx);
            file << " Grad of x = " << gradx << std::endl;
            gel->Jacobian(triplet_xi_eta_zeta, jac, axes, detjac, jacinv);
            file << " axes = " << axes << std::endl;
            file << " jacobian = " << jac << std::endl;
            file << " detjac = " << detjac << std::endl;
            file << " *********************************** " << std::endl;
            gel->JacobianXYZ(triplet_xi_eta_zeta, jac, axes, detjac, jacinv);
            file << " axes = " << axes << std::endl;
            file << " jacobian = " << jac << std::endl;
            file << " detjac = " << detjac << std::endl;
            file  << std::endl;
            
        }
        
    }
    
    file.flush();
    
    return;
    
}


TPZGeoMesh * OneDimensional(){
    
    int mat_id = 1;
    long nodes  = 2;
    
    TPZGeoMesh * geo_mesh= new TPZGeoMesh;
    
    geo_mesh->SetMaxNodeId(nodes-1);
    geo_mesh->NodeVec().Resize(nodes);
    TPZVec<TPZGeoNode> Node(nodes);
    
    TPZVec <long> TopolLine(2);
    REAL x, y, z;
    
    // Nodes
    long id = 0;
    
    Node[id].SetNodeId(id);
    x = 1.0;
    y = 1.0;
    z = -1.0;
    Node[id].SetCoord(0 , x);     //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 1.0;
    y = 1.0;
    z = 1.0;
    Node[id].SetCoord(0 , x);     //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    //  Geometric Elements
    int elid = 0;
    
    TopolLine[0] = 0;
    TopolLine[1] = 1;
    new TPZGeoElRefPattern< pzgeom::TPZGeoLinear > (elid,TopolLine,mat_id,*geo_mesh);
    elid++;
    
    geo_mesh->BuildConnectivity();
    
    std::string out_name_text = "GeometicMesh1D.txt";
    std::string out_name_vtk = "GeometricMesh1D.vtk";
    std::ofstream argument(out_name_text.c_str());
    std::ofstream Dummyfile(out_name_vtk.c_str());
    geo_mesh->Print(argument);
    TPZVTKGeoMesh::PrintGMeshVTK(geo_mesh,Dummyfile, true);
    
    return geo_mesh;
    
}

TPZGeoMesh * TwoDimensionalT(){
    
    int mat_id = 1;
    long nodes  = 3;
    
    TPZGeoMesh * geo_mesh= new TPZGeoMesh;
    
    geo_mesh->SetMaxNodeId(nodes-1);
    geo_mesh->NodeVec().Resize(nodes);
    TPZVec<TPZGeoNode> Node(nodes);
    
    TPZVec <long> TopolTriangle(3);
    REAL x, y, z;
    
    // Nodes
    long id = 0;
    
    Node[id].SetNodeId(id);
    x = 0.0;
    y = 0.0;
    z = 0.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 0.0;
    y = 1.0;
    z = 0.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 0.0;
    y = 0.0;
    z = 1.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    //  Geometric Elements
    int elid = 0;
    
    TopolTriangle[0] = 0;
    TopolTriangle[1] = 1;
    TopolTriangle[2] = 2;
    new TPZGeoElRefPattern< pzgeom::TPZGeoTriangle > (elid,TopolTriangle,mat_id,*geo_mesh);
    elid++;
    
    geo_mesh->BuildConnectivity();
    
    std::string out_name_text = "GeometicMesh2DT.txt";
    std::string out_name_vtk = "GeometricMesh2DT.vtk";
    std::ofstream argument(out_name_text.c_str());
    std::ofstream Dummyfile(out_name_vtk.c_str());
    geo_mesh->Print(argument);
    TPZVTKGeoMesh::PrintGMeshVTK(geo_mesh,Dummyfile, true);
    
    return geo_mesh;
    
}

TPZGeoMesh * TwoDimensionalQ(){
    
    int mat_id = 1;
    long nodes  = 4;
    
    TPZGeoMesh * geo_mesh= new TPZGeoMesh;
    
    geo_mesh->SetMaxNodeId(nodes-1);
    geo_mesh->NodeVec().Resize(nodes);
    TPZVec<TPZGeoNode> Node(nodes);
    
    TPZVec <long> TopolQuadrilateral(4);
    REAL x, y, z;
    
    // Nodes
    long id = 0;
    
    Node[id].SetNodeId(id);
    x = 0.0;
    y = 0.0;
    z = 0.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 1.0;
    y = 0.0;
    z = 0.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 1.0;
    y = 0.0;
    z = 1.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 0.0;
    y = 0.0;
    z = 1.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    //  Geometric Elements
    int elid = 0;
    
    TopolQuadrilateral[0] = 0;
    TopolQuadrilateral[1] = 1;
    TopolQuadrilateral[2] = 2;
    TopolQuadrilateral[3] = 3;
    new TPZGeoElRefPattern< pzgeom::TPZGeoQuad > (elid,TopolQuadrilateral,mat_id,*geo_mesh);
    elid++;
    
    geo_mesh->BuildConnectivity();
    
    std::string out_name_text = "GeometicMesh2DQ.txt";
    std::string out_name_vtk = "GeometricMesh2DQ.vtk";
    std::ofstream argument(out_name_text.c_str());
    std::ofstream Dummyfile(out_name_vtk.c_str());
    geo_mesh->Print(argument);
    TPZVTKGeoMesh::PrintGMeshVTK(geo_mesh,Dummyfile, true);
    
    return geo_mesh;
    
}

TPZGeoMesh * ThreeDimensionalT(){
    
    int mat_id = 1;
    long nodes  = 4;
    
    TPZGeoMesh * geo_mesh= new TPZGeoMesh;
    
    geo_mesh->SetMaxNodeId(nodes-1);
    geo_mesh->NodeVec().Resize(nodes);
    TPZVec<TPZGeoNode> Node(nodes);
    
    TPZVec <long> TopolTetrahedron(4);
    REAL x, y, z;
    
    // Nodes
    long id = 0;
    
    Node[id].SetNodeId(id);
    x = 0.0;
    y = 0.0;
    z = 0.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 0.5;
    y = 0.0;
    z = 0.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 0.0;
    y = 0.5;
    z = 0.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 0.0;
    y = 0.0;
    z = 0.5;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    //  Geometric Elements
    int elid = 0;
    
    TopolTetrahedron[0] = 0;
    TopolTetrahedron[1] = 1;
    TopolTetrahedron[2] = 2;
    TopolTetrahedron[3] = 3;
    new TPZGeoElRefPattern< pzgeom::TPZGeoTetrahedra > (elid,TopolTetrahedron,mat_id,*geo_mesh);
    elid++;
    
    geo_mesh->BuildConnectivity();
    
    std::string out_name_text = "GeometicMesh3DT.txt";
    std::string out_name_vtk = "GeometricMesh3DT.vtk";
    std::ofstream argument(out_name_text.c_str());
    std::ofstream Dummyfile(out_name_vtk.c_str());
    geo_mesh->Print(argument);
    TPZVTKGeoMesh::PrintGMeshVTK(geo_mesh,Dummyfile, true);
    
    return geo_mesh;
    
}

TPZGeoMesh * ThreeDimensionalH(){
    
    int mat_id = 1;
    long nodes  = 8;
    
    TPZGeoMesh * geo_mesh= new TPZGeoMesh;
    
    geo_mesh->SetMaxNodeId(nodes-1);
    geo_mesh->NodeVec().Resize(nodes);
    TPZVec<TPZGeoNode> Node(nodes);
    
    TPZVec <long> TopolHexahedron(8);
    REAL x, y, z;
    
    // Nodes
    long id = 0;
    
    Node[id].SetNodeId(id);
    x = 0.0;
    y = 0.0;
    z = 0.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 0.5;
    y = 0.0;
    z = 0.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 0.5;
    y = 0.5;
    z = 0.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 0.0;
    y = 0.5;
    z = 0.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 0.0;
    y = 0.0;
    z = 0.5;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 0.5;
    y = 0.0;
    z = 0.5;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 0.5;
    y = 0.5;
    z = 0.5;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 0.0;
    y = 0.5;
    z = 0.5;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    //  Geometric Elements
    int elid = 0;
    
    TopolHexahedron[0] = 0;
    TopolHexahedron[1] = 1;
    TopolHexahedron[2] = 2;
    TopolHexahedron[3] = 3;
    TopolHexahedron[4] = 4;
    TopolHexahedron[5] = 5;
    TopolHexahedron[6] = 6;
    TopolHexahedron[7] = 7;
    new TPZGeoElRefPattern< pzgeom::TPZGeoCube > (elid,TopolHexahedron,mat_id,*geo_mesh);
    elid++;
    
    geo_mesh->BuildConnectivity();
    
    std::string out_name_text = "GeometicMesh3DH.txt";
    std::string out_name_vtk = "GeometricMesh3DH.vtk";
    std::ofstream argument(out_name_text.c_str());
    std::ofstream Dummyfile(out_name_vtk.c_str());
    geo_mesh->Print(argument);
    TPZVTKGeoMesh::PrintGMeshVTK(geo_mesh,Dummyfile, true);
    
    return geo_mesh;
    
}

TPZGeoMesh * ThreeDimensionalPr(){
    
    int mat_id = 1;
    long nodes  = 6;
    
    TPZGeoMesh * geo_mesh= new TPZGeoMesh;
    
    geo_mesh->SetMaxNodeId(nodes-1);
    geo_mesh->NodeVec().Resize(nodes);
    TPZVec<TPZGeoNode> Node(nodes);
    
    TPZVec <long> TopolPrism(6);
    REAL x, y, z;
    
    // Nodes
    long id = 0;
    
    Node[id].SetNodeId(id);
    x = 0.0;
    y = 0.0;
    z = 0.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 0.5;
    y = 0.0;
    z = 0.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 0.0;
    y = 0.5;
    z = 0.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 0.0;
    y = 0.0;
    z = 0.5;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 0.5;
    y = 0.0;
    z = 0.5;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 0.0;
    y = 0.5;
    z = 0.5;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    //  Geometric Elements
    int elid = 0;
    
    TopolPrism[0] = 0;
    TopolPrism[1] = 1;
    TopolPrism[2] = 2;
    TopolPrism[3] = 3;
    TopolPrism[4] = 4;
    TopolPrism[5] = 5;
    new TPZGeoElRefPattern< pzgeom::TPZGeoPrism > (elid,TopolPrism,mat_id,*geo_mesh);
    elid++;
    
    geo_mesh->BuildConnectivity();
    
    std::string out_name_text = "GeometicMesh3DPr.txt";
    std::string out_name_vtk = "GeometricMesh3DPr.vtk";
    std::ofstream argument(out_name_text.c_str());
    std::ofstream Dummyfile(out_name_vtk.c_str());
    geo_mesh->Print(argument);
    TPZVTKGeoMesh::PrintGMeshVTK(geo_mesh,Dummyfile, true);
    
    return geo_mesh;
    
}

TPZGeoMesh * ThreeDimensionalPy(){
    
    int mat_id = 1;
    long nodes  = 5;
    
    TPZGeoMesh * geo_mesh= new TPZGeoMesh;
    
    geo_mesh->SetMaxNodeId(nodes-1);
    geo_mesh->NodeVec().Resize(nodes);
    TPZVec<TPZGeoNode> Node(nodes);
    
    TPZVec <long> TopolPyramid(5);
    REAL x, y, z;
    
    // Nodes
    long id = 0;
    
    Node[id].SetNodeId(id);
    x = 0.0;
    y = 0.0;
    z = 0.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 1.0;
    y = 0.0;
    z = 0.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 1.0;
    y = 1.0;
    z = 0.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 0.0;
    y = 1.0;
    z = 0.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    Node[id].SetNodeId(id);
    x = 0.25;
    y = 0.25;
    z = 1.0;
    Node[id].SetCoord(0 , x);         //coord x
    Node[id].SetCoord(1 , y);     //coord y
    Node[id].SetCoord(2 , z);     //coord z
    geo_mesh->NodeVec()[id] = Node[id];
    id++;
    
    //  Geometric Elements
    int elid = 0;
    
    TopolPyramid[0] = 0;
    TopolPyramid[1] = 1;
    TopolPyramid[2] = 2;
    TopolPyramid[3] = 3;
    TopolPyramid[4] = 4;
    new TPZGeoElRefPattern< pzgeom::TPZGeoPyramid> (elid,TopolPyramid,mat_id,*geo_mesh);
    elid++;
    
    geo_mesh->BuildConnectivity();
    
    std::string out_name_text = "GeometicMesh3DPy.txt";
    std::string out_name_vtk = "GeometricMesh3DPy.vtk";
    std::ofstream argument(out_name_text.c_str());
    std::ofstream Dummyfile(out_name_vtk.c_str());
    geo_mesh->Print(argument);
    TPZVTKGeoMesh::PrintGMeshVTK(geo_mesh,Dummyfile, true);
    
    return geo_mesh;
    
}
