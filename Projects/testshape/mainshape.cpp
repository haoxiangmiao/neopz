#include "pzreal.h"

#include "pzgmesh.h"
#include "pzcmesh.h"
#include "pzgengrid.h"
#include "pzintel.h"

#include "pzl2projection.h"

#include "pzshapelinear.h"
#include "pzshapequad.h"
#include "pzshapetriang.h"
#include "pzshapetetra.h"
#include "pzshapepiram.h"
#include "pzshapeprism.h"
#include "pzshapecube.h"

// integration rule
#include <pzquad.h>

#include <iostream>
#include <math.h>

using namespace pzshape;

ofstream saida("malha.txt");

TPZCompMesh *InitialMesh(int dim,int order);
void TestShape(TPZInterpolatedElement *el,int order);

int main() {
	int i;
	int dim = 2, int_order;
	TPZInterpolatedElement *el;

	// Insere malha geometrica e computacional
	cout << "Ordem de integração = ";
	cin >> int_order;
	TPZCompMesh *cmesh = InitialMesh(dim,int_order);

	// Calcula os valores das funcoes shape no lado e no elemento
	for(i=0;i<cmesh->NElements();i++) {
		el = (TPZInterpolatedElement *)cmesh->ElementVec()[i];
		TestShape(el,int_order);
	}
	saida.close();
	return 0;

	// Funcoes shape
	TPZFMatrix phi(100,1,0.),dphi(1,100,0.);

	TPZVec<int> order(27,3);
	TPZVec<int> nodeids(27,0);
	for(i=0; i<27; i++) nodeids[i] = i;

	TPZVec<REAL> point(3,0.2);

	int nc,nshape;

	{
		TPZShapeLinear sel;

		nc = sel.NConnects ();
		nshape = sel.NShapeF (order);

		phi.Resize(nshape,1);
		dphi.Resize(1,nshape);
		
		sel.Shape (point,nodeids,order,phi,dphi);

		phi.Print("Funcoes de forma uni dimensionais");
		dphi.Print("Derivadas de funcoes de forma uni dimensionais");
	}
/*	{
		TPZShapeQuad sel;

		nc = sel.NConnects ();
		nshape = sel.NShapeF (order);

		phi.Resize(nshape,1);
		dphi.Resize(2,nshape);
		
		sel.Shape (point,nodeids, order,phi,dphi);

		phi.Print("Funcoes de forma quadrilaterais");
		dphi.Print("Derivadas de funcoes de forma quadri laterais");
	}
	{
		TPZShapeTriang sel;

		nc = sel.NConnects ();
		nshape = sel.NShapeF (order);

		phi.Resize(nshape,1);
		dphi.Resize(2,nshape);
		
		sel.Shape (point,nodeids, order,phi,dphi);

		phi.Print("Funcoes de forma triangular");
		dphi.Print("Derivadas de funcoes de forma triangular");
	}
	{
		TPZShapeCube sel;

		nc = sel.NConnects ();
		nshape = sel.NShapeF (order);

		phi.Resize(nshape,1);
		dphi.Resize(3,nshape);
		
		sel.Shape (point,nodeids, order,phi,dphi);

		phi.Print("Funcoes de forma cubo");
		dphi.Print("Derivadas de funcoes de forma cubo");
	}
	{
		TPZShapePiram sel;

		nc = sel.NConnects ();
		nshape = sel.NShapeF (order);

		phi.Resize(nshape,1);
		dphi.Resize(3,nshape);
		
		sel.Shape (point,nodeids, order,phi,dphi);

		phi.Print("Funcoes de forma piramide");
		dphi.Print("Derivadas de funcoes de forma piramide");
	}
	{
		TPZShapePrism sel;

		nc = sel.NConnects ();
		nshape = sel.NShapeF (order);

		phi.Resize(nshape,1);
		dphi.Resize(3,nshape);
		
		sel.Shape (point,nodeids, order,phi,dphi);

		phi.Print("Funcoes de forma prisme");
		dphi.Print("Derivadas de funcoes de forma prisme");
	}
	{
		TPZShapeTetra sel;

		nc = sel.NConnects ();
		nshape = sel.NShapeF (order);

		phi.Resize(nshape,1);
		dphi.Resize(3,nshape);
		
		sel.Shape (point,nodeids, order,phi,dphi);

		phi.Print("Funcoes de forma tetrahedra");
		dphi.Print("Derivadas de funcoes de forma tetrahedra");
	}
*/
	return 0;
}

TPZCompMesh *InitialMesh(int dim,int order) {
	if(dim < 1) {
		cout << "Mesh with bad dimension.";
		return 0;
	}

	TPZGeoMesh *g = new TPZGeoMesh;
	TPZVec<int> nx(dim);
	TPZVec<REAL> X0(dim);
	TPZVec<REAL> X1(dim);
	for(int i=0;i<dim;i++) {
		nx[i] = 3;
		X0[i] = -3.;
		X1[i] = 3.;
	}
	TPZGenGrid gen(nx,X0,X1,1,0.5);
	gen.Read(*g);
	TPZCompMesh *c = new TPZCompMesh(g);
	TPZVec<REAL> sol(1,0.);
	TPZAutoPointer<TPZMaterial> material = new TPZL2Projection(1,2,1,sol);
	c->InsertMaterialObject(material);
	c->SetDefaultOrder(order);
	c->AutoBuild();
	return c;
}

void TestShape(TPZInterpolatedElement *el,int order) {
	TPZGeoEl *gel = el->Reference();
	int nsides = gel->NSides();
	int npoints;
	TPZVec<REAL> p(3,0);
	TPZVec<REAL> p_int(3,0);
	TPZFMatrix phi(100,1,0.), dphi(3,100,0.);
	TPZFMatrix phiint(100,1,0.), dphiint(3,100,0.);
	int nsideconnects, nsideshapef;
	int nconnects, nshapef;
	int i, j, k;
	TPZIntPoints *pointIntRule = 0;
	TPZVec<REAL> peso(10,0);
	TPZTransform transform;

	saida << "Elemento: " << el->Index() << endl;
	nconnects = el->NConnects();
	nshapef = el->NShapeF();
	saida << " " << "connects: " << nconnects << "  n shape: " << nshapef << endl;
	for(i=0;i<nsides;i++) {
		nsideconnects = el->NSideConnects(i);
		nsideshapef = el->NSideShapeF(i);
		saida << "  Side " << i << ": " << "  side connects: " << nsideconnects << "  n side shape: " << nsideshapef << endl;
		transform = gel->SideToSideTransform(i,nsides-1);
		pointIntRule = el->Reference()->CreateSideIntegrationRule(i,order);
		npoints = pointIntRule->NPoints();
		for(j=0;j<npoints;j++) {
			pointIntRule->Point(j,p_int,peso[j]);
			transform.Apply(p_int,p);
			//el->Reference()->X(p,p_int);
			el->SideShapeFunction(i,p_int,phi,dphi);
			el->Shape(p,phiint,dphiint);
			saida << " " << " Point " << j << " (" << p[0] << "," << p[1] << "," << p[1] << ")" << "  peso = " << peso[j] << endl;
			saida << " " << " " << " MATRIX Side Shape: ";
			for(k=0;k<nsideconnects;k++) saida << phi(k,0) << "  ";
			saida << endl;
			saida << " " << " " << " MATRIX Shape:      ";
			for(k=0;k<nconnects;k++) 
				saida << phiint(k,0) << "  ";
			saida << endl;
		}
		saida << endl;
	}
	delete pointIntRule;
}
