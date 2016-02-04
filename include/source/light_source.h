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
/*  Author: xianghua zhang   zhangxih@163.com                                   */
/*                                                                              */
/********************************************************************************/

#ifndef __light_source_h__
#define __light_source_h__

//C++ include
#include <string>
#include <cmath>

#include "auto_ptr.h"
#include "parser.h"
#include "interpolation_base.h"
#include "vector_value.h"
#include "tensor_value.h"
#include "waveform.h"

namespace Parser{
  class InputParser;
}
class SimulationSystem;
class FVM_Node;
class Waveform;

class Light_Source
{
public:

  /**
   * constructor
   */
  Light_Source(SimulationSystem & system)
  : _system(system), _waveform(0), _global_waveform(0)
  {}

  /**
   * virtual destructor
   */
  virtual ~Light_Source() {}

  /**
   * @return the type if light source
   */
  virtual std::string light_source_type() = 0;

  /**
   * calculate carrier generation at time t and update PatG
   */
  virtual void carrier_generation(double t);

  /**
   * virtual function to update OptG
   */
  virtual void update_source() {}

  /**
   * virtual function for limit the time step
   */
  virtual double limit_dt(double time, double dt, double dt_min) const;


  void set_waveform(Waveform * waveform) { _waveform = waveform; }


  void set_global_waveform(Waveform * waveform) { _global_waveform = waveform; }


  Waveform * waveform() { return _waveform; }


protected:

  /**
   * I should know something about simulation system
   */
  SimulationSystem &_system;


  /**
   * wavefrom
   */
  Waveform * _waveform;


  /**
   * wavefrom
   */
  Waveform * _global_waveform;


  /**
   * light energy deposit for on processor FVM node
   */
  std::map<const FVM_Node *, double> _fvm_node_particle_deposit;

};


/**
 * set the carrier generation of Light from file
 */
class Light_Source_From_File : public Light_Source
{
public:

  /**
   *  constructor
   */

  Light_Source_From_File(SimulationSystem &, const Parser::Card &c,
               const std::string &fname_ext,
               const double wave_length, const double power,
               const double eta=1.0, const bool eta_auto=false);

  /**
   * virtual destructor
   */
  virtual ~Light_Source_From_File() {}


  /**
   * @return the type if light source
   */
  virtual std::string light_source_type() { return "light_source_from_file"; }

  /**
   * update OptG
   */
  virtual void update_source();


  std::string fname() const { return _fname; }
  double waveLength() const { return _wave_length; }
  double power()      const { return _power; }
  double eta()        const { return _eta; }
  bool   etaAuto()   const { return _eta_auto; }

  void setFname  (const std::string fname) { _fname = fname; }
  void setWaveLength(const double wave_length) { _wave_length = wave_length; }
  void setPower     (const double power)       { _power = power; }
  void setEta       (const double eta)         { _eta = eta; }
  void setEtaAuto   (const bool eta_auto)      { _eta_auto = eta_auto; }

private:

  int load_light_elec_profile_fromfile(InterpolationBase * interpolator, const std::string &fname, const int skip_line=0);
  int load_light_pow_profile_fromfile(InterpolationBase * interpolator, const std::string &fname, const int skip_line=0);

  std::string _fname;
  double _wave_length;
  double _power;
  double _eta;
  bool   _eta_auto;

  int _dim;
  int _skip_line;

  double _LUnit;
  double _FUnit;

  std::string _field_type;

  VectorValue<double> _translate;
  TensorValue<double> _transform;

};


/**
 * set the carrier generation of Light by RayTracing
 */
class Light_Source_RayTracing : public Light_Source
{
  public:

    Light_Source_RayTracing(SimulationSystem &system, const Parser::Card &c)
    :Light_Source(system),_card(c)
    {}

    /**
     * virtual destructor
     */
    virtual ~Light_Source_RayTracing() {}


   /**
    * @return the type if light source
    */
    virtual std::string light_source_type() { return "light_source_raytracing"; }

    /**
     * update OptG
     */
    virtual void update_source();

  private:

    const Parser::Card _card;

};


/**
 * set the carrier generation of Light by EMFEM2D
 */
class Light_Source_EMFEM2D : public Light_Source
{
  public:

    Light_Source_EMFEM2D(SimulationSystem &system, const Parser::Card &c)
    :Light_Source(system),_card(c)
    {}

    /**
     * virtual destructor
     */
    virtual ~Light_Source_EMFEM2D() {}

    /**
     * @return the type if light source
     */
    virtual std::string light_source_type() { return "light_source_emfem2d"; }

    /**
     * update OptG
     */
    virtual void update_source();

  private:

    const Parser::Card _card;

};


/**
 * set the uniform carrier generation
 */
class Light_Source_Uniform : public Light_Source
{
  public:

    Light_Source_Uniform(SimulationSystem &system, const Parser::Card &c)
    :Light_Source(system),_card(c)
    {}

    /**
     * virtual destructor
     */
    virtual ~Light_Source_Uniform() {}

    /**
     * @return the type if light source
     */
    virtual std::string light_source_type() { return "light_source_uniform"; }

    /**
     * update OptG
     */
    virtual void update_source();

  private:

    const Parser::Card _card;

};



/**
 * set the carrier generation due to xray pulse
 */
class Light_Source_Xray : public Light_Source
{
  public:

    Light_Source_Xray(SimulationSystem &system, double doserate)
    :Light_Source(system), _doserate(doserate)
    {}

    /**
     * virtual destructor
     */
    virtual ~Light_Source_Xray() {}

    /**
     * @return the type if light source
     */
    virtual std::string light_source_type() { return "light_source_xray"; }

    /**
     * update OptG
     */
    virtual void update_source();

  private:

    const double _doserate;

};



#endif
