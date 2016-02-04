/*****************************************************************************/
/*                                                                           */
/*              8888888         88888888         88888888                    */
/*            8                8                8                            */
/*           8                 8                8                            */
/*           8                  88888888         88888888                    */
/*           8      8888                8                8                   */
/*            8       8                 8                8                   */
/*              888888         888888888        888888888                    */
/*                                                                           */
/*       A Two-Dimensional General Purpose Semiconductor Simulator.          */
/*                                                                           */
/*  GSS material database Version 0.4                                        */
/*  Last update: July 25, 2007                                               */
/*                                                                           */
/*  Gong Ding                                                                */
/*  gdiso@ustc.edu                                                           */
/*  NINT, No.69 P.O.Box, Xi'an City, China                                   */
/*                                                                           */
/*****************************************************************************/
//
// Material Type: Nitride


#include "PMI.h"

class GSS_Nitride_BandStructure : public PMII_BandStructure
{
private:

  PetscScalar BANDGAP;   // The bandgap for the material.


  PetscScalar HCI_ECN;   // critical electric field for electron scattering in the insulator
  PetscScalar HCI_ECP;   // critical electric field for hole scattering in the insulator
  PetscScalar HCI_BARLN; // represents barrier lowering effects to electron due to the image field
  PetscScalar HCI_TUNLN; // accounts for the possibility of tunneling to electron
  PetscScalar HCI_BARLP; // represents barrier lowering effects to hole due to the image field
  PetscScalar HCI_TUNLP; // accounts for the possibility of tunneling to hole

  void   Band_Init()
  {
    BANDGAP  = 4.7*eV;

    HCI_ECN  = 8.790000e+04*V/cm;
    HCI_ECP  = 8.790000e+04*V/cm;
    HCI_BARLN = 2.590000E-04*std::pow(V*cm, 0.5);
    HCI_TUNLN = 3.000000E-05*std::pow(V*cm*cm, 1.0/3.0);
    HCI_BARLP = 2.590000E-04*std::pow(V*cm, 0.5);
    HCI_TUNLP = 3.000000E-05*std::pow(V*cm*cm, 1.0/3.0);
  }

public:

  PetscScalar Eg            (const PetscScalar &Tl) const { return BANDGAP;  }
  
  PetscScalar EffecElecMass (const PetscScalar &Tl) const { return me; }
  
  PetscScalar EffecHoleMass  (const PetscScalar &Tl) const { return me; }
  
  PetscScalar ARichardson() const { return 1.100000e+02*A/(K*cm)/(K*cm); }

public:

  PetscScalar HCI_Probability_Insulator_n(const PetscScalar &t_ins, const PetscScalar &E_ins) const
  {
    if(E_ins < HCI_ECN/900.0 ) return 0.0;
    return exp( -sqrt(HCI_ECN/E_ins) );
  }

  PetscScalar HCI_Probability_Insulator_p(const PetscScalar &t_ins, const PetscScalar &E_ins) const
  {
    if(E_ins < HCI_ECP/900.0 ) return 0.0;
    return exp( -sqrt(HCI_ECP/E_ins) );
  }

  PetscScalar HCI_Barrier_n(const PetscScalar &affinity_semi, const PetscScalar &, const PetscScalar &affinity_ins,
                            const PetscScalar &t_ins, const PetscScalar &E_ins) const
  {
    if(E_ins > 0)
      return (affinity_semi - affinity_ins) - HCI_BARLN*std::pow(E_ins, 0.5) - HCI_TUNLN*std::pow(E_ins, 2.0/3.0) ;
    return (affinity_semi - affinity_ins) - E_ins*t_ins;
  }

  PetscScalar HCI_Barrier_p(const PetscScalar &affinity_semi, const PetscScalar & Eg_semi, const PetscScalar &affinity_ins,
                            const PetscScalar &t_ins, const PetscScalar &E_ins) const
  {
    if(E_ins < 0)
      return (affinity_ins + BANDGAP - affinity_semi -  Eg_semi) - HCI_BARLP*std::pow(fabs(E_ins), 0.5) - HCI_TUNLP*std::pow(fabs(E_ins), 2.0/3.0) ;
    return (affinity_ins + BANDGAP - affinity_semi -  Eg_semi) + E_ins*t_ins;
  }

  PetscScalar J_FN_Tunneling(const PetscScalar &E_ins, const PetscScalar &alpha) const
  { return 0.0; }

  PetscScalar J_CBET_Tunneling(const PetscScalar &m, const PetscScalar &Tl,
                               const PetscScalar &Efn1, const PetscScalar &Efn2,
                               const PetscScalar &Ec1,  const PetscScalar &Ec2,
                               const PetscScalar &B1,   const PetscScalar &B2,
                               const PetscScalar &t) const
  { return 0.0; }

  AutoDScalar J_CBET_Tunneling(const PetscScalar &m, const AutoDScalar &Tl,
                               const AutoDScalar &Efn1, const AutoDScalar &Efn2,
                               const AutoDScalar &Ec1,  const AutoDScalar &Ec2,
                               const AutoDScalar &B1,   const AutoDScalar &B2,
                               const PetscScalar &t) const
  { return 0.0; }


  PetscScalar J_VBHT_Tunneling(const PetscScalar &m, const PetscScalar &Tl,
                               const PetscScalar &Efp1, const PetscScalar &Efp2,
                               const PetscScalar &Ev1,  const PetscScalar &Ev2,
                               const PetscScalar &B1,   const PetscScalar &B2,
                               const PetscScalar &t) const
  { return 0.0; }

  AutoDScalar J_VBHT_Tunneling(const PetscScalar &m, const AutoDScalar &Tl,
                               const AutoDScalar &Efp1, const AutoDScalar &Efp2,
                               const AutoDScalar &Ev1,  const AutoDScalar &Ev2,
                               const AutoDScalar &B1,   const AutoDScalar &B2,
                               const PetscScalar &t) const
  { return 0.0; }

  PetscScalar J_VBET_Tunneling(const PetscScalar &m, const PetscScalar &Tl,
                               const PetscScalar &Efn1, const PetscScalar &Efn2,
                               const PetscScalar &Ec1,  const PetscScalar &Ec2,
                               const PetscScalar &Ev1,  const PetscScalar &Ev2,
                               const PetscScalar &B1,   const PetscScalar &B2,
                               const PetscScalar &t) const
  { return 0.0; }

  AutoDScalar J_VBET_Tunneling(const PetscScalar &m, const AutoDScalar &Tl,
                               const AutoDScalar &Efn1, const AutoDScalar &Efn2,
                               const AutoDScalar &Ec1,  const AutoDScalar &Ec2,
                               const AutoDScalar &Ev1,  const AutoDScalar &Ev2,
                               const AutoDScalar &B1,   const AutoDScalar &B2,
                               const PetscScalar &t) const
  { return 0.0; }

public:
  GSS_Nitride_BandStructure(const PMII_Environment &env):PMII_BandStructure(env)
  {
    Band_Init();
  }
  ~GSS_Nitride_BandStructure()
  {}
}
;

extern "C"
{
  DLL_EXPORT_DECLARE  PMII_BandStructure* PMII_Nitride_BandStructure_Default (const PMII_Environment& env)
  {
    return new GSS_Nitride_BandStructure(env);
  }
}
