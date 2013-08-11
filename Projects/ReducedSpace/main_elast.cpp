//
//  File.cpp
//  PZ
//
//  Created by Agnaldo Farias on 7/31/12.
//  Copyright (c) 2012 LabMec-Unicamp. All rights reserved.
//


#include "pzfstrmatrix.h"
#include "toolstransienttime.h"

#include "TPZVTKGeoMesh.h"
#include "TPZRefPatternDataBase.h"

//#ifdef LOG4CXX
//static LoggerPtr logger(Logger::getLogger("pz.reducedspace.data"));
//#endif



int main(int argc, char *argv[])
{
//    gRefDBase.InitializeRefPatterns();
//    TPZAutoPointer<TPZRefPattern> refp = gRefDBase.FindRefPattern("Hex000000001101000011010000000");
//    if(refp)
//    {
//        std::ofstream outR("jubilula.vtk");
//        refp->PrintVTK(outR);
//    }
//    std::ofstream outR("RefPatterns.rpt");
//    gRefDBase.WriteRefPatternDBase(outR);
    
    //Propagation criterion
    REAL Lx = 400.;
    REAL Ly = 400.;
    REAL Lf = 50.;
    REAL Hf = 1.;
    REAL Young = 3.9E4;
    REAL Poiss = 0.25;
    REAL Fx = 0.;
    REAL Fy = 0.;
    int NStripes = 1;
    REAL Visc = 0.001E-6;
    
    REAL SigN = 6.15;
    
    /**
     * Lembre-se que a divisao por 2 (1 asa) e por Hf (na secao de 1 asa) eh feita no kernel.
     * Aqui vai Qinj total mesmo!!!
     */
    REAL QinjTot  = -0.2;

    REAL Ttot = 16; /** em segundos */
    REAL maxDeltaT = Ttot/2.;//5.; /** em segundos */
    int nTimes = 1; /**  */
    
    REAL Cl = 0.005;
    REAL Pe = 10.;
    REAL SigmaConf = 11.;
    REAL Pref = 60000.;
    REAL vsp = 0.001;
    REAL KIc = 1.E15;
    REAL Jradius = 0.5;
    
    int p = 2;
    
    globFractInputData.SetData(Lx, Ly, Lf, Hf, Young, Poiss, Fx, Fy, NStripes, Visc, SigN, QinjTot, Ttot, maxDeltaT, nTimes, Cl, Pe, SigmaConf, Pref, vsp, KIc, Jradius);
    ToolsTransient ToolTrans(p);
    
    std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
    std::cout << "****\n";
    std::cout << "*****\n";
    std::cout << "******\n";
    std::cout << "*******\n";
    std::cout << "********\n";
    std::cout << "**********\n";
    std::cout << "*************\n";
    std::cout << "*******************\n";
    std::cout << "*************************\n";
    std::cout << "*************************************\n";
    std::cout << "*************************************************\n";
    std::cout << "*******************************************************\n";
    std::cout << "**********************************************************************\n";
    std::cout << "**********************************************************************\n";
    std::cout << "**********************************************************************\n";
    std::cout << "**********************************************************************\n";
    std::cout << "**********************************************************************\n";
    std::cout << "**********************************************************************\n";
    std::cout << "**********************************************************************\n";
    std::cout << "**********************************************************************\n";
    std::cout << "**********************************************************************\n";
    std::cout << "Lembre-se de deixar os minT, maxT, actT etc como inteiros!!!\n";
    std::cout << "*******************************************************\n";
    std::cout << "*******************************************************\n";
    std::cout << "*******************************************************\n";
    std::cout << "*******************************************************\n";
    std::cout << "*******************************************************\n";
//    LEIA ACIMA!!!
	
	ToolTrans.Run();
    
    return 0;
}


