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


// C++ includes
#include <numeric>

// Local includes
#include "simulation_system.h"
#include "semiconductor_region.h"
#include "conductor_region.h"
#include "insulator_region.h"
#include "boundary_condition_gate.h"
#include "parallel.h"
#include "petsc_utils.h"


using PhysicalUnit::kb;
using PhysicalUnit::e;
using PhysicalUnit::Ohm;

/*---------------------------------------------------------------------
 * fill gate electrode potential into initial vector
 */
void GateContactBC::DDM2_Fill_Value(Vec x, Vec L)
{
  const PetscScalar current_scale = this->z_width();

  if(Genius::processor_id() == Genius::n_processors() -1)
  {
    VecSetValue(x, this->global_offset(), this->ext_circuit()->potential(), INSERT_VALUES);

    if(this->is_inter_connect_bc())
    {
      VecSetValue(L, this->global_offset(), 1.0, INSERT_VALUES);
    }
    //for stand alone electrode
    else
    {
      const PetscScalar s = ext_circuit()->electrode_scaling(SolverSpecify::dt);
      VecSetValue(L, this->global_offset(), s, INSERT_VALUES);
    }

  }
}


///////////////////////////////////////////////////////////////////////
//----------------Function and Jacobian evaluate---------------------//
///////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------
 * do pre-process to function for DDML2 solver
 */
void GateContactBC::DDM2_Function_Preprocess(PetscScalar * ,Vec f, std::vector<PetscInt> &src_row,
    std::vector<PetscInt> &dst_row, std::vector<PetscInt> &clear_row)
{
  BoundaryCondition::const_node_iterator node_it = nodes_begin();
  BoundaryCondition::const_node_iterator end_it = nodes_end();
  for(; node_it!=end_it; ++node_it )
  {
    // skip node not belongs to this processor
    if( (*node_it)->processor_id()!=Genius::processor_id() ) continue;

    // buffer for saving regions and fvm_nodes this *node_it involves
    std::vector<const SimulationRegion *> regions;
    std::vector<const FVM_Node *> fvm_nodes;

    // search all the fvm_node which has *node_it as root node
    BoundaryCondition::region_node_iterator  rnode_it     = region_node_begin(*node_it);
    BoundaryCondition::region_node_iterator  end_rnode_it = region_node_end(*node_it);

    // should clear all the rows related with this boundary condition
    for(unsigned int i=0 ; rnode_it!=end_rnode_it; ++i, ++rnode_it  )
    {
      regions.push_back( (*rnode_it).second.first );
      fvm_nodes.push_back( (*rnode_it).second.second );

      switch ( regions[i]->type() )
      {
          // insulator region.
          case InsulatorRegion:
          {
            clear_row.push_back(fvm_nodes[i]->global_offset()+0);
            break;
          }
          case MetalRegion:
          case ElectrodeRegion:
          {
            src_row.push_back(fvm_nodes[i]->global_offset()+1);
            dst_row.push_back(fvm_nodes[0]->global_offset()+1);

            clear_row.push_back(fvm_nodes[i]->global_offset()+0);
            clear_row.push_back(fvm_nodes[i]->global_offset()+1);

            break;
          }

      }
    }
  }
}



/*---------------------------------------------------------------------
 * build function and its jacobian for DDML2 solver
 */
void GateContactBC::DDM2_Function(PetscScalar * x, Vec f, InsertMode &add_value_flag)
{
  // Gate boundary condition is processed here

  // note, we will use ADD_VALUES to set values of vec f
  // if the previous operator is not ADD_VALUES, we should assembly the vec
  if( (add_value_flag != ADD_VALUES) && (add_value_flag != NOT_SET_VALUES) )
  {
    VecAssemblyBegin(f);
    VecAssemblyEnd(f);
  }

  // data buffer for INSERT_VALUES
  std::vector<int> iy;
  std::vector<PetscScalar> y;


  // the electrode current, since the electrode may be partitioned into several processor,
  // we should collect it.
  std::vector<double> current_buffer;

  // for 2D mesh, z_width() is the device dimension in Z direction; for 3D mesh, z_width() is 1.0
  PetscScalar current_scale = this->z_width();

  const PetscScalar Work_Function = this->scalar("workfunction");
  const PetscScalar Heat_Transfer = this->scalar("heat.transfer");

  // the electrode potential in current iteration
  genius_assert( local_offset()!=invalid_uint );
  PetscScalar Ve = x[this->local_offset()];

  BoundaryCondition::const_node_iterator node_it = nodes_begin();
  BoundaryCondition::const_node_iterator end_it = nodes_end();
  for(; node_it!=end_it; ++node_it )
  {
    // skip node not belongs to this processor
    if( (*node_it)->processor_id()!=Genius::processor_id() ) continue;

    // buffer for saving regions and fvm_nodes this *node_it involves
    std::vector<const SimulationRegion *> regions;
    std::vector<const FVM_Node *> fvm_nodes;

    // search all the fvm_node which has *node_it as root node, these fvm_nodes have the same location in geometry,
    // but belong to different regions in logic.
    BoundaryCondition::region_node_iterator  rnode_it     = region_node_begin(*node_it);
    BoundaryCondition::region_node_iterator  end_rnode_it = region_node_end(*node_it);
    for(unsigned int i=0 ; rnode_it!=end_rnode_it; ++i, ++rnode_it  )
    {
      regions.push_back( (*rnode_it).second.first );
      fvm_nodes.push_back( (*rnode_it).second.second );

      switch ( regions[i]->type() )
      {
          // insulator region.
          case InsulatorRegion:
          {

            const FVM_NodeData * node_data = fvm_nodes[i]->node_data();

            PetscScalar V = x[fvm_nodes[i]->local_offset()+0];  // psi of this node
            PetscScalar T = x[fvm_nodes[i]->local_offset()+1];  // lattice temperature

            // the governing equation
            PetscScalar f_psi = V + Work_Function - Ve;

            // set governing equation to function vector
            iy.push_back(fvm_nodes[i]->global_offset());
            y.push_back( f_psi );


            // add heat flux out of gate boundary to lattice temperature equatiuon
            // when this bc is external boundary
            if( node_on_boundary(*node_it) || has_associated_region(*node_it, VacuumRegion))
            {
              PetscScalar S  = fvm_nodes[i]->outside_boundary_surface_area();
              PetscScalar fT = Heat_Transfer*(T_external()-T)*S;
              VecSetValue(f, fvm_nodes[i]->global_offset()+1, fT, ADD_VALUES);
            }

            // MOS gate can have displacement current and tunneling current

            // displacement current, only first order in time.
            if(SolverSpecify::TimeDependent == true)
            {
              FVM_Node::fvm_neighbor_node_iterator nb_it = fvm_nodes[i]->neighbor_node_begin();
              for(; nb_it != fvm_nodes[i]->neighbor_node_end(); ++nb_it)
              {
                const FVM_Node *nb_node = (*nb_it).first;
                const FVM_NodeData * nb_node_data = nb_node->node_data();
                // the psi of neighbor node
                PetscScalar V_nb = x[nb_node->local_offset()+0];
                // distance from nb node to this node
                PetscScalar distance = fvm_nodes[i]->distance(nb_node);
                // area of out surface of control volume related with neighbor node
                PetscScalar cv_boundary = fvm_nodes[i]->cv_surface_area(nb_node);
                PetscScalar dEdt;
                if(SolverSpecify::TS_type==SolverSpecify::BDF2 && SolverSpecify::BDF2_LowerOrder==false) //second order
                {
                  PetscScalar r = SolverSpecify::dt_last/(SolverSpecify::dt_last + SolverSpecify::dt);
                  dEdt = ( (2-r)/(1-r)*(V-V_nb)
                           - 1.0/(r*(1-r))*(node_data->psi()-nb_node_data->psi())
                           + (1-r)/r*(node_data->psi_last()-nb_node_data->psi_last()))/distance/(SolverSpecify::dt_last+SolverSpecify::dt);
                }
                else//first order
                {
                  dEdt = ((V-V_nb)-(node_data->psi()-nb_node_data->psi()))/distance/SolverSpecify::dt;
                }

                current_buffer.push_back( cv_boundary*node_data->eps()*dEdt );
              }
            }
            // FIXME: tunneling current.

            break;
          }


          // conductor region
          case MetalRegion:
          case ElectrodeRegion:
          {

            PetscScalar V = x[fvm_nodes[i]->local_offset()+0];   // psi of this node
            PetscScalar T = x[fvm_nodes[i]->local_offset()+1];   // lattice temperature of this node

            // since the region is sorted, we know region[0] is Insulator region
            // as a result, x[fvm_nodes[0]->local_offset()] is psi for corresponding Insulator region
            PetscScalar V_in = x[fvm_nodes[0]->local_offset()+0];
            PetscScalar T_in = x[fvm_nodes[0]->local_offset()+1];

            // let insulator node process the complete governing equation of heat transfer at interface
            // then we can set the node temperature of conductor region equal to node temperature at insulator region

            // the psi of this node is equal to corresponding psi of Insulator node
            PetscScalar ff1 = V - V_in;
            // the T of this node is equal to corresponding T of Insulator node
            PetscScalar ff2 = T - T_in;

            // set governing equation to function vector
            iy.push_back(fvm_nodes[i]->global_offset()+0);
            y.push_back( ff1 );

            iy.push_back(fvm_nodes[i]->global_offset()+1);
            y.push_back( ff2 );

            break;
          }
          case VacuumRegion:
          break;
          default: genius_error(); //we should never reach here
      }
    }
  }


  if(iy.size()) VecSetValues(f, iy.size(), &iy[0], &y[0], ADD_VALUES) ;


  // the extra equation of gate boundary
  // For voltage driven
  //
  //          _____                Ve
  //    -----|_____|----/\/\/\/\-------> to gate electrode (Ve, I)
  //    | +     R          L       |
  //   Vapp                     C ===
  //    | -                        |
  //    |__________________________|

  //           GND
  //
  // And for current driven
  // NOTE: It is dangerous to attach current source to MOS gate!
  //                               Ve
  //    -->-----------------------------> to gate electrode (Ve, I)
  //    |                          |
  //   Iapp                     C ===
  //    |__________________________|
  //           GND

  // Or for inter connect
  //
  //          _____                Ve
  //    -----|_____|-------------------> to gate electrode (Ve, I)
  //    |       R
  //    |
  // V_inter_connect
  //
  //

  // for get the current, we must sum all the terms in current_buffer
  // NOTE: only statistic current flow belongs to on processor node
  PetscScalar current = current_scale*std::accumulate(current_buffer.begin(), current_buffer.end(), 0.0 );

  ext_circuit()->potential() = Ve;
  ext_circuit()->current() = current;

  PetscScalar mna_scaling = ext_circuit()->mna_scaling(SolverSpecify::dt);

  //for inter connect electrode
  if(this->is_inter_connect_bc())
  {
    PetscScalar R = ext_circuit()->inter_connect_resistance();                               // resistance
    PetscScalar f_ext = R*current;
    VecSetValue(f, this->global_offset(), f_ext, ADD_VALUES);
  }
  // for stand alone electrode
  else
  {
    PetscScalar f_ext = mna_scaling*current;
    VecSetValue(f, this->global_offset(), f_ext, ADD_VALUES);
  }

  if(Genius::is_last_processor())
  {
    //for inter connect electrode
    if(this->is_inter_connect_bc())
    {
      PetscScalar V_ic = x[this->inter_connect_hub()->local_offset()];  // potential at inter connect node
      PetscScalar f_ext = Ve - V_ic;
      VecSetValue(f, this->global_offset(), f_ext, ADD_VALUES);
    }
    // for stand alone electrode
    else
    {
      PetscScalar f_ext = ext_circuit()->mna_function(SolverSpecify::dt);
      VecSetValue(f, this->global_offset(), f_ext, ADD_VALUES);
    }
  }


  // the last operator is ADD_VALUES
  add_value_flag = ADD_VALUES;

}








/*---------------------------------------------------------------------
 * do pre-process to jacobian matrix for DDML2 solver
 */
void GateContactBC::DDM2_Jacobian_Preprocess(PetscScalar *,SparseMatrix<PetscScalar> *jac, std::vector<PetscInt> &src_row,
    std::vector<PetscInt> &dst_row, std::vector<PetscInt> &clear_row)
{
  BoundaryCondition::const_node_iterator node_it = nodes_begin();
  BoundaryCondition::const_node_iterator end_it = nodes_end();
  for(; node_it!=end_it; ++node_it )
  {
    // skip node not belongs to this processor
    if( (*node_it)->processor_id()!=Genius::processor_id() ) continue;

    // buffer for saving regions and fvm_nodes this *node_it involves
    std::vector<const SimulationRegion *> regions;
    std::vector<const FVM_Node *> fvm_nodes;

    // search all the fvm_node which has *node_it as root node
    BoundaryCondition::region_node_iterator  rnode_it     = region_node_begin(*node_it);
    BoundaryCondition::region_node_iterator  end_rnode_it = region_node_end(*node_it);

    // should clear all the rows related with this boundary condition
    for(unsigned int i=0 ; rnode_it!=end_rnode_it; ++i, ++rnode_it  )
    {
      regions.push_back( (*rnode_it).second.first );
      fvm_nodes.push_back( (*rnode_it).second.second );

      switch ( regions[i]->type() )
      {
          // insulator region.
          case InsulatorRegion:
          {
            clear_row.push_back(fvm_nodes[i]->global_offset()+0);
            break;
          }
          case MetalRegion:
          case ElectrodeRegion:
          {
            src_row.push_back(fvm_nodes[i]->global_offset()+1);
            dst_row.push_back(fvm_nodes[0]->global_offset()+1);

            clear_row.push_back(fvm_nodes[i]->global_offset()+0);
            clear_row.push_back(fvm_nodes[i]->global_offset()+1);

            break;
          }

      }
    }
  }
}



/*---------------------------------------------------------------------
 * build function and its jacobian for DDML2 solver
 */
void GateContactBC::DDM2_Jacobian(PetscScalar * x, SparseMatrix<PetscScalar> *jac, InsertMode &add_value_flag)
{
  // the Jacobian of GateContact boundary condition is processed here
  // we use AD again. no matter it is overkill here.

  PetscInt bc_global_offset = this->global_offset();

  // after that, we should do gate boundary process here

  // for 2D mesh, z_width() is the device dimension in Z direction; for 3D mesh, z_width() is 1.0
  PetscScalar current_scale = this->z_width();

  const PetscScalar Work_Function = this->scalar("workfunction");
  const PetscScalar Heat_Transfer = this->scalar("heat.transfer");

  // loop again
  BoundaryCondition::const_node_iterator node_it = nodes_begin();
  BoundaryCondition::const_node_iterator end_it = nodes_end();
  for(node_it = nodes_begin(); node_it!=end_it; ++node_it )
  {

    // skip node not belongs to this processor
    if( (*node_it)->processor_id()!=Genius::processor_id() ) continue;

    // buffer for saving regions and fvm_nodes this *node_it involves
    std::vector<const SimulationRegion *> regions;
    std::vector<const FVM_Node *> fvm_nodes;

    // search all the fvm_node which has *node_it as root node, these fvm_nodes have the same location in geometry,
    // but belong to different regions in logic.
    BoundaryCondition::region_node_iterator  rnode_it     = region_node_begin(*node_it);
    BoundaryCondition::region_node_iterator  end_rnode_it = region_node_end(*node_it);
    for(unsigned int i=0 ; rnode_it!=end_rnode_it; ++i, ++rnode_it  )
    {
      regions.push_back( (*rnode_it).second.first );
      fvm_nodes.push_back( (*rnode_it).second.second );

      switch ( regions[i]->type() )
      {
          case InsulatorRegion:
          {

            const FVM_NodeData * node_data = fvm_nodes[i]->node_data();

            //the indepedent variable number, we need 3 here.
            adtl::AutoDScalar::numdir=3;


            AutoDScalar  V = x[fvm_nodes[i]->local_offset()+0]; V.setADValue(0, 1.0); // psi of this node
            AutoDScalar  T = x[fvm_nodes[i]->local_offset()+1]; T.setADValue(1, 1.0); // psi of this node

            // the electrode potential in current iteration
            genius_assert( local_offset()!=invalid_uint );
            AutoDScalar Ve = x[this->local_offset()];         Ve.setADValue(2, 1.0);

            // the governing equation of potential
            AutoDScalar f_psi = V + Work_Function - Ve;

            // the insert position
            std::vector<PetscInt> row, col;
            row.push_back(fvm_nodes[i]->global_offset()+0);
            row.push_back(fvm_nodes[i]->global_offset()+1);
            col = row;
            col.push_back(this->global_offset()); // the position of electrode equation

            // process the Jacobian of governing equation of potential
            jac->add_row(  row[0],  col.size(),  &col[0],  f_psi.getADValue() );

            // process the Jacobian of equation of T
            // if this gate bc is external boundary, set heat flux here
            if( node_on_boundary(*node_it) || has_associated_region(*node_it, VacuumRegion) )
            {
              PetscScalar S  = fvm_nodes[i]->outside_boundary_surface_area();
              AutoDScalar fT = Heat_Transfer*(T_external()-T)*S;
              jac->add_row(  row[1],  col.size(),  &col[0],  fT.getADValue() );
            }

            /*
             * process the Jacobian of current flow out of gate electrode
             */

            // compute displacement current

            //the indepedent variable number, we need 2 here.
            adtl::AutoDScalar::numdir=2;

            // displacement current
            if(SolverSpecify::TimeDependent == true)
            {
              FVM_Node::fvm_neighbor_node_iterator nb_it = fvm_nodes[i]->neighbor_node_begin();
              for(; nb_it != fvm_nodes[i]->neighbor_node_end(); ++nb_it)
              {
                const FVM_Node *nb_node = (*nb_it).first;
                const FVM_NodeData * nb_node_data = nb_node->node_data();

                // the psi of this node
                AutoDScalar  V = x[fvm_nodes[i]->local_offset()]; V.setADValue(0, 1.0);
                // the psi of neighbor node
                AutoDScalar V_nb = x[nb_node->local_offset()+0]; V_nb.setADValue(1, 1.0);

                // distance from nb node to this node
                PetscScalar distance = fvm_nodes[i]->distance(nb_node);

                // area of out surface of control volume related with neighbor node
                PetscScalar cv_boundary = fvm_nodes[i]->cv_surface_area(nb_node);
                AutoDScalar dEdt;
                if(SolverSpecify::TS_type==SolverSpecify::BDF2 && SolverSpecify::BDF2_LowerOrder==false) //second order
                {
                  PetscScalar r = SolverSpecify::dt_last/(SolverSpecify::dt_last + SolverSpecify::dt);
                  dEdt = ( (2-r)/(1-r)*(V-V_nb)
                           - 1.0/(r*(1-r))*(node_data->psi()-nb_node_data->psi())
                           + (1-r)/r*(node_data->psi_last()-nb_node_data->psi_last()))/distance/(SolverSpecify::dt_last+SolverSpecify::dt);
                }
                else//first order
                {
                  dEdt = ((V-V_nb)-(node_data->psi()-nb_node_data->psi()))/distance/SolverSpecify::dt;
                }

                AutoDScalar current_disp = cv_boundary*node_data->eps()*dEdt*current_scale;

                // consider electrode connect

                //for inter connect electrode
                if(this->is_inter_connect_bc())
                {
                  PetscScalar R = ext_circuit()->inter_connect_resistance();
                  current_disp = R*current_disp;
                }
                // for stand alone electrode
                else
                {
                  PetscScalar mna_scaling = ext_circuit()->mna_scaling(SolverSpecify::dt);
                  current_disp = mna_scaling*current_disp;
                }

                jac->add( bc_global_offset,  fvm_nodes[i]->global_offset(),  current_disp.getADValue(0) );
                jac->add( bc_global_offset,  nb_node->global_offset(),  current_disp.getADValue(1) );
              }
            }
            //FIXME tunneling current should be considered here

            break;
          }
          // conductor region (gate) which has an interface with insulator region
          case MetalRegion:
          case ElectrodeRegion:
          {

            //the indepedent variable number, we need 4 here.
            adtl::AutoDScalar::numdir=4;

            AutoDScalar  V = x[fvm_nodes[i]->local_offset()+0]; V.setADValue(0,1.0); // psi of this node
            AutoDScalar  T = x[fvm_nodes[i]->local_offset()+1]; T.setADValue(1,1.0); // lattice temperature of this node

            // since the region is sorted, we know region[0] is insulator region
            // as a result, x[fvm_nodes[0]->local_offset()] is psi for corresponding insulator region
            AutoDScalar  V_in = x[fvm_nodes[0]->local_offset()+0]; V_in.setADValue(2,1.0);
            AutoDScalar  T_in = x[fvm_nodes[0]->local_offset()+1]; T_in.setADValue(3,1.0);

            // the psi of this node is equal to corresponding psi of insulator node
            AutoDScalar  ff1 = V - V_in;
            // the T of this node is equal to corresponding T of insulator node
            AutoDScalar  ff2 = T - T_in;

            // set Jacobian of governing equation ff1 and ff2
            std::vector<PetscInt> rows,cols;
            rows.push_back(fvm_nodes[i]->global_offset()+0);
            rows.push_back(fvm_nodes[i]->global_offset()+1);
            cols = rows;
            cols.push_back(fvm_nodes[0]->global_offset()+0);
            cols.push_back(fvm_nodes[0]->global_offset()+1);

            jac->add_row(  rows[0],  cols.size(),  &cols[0],  ff1.getADValue() );
            jac->add_row(  rows[1],  cols.size(),  &cols[0],  ff2.getADValue() );

            break;
          }
          case VacuumRegion:
          break;
          default: genius_error(); //we should never reach here
      }
    }

  }

  // the extra equation of gate boundary
  // For voltage driven
  //
  //          _____                Ve
  //    -----|_____|----/\/\/\/\-------> to gate electrode (Ve, I)
  //    |       R          L       |
  //   Vapp                     C ===
  //    |__________________________|
  //           GND
  //
  // And for current driven
  // NOTE: It is dangerous to attach current source to MOS gate!
  //                               Ve
  //    --------------------------------> to gate electrode (Ve, I)
  //    |                          |
  //   Iapp                     C ===
  //    |__________________________|
  //           GND
  //
  // Or for inter connect
  //
  //          _____                Ve
  //    -----|_____|-------------------> to gate electrode (Ve, I)
  //    |       R
  //    |
  // V_inter_connect
  //
  //

  if(Genius::is_last_processor())
  {

    //for inter connect electrode
    if(this->is_inter_connect_bc())
    {
      // the external electrode equation is:
      // f_ext = Ve - V_ic + R*current;

      // d(f_ext)/d(Ve)
      jac->add( bc_global_offset,  bc_global_offset,  1.0 );
      // d(f_ext)/d(V_ic)
      jac->add( bc_global_offset,  this->inter_connect_hub()->global_offset(),  -1.0 );
    }
    //for stand alone electrode
    else
    {
      ext_circuit()->potential() = x[this->local_offset()];
      jac->add( bc_global_offset,  bc_global_offset,  ext_circuit()->mna_jacobian(SolverSpecify::dt) );
    }
  }

  // the last operator is ADD_VALUES
  add_value_flag = ADD_VALUES;

}




/*---------------------------------------------------------------------
 * update electrode IV
 */
void GateContactBC::DDM2_Update_Solution(PetscScalar *)
{
  Parallel::sum(ext_circuit()->current());
  this->ext_circuit()->update();
}

