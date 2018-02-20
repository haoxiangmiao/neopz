
#include <complex.h>
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
  prm.enter_subsection ("Physical options");
  {
    prm.declare_entry("01 - Mesh file", "",
                      Patterns::Anything(),
                      "Path to .msh gmsh generated mesh");
    prm.declare_entry("02 - Cut-off analysis", "false",
                      Patterns::Bool(),
                      "Whether to perform cut-off analysis on the waveguide or"
                          "to calculate its modes for a given "
                          "operational frequency");
    prm.declare_entry("03 - Operational frequency", "25e+9",
                      Patterns::Double(0.),
                      "Operational frequency of the waveguide (it will "
                          "be ignored if 2Cut-off analysis is true");
    prm.declare_entry("04 - Number of materials", "1",
                      Patterns::Integer(1.),
                      "How many dielectric materials are present "
                          "in the waveguide");
    prm.declare_entry("05 - Electric permittivity vector", "1.",
                      Patterns::List(Patterns::Double(0.),1),
                      "The REAL part of the electric permittivity of "
                          "the dielectric materials, separated by commas");
    prm.declare_entry("06 - Is lossless(epsilon)","true", Patterns:: Bool(),
                      "Whether the dielectric material has complex "
                          "permittivity or not.");
    prm.declare_entry("07 - Dielectric losses vector(epsilon)", "0.",
                      Patterns::List(Patterns::Double(0.),1),
                      "The IMAGINARY part of the electric permittivity"
                          " of the dielectric materials, separated by "
                          "commas (it will be ignored"
                          " if Is lossless(epsilon) is false");
    prm.declare_entry("08 - Magnetic permeability vector", "1.",
                      Patterns::List(Patterns::Double(0.),1),
                      "The REAL part of the magnetic permeability of the "
                          "dielectric materials, separated by commas");
    prm.declare_entry("09 - Is lossless(mu)","true", Patterns:: Bool(),
                      "Whether the dielectric material has complex permeability or not.");
    prm.declare_entry("10 - Dielectric losses vector(mu)", "0.",
                      Patterns::List(Patterns::Double(0.),1),
                      "The IMAGINARY part of the magnetic permeability"
                          " of the dielectric materials, "
                          "separated by commas (it will be ignored"
                          " if Is lossless(mu) is false");
  }
  prm.leave_subsection ();

  prm.enter_subsection("NeoPZ options");
  {
    prm.declare_entry("01 - Polynomial order","1", Patterns::Integer(1),
                      "Default polynomial order of the Pk space used to build"
                          "the Nédélec elements(The H1 elements"
                          " will be built accordingly");
    prm.declare_entry("02 - Generate VTK","false", Patterns::Bool(),
                      "If set to true, a .vtk plot of the electric field will be generated"
                          "for the calculated modes");
    prm.declare_entry("03 - L2 error","false", Patterns::Bool(),
                      "If set to true, the error (L2 norm) will be calculated"
                          " for the electric field");
    prm.declare_entry("04 - Export L2 error","false", Patterns::Bool(),
                      "File name in which the L2 error will be saved "
                          "if L2 error is true. If not set, it will "
                          "be shown in std::cout only");
    prm.declare_entry("05 - Export eigenvalues","false",Patterns::Bool(),
                      "If set to true, eigenvalues will"
                          "be exported to a text file.");
    prm.declare_entry("06 - Number of threads","4",Patterns::Integer(0),
                      "Number of threads to use in NeoPZ assembly.");
    prm.declare_entry("07 - Prefix","",Patterns::Anything(),
                      "Prefix to be added to exported files(default is 1Mesh File)");
    prm.declare_entry("08 - Abs|Re","Re",Patterns::Selection("Abs|Re"),
                      "Whether to export magnitude or real part of eigenvector");
    prm.declare_entry("09 - VTK resolution","0",Patterns::Integer(0),
                      "Resolution to be used during post-processing.");
    prm.declare_entry("10 - Export geomesh","false",Patterns::Bool(),
                      "Whether to export the geometrical mesh in both .txt and .vtk formats.");
    prm.declare_entry("11 - Export compmesh","false",Patterns::Bool(),
                      "Whether to export the computational mesh in both .txt and .vtk formats.");
    prm.declare_entry("12 - Scale factor","1.",Patterns::Double(0),
                      "Maximum dimension for geometric domain(frequency will be scaled)");
  }
  prm.leave_subsection ();
  //IN THE FOLLOWING OPTIONS, -1 =  PETSC_DECIDE and -2 = PETSC_DEFAULT
  prm.enter_subsection("SLEPc solver options");
  {
    prm.declare_entry("01 - Problem type", "EPS_NHEP",
                      Patterns::Selection("EPS_HEP|EPS_GHEP|EPS_NHEP"
                                              "|EPS_GNHEP|EPS_PGNHEP|EPS_GHIEP"),
                      "Sets the type of the eigenvalue problem type.\n"
                          "G stands for Generalized, H for Hermitian,\n"
                          "P for positive semi-definite B matrix,\n"
                          "and I for generalized Hermitian-indefinite.");
    prm.declare_entry("02 - Eigensolver","EPSKRYLOVSCHUR",Patterns::Selection("EPSPOWER|EPSSUBSPACE|EPSARNOLDI|EPSLANCZOS|"
                                                                             "EPSKRYLOVSCHUR|EPSGD|EPSJD|EPSRQCG|"
                                                                             "EPSLOBPCG|EPSCISS|EPSLAPACK|EPSARPACK|"
                                                                             "EPSBLZPACK|EPSTRLAN|EPSBLOPEX|"
                                                                             "EPSPRIMME|EPSFEAST"),
    "Sets the particular eigensolver to be used");
    prm.declare_entry("03 - Krylov locking","false",Patterns::Bool(),"If Eigensolver is Krylov-Schur,"
        " whether to use the locking variant or not.");
    prm.declare_entry("04 - Krylov restart","0.5",Patterns::Double(0.,1.),"Sets the restart parameter for the "
        "Krylov-Schur method, in particular the proportion of basis vectors that must be kept after restart.");
    prm.declare_entry("05 - Convergence test", "EPS_CONV_REL",
                      Patterns::Selection("EPS_CONV_ABS|EPS_CONV_REL"
                                              "|EPS_CONV_NORM"),
                      "Specifies how to compute the error"
                      " estimate used in the convergence test.");
    prm.declare_entry("06 - True residual","false",Patterns::Bool(),"Whether to calculate the true residual"
        " if using shift-and-invert as a spectral transformation.");
    prm.declare_entry("07 - Which eigenvalues", "EPS_LARGEST_MAGNITUDE",
                      Patterns::Selection("EPS_LARGEST_MAGNITUDE|EPS_SMALLEST_MAGNITUDE"
                                              "|EPS_LARGEST_REAL|EPS_SMALLEST_REAL|"
                                              "EPS_LARGEST_IMAGINARY|EPS_SMALLEST_IMAGINARY|"
                                              "EPS_TARGET_MAGNITUDE|EPS_TARGET_REAL|"
                                              "EPS_TARGET_IMAGINARY|EPS_ALL"),
                      "Specifies which portion of the spectrum is to be sought.");
    prm.declare_entry("08 - Target eigenvalue", "-1.", Patterns::Double(),
                      "Target eigenvalue to be sought if Which eigenvalues is equal "
                          "to EPS_TARGET_(MAGNITUDE|IMAGINARY|REAL).");
    prm.declare_entry("09 - Eigensolver tolerance", "0", Patterns::Double(-1.e-5),
                      "Eigensolver convergence tolerance(0 for PETSC_DEFAULT).");
    prm.declare_entry("10 - Eigensolver maximum iterations", "0", Patterns::Integer(0),
                      "Maximum number of iterations of the eigensolver(0 for PETSC_DEFAULT).");
    prm.declare_entry("11 - How many eigenvalues(nev)", "5", Patterns::Integer(1),
                      "Number of eigenvalues that must satisfy the convergence test");
    prm.declare_entry("12 - Solver subspace dimension(ncv)", "0", Patterns::Integer(0),
                      "The maximum dimension of the subspace to be "
                          "used by the eigensolver(0 for PETSC_DECIDE)");
    prm.declare_entry("13 - Maximum projected dimension(mpd)", "0", Patterns::Integer(0),
                      "The maximum projected dimension of the "
                          "eigensolver(0 for PETSC_DECIDE)");
    prm.declare_entry("14 - Solver verbosity", "false", Patterns::Bool(),
                      "Whether the solver should print data during the simulation.");
    prm.declare_entry("15 - Preconditioner","PCREDUNDANT",Patterns::Selection("PCNONE|PCCHOLESKY|PCJACOBI|PCSOR|PCLU|"
                                                                      "PCSHELL|PCBJACOBI|PCMG|PCEISENSTAT|PCILU|"
                                                                      "PCICC|PCASM|PCGASM|PCKSP|"
                                                                      "PCCOMPOSITE|PCREDUNDANT"),
                      "Sets the preconditioner to be used.");
    prm.declare_entry("16 - Linear solver","KSPPREONLY",Patterns::Selection("KSPRICHARDSON|KSPCHEBYSHEV|KSPCG|KSPGROPPCG|"
                                                                           "KSPPIPECG|KSPPIPECGRR|KSPCGNE|KSPCGNASH|"
                                                                           "KSPCGSTCG|KSPCGGLTR|KSPFCG|KSPPIPEFCG|"
                                                                           "KSPGMRES|KSPPIPEFGMRES|KSPFGMRES|"
                                                                           "KSPLGMRES|KSPDGMRES|KSPPGMRES|KSPTCQMR|"
                                                                           "KSPBCGS|KSPIBCGS|KSPFBCGS|KSPFBCGSR|"
                                                                           "KSPBCGSL|KSPPIPEBCGS|KSPCGS|KSPTFQMR|"
                                                                           "KSPCR|KSPPIPECR|KSPLSQR|KSPPREONLY"),
                      "Sets the preconditioner to be used.");
    prm.declare_entry("17 - Linear solver relative tolerance", "0", Patterns::Double(-1.e-5),
                      "Sets the relative convergence tolerance, relative decrease in"
                          " the residual norm(0 for PETSC_DEFAULT)");
    prm.declare_entry("18 - Linear solver absolute tolerance", "0", Patterns::Double(-1.e-5),
                      "Sets the absolute convergence tolerance absolute size of the "
                          "residual norm(0 for PETSC_DEFAULT)");
    prm.declare_entry("19 - Linear solver divergence tolerance", "0", Patterns::Double(-1.e-5),
                      "Sets the divergence tolerance, amount residual norm can increase before "
                          "the convergence test concludes that the method is diverging(0 for PETSC_DEFAULT)");
    prm.declare_entry("20 - Linear solver maximum iterations", "0", Patterns::Integer(0),
                      "Maximum number of iterations to use(0 for PETSC_DEFAULT)");
    prm.declare_entry("21 - Spectral transformation","STSINVERT",Patterns::Selection("STSHELL|STSHIFT|STSINVERT|"
                                                                                    "STCAYLEY|STPRECOND|STFILTER"),
                      "Sets the spectral transformation to be used.");

  }
  prm.leave_subsection ();
}

void SPZModalAnalysisDataReader::ReadParameters(SPZModalAnalysisData &data) {

  prm.enter_subsection ("Physical options");
  {
    data.physicalOpts.meshFile = path + prm.get("01 - Mesh file");//anything
    std::string &str = data.physicalOpts.meshFile;
    if(str.size() == 0 || str.substr(str.size()-4,4) != ".msh" || !FileExists(str)){
      std::cout<<"Must input a valid mesh file: "<<std::endl;
      std::cin >> data.physicalOpts.meshFile;
      if(str.size() == 0 || str.substr(str.size()-4,4) != ".msh" || !FileExists(str)) {
        std::cout<<"Not a valid name."<<std::endl;
        DebugStop();
      }
    }

    data.physicalOpts.isCutOff=prm.get_bool("02 - Cut-off analysis");//bool
    data.physicalOpts.fOp=(REAL)prm.get_double("03 - Operational frequency");//double
    data.physicalOpts.nMaterials=(int)prm.get_integer("04 - Number of materials");//integer
    ReadComplexVector(data.physicalOpts.nMaterials,"05 - Electric permittivity vector",
                      "07 - Dielectric losses vector(epsilon)","06 - Is lossless(epsilon)",
                      data.physicalOpts.erVec);
    ReadComplexVector(data.physicalOpts.nMaterials,"08 - Magnetic permeability vector",
                      "10 - Dielectric losses vector(mu)","09 - Is lossless(mu)",
                      data.physicalOpts.urVec);
  }
  prm.leave_subsection();
  prm.enter_subsection("NeoPZ options");
  {
    data.pzOpts.pOrder = (int) prm.get_integer("01 - Polynomial order");//integer
    data.pzOpts.genVTK = prm.get_bool("02 - Generate VTK");//bool
    data.pzOpts.l2error = prm.get_bool("03 - L2 error");//bool
    data.pzOpts.exportl2error = prm.get_bool("04 - Export L2 error");//bool
    data.pzOpts.exportEigen = prm.get_bool("05 - Export eigenvalues");//bool
    data.pzOpts.nThreads = (int) prm.get_integer("06 - Number of threads");//integer
    data.pzOpts.prefix = data.physicalOpts.meshFile.substr(0,data.physicalOpts.meshFile.size()-4);
    data.pzOpts.prefix += prm.get("07 - Prefix");//anything
    data.pzOpts.absVal = prm.get("08 - Abs|Re") == "Abs" ? true : false;//selection
    data.pzOpts.vtkRes = prm.get_integer("09 - VTK resolution");//integer
    data.pzOpts.exportGMesh = prm.get_bool("10 - Export geomesh");//bool
    data.pzOpts.exportCMesh = prm.get_bool("11 - Export compmesh");//bool
    data.pzOpts.scaleFactor = prm.get_double("12 - Scale factor");//double
  }
  prm.leave_subsection();
  prm.enter_subsection("SLEPc solver options");
  {
      std::string str = prm.get("01 - Problem type");
      switch(Utilities::str_to_constexpr(str.c_str())) {
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
          std::cout<<"Error while reading"<<"Problem type"<<" with value "
                   <<str<<std::endl;
          DebugStop();
      }

      str = prm.get("02 - Eigensolver");
      switch(Utilities::str_to_constexpr(str.c_str())){
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
          std::cout<<"Error while reading"<<"Eigensolver"<<" with value "
                   <<str<<std::endl;
          DebugStop();
      }
    data.solverOpts.eps_krylov_locking = prm.get_bool("03 - Krylov locking");//bool
    data.solverOpts.eps_krylov_restart = prm.get_double("04 - Krylov restart");//double
    str = prm.get("05 - Convergence test");
    switch(Utilities::str_to_constexpr(str.c_str())){
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
          std::cout<<"Error while reading"<<"Convergence test"<<" with value "
                   <<str<<std::endl;
          DebugStop();
    }
    data.solverOpts.eps_true_res = prm.get_bool("06 - True residual");//bool

    str =prm.get("07 - Which eigenvalues");
    switch(Utilities::str_to_constexpr(str.c_str())){
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
        std::cout<<"Error while reading"<<"Which eigenvalues"<<" with value "
                 <<str<<std::endl;
        DebugStop();
    }

    data.solverOpts.target = prm.get_double("08 - Target eigenvalue");//double
    data.solverOpts.eps_tol = prm.get_double("09 - Eigensolver tolerance");//double
    if(!data.solverOpts.eps_tol) data.solverOpts.eps_tol = -2;//PETSC_DEFAULT
    data.solverOpts.eps_max_its = (int) prm.get_integer("10 - Eigensolver maximum iterations");//integer
    if(!data.solverOpts.eps_max_its) data.solverOpts.eps_max_its = -2;//PETSC_DEFAULT
    data.solverOpts.eps_nev = (int) prm.get_integer("11 - How many eigenvalues(nev)");//integer
    data.solverOpts.eps_ncv = (int) prm.get_integer("12 - Solver subspace dimension(ncv)");//integer
    if(!data.solverOpts.eps_ncv) data.solverOpts.eps_ncv = -1;//PETSC_DECIDE
    data.solverOpts.eps_mpd = (int) prm.get_integer("13 - Maximum projected dimension(mpd)");//integer
    if(!data.solverOpts.eps_mpd) data.solverOpts.eps_mpd = -1;//PETSC_DECIDE
    data.solverOpts.eps_verbose = prm.get_bool("14 - Solver verbosity");//bool

    str =prm.get("15 - Preconditioner");
    switch(Utilities::str_to_constexpr(str.c_str())){
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
        std::cout<<"Error while reading"<<"Preconditioner"<<" with value "
                 <<str<<std::endl;
        DebugStop();
    }

    str =prm.get("16 - Linear solver");
    switch(Utilities::str_to_constexpr(str.c_str())){
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
        std::cout<<"Error while reading"<<"Linear solver"<<" with value "
                 <<str<<std::endl;
        DebugStop();
    }

    data.solverOpts.ksp_rtol = prm.get_double("17 - Linear solver relative tolerance");//double
    if(!data.solverOpts.ksp_rtol) data.solverOpts.ksp_rtol = -2;//PETSC_DEFAULT
    data.solverOpts.ksp_atol = prm.get_double("18 - Linear solver absolute tolerance");//double
    if(!data.solverOpts.ksp_atol) data.solverOpts.ksp_atol = -2;//PETSC_DEFAULT
    data.solverOpts.ksp_dtol = prm.get_double("19 - Linear solver divergence tolerance");//double
    if(!data.solverOpts.ksp_dtol) data.solverOpts.ksp_dtol = -2;//PETSC_DEFAULT
    data.solverOpts.ksp_max_its = (int) prm.get_integer("20 - Linear solver maximum iterations");//integer
    if(!data.solverOpts.ksp_max_its) data.solverOpts.ksp_max_its = -2;//PETSC_DEFAULT

    str =prm.get("21 - Spectral transformation");
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
