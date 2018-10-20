/********************************************************************************/
/*     888888    888888888   88     888  88888   888      888    88888888       */
/*   8       8   8           8 8     8     8      8        8    8               */
/*  8            8           8  8    8     8      8        8    8               */
/*  8            888888888   8   8   8     8      8        8     8888888        */
/*  8      8888  8           8    8  8     8      8        8            8       */
/*   8       8   8           8     8 8     8      8        8            8       */
/*     888888    888888888  888     88   88888     88888888     88888888        */
/*                                                                              */
/*       A Three-Dimensional General Purpose Semiconductor Simulator.           */
/*                                                                              */
/*                                                                              */
/*  Copyright (C) 2007-2008                                                     */
/*  Cogenda Pte Ltd                                                             */
/*                                                                              */
/*  Please contact Cogenda Pte Ltd for license information                      */
/*                                                                              */
/*  Author: Gong Ding   gdiso@ustc.edu                                          */
/*                                                                              */
/********************************************************************************/



#include <numeric>

#include "fem_linear_solver.h"
#include "parallel.h"


/*------------------------------------------------------------------
 * constructor, setup context
 */
FEM_LinearSolver::FEM_LinearSolver(SimulationSystem & system): FEM_PDESolver(system)
{}


/*------------------------------------------------------------------
 * setup nonlinear matrix/vector
 */
void FEM_LinearSolver::setup_linear_data()
{
  // map mesh to PETSC solver
  build_dof_map();

  // set petsc routine

  PetscErrorCode ierr;

  // create the global solution vector
  ierr = VecCreateMPI(PETSC_COMM_WORLD, n_local_dofs, n_global_dofs, &x); genius_assert(!ierr);
  // use VecDuplicate to create vector with same pattern
  ierr = VecDuplicate(x, &b);genius_assert(!ierr);
  ierr = VecDuplicate(x, &L);genius_assert(!ierr);

  // set all the components of scale vector L to 1.0
  ierr = VecSet(L, 1.0); genius_assert(!ierr);

  // create local vector, which has extra room for ghost dofs! the MPI_COMM here is PETSC_COMM_SELF
  ierr = VecCreateSeq(PETSC_COMM_SELF,  local_index_array.size(), &lx); genius_assert(!ierr);
  // use VecDuplicate to create vector with same pattern
  ierr = VecDuplicate(lx, &lb);genius_assert(!ierr);

  // create the index for vector statter
#if PETSC_VERSION_GE(3,2,0)
  ierr = ISCreateGeneral(PETSC_COMM_WORLD, global_index_array.size(), &global_index_array[0] , PETSC_COPY_VALUES, &gis); genius_assert(!ierr);
  ierr = ISCreateGeneral(PETSC_COMM_SELF,  local_index_array.size(),  &local_index_array[0] ,  PETSC_COPY_VALUES, &lis); genius_assert(!ierr);
#else
  ierr = ISCreateGeneral(PETSC_COMM_WORLD, global_index_array.size(), &global_index_array[0] , &gis); genius_assert(!ierr);
  ierr = ISCreateGeneral(PETSC_COMM_SELF,  local_index_array.size(),  &local_index_array[0] ,  &lis); genius_assert(!ierr);
#endif
  // it seems we can free global_index_array and local_index_array to save the memory

  // create the vector statter
#if defined(PETSC_HAVE_MPI_WIN_CREATE) && defined(AIX)
  //NOTE Scatter with default settings will crash on AIX6.1 with POE. vecscatter_window can be a workaround here.
  // this line should before VecScatterCreate
  ierr = set_petsc_option("-vecscatter_window","1"); genius_assert(!ierr);
#endif
  ierr = VecScatterCreate(x, gis, lx, lis, &scatter); genius_assert(!ierr);


  // create the matrix
  ierr = MatCreate(PETSC_COMM_WORLD, &A); genius_assert(!ierr);
  ierr = MatSetSizes(A, n_local_dofs, n_local_dofs, n_global_dofs, n_global_dofs); genius_assert(!ierr);


  // we are using petsc-devel
  if (Genius::n_processors()>1)
  {
    ierr = MatSetType(A,MATMPIAIJ); genius_assert(!ierr);
    // alloc memory for parallel matrix here
    ierr = MatMPIAIJSetPreallocation(A, 0, &n_nz[0], 0, &n_oz[0]); genius_assert(!ierr);
  }
  else
  {
    ierr = MatSetType(A,MATSEQAIJ); genius_assert(!ierr);
    // alloc memory for sequence matrix here
    ierr = MatSeqAIJSetPreallocation(A, 0, &n_nz[0]); genius_assert(!ierr);
  }


  // indicates when PetscUtils::MatZeroRows() is called the zeroed entries are kept in the nonzero structure
#if PETSC_VERSION_GE(3,1,0)
  ierr = MatSetOption(A, MAT_KEEP_NONZERO_PATTERN, PETSC_TRUE); genius_assert(!ierr);
#endif

#if PETSC_VERSION_EQ(3,0,0)
  ierr = MatSetOption(A, MAT_KEEP_ZEROED_ROWS, PETSC_TRUE); genius_assert(!ierr);
#endif

  // the matrix is not assembled yet.
  matrix_first_assemble = false;

  ierr = MatSetFromOptions(A); genius_assert(!ierr);

  // create petsc linear solver context
  ierr = KSPCreate(PETSC_COMM_WORLD, &ksp); genius_assert(!ierr);

  // set corresponding matrix
#if PETSC_VERSION_GE(3,5,0)
  ierr = KSPSetOperators(ksp,A,A); genius_assert(!ierr);
#else
  ierr = KSPSetOperators(ksp,A,A,SAME_NONZERO_PATTERN); genius_assert(!ierr);
#endif

  // get petsc preconditional context
  ierr = KSPGetPC(ksp, &pc); genius_assert(!ierr);

  ierr = KSPSetOptionsPrefix(ksp, this->ksp_prefix().c_str()); genius_assert(!ierr);

  // Set user-specified linear solver and preconditioner types
  set_petsc_linear_solver_type    ( SolverSpecify::LS );
  set_petsc_preconditioner_type   ( SolverSpecify::PC );

}


/*------------------------------------------------------------------
 * destroy nonlinear data
 */
void FEM_LinearSolver::clear_linear_data()
{
  PetscErrorCode ierr;
  // free everything
  ierr = VecDestroy(PetscDestroyObject(x));              genius_assert(!ierr);
  ierr = VecDestroy(PetscDestroyObject(b));              genius_assert(!ierr);
  ierr = VecDestroy(PetscDestroyObject(L));              genius_assert(!ierr);
  ierr = VecDestroy(PetscDestroyObject(lx));             genius_assert(!ierr);
  ierr = VecDestroy(PetscDestroyObject(lb));             genius_assert(!ierr);
  ierr = ISDestroy(PetscDestroyObject(gis));             genius_assert(!ierr);
  ierr = ISDestroy(PetscDestroyObject(lis));             genius_assert(!ierr);
  ierr = VecScatterDestroy(PetscDestroyObject(scatter)); genius_assert(!ierr);
  ierr = MatDestroy(PetscDestroyObject(A));              genius_assert(!ierr);
  ierr = KSPDestroy(PetscDestroyObject(ksp));            genius_assert(!ierr);

  // clear petsc options
  std::map<std::string, std::string>::const_iterator it = petsc_options.begin();
  for(; it != petsc_options.end(); ++it)
#if PETSC_VERSION_GE(3,7,0)
    PetscOptionsClearValue(PETSC_NULL, it->first.c_str());
#else
  PetscOptionsClearValue(it->first.c_str());
#endif
}


/*------------------------------------------------------------------
 * destructor: destroy context
 */
FEM_LinearSolver::~FEM_LinearSolver()
{

}






void FEM_LinearSolver::set_petsc_linear_solver_type(SolverSpecify::LinearSolverType linear_solver_type)
{
  int ierr = 0;

  _linear_solver_type = linear_solver_type;

  switch (linear_solver_type)
  {

    case SolverSpecify::CG:
      MESSAGE<< "Using CG linear solver..."<<std::endl;  RECORD();
      ierr = KSPSetType (ksp, (char*) KSPCG);         genius_assert(!ierr); return;

    case SolverSpecify::CR:
      MESSAGE<< "Using CR linear solver..."<<std::endl;  RECORD();
      ierr = KSPSetType (ksp, (char*) KSPCR);         genius_assert(!ierr); return;

    case SolverSpecify::CGS:
      MESSAGE<< "Using CGS linear solver..."<<std::endl;  RECORD();
      ierr = KSPSetType (ksp, (char*) KSPCGS);        genius_assert(!ierr); return;

    case SolverSpecify::BICG:
      MESSAGE<< "Using BICG linear solver..."<<std::endl;  RECORD();
      ierr = KSPSetType (ksp, (char*) KSPBICG);       genius_assert(!ierr); return;

    case SolverSpecify::TCQMR:
      MESSAGE<< "Using TCQMR linear solver..."<<std::endl;  RECORD();
      ierr = KSPSetType (ksp, (char*) KSPTCQMR);      genius_assert(!ierr); return;

    case SolverSpecify::TFQMR:
      MESSAGE<< "Using TFQMR linear solver..."<<std::endl;  RECORD();
      ierr = KSPSetType (ksp, (char*) KSPTFQMR);      genius_assert(!ierr); return;

    case SolverSpecify::LSQR:
      MESSAGE<< "Using LSQR linear solver..."<<std::endl;  RECORD();
      ierr = KSPSetType (ksp, (char*) KSPLSQR);       genius_assert(!ierr); return;

    case SolverSpecify::BICGSTAB:
      MESSAGE<< "Using BCGS linear solver..."<<std::endl;  RECORD();
      ierr = KSPSetType (ksp, (char*) KSPBCGS);       genius_assert(!ierr); return;

    case SolverSpecify::BCGSL:
      MESSAGE<< "Using BCGS(l) linear solver..."<<std::endl;  RECORD();
      ierr = KSPSetType (ksp, (char*) KSPBCGSL);      genius_assert(!ierr);
      //ierr = set_petsc_option("-ksp_bcgsl_ell", "4");  genius_assert(!ierr);
      return;

    case SolverSpecify::MINRES:
      MESSAGE<< "Using MINRES linear solver..."<<std::endl;  RECORD();
      ierr = KSPSetType (ksp, (char*) KSPMINRES);     genius_assert(!ierr); return;

    case SolverSpecify::GMRES:
      MESSAGE<< "Using GMRES linear solver..."<<std::endl;  RECORD();
      ierr = KSPSetType (ksp, (char*) KSPGMRES);      genius_assert(!ierr);
      // for GMRES method, we need to enlarge restart step (default is 30)
      ierr = KSPGMRESSetRestart(ksp, 100);            genius_assert(!ierr);
      return;

    case SolverSpecify::FGMRES:
      MESSAGE<< "Using FGMRES linear solver..."<<std::endl;  RECORD();
      ierr = KSPSetType (ksp, (char*) KSPFGMRES);      genius_assert(!ierr);
      // for GMRES method, we need to enlarge restart step (default is 30)
      ierr = KSPGMRESSetRestart(ksp, 100);            genius_assert(!ierr);
      return;

#if PETSC_VERSION_GE(3,2,0)
    case SolverSpecify::DGMRES:
      MESSAGE<< "Using DGMRES linear solver..."<<std::endl;  RECORD();
      ierr = KSPSetType (ksp, (char*) KSPDGMRES);      genius_assert(!ierr);
      // for GMRES method, we need to enlarge restart step (default is 30)
      ierr = KSPGMRESSetRestart(ksp, 50);            genius_assert(!ierr);
      return;
#endif

    case SolverSpecify::RICHARDSON:
      MESSAGE<< "Using RICHARDSON linear solver..."<<std::endl;  RECORD();
      ierr = KSPSetType (ksp, (char*) KSPRICHARDSON); genius_assert(!ierr); return;

    case SolverSpecify::CHEBYSHEV:
      MESSAGE<< "Using CHEBYSHEV linear solver..."<<std::endl;  RECORD();
      ierr = KSPSetType (ksp, "chebyshev");  genius_assert(!ierr); return;

    case SolverSpecify::LU:
    case SolverSpecify::UMFPACK:
    case SolverSpecify::SuperLU:
    case SolverSpecify::MUMPS:
    case SolverSpecify::PASTIX:
    case SolverSpecify::SuperLU_DIST:
      if (Genius::n_processors()>1)
      {
        switch(linear_solver_type)
        {
          case   SolverSpecify::LU :
          case   SolverSpecify::MUMPS :
            // if LU method is required, we should check if SuperLU_DIST/MUMPS is installed
            // the default parallel LU solver is set to MUMPS
#ifdef PETSC_HAVE_MUMPS
            MESSAGE<< "Using MUMPS linear solver..."<<std::endl;
            RECORD();
            ierr = KSPSetType (ksp, (char*) KSPPREONLY); genius_assert(!ierr);
            ierr = PCSetType  (pc, (char*) PCLU); genius_assert(!ierr);
            ierr = PCFactorSetMatSolverPackage (pc, "mumps"); genius_assert(!ierr);
            // required for eliminate INFO -9 error
            ierr = set_petsc_option("-mat_mumps_icntl_14", MUMPS_ICNTL_14, false);  genius_assert(!ierr);
            ierr = set_petsc_option("-mat_mumps_icntl_23", MUMPS_ICNTL_23, false);
#else
            MESSAGE<< "Warning:  no MUMPS solver configured, use BCGS instead!" << std::endl;
            RECORD();
            ierr = KSPSetType (ksp, (char*) KSPBCGSL);  genius_assert(!ierr);
            ierr = PCSetType (pc, (char*) PCASM);       genius_assert(!ierr);
            return;
#endif
            break;

          case SolverSpecify::PASTIX:
#ifdef PETSC_HAVE_PASTIX
            MESSAGE<< "Using PaStiX linear solver..."<<std::endl;
            RECORD();
            ierr = KSPSetType (ksp, (char*) KSPPREONLY); genius_assert(!ierr);
            ierr = PCSetType  (pc, (char*) PCLU); genius_assert(!ierr);
            ierr = PCFactorSetMatSolverPackage (pc, "pastix"); genius_assert(!ierr);
#else
            MESSAGE<< "Warning:  no PaStiX solver configured, use BCGS instead!" << std::endl;
            RECORD();
            ierr = KSPSetType (ksp, (char*) KSPBCGSL);  genius_assert(!ierr);
            ierr = PCSetType (pc, (char*) PCASM);       genius_assert(!ierr);
            return;
#endif
            break;

          case SolverSpecify::SuperLU_DIST:
#ifdef PETSC_HAVE_SUPERLU_DIST
            MESSAGE<< "Using SuperLU_DIST linear solver..."<<std::endl;
            RECORD();
            ierr = KSPSetType (ksp, (char*) KSPPREONLY); genius_assert(!ierr);
            ierr = PCSetType  (pc, (char*) PCLU); genius_assert(!ierr);
            ierr = PCFactorSetMatSolverPackage (pc, "superlu_dist"); genius_assert(!ierr);
#else
            MESSAGE<< "Warning:  no SuperLU_DIST solver configured, use BCGS instead!" << std::endl;
            RECORD();
            ierr = KSPSetType (ksp, (char*) KSPBCGSL);  genius_assert(!ierr);
            ierr = PCSetType (pc, (char*) PCASM);       genius_assert(!ierr);
            return;
#endif
            break;
          default:
            ierr = KSPSetType (ksp, (char*) KSPBCGSL);  genius_assert(!ierr);
            ierr = PCSetType (pc, (char*) PCASM);       genius_assert(!ierr);
            return;
        }
      }
      else
      {
        ierr = KSPSetType (ksp, (char*) KSPPREONLY); genius_assert(!ierr);
        ierr = PCSetType  (pc, (char*) PCLU); genius_assert(!ierr);
        switch (linear_solver_type)
        {
          case SolverSpecify::LU :
          case SolverSpecify::MUMPS :
#ifdef PETSC_HAVE_MUMPS
            MESSAGE<< "Using MUMPS linear solver..."<<std::endl;
            RECORD();
            ierr = PCFactorSetMatSolverPackage (pc, "mumps"); genius_assert(!ierr);
            ierr = set_petsc_option("-mat_mumps_icntl_14",MUMPS_ICNTL_14,false);  genius_assert(!ierr);
            ierr = set_petsc_option("-mat_mumps_icntl_23",MUMPS_ICNTL_23,false);  genius_assert(!ierr);
#else
            MESSAGE << "Warning:  no MUMPS solver configured, use default LU solver instead!" << std::endl;
            RECORD();
#endif
            break;

          case SolverSpecify::UMFPACK :
#ifdef PETSC_HAVE_UMFPACK
            MESSAGE<< "Using UMFPACK linear solver..."<<std::endl;
            RECORD();
            ierr = PCFactorSetMatSolverPackage (pc, "umfpack"); genius_assert(!ierr);
#else
            MESSAGE << "Warning:  no UMFPACK solver configured, use default LU solver instead!" << std::endl;
            RECORD();
#endif
            break;

          case SolverSpecify::SuperLU :
#ifdef PETSC_HAVE_SUPERLU
            MESSAGE<< "Using SuperLU linear solver..."<<std::endl;
            RECORD();
            ierr = PCFactorSetMatSolverPackage (pc, "superlu"); genius_assert(!ierr);
#else
            MESSAGE << "Warning:  no SuperLU solver configured, use default LU solver instead!" << std::endl;
            RECORD();
#endif
            break;

          case SolverSpecify::PASTIX:
#ifdef PETSC_HAVE_PASTIX
            MESSAGE<< "Using PaStiX linear solver..."<<std::endl;
            RECORD();
            ierr = PCFactorSetMatSolverPackage (pc, "pastix"); genius_assert(!ierr);
#else
            MESSAGE<< "Warning:  no PaStiX solver configured, use default LU solver instead!" << std::endl;
            RECORD();
#endif
            break;

          case   SolverSpecify::SuperLU_DIST:
#ifdef PETSC_HAVE_SUPERLU_DIST
            MESSAGE<< "Using SuperLU_DIST linear solver..."<<std::endl;
            RECORD();
            ierr = PCFactorSetMatSolverPackage (pc, "superlu_dist"); genius_assert(!ierr);
#else
            MESSAGE << "Warning:  no SuperLU_DIST solver configured, use default LU solver instead!" << std::endl;
            RECORD();
#endif
            break;

          default:
            // should never reach here
            genius_error();
        }
      }


      ierr = PCFactorSetReuseFill(pc, PETSC_TRUE);genius_assert(!ierr);
      ierr = PCFactorSetReuseOrdering(pc, PETSC_TRUE); genius_assert(!ierr);
      // prevent zero pivot in LU factorization
      ierr = PCFactorSetColumnPivot(pc, 1.0); genius_assert(!ierr);
      //ierr = PCFactorReorderForNonzeroDiagonal(pc, 1e-20); genius_assert(!ierr);<-- Caught signal number 11 SEGV error will occure when diag value < 1e-20
      ierr = PCFactorSetShiftType(pc,MAT_SHIFT_NONZERO);genius_assert(!ierr);

      return;

    default:
      std::cerr << "ERROR:  Unsupported PETSC Solver: "
          << linear_solver_type        << std::endl
          << "Continuing with PETSC defaults" << std::endl;
  }

}



void FEM_LinearSolver::set_petsc_preconditioner_type(SolverSpecify::PreconditionerType preconditioner_type)
{
  int ierr = 0;

  // skip direct methods
  if (_linear_solver_type == SolverSpecify::LU ||
      _linear_solver_type == SolverSpecify::UMFPACK ||
      _linear_solver_type == SolverSpecify::SuperLU ||
      _linear_solver_type == SolverSpecify::MUMPS   ||
      _linear_solver_type == SolverSpecify::PASTIX  ||
      _linear_solver_type == SolverSpecify::SuperLU_DIST
     )
  {
    return;
  }

  _preconditioner_type = preconditioner_type;

  switch (preconditioner_type)
  {
    case SolverSpecify::IDENTITY_PRECOND:
      ierr = PCSetType (pc, (char*) PCNONE);      genius_assert(!ierr); return;

    case SolverSpecify::CHOLESKY_PRECOND:
      ierr = PCSetType (pc, (char*) PCCHOLESKY);  genius_assert(!ierr); return;

    case SolverSpecify::ICC_PRECOND:
      ierr = PCSetType (pc, (char*) PCICC);       genius_assert(!ierr); return;

    case SolverSpecify::ILU_PRECOND:
      if (Genius::n_processors()>1)
      {
#ifdef PETSC_HAVE_LIBHYPRE
      MESSAGE<< "Using Hypre/Euclid ILU preconditioner..."<<std::endl;
      RECORD();
      ierr = PCSetType (pc, (char*) PCHYPRE);     genius_assert(!ierr);
      ierr = PCHYPRESetType (pc, "euclid");       genius_assert(!ierr);
      return;
#else
      MESSAGE << "Warning:  no parallel ILU preconditioner configured, use ASM instead!" << std::endl;
      RECORD();
      ierr = PCSetType (pc, (char*) PCASM);       genius_assert(!ierr);
      return;
#endif

      }
      else
      {
#ifdef PETSC_HAVE_LIBHYPRE
      MESSAGE<< "Using Hypre/Euclid ILU preconditioner..."<<std::endl;
      RECORD();
      ierr = PCSetType (pc, (char*) PCHYPRE);     genius_assert(!ierr);
      ierr = PCHYPRESetType (pc, "euclid");       genius_assert(!ierr);
      return;
#else
      ierr = PCSetType (pc, (char*) PCILU);            genius_assert(!ierr);
      ierr = PCFactorSetReuseFill(pc, PETSC_TRUE);     genius_assert(!ierr);
      ierr = PCFactorSetReuseOrdering(pc, PETSC_TRUE); genius_assert(!ierr);
      ierr = PCFactorSetColumnPivot(pc, 1.0); genius_assert(!ierr);
      ierr = PCFactorSetShiftType(pc,MAT_SHIFT_NONZERO);genius_assert(!ierr);
      //ierr = PCFactorSetMatOrderingType(pc, MATORDERING_ND); genius_assert(!ierr);
      //ierr = PCFactorSetMatOrderingType(pc, MATORDERING_RCM);
      //ierr = set_petsc_option("-pc_factor_nonzeros_along_diagonal", 0); genius_assert(!ierr);
#if PETSC_VERSION_GE(3, 6, 0)        
      ierr = PCFactorSetAllowDiagonalFill(pc, PETSC_TRUE);genius_assert(!ierr);
#else
      ierr = PCFactorSetAllowDiagonalFill(pc);genius_assert(!ierr);
#endif      
      //ierr = set_petsc_option("-pc_factor_diagonal_fill", 0);
      return;
#endif
      }

    case SolverSpecify::ILUT_PRECOND:
      if (Genius::n_processors()==1)
      {
#ifdef PETSC_HAVE_SUPERLU
        MESSAGE<< "Using SuperLU ILUT preconditioner..."<<std::endl;
        RECORD();
        ierr = PCSetType (pc, (char*) PCILU);     genius_assert(!ierr);
        ierr = PCFactorSetMatSolverPackage (pc, "superlu"); genius_assert(!ierr);
        ierr = set_petsc_option("-mat_superlu_rowperm","LargeDiag", false); genius_assert(!ierr);
        ierr = set_petsc_option("-mat_superlu_ilu_droptol","1e-4", false); genius_assert(!ierr);
        ierr = set_petsc_option("-mat_superlu_ilu_filltol","1e-2", false); genius_assert(!ierr);
        ierr = set_petsc_option("-mat_superlu_ilu_fillfactor","30", false); genius_assert(!ierr);
        return;
#else
        MESSAGE << "Warning:  no ILUT preconditioner configured, use ILU0 instead!" << std::endl;
        RECORD();
        ierr = PCSetType (pc, (char*) PCASM);       genius_assert(!ierr);
        return;
#endif
      }
      else
      {
#ifdef PETSC_HAVE_SUPERLU
        MESSAGE<< "Using ASM + SuperLU ILUT preconditioner..."<<std::endl;
        RECORD();
        ierr = PCSetType (pc, (char*) PCASM);       genius_assert(!ierr);
        ierr = set_petsc_option("-sub_pc_type","ilu"); genius_assert(!ierr);
        ierr = set_petsc_option("-sub_pc_factor_mat_solver_package","superlu"); genius_assert(!ierr);
        ierr = set_petsc_option("-mat_superlu_rowperm","LargeDiag", false); genius_assert(!ierr);
        ierr = set_petsc_option("-mat_superlu_ilu_droptol","1e-4", false); genius_assert(!ierr);
        ierr = set_petsc_option("-mat_superlu_ilu_filltol","1e-2", false); genius_assert(!ierr);
        ierr = set_petsc_option("-mat_superlu_ilu_fillfactor","30", false); genius_assert(!ierr);
        //ierr = set_petsc_option("-mat_superlu_ilu_milu","2", false); genius_assert(!ierr);
        //ierr = set_petsc_option("-mat_superlu_replacetinypivot","1", false); genius_assert(!ierr);
        return;
#else
        MESSAGE << "Warning:  no ILUT preconditioner configured, use ILU0 instead!" << std::endl;
        RECORD();
        ierr = PCSetType (pc, (char*) PCASM);       genius_assert(!ierr);
        return;
#endif
      }

    // some times, we still need a LU solver as strong preconditioner
    case SolverSpecify::LU_PRECOND :
    {
      if (_linear_solver_type != SolverSpecify::GMRES )
      {
        MESSAGE << "Warning:  Set Linear solver to GMRES with LU preconditioner!" << std::endl;
        RECORD();
        _linear_solver_type = SolverSpecify::GMRES;
        ierr = KSPSetType (ksp, (char*) KSPGMRES);      genius_assert(!ierr);
      }

      if (Genius::n_processors()==1)
      {
        ierr = PCSetType (pc, (char*) PCLU);       genius_assert(!ierr);
#ifdef PETSC_HAVE_MUMPS
        MESSAGE<< "Using MUMPS as LU preconditioner..."<<std::endl;    RECORD();
        ierr = PCFactorSetMatSolverPackage (pc, "mumps"); genius_assert(!ierr);
        ierr = set_petsc_option("-mat_mumps_icntl_14",MUMPS_ICNTL_14,false);  genius_assert(!ierr);
        ierr = set_petsc_option("-mat_mumps_icntl_23",MUMPS_ICNTL_23,false);  genius_assert(!ierr);
#endif
        ierr = PCFactorSetReuseFill(pc, PETSC_TRUE);genius_assert(!ierr);
        ierr = PCFactorSetReuseOrdering(pc, PETSC_TRUE); genius_assert(!ierr);
        ierr = PCFactorSetColumnPivot(pc, 1.0); genius_assert(!ierr);
        ierr = PCFactorSetShiftType(pc, MAT_SHIFT_NONZERO);genius_assert(!ierr);
        return;
      }
      else
      {
#ifdef PETSC_HAVE_MUMPS
        ierr = PCSetType (pc, (char*) PCLU);       genius_assert(!ierr);
        MESSAGE<< "Using MUMPS as parallel LU preconditioner..."<<std::endl;    RECORD();
        ierr = PCFactorSetMatSolverPackage (pc, "mumps"); genius_assert(!ierr);
        ierr = set_petsc_option("-mat_mumps_icntl_14", MUMPS_ICNTL_14,false);  genius_assert(!ierr);
        ierr = set_petsc_option("-mat_mumps_icntl_23", MUMPS_ICNTL_23,false);  genius_assert(!ierr);
        ierr = PCFactorSetReuseFill(pc, PETSC_TRUE);genius_assert(!ierr);
        ierr = PCFactorSetReuseOrdering(pc, PETSC_TRUE); genius_assert(!ierr);
        ierr = PCFactorSetColumnPivot(pc, 1.0); genius_assert(!ierr);
        ierr = PCFactorSetShiftType(pc,MAT_SHIFT_NONZERO);genius_assert(!ierr);
#else
        MESSAGE << "Warning:  no parallel LU preconditioner configured, use ASM instead!" << std::endl;
        RECORD();
        ierr = PCSetType (pc, (char*) PCASM);       genius_assert(!ierr);
#endif
        return;
      }
    }

    case SolverSpecify::ASM_PRECOND:
    case SolverSpecify::ASMILU0_PRECOND:
    case SolverSpecify::ASMILU1_PRECOND:
    case SolverSpecify::ASMILU2_PRECOND:
    case SolverSpecify::ASMILU3_PRECOND:
    {
      if (Genius::n_processors() > 1)
      {
        ierr = PCSetType (pc, (char*) PCASM);       genius_assert(!ierr);
        ierr = set_petsc_option("-sub_pc_type","ilu"); genius_assert(!ierr);
        ierr = set_petsc_option("-sub_pc_factor_reuse_fill","true"); genius_assert(!ierr);
        ierr = set_petsc_option("-sub_pc_factor_reuse_ordering","true"); genius_assert(!ierr);
        switch ( _preconditioner_type )
        {
          case SolverSpecify::ASMILU0_PRECOND: set_petsc_option("-sub_pc_factor_levels","0"); break;
          case SolverSpecify::ASMILU1_PRECOND: set_petsc_option("-sub_pc_factor_levels","1"); break;
          case SolverSpecify::ASMILU2_PRECOND: set_petsc_option("-sub_pc_factor_levels","2"); break;
          case SolverSpecify::ASMILU3_PRECOND: set_petsc_option("-sub_pc_factor_levels","3"); break;
        }
        ierr = set_petsc_option("-sub_pc_factor_shift_type","NONZERO"); genius_assert(!ierr);
        ierr = PCFactorSetReuseFill(pc, PETSC_TRUE);genius_assert(!ierr);
        ierr = PCFactorSetReuseOrdering(pc, PETSC_TRUE); genius_assert(!ierr);
      }
      else
      {
        ierr = PCSetType (pc, (char*) PCILU);       genius_assert(!ierr);
        ierr = PCFactorSetReuseFill(pc, PETSC_TRUE);genius_assert(!ierr);
        ierr = PCFactorSetReuseOrdering(pc, PETSC_TRUE); genius_assert(!ierr);
        ierr = PCFactorSetColumnPivot(pc, 1.0); genius_assert(!ierr);
        ierr = PCFactorSetShiftType(pc,MAT_SHIFT_NONZERO);genius_assert(!ierr);
#if PETSC_VERSION_GE(3, 6, 0)        
        ierr = PCFactorSetAllowDiagonalFill(pc, PETSC_TRUE);genius_assert(!ierr);
#else
        ierr = PCFactorSetAllowDiagonalFill(pc);genius_assert(!ierr);
#endif          
        switch ( _preconditioner_type )
        {
          case SolverSpecify::ASMILU0_PRECOND: set_petsc_option("-pc_factor_levels","0"); break;
          case SolverSpecify::ASMILU1_PRECOND: set_petsc_option("-pc_factor_levels","1"); break;
          case SolverSpecify::ASMILU2_PRECOND: set_petsc_option("-pc_factor_levels","2"); break;
          case SolverSpecify::ASMILU3_PRECOND: set_petsc_option("-pc_factor_levels","3"); break;
        }
      }
      return;
    }


    case SolverSpecify::ASMLU_PRECOND:
    {
      ierr = PCSetType (pc, (char*) PCASM);       genius_assert(!ierr);
#ifdef PETSC_HAVE_MUMPS
        MESSAGE<< "Using ASM + LU(MUMPS) preconditioner..."<<std::endl;    RECORD();
        ierr = set_petsc_option("-sub_ksp_type","preonly"); genius_assert(!ierr);
        ierr = set_petsc_option("-sub_pc_type","lu"); genius_assert(!ierr);
        ierr = set_petsc_option("-sub_pc_factor_mat_solver_package","mumps"); genius_assert(!ierr);
#else
        MESSAGE<< "Using ASM + ILU preconditioner..."<<std::endl;    RECORD();
        ierr = set_petsc_option("-sub_pc_type","ilu"); genius_assert(!ierr);
#endif
        ierr = set_petsc_option("-sub_pc_factor_reuse_fill","1"); genius_assert(!ierr);
        ierr = set_petsc_option("-sub_pc_factor_reuse_ordering","1"); genius_assert(!ierr);
        ierr = set_petsc_option("-sub_pc_factor_shift_type","NONZERO"); genius_assert(!ierr);
        return;
    }


    case SolverSpecify::PARMS_PRECOND:
    {
#ifdef PETSC_HAVE_PARMS
        MESSAGE<< "Using pARMS preconditioner..."<<std::endl;
        RECORD();
        ierr = PCSetType (pc, (char*) PCPARMS);     genius_assert(!ierr);
        return;
#else
        MESSAGE << "Warning:  no pARMS preconditioner configured, use ASM instead!" << std::endl;
        RECORD();
        ierr = PCSetType (pc, (char*) PCASM);       genius_assert(!ierr);
        return;
#endif

    }


    case SolverSpecify::BOOMERAMG_PRECOND:
    {
#ifdef PETSC_HAVE_LIBHYPRE
      MESSAGE<< "Using Hypre/BoomerAMG preconditioner..."<<std::endl;
      RECORD();
      ierr = PCSetType (pc, (char*) PCHYPRE);     genius_assert(!ierr);
      ierr = PCHYPRESetType (pc, "boomeramg");    genius_assert(!ierr);
      return;
#else
      MESSAGE << "Warning:  no AMG preconditioner configured, use ASM instead!" << std::endl;
      RECORD();
      ierr = PCSetType (pc, (char*) PCASM);       genius_assert(!ierr);
      return;
#endif
    }

    case SolverSpecify::JACOBI_PRECOND:
      ierr = PCSetType (pc, (char*) PCJACOBI);    genius_assert(!ierr); return;

    case SolverSpecify::BLOCK_JACOBI_PRECOND:
      ierr = PCSetType (pc, (char*) PCBJACOBI);   genius_assert(!ierr); return;

    case SolverSpecify::SOR_PRECOND:
      ierr = PCSetType (pc, (char*) PCSOR);       genius_assert(!ierr); return;

    case SolverSpecify::EISENSTAT_PRECOND:
      ierr = PCSetType (pc, (char*) PCEISENSTAT); genius_assert(!ierr); return;


    case SolverSpecify::USER_PRECOND:
      ierr = PCSetType (pc, (char*) PCMAT);       genius_assert(!ierr); return;


    case SolverSpecify::SHELL_PRECOND:
      ierr = PCSetType (pc, (char*) PCSHELL);     genius_assert(!ierr); return;

    default:
      std::cerr
          << "ERROR:  Unsupported PETSC Preconditioner: "
          << preconditioner_type       << std::endl
          << "Continuing with PETSC defaults" << std::endl;
  }
}



int FEM_LinearSolver::set_petsc_option(const std::string &key, const std::string &value, bool has_prefix)
{
  // insert ksp_prefix to the key
  std::string ukey;
  if(has_prefix)
    ukey = std::string("-") + this->ksp_prefix() + key.substr(1) ;
  else
    ukey = key;

  // if the option has been set in command line
  PetscBool  set;
#if PETSC_VERSION_GE(3,7,0)
  PetscOptionsHasName(PETSC_NULL, NULL, ukey.c_str(), &set);
#else
  PetscOptionsHasName(NULL, ukey.c_str(), &set);
#endif
  if(set) return 0;

  // set the option
  petsc_options[ukey] = value;
#if PETSC_VERSION_GE(3,7,0)
  return PetscOptionsSetValue(PETSC_NULL, ukey.c_str(), value.c_str());
#else
  return PetscOptionsSetValue(ukey.c_str(), value.c_str());
#endif
}

