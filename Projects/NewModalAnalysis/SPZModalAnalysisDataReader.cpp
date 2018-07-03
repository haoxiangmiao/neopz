
#include <complex>
#include <fstream>
#include "SPZModalAnalysisDataReader.h"
#include "SPZModalAnalysisData.h"

SPZModalAnalysisDataReader::SPZModalAnalysisDataReader(ParameterHandler &param,const int argc, char *const *argv)
: prm(param)
{
  DeclareParameters();
  ParseCommandLine(argc,argv);
}

void SPZModalAnalysisDataReader::DeclareParameters() {
  prm.enter_subsection("NeoPZ options");
  {
      prm.declare_entry("Using NeoPZ mesh", "true",Patterns::Bool(),
                        "Whether to use NeoPZ simulation case instead of providing a .geo/.msh file.\n"
                        "If it is true, the mesh is specified in Which NeoPZ mesh and the corresponding settings are in\n"
                        "Physical options/(Step Fiber|Rectangular Waveguide) options."
                        "If it is false, the mesh is specified in Mesh file and the corresponding settings are in\n"
                        "Physical options/GMSH mesh options.");
      prm.declare_entry("Which NeoPZ mesh","Step Fiber",Patterns::Selection("Step Fiber|Rectangular Waveguide|Holey Fiber|"),
                        "If using NeoPZ generated mesh, this attribute specifies the simulation case\n");
      prm.declare_entry("Number of threads","4",Patterns::Integer(0),
                        "Number of threads to use in NeoPZ assembly.");
      prm.declare_entry("Polynomial order","1", Patterns::Integer(1),
                        "Default polynomial order of the Pk space used to build"
                        "the Nédélec elements(The H1 elements"
                        " will be built accordingly)");
      prm.declare_entry("Number of iterations(p)","1",Patterns::Integer(0),
                        "Option with self-explaining name.");
      prm.declare_entry("Factor","2",Patterns::List(Patterns::Integer(1),1),
                        "Vector with factor values related to element"
                        "sizes in mesh. In NeoPZ meshes it represents the number of nodes"
                        " that will be used to divide a quadrilateral edge.\n"
                        "If not using NeoPZ mesh, gmsh variable must be named factor");
  }
  prm.leave_subsection ();
  prm.enter_subsection ("Physical options");
  {
    prm.declare_entry("Cut-off analysis", "false",
                      Patterns::Bool(),
                      "Whether to perform cut-off analysis on the waveguide or"
                          "to calculate its modes for a given "
                          "operational frequency");
    prm.declare_entry("Frequency or wavelength", "wavelength",
                      Patterns::Selection("frequency|wavelength"),
                      "Whether to specify the wavelength in free space(lambda) or the frequency(f) at "
                          "which the system operates.");
    prm.enter_subsection("Frequency sweep");
    {
      prm.declare_entry("Wavelengths/frequencies vector", "1.55e-6",
                        Patterns::List(Patterns::Double(0.),1),
                        "Wavelength(in free-space) or ferquency at which the analysis is being done"
                                "(it will be ignored if Cut-off analysis is true)");
    }
    prm.leave_subsection ();
    prm.enter_subsection("Rectangular Waveguide Options");
    {
      prm.declare_entry("Width", "0.02286",
                        Patterns::Double(0.),
                        "Width of the hollow rectangular waveguide");
      prm.declare_entry("Height", "0.01016",
                        Patterns::Double(0.),
                        "Height of the hollow rectangular waveguide");
      prm.declare_entry("Refractive index or relative permittivity", "n",
                        Patterns::Selection("n|e"),
                        "Whether to specify the refraction index(n) or the relative permittivity(e) of "
                        "the dielectric materials.");
      prm.declare_entry("Electric Permittivity/Refractive index", "1.",
                        Patterns::Double(0.),
                        "Electric Permittivity/Refractive index of the material inside the waveguide");
      prm.declare_entry("Dielectric losses(epsilon)", "0.",
                        Patterns::Double(0.),
                        "The IMAGINARY part of the electric permittivity"
                        " of the dielectric material.");
      prm.declare_entry("Magnetic Permeability", "1.",
                        Patterns::Double(0.),
                        "Magnetic Permeability of the material inside the waveguide");
      prm.declare_entry("Dielectric losses(mu)", "0.",
                        Patterns::Double(0.),
                        "The IMAGINARY part of the electric permittivity"
                        " of the dielectric material.");
      prm.declare_entry("Use symmetry", "false",
                        Patterns::Bool(),
                        "Whether to use horizontal symmetry to reduce domains size.");
      prm.declare_entry("Symmetry type","PMC",Patterns::Selection("PEC|PMC"),
                        "If symmetry is being used, the boundary condition that will be applied.");
    };
    prm.leave_subsection();
    prm.enter_subsection("Step Fiber Options");
    {
      prm.declare_entry("Core radius", ".000008",
                        Patterns::Double(0.),
                        "Radius of the fibers core");
      prm.declare_entry("Core refractive index", "1.4457",
                        Patterns::Double(0.),
                        "Refractive index of the fibers core");
      prm.declare_entry("Core dielectric losses(epsilon)", "0.",
                        Patterns::Double(0.),
                        "The IMAGINARY part of the electric permittivity"
                        " of the core.");
      prm.declare_entry("Cladding refractive index", "1.4378",
                        Patterns::Double(0.),
                        "Refractive index of the fibers cladding");
      prm.declare_entry("Cladding dielectric losses(epsilon)", "0.",
                        Patterns::Double(0.),
                        "The IMAGINARY part of the electric permittivity"
                        " of the cladding.");
      prm.declare_entry("Core magnetic permeability", "1",
                        Patterns::Double(0.),
                        "Refractive index of the fibers core");
      prm.declare_entry("Core dielectric losses(mu)", "0.",
                        Patterns::Double(0.),
                        "The IMAGINARY part of the magnetic permeability"
                        " of the core.");
      prm.declare_entry("Cladding magnetic permeability", "1",
                        Patterns::Double(0.),
                        "Refractive index of the fibers core");
      prm.declare_entry("Cladding dielectric losses(mu)", "0.",
                        Patterns::Double(0.),
                        "The IMAGINARY part of the magnetic permeability"
                        " of the cladding.");
      prm.declare_entry("PML", "true",
                        Patterns::Bool(),
                        "Whether to surround the domain with a PML.");
      prm.declare_entry("PML length", "3",
                        Patterns::Double(0.),
                        "Length of the PML in relationship to the wavelength in the cladding material");
      prm.declare_entry("Boundary distance", "5",
                        Patterns::Double(0.),
                        "Boundary distance (excluding PML) in relationship to the wavelength in the cladding material");
      prm.declare_entry("PML attenuation constant", "5",
                        Patterns::Double(0.),
                        "The module of the IMAGINARY part of "
                        "the s parameters of the PML");
        prm.declare_entry("Number of layers in the PML", "5",
                          Patterns::Integer(1),
                          "The number of layers in the PML will be this value times factor");
    }
    prm.leave_subsection();

    prm.enter_subsection("Holey Fiber Options");
    {
        prm.declare_entry("Hole refractive index", "1.",
                          Patterns::Double(0.),
                          "Refractive index of the fibers hole");
        prm.declare_entry("Hole dielectric losses(epsilon)", "0.",
                          Patterns::Double(0.),
                          "The IMAGINARY part of the electric permittivity"
                          " of the hole.");
        prm.declare_entry("Cladding refractive index", "1.45",
                          Patterns::Double(0.),
                          "Refractive index of the fibers cladding");
        prm.declare_entry("Cladding dielectric losses(epsilon)", "0.",
                          Patterns::Double(0.),
                          "The IMAGINARY part of the electric permittivity"
                          " of the cladding.");
        prm.declare_entry("Hole magnetic permeability", "1",
                          Patterns::Double(0.),
                          "Refractive index of the fibers hole");
        prm.declare_entry("Hole dielectric losses(mu)", "0.",
                          Patterns::Double(0.),
                          "The IMAGINARY part of the magnetic permeability"
                          " of the hole.");
        prm.declare_entry("Cladding magnetic permeability", "1",
                          Patterns::Double(0.),
                          "Refractive index of the fibers core");
        prm.declare_entry("Cladding dielectric losses(mu)", "0.",
                          Patterns::Double(0.),
                          "The IMAGINARY part of the magnetic permeability"
                          " of the cladding.");
        prm.declare_entry("PML length", "2e-6",
                          Patterns::Double(0.),
                          "Length of the PML");
        prm.declare_entry("Boundary distance", "15.75e-6",
                          Patterns::Double(0.),
                          "Boundary distance (excluding PML)");
        prm.declare_entry("PML attenuation constant", "0.00005",
                          Patterns::Double(0.),
                          "The module of the IMAGINARY part of "
                          "the s parameters of the PML");
        prm.declare_entry("Symmetry type","PMC",Patterns::Selection("PEC|PMC"),
                          "If symmetry is being used, the boundary condition that will be applied.");
    }
    prm.leave_subsection();
    prm.enter_subsection("GMSH Mesh Options");
    {
        prm.declare_entry("Mesh file", "",
                          Patterns::Anything(),
                          "Path to .geo gmsh description of the mesh.");
        prm.declare_entry("Mesh order", "1",
                          Patterns::Integer(1),
                          "Order of the geometrical mapping.");
      prm.declare_entry("Number of materials", "1",
                        Patterns::Integer(1),
                        "How many dielectric materials are present "
                            "in the waveguide (WITHOUT PML)");
      prm.declare_entry("Boundary conditions", "PEC",
                        Patterns::List(Patterns::Selection("PEC|PMC"),1),
                        "The types of all boundary conditions in the order they appear "
                        "in the gmsh file.");
      prm.declare_entry("Refractive index or relative permittivity", "n",
                        Patterns::Selection("n|e"),
                        "Whether to specify the refraction index(n) or the relative permittivity(e) of "
                        "the dielectric materials.");
      prm.declare_entry("Refractive/permittivity vector", "1.",
                        Patterns::List(Patterns::Double(0.),1),
                        "The REAL part of the square root of the electric permittivity of "
                            "the dielectric materials, separated by commas");
      prm.declare_entry("Lossless(epsilon)","true", Patterns::Bool(),
                        "Whether the dielectric material has complex "
                            "permittivity or not.");
      prm.declare_entry("Dielectric losses vector(epsilon)", "0.",
                        Patterns::List(Patterns::Double(0.),1),
                        "The IMAGINARY part of the electric permittivity"
                            " of the dielectric materials, separated by "
                            "commas (it will be ignored"
                            " if Lossless(epsilon) is false");
      prm.declare_entry("Magnetic permeability vector", "1.",
                        Patterns::List(Patterns::Double(0.),1),
                        "The REAL part of the magnetic permeability of the "
                            "dielectric materials, separated by commas");
      prm.declare_entry("Lossless(mu)","true", Patterns::Bool(),
                        "Whether the dielectric material has complex permeability or not.");
      prm.declare_entry("Dielectric losses vector(mu)", "0.",
                        Patterns::List(Patterns::Double(0.),1),
                        "The IMAGINARY part of the magnetic permeability"
                            " of the dielectric materials, "
                            "separated by commas (it will be ignored"
                            " if Lossless(mu) is false");
      prm.declare_entry("PML", "false",
                          Patterns::Bool(),
                          "Whether the domain is surrounded by a PML.\n");
      prm.declare_entry("PML attenuation constant", "5.",
                          Patterns::Double(0.),
                          "The module of the IMAGINARY part of "
                                "the s parameters of the PML");
      prm.declare_entry("PML regions", "xp, yp, xm, ym, xpyp, xmyp, xmym, xpym",
                          Patterns::List(Patterns::Selection("xp|yp|xm|ym|xpyp|xmyp|xmym|xpym"),1),
                          "The PML regions present in the gmsh file, according to the following diagram:\n"
                          "-----------------------------------------\n"
                          "| xmyp |           yp            | xpyp |\n"
                          "|---------------------------------------|\n"
                          "|      |                         |      |\n"
                          "|      |                         |      |\n"
                          "|      |                         |      |\n"
                          "|  xm  |                         |  xp  |\n"
                          "|      |                         |      |\n"
                          "|      |                         |      |\n"
                          "|      |                         |      |\n"
                          "-----------------------------------------\n"
                          "| xmym |           ym            | xmym |\n"
                          "-----------------------------------------\n");
    }
      prm.leave_subsection ();
  }
  prm.leave_subsection ();
  prm.enter_subsection("Export options");
  {
    prm.declare_entry("Export compmesh","false",Patterns::Bool(),
                        "Whether to export the computational mesh in both .txt and .vtk formats.");
    prm.declare_entry("Export geomesh","false",Patterns::Bool(),
                        "Whether to export the geometrical mesh in both .txt and .vtk formats.");
    prm.declare_entry("Export eigenvalues","false",Patterns::Bool(),
                        "If set to true, eigenvalues will"
                        "be exported to a text file.");
    prm.declare_entry("Prefix","",Patterns::Anything(),
                        "Prefix to be added to exported files(default is 1Mesh File)");
    prm.declare_entry("VTK","false", Patterns::Bool(),
                        "If set to true, a .vtk plot of the electric field will be generated"
                        "for the calculated modes");
    prm.declare_entry("VTK resolution","0",Patterns::Integer(0),
                        "Resolution to be used during post-processing.");
    prm.declare_entry("VTK Abs|Re","Abs",Patterns::Selection("Abs|Re"),
                        "Whether to export magnitude or real part of eigenvector");
  }
  prm.leave_subsection();
  prm.enter_subsection("Scaling");
  {
    prm.declare_entry("Is target scaled","true",Patterns::Bool(),
                        "Whether the target value is already scaled(frequency/lambda will be scaled)");
    prm.declare_entry("Scale by k0","true",Patterns::Bool(),
                        "If set to true, the eigenvalues will be beta/k0 and scale factor will be ignored.");
    prm.declare_entry("Scale factor","1.",Patterns::Double(0),
                        "Maximum dimension for geometric domain(frequency/lambda will be scaled)");
  }
  prm.leave_subsection();


  //IN THE FOLLOWING OPTIONS, -1 =  PETSC_DECIDE and -2 = PETSC_DEFAULT
  prm.enter_subsection("SLEPc solver options");
  {
    prm.enter_subsection("Eigensolver(EPS)");
    {
      prm.declare_entry("Problem type", "EPS_GNHEP",
                        Patterns::Selection("EPS_HEP|EPS_GHEP|EPS_NHEP"
                                                "|EPS_GNHEP|EPS_PGNHEP|EPS_GHIEP"),
                        "Sets the type of the eigenvalue problem type.\n"
                            "G stands for Generalized, H for Hermitian,\n"
                            "P for positive semi-definite B matrix,\n"
                            "and I for generalized Hermitian-indefinite.");
      prm.declare_entry("Eigensolver","EPSKRYLOVSCHUR",Patterns::Selection("EPSPOWER|EPSSUBSPACE|EPSARNOLDI|EPSLANCZOS|"
                                                                               "EPSKRYLOVSCHUR|EPSGD|EPSJD|EPSRQCG|"
                                                                               "EPSLOBPCG|EPSCISS|EPSLAPACK|EPSARPACK|"
                                                                               "EPSBLZPACK|EPSTRLAN|EPSBLOPEX|"
                                                                               "EPSPRIMME|EPSFEAST"),
                        "Sets the particular eigensolver to be used");

      prm.declare_entry("Krylov locking","false",Patterns::Bool(),"If Eigensolver is Krylov-Schur,"
          " whether to use the locking variant or not.");
      prm.declare_entry("Krylov restart","0.5",Patterns::Double(0.5,0.95),"Sets the restart parameter for the "
          "Krylov-Schur method, in particular the proportion of basis vectors that must be kept after restart.");
      prm.declare_entry("Convergence test", "EPS_CONV_ABS",
                        Patterns::Selection("EPS_CONV_ABS|EPS_CONV_REL"
                                                "|EPS_CONV_NORM"),
                        "Specifies how to compute the error"
                            " estimate used in the convergence test.");
      prm.declare_entry("True residual","false",Patterns::Bool(),"Whether to calculate the true residual"
          " if using shift-and-invert as a spectral transformation.");
      prm.declare_entry("Which eigenvalues", "EPS_TARGET_REAL",
                        Patterns::Selection("EPS_LARGEST_MAGNITUDE|EPS_SMALLEST_MAGNITUDE"
                                                "|EPS_LARGEST_REAL|EPS_SMALLEST_REAL|"
                                                "EPS_LARGEST_IMAGINARY|EPS_SMALLEST_IMAGINARY|"
                                                "EPS_TARGET_MAGNITUDE|EPS_TARGET_REAL|"
                                                "EPS_TARGET_IMAGINARY|EPS_ALL"),
                        "Specifies which portion of the spectrum is to be sought.");
      prm.declare_entry("Target eigenvalue", "-1.", Patterns::Double(),
                        "Target eigenvalue to be sought if Which eigenvalues is equal "
                            "to EPS_TARGET_(MAGNITUDE|IMAGINARY|REAL).");
      prm.declare_entry("Eigensolver tolerance", "1e-50", Patterns::Double(0.),
                        "Eigensolver convergence tolerance(0 for PETSC_DEFAULT).");
      prm.declare_entry("Eigensolver maximum iterations", "0", Patterns::Integer(0),
                        "Maximum number of iterations of the eigensolver(0 for PETSC_DEFAULT).");
      prm.declare_entry("nev(How many eigenvalues)", "5", Patterns::Integer(1),
                        "Number of eigenvalues that must satisfy the convergence test");
      prm.declare_entry("ncv(Solver subspace dimension)", "0", Patterns::Integer(0),
                        "The maximum dimension of the subspace to be "
                            "used by the eigensolver(0 for PETSC_DECIDE)");
      prm.declare_entry("mpd(Maximum projected dimension)", "0", Patterns::Integer(0),
                        "The maximum projected dimension of the "
                            "eigensolver(0 for PETSC_DECIDE)");
      prm.declare_entry("Spectral transformation","STSINVERT",Patterns::Selection("STSHELL|STSHIFT|STSINVERT|"
                                                                                      "STCAYLEY|STPRECOND|STFILTER"),
                        "Sets the spectral transformation to be used.");
      prm.declare_entry("Solver verbosity", "false", Patterns::Bool(),
                        "Whether the solver should print data during the simulation.");
    }
    prm.leave_subsection();
    prm.enter_subsection("Linear solver(KSP)");
    {
      prm.declare_entry("Linear solver","KSPPREONLY",Patterns::Selection("KSPRICHARDSON|KSPCHEBYSHEV|KSPCG|KSPGROPPCG|"
                                                                             "KSPPIPECG|KSPPIPECGRR|KSPCGNE|KSPCGNASH|"
                                                                             "KSPCGSTCG|KSPCGGLTR|KSPFCG|KSPPIPEFCG|"
                                                                             "KSPGMRES|KSPPIPEFGMRES|KSPFGMRES|"
                                                                             "KSPLGMRES|KSPDGMRES|KSPPGMRES|KSPTCQMR|"
                                                                             "KSPBCGS|KSPIBCGS|KSPFBCGS|KSPFBCGSR|"
                                                                             "KSPBCGSL|KSPPIPEBCGS|KSPCGS|KSPTFQMR|"
                                                                             "KSPCR|KSPPIPECR|KSPLSQR|KSPPREONLY"),
                        "Sets the preconditioner to be used.");
      prm.declare_entry("Linear solver relative tolerance", "0", Patterns::Double(0.),
                        "Sets the relative convergence tolerance, relative decrease in"
                            " the residual norm(0 for PETSC_DEFAULT)");
      prm.declare_entry("Linear solver absolute tolerance", "0", Patterns::Double(0.),
                        "Sets the absolute convergence tolerance absolute size of the "
                            "residual norm(0 for PETSC_DEFAULT)");
      prm.declare_entry("Linear solver divergence tolerance", "1000", Patterns::Double(0.),
                        "Sets the divergence tolerance, amount residual norm can increase before "
                            "the convergence test concludes that the method is diverging(0 for PETSC_DEFAULT)");
      prm.declare_entry("Linear solver maximum iterations", "0", Patterns::Integer(0),
                        "Maximum number of iterations to use(0 for PETSC_DEFAULT)");
      prm.declare_entry("Preconditioner","PCLU",Patterns::Selection("PCNONE|PCCHOLESKY|PCJACOBI|PCSOR|PCLU|"
                                                                               "PCSHELL|PCBJACOBI|PCMG|PCEISENSTAT|PCILU|"
                                                                               "PCICC|PCASM|PCGASM|PCKSP|"
                                                                               "PCCOMPOSITE|PCREDUNDANT"),
                        "Sets the preconditioner to be used.");
    }
    prm.leave_subsection();
  }
  prm.leave_subsection ();
}

void SPZModalAnalysisDataReader::ReadParameters(SPZModalAnalysisData &data) {

  prm.enter_subsection("NeoPZ options");
  {
    data.pzOpts.usingNeoPzMesh = prm.get_bool("Using NeoPZ mesh");
    if(data.pzOpts.usingNeoPzMesh){
      auto val = prm.get("Which NeoPZ mesh");
        if(val == "Step Fiber"){
          data.pzOpts.pzCase = SPZModalAnalysisData::StepFiber;
          data.pzOpts.meshFile = "stepFiberPzMesh";
        }
        else if(val == "Rectangular Waveguide"){
          data.pzOpts.pzCase = SPZModalAnalysisData::RectangularWG;
          data.pzOpts.meshFile = "rectangularWGPzMesh";
        }
        else if(val == "Holey Fiber"){
            data.pzOpts.pzCase = SPZModalAnalysisData::HoleyFiber;
            data.pzOpts.meshFile = "holeyFiberPzMesh";
        } else{
            DebugStop();
        }
        data.pzOpts.meshOrder = (int) 314;//useless, actually, just for file naming
    }

    data.pzOpts.nThreads = (int) prm.get_integer("Number of threads");//integer
    data.pzOpts.pOrder = (int) prm.get_integer("Polynomial order");//integer
    data.pzOpts.pSteps  = prm.get_integer("Number of iterations(p)");
    if(!data.pzOpts.externGenMesh){
        std::string rawVec = prm.get("Factor");
        const std::vector<std::string> split_list =
                Utilities::split_string_list(rawVec, ",");
        data.pzOpts.hSteps = split_list.size();
        data.pzOpts.factorVec.Resize(data.pzOpts.hSteps);
        for (int i = 0; i < split_list.size() ; i++){
            const std::string & string = split_list[i];
            data.pzOpts.factorVec[i] = (int)Utilities::string_to_int(string);
        }
    }
  }
  prm.leave_subsection();

  prm.enter_subsection ("Physical options");
  {

    data.physicalOpts.isCutOff = prm.get_bool("Cut-off analysis");//bool
    data.physicalOpts.isLambda = prm.get("Frequency or wavelength")
                    == "wavelength" ? true : false;
    prm.enter_subsection("Frequency sweep");
    {
      std::string rawVec = prm.get("Wavelengths/frequencies vector");
      const std::vector<std::string> split_list =
              Utilities::split_string_list(rawVec, ",");
      if(split_list.size() <1){
        std::cout<<"At least one frequency must be provided. Terminating..."<<std::endl;
        DebugStop();
      }
      data.physicalOpts.freqVec.Resize(split_list.size());
      for (int i = 0; i < split_list.size() ; i++){
        const std::string & string = split_list[i];
        data.physicalOpts.freqVec[i] = (REAL)Utilities::string_to_double(string);
      }
    }
    prm.leave_subsection ();
    if(data.pzOpts.usingNeoPzMesh && data.pzOpts.pzCase == SPZModalAnalysisData::RectangularWG){
        prm.enter_subsection("Rectangular Waveguide Options");
        {
            data.physicalOpts.rectangularWgOpts.wDomain = prm.get_double("Width");
            data.physicalOpts.rectangularWgOpts.hDomain = prm.get_double("Height");
            data.physicalOpts.erVec.Resize(1);
            data.physicalOpts.urVec.Resize(1);
            data.physicalOpts.erVec[0] = prm.get_double("Electric Permittivity/Refractive index");
            if(prm.get("Refractive index or relative permittivity") == std::string("n")){
                data.physicalOpts.erVec[0] *= data.physicalOpts.erVec[0];
            }
            data.physicalOpts.erVec[0] += sqrt(std::complex<REAL>(-1)) * prm.get_double("Dielectric losses(epsilon)");
            data.physicalOpts.urVec[0] =(STATE) prm.get_double("Magnetic Permeability");
            data.physicalOpts.erVec[0] += sqrt(std::complex<REAL>(-1)) * prm.get_double("Dielectric losses(mu)");
            data.physicalOpts.rectangularWgOpts.usingSymmetry= prm.get_bool("Use symmetry");
            if(data.physicalOpts.rectangularWgOpts.usingSymmetry){
                std::string symType = prm.get("Symmetry type");
                data.physicalOpts.rectangularWgOpts.symmetryType =
                        symType == std::string("PEC") ? SPZModalAnalysisData::PEC : SPZModalAnalysisData::PMC ;
            }
        }
        prm.leave_subsection();
    }
    if(data.pzOpts.usingNeoPzMesh && data.pzOpts.pzCase == SPZModalAnalysisData::StepFiber){
        prm.enter_subsection("Step Fiber Options");
        {
            data.physicalOpts.stepFiberOpts.realRCore = prm.get_double("Core radius");
            data.physicalOpts.erVec.Resize(2);
            data.physicalOpts.urVec.Resize(2);
            data.physicalOpts.erVec[0] = (STATE)prm.get_double("Core refractive index");
            data.physicalOpts.erVec[0] *= data.physicalOpts.erVec[0];
            data.physicalOpts.erVec[0] += sqrt(std::complex<REAL>(-1)) * prm.get_double("Core dielectric losses(epsilon)");
            data.physicalOpts.erVec[1] = (STATE)prm.get_double("Cladding refractive index");
            data.physicalOpts.erVec[1] *= data.physicalOpts.erVec[1];
            data.physicalOpts.erVec[1] += sqrt(std::complex<REAL>(-1)) * prm.get_double("Cladding dielectric losses(epsilon)");

            data.physicalOpts.urVec[0] = (STATE)prm.get_double("Core magnetic permeability");
            data.physicalOpts.urVec[0] *= data.physicalOpts.urVec[0];
            data.physicalOpts.urVec[0] += sqrt(std::complex<REAL>(-1)) * prm.get_double("Core dielectric losses(mu)");
            data.physicalOpts.urVec[1] = (STATE)prm.get_double("Cladding magnetic permeability");
            data.physicalOpts.urVec[1] *= data.physicalOpts.urVec[1];
            data.physicalOpts.urVec[1] += sqrt(std::complex<REAL>(-1)) * prm.get_double("Cladding dielectric losses(mu)");
            data.physicalOpts.stepFiberOpts.boundDist = prm.get_double("Boundary distance");
            data.physicalOpts.stepFiberOpts.hasPML = prm.get_bool("PML");
            if(data.physicalOpts.stepFiberOpts.hasPML){
                data.physicalOpts.stepFiberOpts.dPML = prm.get_double("PML length");
                data.physicalOpts.alphaMax = prm.get_double("PML attenuation constant");
                data.physicalOpts.stepFiberOpts.nLayersPml = prm.get_integer("Number of layers in the PML");
            }
        }
        prm.leave_subsection();
    }

    if(data.pzOpts.usingNeoPzMesh && data.pzOpts.pzCase == SPZModalAnalysisData::HoleyFiber){
        prm.enter_subsection("Holey Fiber Options");
        {
            data.physicalOpts.erVec.Resize(2);
            data.physicalOpts.urVec.Resize(2);
            data.physicalOpts.erVec[0] = (STATE)prm.get_double("Hole refractive index");
            data.physicalOpts.erVec[0] *= data.physicalOpts.erVec[0];
            data.physicalOpts.erVec[0] += sqrt(std::complex<REAL>(1)) * prm.get_double("Hole dielectric losses(epsilon)");
            data.physicalOpts.erVec[1] = (STATE)prm.get_double("Cladding refractive index");
            data.physicalOpts.erVec[1] *= data.physicalOpts.erVec[1];
            data.physicalOpts.erVec[1] += sqrt(std::complex<REAL>(-1)) * prm.get_double("Cladding dielectric losses(epsilon)");
            data.physicalOpts.urVec[0] = (STATE)prm.get_double("Hole magnetic permeability");
            data.physicalOpts.urVec[0] *= data.physicalOpts.urVec[0];
            data.physicalOpts.urVec[0] += sqrt(std::complex<REAL>(-1)) * prm.get_double("Hole dielectric losses(mu)");
            data.physicalOpts.urVec[1] = (STATE)prm.get_double("Cladding magnetic permeability");
            data.physicalOpts.urVec[1] *= data.physicalOpts.urVec[1];
            data.physicalOpts.urVec[1] += sqrt(std::complex<REAL>(-1)) * prm.get_double("Cladding dielectric losses(mu)");
            data.physicalOpts.holeyFiberOpts.boundDist = prm.get_double("Boundary distance");
            data.physicalOpts.holeyFiberOpts.dPML = prm.get_double("PML length");
            data.physicalOpts.alphaMax = prm.get_double("PML attenuation constant");
            std::string symType = prm.get("Symmetry type");
            data.physicalOpts.holeyFiberOpts.symmetry =
                    symType == std::string("PEC") ? SPZModalAnalysisData::PEC : SPZModalAnalysisData::PMC ;
        }
        prm.leave_subsection();
    }
    if(data.pzOpts.usingNeoPzMesh == false){
        prm.enter_subsection("GMSH Mesh Options");
        {
            data.pzOpts.meshFile = path + prm.get("Mesh file");//anything
            std::string str = data.pzOpts.meshFile;
            data.pzOpts.externGenMesh = false;
            while(str.size() == 0 || str.substr(str.size()-4,4) != ".geo" || !FileExists(str)){
                if(str.substr(str.size()-4,4) == ".msh"){
                    if(FileExists(str)){
                        data.pzOpts.externGenMesh = true;
                        break;
                    }
                }
                std::cout<<"Input a valid mesh file: "<<std::endl;
                std::cin >> data.pzOpts.meshFile;
                str = path + data.pzOpts.meshFile;
                if(str.size() == 0 || str.substr(str.size()-4,4) != ".geo" || !FileExists(str)){
                    if(str.substr(str.size()-4,4) == ".msh"){
                        if(FileExists(str)){
                            data.pzOpts.externGenMesh = true;
                            break;
                        }
                    }
                    std::cout<<"Not a valid name."<<std::endl;
                    DebugStop();
                }
            }
            data.pzOpts.meshOrder = (int) prm.get_integer("Mesh order");
            data.physicalOpts.nMaterials=(int)prm.get_integer("Number of materials");//int
            bool isRefractive = prm.get("Refractive index or relative permittivity")
                                == "n" ? true : false;
            ReadComplexVector(data.physicalOpts.nMaterials,"Refractive/permittivity vector",
                              "Dielectric losses vector(epsilon)","Lossless(epsilon)",
                              data.physicalOpts.erVec);
            if(isRefractive){
                for (int i = 0; i < data.physicalOpts.erVec.size(); ++i) {
                    data.physicalOpts.erVec[i] *= data.physicalOpts.erVec[i];//gets er from n
                }
            }
            ReadComplexVector(data.physicalOpts.nMaterials,"Magnetic permeability vector",
                              "Dielectric losses vector(mu)","Lossless(mu)",
                              data.physicalOpts.urVec);
            {
                std::string rawVec = prm.get("Boundary conditions");//double
                const std::vector<std::string> split_list =
                        Utilities::split_string_list(rawVec, ",");
                data.physicalOpts.boundTypeVec.Resize(split_list.size());
                for (int i = 0; i < split_list.size(); ++i) {
                    switch(Utilities::str_to_constexpr(split_list[i].c_str())){
                        case Utilities::str_to_constexpr("PEC"):
                            data.physicalOpts.boundTypeVec[i] = SPZModalAnalysisData::PEC;
                            break;
                        case Utilities::str_to_constexpr("PMC"):
                            data.physicalOpts.boundTypeVec[i] = SPZModalAnalysisData::PMC;
                            break;
                    }
                }
            }
            if(prm.get_bool("PML")){
                data.physicalOpts.alphaMax = prm.get_double("PML attenuation constant");
                std::string rawVec = prm.get("PML regions");//double
                const std::vector<std::string> split_list =
                        Utilities::split_string_list(rawVec, ",");
                data.physicalOpts.pmlTypeVec.Resize(split_list.size());
                for(int i = 0; i < split_list.size(); i++ ){
                    switch(Utilities::str_to_constexpr(split_list[i].c_str())){
                        case Utilities::str_to_constexpr("xp"):
                            data.physicalOpts.pmlTypeVec[i] = SPZModalAnalysisData::xp;
                            break;
                        case Utilities::str_to_constexpr("yp"):
                            data.physicalOpts.pmlTypeVec[i] = SPZModalAnalysisData::yp;
                            break;
                        case Utilities::str_to_constexpr("xm"):
                            data.physicalOpts.pmlTypeVec[i] = SPZModalAnalysisData::xm;
                            break;
                        case Utilities::str_to_constexpr("ym"):
                            data.physicalOpts.pmlTypeVec[i] = SPZModalAnalysisData::ym;
                            break;
                        case Utilities::str_to_constexpr("xpyp"):
                            data.physicalOpts.pmlTypeVec[i] = SPZModalAnalysisData::xpyp;
                            break;
                        case Utilities::str_to_constexpr("xmyp"):
                            data.physicalOpts.pmlTypeVec[i] = SPZModalAnalysisData::xmyp;
                            break;
                        case Utilities::str_to_constexpr("xmym"):
                            data.physicalOpts.pmlTypeVec[i] = SPZModalAnalysisData::xmym;
                            break;
                        case Utilities::str_to_constexpr("xpym"):
                            data.physicalOpts.pmlTypeVec[i] = SPZModalAnalysisData::xpym;
                            break;
                    }
                }
            }
        }
        prm.leave_subsection ();
    }
  }
  prm.leave_subsection();
  prm.enter_subsection("Export options");
  {
      data.pzOpts.exportCMesh = prm.get_bool("Export compmesh");//bool
      data.pzOpts.exportGMesh = prm.get_bool("Export geomesh");//bool
      data.pzOpts.exportEigen = prm.get_bool("Export eigenvalues");//bool
      data.pzOpts.prefix = path + prm.get("Prefix");//anything
      data.pzOpts.prefix += data.pzOpts.meshFile.substr(0, data.pzOpts.meshFile.size() - 4);
      data.pzOpts.genVTK = prm.get_bool("VTK");//bool
      data.pzOpts.vtkRes = prm.get_integer("VTK resolution");//integer
      data.pzOpts.absVal = prm.get("VTK Abs|Re") == "Abs" ? true : false;//selection
  }
  prm.leave_subsection();
  prm.enter_subsection("Scaling");
  {
      data.pzOpts.scaleByk0 = prm.get_bool("Scale by k0");
      data.pzOpts.scaleFactor = prm.get_double("Scale factor");//double
      data.pzOpts.isTargetScaled = prm.get_bool("Is target scaled");//bool
  }
  prm.leave_subsection ();
  prm.enter_subsection("SLEPc solver options");
  {
    std::string str;
    prm.enter_subsection("Eigensolver(EPS)");
    {
      str = prm.get("Problem type");
      switch (Utilities::str_to_constexpr(str.c_str())) {
        case Utilities::str_to_constexpr("EPS_HEP") :
          data.solverOpts.eps_prob_type = EPS_HEP;
          break;
        case Utilities::str_to_constexpr("EPS_GHEP") :
          data.solverOpts.eps_prob_type = EPS_GHEP;
          break;
        case Utilities::str_to_constexpr("EPS_NHEP") :
          data.solverOpts.eps_prob_type = EPS_NHEP;
          break;
        case Utilities::str_to_constexpr("EPS_GNHEP") :
          data.solverOpts.eps_prob_type = EPS_GNHEP;
          break;
        case Utilities::str_to_constexpr("EPS_PGNHEP") :
          data.solverOpts.eps_prob_type = EPS_PGNHEP;
          break;
        case Utilities::str_to_constexpr("EPS_GHIEP") :
          data.solverOpts.eps_prob_type = EPS_GHIEP;
          break;
        default:
          std::cout << "Error while reading" << "Problem type" << " with value "
                    << str << std::endl;
          DebugStop();
      }

      str = prm.get("Eigensolver");
      switch (Utilities::str_to_constexpr(str.c_str())) {
        case Utilities::str_to_constexpr("EPSPOWER"):
          data.solverOpts.eps_type = EPSPOWER;
          break;
        case Utilities::str_to_constexpr("EPSSUBSPACE"):
          data.solverOpts.eps_type = EPSSUBSPACE;
          break;
        case Utilities::str_to_constexpr("EPSARNOLDI"):
          data.solverOpts.eps_type = EPSARNOLDI;
          break;
        case Utilities::str_to_constexpr("EPSLANCZOS"):
          data.solverOpts.eps_type = EPSLANCZOS;
          break;
        case Utilities::str_to_constexpr("EPSKRYLOVSCHUR"):
          data.solverOpts.eps_type = EPSKRYLOVSCHUR;
          break;
        case Utilities::str_to_constexpr("EPSGD"):
          data.solverOpts.eps_type = EPSGD;
          break;
        case Utilities::str_to_constexpr("EPSJD"):
          data.solverOpts.eps_type = EPSJD;
          break;
        case Utilities::str_to_constexpr("EPSRQCG"):
          data.solverOpts.eps_type = EPSRQCG;
          break;
        case Utilities::str_to_constexpr("EPSLOBPCG"):
          data.solverOpts.eps_type = EPSLOBPCG;
          break;
        case Utilities::str_to_constexpr("EPSCISS"):
          data.solverOpts.eps_type = EPSCISS;
          break;
        case Utilities::str_to_constexpr("EPSLAPACK"):
          data.solverOpts.eps_type = EPSLAPACK;
          break;
        case Utilities::str_to_constexpr("EPSARPACK"):
          data.solverOpts.eps_type = EPSARPACK;
          break;
        case Utilities::str_to_constexpr("EPSBLZPACK"):
          data.solverOpts.eps_type = EPSBLZPACK;
          break;
        case Utilities::str_to_constexpr("EPSTRLAN"):
          data.solverOpts.eps_type = EPSTRLAN;
          break;
        case Utilities::str_to_constexpr("EPSBLOPEX"):
          data.solverOpts.eps_type = EPSBLOPEX;
          break;
        case Utilities::str_to_constexpr("EPSPRIMME"):
          data.solverOpts.eps_type = EPSPRIMME;
          break;
        case Utilities::str_to_constexpr("EPSFEAST"):
          data.solverOpts.eps_type = EPSFEAST;
          break;
        default:
          std::cout << "Error while reading" << "Eigensolver" << " with value "
                    << str << std::endl;
          DebugStop();
      }
      data.solverOpts.eps_krylov_locking = prm.get_bool("Krylov locking");//bool
      data.solverOpts.eps_krylov_restart = prm.get_double("Krylov restart");//double
      str = prm.get("Convergence test");
      switch (Utilities::str_to_constexpr(str.c_str())) {
        case Utilities::str_to_constexpr("EPS_CONV_ABS") :
          data.solverOpts.eps_conv_test = EPS_CONV_ABS;
          break;
        case Utilities::str_to_constexpr("EPS_CONV_REL") :
          data.solverOpts.eps_conv_test = EPS_CONV_REL;
          break;
        case Utilities::str_to_constexpr("EPS_CONV_NORM") :
          data.solverOpts.eps_conv_test = EPS_CONV_NORM;
          break;
        default:
          std::cout << "Error while reading" << "Convergence test" << " with value "
                    << str << std::endl;
          DebugStop();
      }
      data.solverOpts.eps_true_res = prm.get_bool("True residual");//bool

      str = prm.get("Which eigenvalues");
      switch (Utilities::str_to_constexpr(str.c_str())) {
        case Utilities::str_to_constexpr("EPS_LARGEST_MAGNITUDE") :
          data.solverOpts.eps_which_eig = EPS_LARGEST_MAGNITUDE;
          break;
        case Utilities::str_to_constexpr("EPS_SMALLEST_MAGNITUDE") :
          data.solverOpts.eps_which_eig = EPS_SMALLEST_MAGNITUDE;
          break;
        case Utilities::str_to_constexpr("EPS_LARGEST_REAL") :
          data.solverOpts.eps_which_eig = EPS_LARGEST_REAL;
          break;
        case Utilities::str_to_constexpr("EPS_SMALLEST_REAL") :
          data.solverOpts.eps_which_eig = EPS_SMALLEST_REAL;
          break;
        case Utilities::str_to_constexpr("EPS_LARGEST_IMAGINARY") :
          data.solverOpts.eps_which_eig = EPS_LARGEST_IMAGINARY;
          break;
        case Utilities::str_to_constexpr("EPS_SMALLEST_IMAGINARY") :
          data.solverOpts.eps_which_eig = EPS_SMALLEST_IMAGINARY;
          break;
        case Utilities::str_to_constexpr("EPS_TARGET_MAGNITUDE") :
          data.solverOpts.eps_which_eig = EPS_TARGET_MAGNITUDE;
          break;
        case Utilities::str_to_constexpr("EPS_TARGET_REAL") :
          data.solverOpts.eps_which_eig = EPS_TARGET_REAL;
          break;
        case Utilities::str_to_constexpr("EPS_TARGET_IMAGINARY") :
          data.solverOpts.eps_which_eig = EPS_TARGET_IMAGINARY;
          break;
        case Utilities::str_to_constexpr("EPS_ALL") :
          data.solverOpts.eps_which_eig = EPS_ALL;
          break;
        default:
          std::cout << "Error while reading" << "Which eigenvalues" << " with value "
                    << str << std::endl;
          DebugStop();
      }

      data.solverOpts.target = prm.get_double("Target eigenvalue");//double
      if(! data.pzOpts.isTargetScaled){
        data.solverOpts.target = data.solverOpts.target /( data.pzOpts.scaleFactor * data.pzOpts.scaleFactor);
      }
      data.solverOpts.eps_tol = prm.get_double("Eigensolver tolerance");//double
      if (!data.solverOpts.eps_tol) data.solverOpts.eps_tol = -2;//PETSC_DEFAULT
      data.solverOpts.eps_max_its = (int) prm.get_integer("Eigensolver maximum iterations");//integer
      if (!data.solverOpts.eps_max_its) data.solverOpts.eps_max_its = -2;//PETSC_DEFAULT
      data.solverOpts.eps_nev = (int) prm.get_integer("nev(How many eigenvalues)");//integer
      data.solverOpts.eps_ncv = (int) prm.get_integer("ncv(Solver subspace dimension)");//integer
      if (!data.solverOpts.eps_ncv) data.solverOpts.eps_ncv = -1;//PETSC_DECIDE
      data.solverOpts.eps_mpd = (int) prm.get_integer("mpd(Maximum projected dimension)");//integer
      if (!data.solverOpts.eps_mpd) data.solverOpts.eps_mpd = -1;//PETSC_DECIDE

      str =prm.get("Spectral transformation");
      switch(Utilities::str_to_constexpr(str.c_str())){
        case Utilities::str_to_constexpr("STSHELL") :
          data.solverOpts.st_type = STSHELL;
          break;
        case Utilities::str_to_constexpr("STSHIFT") :
          data.solverOpts.st_type = STSHIFT;
          break;
        case Utilities::str_to_constexpr("STSINVERT") :
          data.solverOpts.st_type = STSINVERT;
          break;
        case Utilities::str_to_constexpr("STCAYLEY") :
          data.solverOpts.st_type = STCAYLEY;
          break;
        case Utilities::str_to_constexpr("STPRECOND") :
          data.solverOpts.st_type = STPRECOND;
          break;
        default:
          std::cout<<"Error while reading"<<"Spectral transformation"<<" with value "
                   <<str<<std::endl;
          DebugStop();
      }

      data.solverOpts.eps_verbose = prm.get_bool("Solver verbosity");//bool
    }
    prm.leave_subsection();

    prm.enter_subsection("Linear solver(KSP)");
    {
      str = prm.get("Linear solver");
      switch (Utilities::str_to_constexpr(str.c_str())) {
        case Utilities::str_to_constexpr("KSPRICHARDSON") :
          data.solverOpts.st_solver = KSPRICHARDSON;
          break;
        case Utilities::str_to_constexpr("KSPCHEBYSHEV") :
          data.solverOpts.st_solver = KSPCHEBYSHEV;
          break;
        case Utilities::str_to_constexpr("KSPCG") :
          data.solverOpts.st_solver = KSPCG;
          break;
        case Utilities::str_to_constexpr("KSPGROPPCG") :
          data.solverOpts.st_solver = KSPGROPPCG;
          break;
        case Utilities::str_to_constexpr("KSPPIPECG") :
          data.solverOpts.st_solver = KSPPIPECG;
          break;
        case Utilities::str_to_constexpr("KSPPIPECGRR") :
          data.solverOpts.st_solver = KSPPIPECGRR;
          break;
        case Utilities::str_to_constexpr("KSPCGNE") :
          data.solverOpts.st_solver = KSPCGNE;
          break;
        case Utilities::str_to_constexpr("KSPFCG") :
          data.solverOpts.st_solver = KSPFCG;
          break;
        case Utilities::str_to_constexpr("KSPPIPEFCG") :
          data.solverOpts.st_solver = KSPPIPEFCG;
          break;
        case Utilities::str_to_constexpr("KSPGMRES") :
          data.solverOpts.st_solver = KSPGMRES;
          break;
        case Utilities::str_to_constexpr("KSPPIPEFGMRES") :
          data.solverOpts.st_solver = KSPPIPEFGMRES;
          break;
        case Utilities::str_to_constexpr("KSPFGMRES") :
          data.solverOpts.st_solver = KSPFGMRES;
          break;
        case Utilities::str_to_constexpr("KSPLGMRES") :
          data.solverOpts.st_solver = KSPLGMRES;
          break;
        case Utilities::str_to_constexpr("KSPDGMRES") :
          data.solverOpts.st_solver = KSPDGMRES;
          break;
        case Utilities::str_to_constexpr("KSPPGMRES") :
          data.solverOpts.st_solver = KSPPGMRES;
          break;
        case Utilities::str_to_constexpr("KSPTCQMR") :
          data.solverOpts.st_solver = KSPTCQMR;
          break;
        case Utilities::str_to_constexpr("KSPBCGS") :
          data.solverOpts.st_solver = KSPBCGS;
          break;
        case Utilities::str_to_constexpr("KSPIBCGS") :
          data.solverOpts.st_solver = KSPIBCGS;
          break;
        case Utilities::str_to_constexpr("KSPFBCGS") :
          data.solverOpts.st_solver = KSPFBCGS;
          break;
        case Utilities::str_to_constexpr("KSPFBCGSR") :
          data.solverOpts.st_solver = KSPFBCGSR;
          break;
        case Utilities::str_to_constexpr("KSPBCGSL") :
          data.solverOpts.st_solver = KSPBCGSL;
          break;
        case Utilities::str_to_constexpr("KSPCGS") :
          data.solverOpts.st_solver = KSPCGS;
          break;
        case Utilities::str_to_constexpr("KSPTFQMR") :
          data.solverOpts.st_solver = KSPTFQMR;
          break;
        case Utilities::str_to_constexpr("KSPCR") :
          data.solverOpts.st_solver = KSPCR;
          break;
        case Utilities::str_to_constexpr("KSPPIPECR") :
          data.solverOpts.st_solver = KSPPIPECR;
          break;
        case Utilities::str_to_constexpr("KSPLSQR") :
          data.solverOpts.st_solver = KSPLSQR;
          break;
        case Utilities::str_to_constexpr("KSPPREONLY") :
          data.solverOpts.st_solver = KSPPREONLY;
          break;
        default:
          std::cout << "Error while reading" << "Linear solver" << " with value "
                    << str << std::endl;
          DebugStop();
      }

      data.solverOpts.ksp_rtol = prm.get_double("Linear solver relative tolerance");//double
      if (!data.solverOpts.ksp_rtol) data.solverOpts.ksp_rtol = -2;//PETSC_DEFAULT
      data.solverOpts.ksp_atol = prm.get_double("Linear solver absolute tolerance");//double
      if (!data.solverOpts.ksp_atol) data.solverOpts.ksp_atol = -2;//PETSC_DEFAULT
      data.solverOpts.ksp_dtol = prm.get_double("Linear solver divergence tolerance");//double
      if (!data.solverOpts.ksp_dtol) data.solverOpts.ksp_dtol = -2;//PETSC_DEFAULT
      data.solverOpts.ksp_max_its = (int) prm.get_integer("Linear solver maximum iterations");//integer
      if (!data.solverOpts.ksp_max_its) data.solverOpts.ksp_max_its = -2;//PETSC_DEFAULT
      str = prm.get("Preconditioner");
      switch (Utilities::str_to_constexpr(str.c_str())) {
        case Utilities::str_to_constexpr("PCNONE") :
          data.solverOpts.st_precond = PCNONE;
          break;
        case Utilities::str_to_constexpr("PCCHOLESKY") :
          data.solverOpts.st_precond = PCCHOLESKY;
          break;
        case Utilities::str_to_constexpr("PCJACOBI") :
          data.solverOpts.st_precond = PCJACOBI;
          break;
        case Utilities::str_to_constexpr("PCSOR") :
          data.solverOpts.st_precond = PCSOR;
          break;
        case Utilities::str_to_constexpr("PCLU") :
          data.solverOpts.st_precond = PCLU;
          break;
        case Utilities::str_to_constexpr("PCSHELL") :
          data.solverOpts.st_precond = PCSHELL;
          break;
        case Utilities::str_to_constexpr("PCBJACOBI") :
          data.solverOpts.st_precond = PCBJACOBI;
          break;
        case Utilities::str_to_constexpr("PCMG") :
          data.solverOpts.st_precond = PCMG;
          break;
        case Utilities::str_to_constexpr("PCEISENSTAT") :
          data.solverOpts.st_precond = PCEISENSTAT;
          break;
        case Utilities::str_to_constexpr("PCILU") :
          data.solverOpts.st_precond = PCILU;
          break;
        case Utilities::str_to_constexpr("PCICC") :
          data.solverOpts.st_precond = PCICC;
          break;
        case Utilities::str_to_constexpr("PCASM") :
          data.solverOpts.st_precond = PCASM;
          break;
        case Utilities::str_to_constexpr("PCGASM") :
          data.solverOpts.st_precond = PCGASM;
          break;
        case Utilities::str_to_constexpr("PCKSP") :
          data.solverOpts.st_precond = PCKSP;
          break;
        case Utilities::str_to_constexpr("PCCOMPOSITE") :
          data.solverOpts.st_precond = PCCOMPOSITE;
          break;
        case Utilities::str_to_constexpr("PCREDUNDANT") :
          data.solverOpts.st_precond = PCREDUNDANT;
          break;
        default:
          std::cout << "Error while reading" << "Preconditioner" << " with value "
                    << str << std::endl;
          DebugStop();
      }
    }
  }
  prm.leave_subsection();
  #ifdef PZDEBUGPARAM
  prm.print_parameters(std::cout,ParameterHandler::Text);
  #endif
}

void SPZModalAnalysisDataReader::ReadComplexVector(const int &nEntries, const std::string &rName, const std::string &iName,
                                             const std::string &condName, TPZVec<STATE> &dest) {
  std::string rawVec = prm.get(rName);//double
  const std::vector<std::string> split_list =
      Utilities::split_string_list(rawVec, ",");
  if(nEntries != split_list.size() ){
    std::cout<<"Input data error while reading "<<rName<<".\n";
    DebugStop();
  }
  dest.Resize(nEntries);
  for (int i = 0; i < split_list.size() ; i++){
    const std::string & string = split_list[i];
    dest[i] = (STATE)Utilities::string_to_double(string);
  }
  if(!prm.get_bool(condName)){
    rawVec = prm.get(iName);
    const std::vector<std::string> split_list =
        Utilities::split_string_list(rawVec, ",");
    if(nEntries != split_list.size() ){
      std::cout<<"Input data error while reading "<<iName<<"\n";
      DebugStop();
    }
    for (int i = 0; i < split_list.size() ; i++){
      const std::string & string = split_list[i];
      dest[i] += sqrt(std::complex<double>(-1)) * (STATE)Utilities::string_to_double(string);
    }
  }
}

void SPZModalAnalysisDataReader::PrintUsageMessage() {
  static const char *message
      =
      "\n"
          "Example of reading parameters from a text file.\n"
          "\n"
          "Usage:\n"
          "    ./newModalAnalysis file \n"
          "    ./newModalAnalysis -p | --print <defaultfile> \n"
          "    ./newModalAnalysis -h | --help\n"
          "\n"
          "Options:\n"
          "    -p --print Print default values to the file defaultfile\n"
          "    -h --help  Shows this usage message. \n"
          "\n"
          "The parameter file has the following format and allows the following\n"
          "values:\n"
          "\n";
  std::cout << message;
  prm.print_parameters (std::cout, ParameterHandler::Text);
}

void SPZModalAnalysisDataReader::ParseCommandLine(const int argc, char *const *argv) {
  if(argc == 1){
    std::cout<<"Input parameters file: "<<std::endl;
    std::string parameter_file;
    std::cin >> parameter_file;
    if(!FileExists(parameter_file)){
      PZError<<"Invalid parameter file."<<std::endl;
      DebugStop();
    }
    std::vector<std::string> str_list_path = Utilities::split_string_list(parameter_file,'/');
    path = "";
    for (int i = 0; i < str_list_path.size()-1; ++i) {
      path += str_list_path[i] + "/";
    }
    prm.parse_input (parameter_file);
    return;
  }

  if (argc > 3)
  {
    PrintUsageMessage();
    exit (1);
  }
  //testing for -p or --print
  if(std::string(argv[1]) == std::string("-p")
     ||
     std::string(argv[1]) == std::string("--print")){
    if(argc != 3){
      PrintUsageMessage();
      exit(1);
    }
    std::ofstream outFile(argv[2]);
    prm.print_parameters (outFile, ParameterHandler::Text);
    outFile.close();
    exit(0);
  }

  //testing for -h or --help
  if(std::string(argv[1]) == std::string("-h")
     ||
     std::string(argv[1]) == std::string("--help")){
    PrintUsageMessage();
    if(argc != 2){
      exit(1);
    }
    exit(0);
  }

  const std::string parameter_file = argv[1];
  if(!FileExists(parameter_file)){
    PZError<<"Invalid parameter file."<<std::endl;
    DebugStop();
  }
  std::vector<std::string> str_list_path = Utilities::split_string_list(parameter_file,'/');
  path = "";
  for (int i = 0; i < str_list_path.size()-1; ++i) {
    path += str_list_path[i] + "/";
  }
  prm.parse_input (parameter_file);
}

bool SPZModalAnalysisDataReader::FileExists(const std::string &name) const {
  if (FILE *file = fopen(name.c_str(), "r")) {
    fclose(file);
    return true;
  } else {
    return false;
  }
}

const std::string &SPZModalAnalysisDataReader::GetPath() const {
    return path;
}
