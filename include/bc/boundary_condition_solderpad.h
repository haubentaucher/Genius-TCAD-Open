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

#ifndef __boundary_condition_solderpad_h__
#define __boundary_condition_solderpad_h__

#include "boundary_condition.h"

/**
 * The pad Boundary Condition
 */
class SolderPadBC : public BoundaryCondition
{
public:

  /**
   * constructor, set default value
   */
  SolderPadBC(SimulationSystem  & system, const std::string & label="");

  /**
   * destructor
   */
  virtual ~SolderPadBC(){}


  /**
   * @return boundary condition type
   */
  virtual BCType bc_type() const
    { return SolderPad; }

  /**
   * @return boundary condition type in string
   */
  virtual std::string bc_type_name() const
  { return "SolderPad"; }

  /**
   * @return boundary type
   */
  virtual BoundaryType boundary_type() const
    { return BOUNDARY; }


  /**
   * @return true iff this boundary has a current flow
   */
  virtual bool has_current_flow() const
  { return true; }


  /**
   * @return the current flow of this boundary.
   */
  virtual PetscScalar current() const
  {return _current_flow;}

  /**
   * @return writable reference to current flow of this boundary
   */
  virtual PetscScalar & current()
  { return _current_flow;}


  /**
   * @return the string which indicates the boundary condition
   */
  virtual std::string boundary_condition_in_string() const;

private:
  // current buffer
  std::vector<PetscScalar> _current_buffer;

  // Jacobian buffer for current
  std::vector< PetscInt > _buffer_rows;
  std::vector< std::vector<PetscInt> >    _buffer_cols;
  std::vector< std::vector<PetscScalar> > _buffer_jacobian_entries;

  /**
   * current flow in/out
   */
  PetscScalar   _current_flow;

public:

#ifdef TCAD_SOLVERS
  //////////////////////////////////////////////////////////////////////////////////////////////
  //----------------Function and Jacobian evaluate for Poisson's Equation---------------------//
  //////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * fill solution data and scaling constant into petsc vector
   */
  virtual void Poissin_Fill_Value(Vec x, Vec L);

  /**
   * preprocess Jacobian function for poisson solver
   */
  virtual void Poissin_Function_Preprocess(PetscScalar *, Vec, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for poisson solver, nothing to do
   */
  virtual void Poissin_Function(PetscScalar * , Vec , InsertMode &);

  /**
   * preprocess Jacobian Matrix for poisson solver
   */
  virtual void Poissin_Jacobian_Preprocess(PetscScalar *, SparseMatrix<PetscScalar> *, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for poisson solver, nothing to do
   */
  virtual void Poissin_Jacobian(PetscScalar * , SparseMatrix<PetscScalar> *, InsertMode &);

  /**
   * update solution value of poisson's equation
   */
  virtual void Poissin_Update_Solution(PetscScalar *);


  //////////////////////////////////////////////////////////////////////////////////
  //----------------Function and Jacobian evaluate for L1 DDM---------------------//
  //////////////////////////////////////////////////////////////////////////////////

  /**
   * fill solution data into petsc vector of level 1 DDM equation.
   */
  virtual void DDM1_Fill_Value(Vec x, Vec L);

  /**
   * preprocess function for level 1 DDM solver
   */
  virtual void DDM1_Function_Preprocess(PetscScalar *, Vec, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for level 1 DDM solver, nothing to do
   */
  virtual void DDM1_Function(PetscScalar *x , Vec f, InsertMode &add_value_flag);

  /**
   * preprocess Jacobian Matrix of level 1 DDM equation.
   */
  virtual void DDM1_Jacobian_Preprocess(PetscScalar *, SparseMatrix<PetscScalar> *, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for level 1 DDM solver, nothing to do
   */
  virtual void DDM1_Jacobian(PetscScalar *x , SparseMatrix<PetscScalar> *jac, InsertMode &add_value_flag);

  /**
   * build trace parameter for DDML1 solver
   */
  virtual void DDM1_Electrode_Trace(Vec, SparseMatrix<PetscScalar> *, Vec , Vec);

  /**
   * update solution data of DDML1 solver.
   */
  virtual void DDM1_Update_Solution(PetscScalar *x);




  //////////////////////////////////////////////////////////////////////////////////
  //---------------Function and Jacobian evaluate for Mixed DDML1-----------------//
  //////////////////////////////////////////////////////////////////////////////////

  /**
   * fill solution data into petsc vector of level 1 DDM equation.
   */
  virtual void Mix_DDM1_Fill_Value(Vec x, Vec L);

  /**
   * preprocess function for Mixed DDML1 solver
   */
  virtual void Mix_DDM1_Function_Preprocess(PetscScalar *, Vec, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for Mixed DDML1 solver
   */
  virtual void Mix_DDM1_Function(PetscScalar * , Vec , InsertMode &);

  /**
   * preprocess Jacobian Matrix for Mixed type level 1 DDM equation.
   */
  virtual void Mix_DDM1_Jacobian_Preprocess(PetscScalar *, SparseMatrix<PetscScalar> *, std::vector<PetscInt> &,  std::vector<PetscInt> &, std::vector<PetscInt> &);

  /**
   * build function and its jacobian for Mixed DDML1 solver
   */
  virtual void Mix_DDM1_Jacobian(PetscScalar * , SparseMatrix<PetscScalar> *, InsertMode &);

  /**
   * virtual function for update solution value of Mixed type of level 1 DDM equation.
   */
  virtual void Mix_DDM1_Update_Solution(PetscScalar *);

  //////////////////////////////////////////////////////////////////////////////////
  //----------Function and Jacobian evaluate for Advanced Mixed DDML1-------------//
  //////////////////////////////////////////////////////////////////////////////////

  /**
   * fill solution data into petsc vector of level 1 DDM equation.
   */
  virtual void MixA_DDM1_Fill_Value(Vec x, Vec L);

  /**
   * preprocess function for Advanced Mixed DDML1 solver
   */
  virtual void MixA_DDM1_Function_Preprocess(PetscScalar *, Vec, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for Advanced Mixed DDML1 solver
   */
  virtual void MixA_DDM1_Function(PetscScalar * , Vec , InsertMode &);

  /**
   * preprocess Jacobian Matrix for Advanced Mixed type level 1 DDM equation.
   */
  virtual void MixA_DDM1_Jacobian_Preprocess(PetscScalar *, SparseMatrix<PetscScalar> *, std::vector<PetscInt> &,  std::vector<PetscInt> &, std::vector<PetscInt> &);

  /**
   * build function and its jacobian for Advanced Mixed DDML1 solver
   */
  virtual void MixA_DDM1_Jacobian(PetscScalar * , SparseMatrix<PetscScalar> *, InsertMode &);

  /**
   * virtual function for update solution value of Advanced Mixed type of level 1 DDM equation.
   */
  virtual void MixA_DDM1_Update_Solution(PetscScalar *);

  //////////////////////////////////////////////////////////////////////////////////
  //----------------Function and Jacobian evaluate for L2 DDM---------------------//
  //////////////////////////////////////////////////////////////////////////////////

  /**
   * fill solution data into petsc vector of level 2 DDM equation.
   */
  virtual void DDM2_Fill_Value(Vec x, Vec L);

  /**
   * preprocess function for level 2 DDM solver
   */
  virtual void DDM2_Function_Preprocess(PetscScalar * ,Vec, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for level 2 DDM solver
   */
  virtual void DDM2_Function(PetscScalar *x , Vec f, InsertMode &add_value_flag);

  /**
   * preprocess Jacobian Matrix of level 2 DDM equation.
   */
  virtual void DDM2_Jacobian_Preprocess(PetscScalar *,SparseMatrix<PetscScalar> *, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for level 2 DDM solver
   */
  virtual void DDM2_Jacobian(PetscScalar *x , SparseMatrix<PetscScalar> *jac, InsertMode &add_value_flag);

  /**
   * build trace parameter for level 2 DDM solver
   */
  virtual void DDM2_Electrode_Trace(Vec, SparseMatrix<PetscScalar> *, Vec , Vec);

  /**
   * update solution data of level 2 DDM solver.
   */
  virtual void DDM2_Update_Solution(PetscScalar *x);


  //////////////////////////////////////////////////////////////////////////////////
  //----------Function and Jacobian evaluate for Advanced Mixed DDML2-------------//
  //////////////////////////////////////////////////////////////////////////////////

  /**
   * fill solution data into petsc vector of level 2 DDM equation.
   */
  virtual void MixA_DDM2_Fill_Value(Vec x, Vec L);

  /**
   * preprocess function for Advanced Mixed DDML2 solver
   */
  virtual void MixA_DDM2_Function_Preprocess(PetscScalar * ,Vec, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for Advanced Mixed DDML2 solver
   */
  virtual void MixA_DDM2_Function(PetscScalar * , Vec , InsertMode &);

  /**
   * preprocess Jacobian Matrix for Advanced Mixed type level 2 DDM equation.
   */
  virtual void MixA_DDM2_Jacobian_Preprocess(PetscScalar *,SparseMatrix<PetscScalar> *, std::vector<PetscInt> &,  std::vector<PetscInt> &, std::vector<PetscInt> &);

  /**
   * build function and its jacobian for Advanced Mixed DDML2 solver
   */
  virtual void MixA_DDM2_Jacobian(PetscScalar * , SparseMatrix<PetscScalar> *, InsertMode &);

  /**
   * virtual function for update solution value of Advanced Mixed type of level 2 DDM equation.
   */
  virtual void MixA_DDM2_Update_Solution(PetscScalar *);

  //////////////////////////////////////////////////////////////////////////////////
  //----------------Function and Jacobian evaluate for L3 EBM---------------------//
  //////////////////////////////////////////////////////////////////////////////////

  /**
   * fill solution data into petsc vector of level 3 EBM equation.
   */
  virtual void EBM3_Fill_Value(Vec x, Vec L);

  /**
   * preprocess function for level 3 EBM solver
   */
  virtual void EBM3_Function_Preprocess(PetscScalar *,Vec, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for level 3 EBM solver
   */
  virtual void EBM3_Function(PetscScalar *x , Vec f, InsertMode &add_value_flag);

  /**
   * preprocess Jacobian Matrix of level 3 EBM equation.
   */
  virtual void EBM3_Jacobian_Preprocess(PetscScalar * ,SparseMatrix<PetscScalar> *, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for level 3 EBM solver
   */
  virtual void EBM3_Jacobian(PetscScalar *x , SparseMatrix<PetscScalar> *jac, InsertMode &add_value_flag);

  /**
   * build trace parameter for level 3 EBM solver
   */
  virtual void EBM3_Electrode_Trace(Vec, SparseMatrix<PetscScalar> *, Vec , Vec);

  /**
   * update solution data of EBM3 solver.
   */
  virtual void EBM3_Update_Solution(PetscScalar *x);



  //////////////////////////////////////////////////////////////////////////////////
  //----------Function and Jacobian evaluate for Advanced Mixed EBM3 -------------//
  //////////////////////////////////////////////////////////////////////////////////

  /**
   * fill solution data into petsc vector of EBM3 equation.
   */
  virtual void MixA_EBM3_Fill_Value(Vec x, Vec L);

  /**
   * preprocess function for Advanced Mixed EBM3 solver
   */
  virtual void MixA_EBM3_Function_Preprocess(PetscScalar *,Vec, std::vector<PetscInt>&, std::vector<PetscInt>&, std::vector<PetscInt>&);

  /**
   * build function and its jacobian for Advanced Mixed EBM3 solver
   */
  virtual void MixA_EBM3_Function(PetscScalar * , Vec , InsertMode &);

  /**
   * preprocess Jacobian Matrix for Advanced Mixed type EBM3 equation.
   */
  virtual void MixA_EBM3_Jacobian_Preprocess(PetscScalar * ,SparseMatrix<PetscScalar> *, std::vector<PetscInt> &,  std::vector<PetscInt> &, std::vector<PetscInt> &);

  /**
   * build function and its jacobian for Advanced Mixed EBM3 solver
   */
  virtual void MixA_EBM3_Jacobian(PetscScalar * , SparseMatrix<PetscScalar> *, InsertMode &);

  /**
   * virtual function for update solution value of Advanced Mixed type of level 3 EBM equation.
   */
  virtual void MixA_EBM3_Update_Solution(PetscScalar *);

  //////////////////////////////////////////////////////////////////////////////////
  //--------------Matrix and RHS Vector evaluate for DDM AC Solver----------------//
  //////////////////////////////////////////////////////////////////////////////////

  /**
   *  evaluating matrix and rhs vector for ddm ac solver
   */
  virtual void DDMAC_Fill_Matrix_Vector( Mat A, Vec b, const Mat J, const PetscScalar omega, InsertMode & add_value_flag );

  /**
   * update solution value for ddm ac solver
   */
  virtual void DDMAC_Update_Solution(const PetscScalar * lxx , const Mat, const PetscScalar omega);


#ifdef COGENDA_COMMERCIAL_PRODUCT
  //////////////////////////////////////////////////////////////////////////////////
  //----------------- functions for Gummel DDML1 solver --------------------------//
  //////////////////////////////////////////////////////////////////////////////////


  /**
   * function for fill vector for half implicit current continuity equation.
   */
  virtual void DDM1_Half_Implicit_Current_Fill_Value(Vec x);

  /**
   * function for reserve none zero pattern in petsc matrix.
   */
  virtual void DDM1_Half_Implicit_Current_Reserve(Mat A, InsertMode &add_value_flag);

  /**
   * function for preprocess build RHS and matrix for half implicit current continuity equation.
   */
  virtual void DDM1_Half_Implicit_Current_Preprocess(Vec f, Mat A, std::vector<PetscInt> &src,  std::vector<PetscInt> &dst, std::vector<PetscInt> &clear);

  /**
   * function for build RHS and matrix for half implicit current continuity equation.
   */
  virtual void DDM1_Half_Implicit_Current(PetscScalar * x, Mat A, Vec r, InsertMode &add_value_flag);

  /**
   * function for update solution value for half implicit current continuity equation.
   */
  virtual void DDM1_Half_Implicit_Current_Update_Solution(PetscScalar *);

  /**
   * function for preprocess build RHS and matrix for half implicit poisson correction equation.
   */
  virtual void DDM1_Half_Implicit_Poisson_Correction_Preprocess(Vec f, std::vector<PetscInt> &src,  std::vector<PetscInt> &dst, std::vector<PetscInt> &clear);

  /**
   * function for build RHS and matrix for half implicit poisson correction equation.
   */
  virtual void DDM1_Half_Implicit_Poisson_Correction(PetscScalar * x, Mat A, Vec r, InsertMode &add_value_flag);

#endif

#endif

};






#endif

