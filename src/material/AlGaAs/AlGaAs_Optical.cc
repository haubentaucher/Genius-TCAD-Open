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
/*       A Two-Dimensional General Purpose Semiconductor GaAsmulator.        */
/*                                                                           */
/*  GSS material database Version 0.4                                        */
/*  Last update: Feb 17, 2006                                                */
/*                                                                           */
/*  Gong Ding                                                                */
/*  gdiso@ustc.edu                                                           */
/*  NINT, No.69 P.O.Box, Xi'an City, China                                   */
/*                                                                           */
/*****************************************************************************/
//
// Material Type: AlGaAs

// Source: Optical properties of Al(x)Ga(1-x)As, J.Appl.Phys. 60(2), 15 July 1986

#include "PMI.h"


class GSS_AlGaAs_Optical : public PMIS_Optical
{
private:

  std::vector<PetscScalar> molex;
  std::vector<RefractionSplineInterp> nk_molex;

  void init_wave_table()
  {
    PetscScalar nk_table_0 [][3] = {
      {0.206667,1.264,2.472},
      {0.210169,1.288,2.557},
      {0.213793,1.311,2.625},
      {0.217544,1.325,2.71},
      {0.221429,1.349,2.815},
      {0.225455,1.383,2.936},
      {0.22963,1.43,3.079},
      {0.233962,1.499,3.255},
      {0.238462,1.599,3.484},
      {0.243137,1.802,3.795},
      {0.248,2.273,4.084},
      {0.253061,2.89,4.047},
      {0.258333,3.342,3.77},
      {0.26383,3.589,3.452},
      {0.269565,3.769,3.169},
      {0.275556,3.913,2.919},
      {0.281818,4.015,2.563},
      {0.288372,3.939,2.26},
      {0.295238,3.81,2.069},
      {0.302439,3.692,1.969},
      {0.31,3.601,1.92},
      {0.317949,3.538,1.904},
      {0.326316,3.501,1.909},
      {0.335135,3.485,1.931},
      {0.344444,3.495,1.965},
      {0.354286,3.531,2.013},
      {0.364706,3.596,2.076},
      {0.375758,3.709,2.162},
      {0.3875,3.938,2.288},
      {0.4,4.373,2.146},
      {0.413333,4.509,1.948},
      {0.427586,5.052,1.721},
      {0.442857,4.959,0.991},
      {0.459259,4.694,0.696},
      {0.476923,4.492,0.539},
      {0.496,4.333,0.441},
      {0.516667,4.205,0.371},
      {0.53913,4.1,0.32},
      {0.563636,4.013,0.276},
      {0.590476,3.94,0.24},
      {0.62,3.878,0.211},
      {0.652632,3.826,0.179},
      {0.688889,3.786,0.157},
      {0.729412,3.742,0.123},
      {0.775,3.7,0.093},
      {0.826667,3.666,0.059},
    };
    PetscScalar nk_table_1 [][3] = {
      {0.206667,1.311,2.457},
      {0.210169,1.318,2.538},
      {0.213793,1.33,2.608},
      {0.217544,1.345,2.698},
      {0.221429,1.371,2.8},
      {0.225455,1.408,2.921},
      {0.22963,1.459,3.059},
      {0.233962,1.531,3.223},
      {0.238462,1.634,3.433},
      {0.243137,1.819,3.704},
      {0.248,2.207,3.983},
      {0.253061,2.772,4.036},
      {0.258333,3.267,3.846},
      {0.26383,3.611,3.536},
      {0.269565,3.829,3.229},
      {0.275556,4.01,2.876},
      {0.281818,4.017,2.507},
      {0.288372,3.992,2.24},
      {0.295238,3.081,2.074},
      {0.302439,3.697,1.983},
      {0.31,3.618,1.937},
      {0.317949,3.566,1.92},
      {0.326316,3.537,1.924},
      {0.335135,3.532,1.945},
      {0.344444,3.552,1.979},
      {0.354286,3.601,2.03},
      {0.364706,3.69,2.1},
      {0.375758,3.864,2.203},
      {0.3875,4.253,2.187},
      {0.4,4.46,1.949},
      {0.413333,4.838,1.836},
      {0.427586,4.968,1.126},
      {0.442857,4.725,0.763},
      {0.459259,4.518,0.575},
      {0.476923,4.353,0.462},
      {0.496,4.22,0.382},
      {0.516667,4.111,0.32},
      {0.53913,4.018,0.276},
      {0.563636,3.94,0.237},
      {0.590476,3.876,0.199},
      {0.62,3.82,0.171},
      {0.652632,3.775,0.127},
      {0.688889,3.716,0.099},
      {0.729412,3.678,0.082},
      {0.775,3.661,0.059},
      {0.826667,3.572,0},
    };
    PetscScalar nk_table_2 [][3] = {
      {0.206667,1.333,2.457},
      {0.210169,1.339,2.531},
      {0.213793,1.349,2.6},
      {0.217544,1.366,2.688},
      {0.221429,1.393,2.794},
      {0.225455,1.433,2.912},
      {0.22963,1.49,3.049},
      {0.233962,1.567,3.208},
      {0.238462,1.677,3.407},
      {0.243137,1.86,3.654},
      {0.248,2.21,3.914},
      {0.253061,2.734,3.997},
      {0.258333,3.238,3.867},
      {0.26383,3.638,3.575},
      {0.269565,3.924,3.223},
      {0.275556,4.053,2.803},
      {0.281818,4.018,2.449},
      {0.288372,3.911,2.206},
      {0.295238,3.795,2.059},
      {0.302439,3.701,1.976},
      {0.31,3.633,1.933},
      {0.317949,3.588,1.917},
      {0.326316,3.568,1.922},
      {0.335135,3.572,1.942},
      {0.344444,3.602,1.979},
      {0.354286,3.668,2.034},
      {0.364706,3.792,2.115},
      {0.375758,4.084,2.18},
      {0.3875,4.379,1.978},
      {0.4,4.607,1.857},
      {0.413333,4.943,1.322},
      {0.427586,4.757,0.865},
      {0.442857,4.547,0.636},
      {0.459259,4.375,0.499},
      {0.476923,4.235,0.409},
      {0.496,4.118,0.341},
      {0.516667,4.022,0.288},
      {0.53913,3.94,0.242},
      {0.563636,3.871,0.202},
      {0.590476,3.815,0.165},
      {0.62,3.759,0.118},
      {0.652632,3.7,0.094},
      {0.688889,3.662,0.082},
      {0.729412,3.635,0.002},
      {0.775,3.536,0.002},
      {0.826667,3.457,0},
    };
    PetscScalar nk_table_3 [][3] = {
      {0.206667,1.347,2.443},
      {0.210169,1.338,2.502},
      {0.213793,1.352,2.577},
      {0.217544,1.367,2.669},
      {0.221429,1.393,2.776},
      {0.225455,1.437,2.893},
      {0.22963,1.497,3.03},
      {0.233962,1.581,3.187},
      {0.238462,1.696,3.376},
      {0.243137,1.878,3.604},
      {0.248,2.198,3.845},
      {0.253061,2.684,3.957},
      {0.258333,3.196,3.881},
      {0.26383,3.669,3.617},
      {0.269565,3.982,3.177},
      {0.275556,4.062,2.733},
      {0.281818,3.999,2.393},
      {0.288372,3.883,2.172},
      {0.295238,3.772,2.04},
      {0.302439,3.686,1.966},
      {0.31,3.625,1.927},
      {0.317949,3.588,1.914},
      {0.326316,3.575,1.921},
      {0.335135,3.589,1.946},
      {0.344444,3.633,1.988},
      {0.354286,3.724,2.054},
      {0.364706,3.922,2.134},
      {0.375758,4.246,2.041},
      {0.3875,4.456,1.879},
      {0.4,4.825,1.558},
      {0.413333,4.781,1.012},
      {0.427586,4.582,0.722},
      {0.442857,4.404,0.556},
      {0.459259,4.258,0.446},
      {0.476923,4.135,0.367},
      {0.496,4.032,0.305},
      {0.516667,3.945,0.258},
      {0.53913,3.872,0.227},
      {0.563636,3.815,0.202},
      {0.590476,3.75,0.167},
      {0.62,3.69,0.145},
      {0.652632,3.65,0.111},
      {0.688889,3.592,0.008},
      {0.729412,3.509,0},
      {0.775,3.456,0},
      {0.826667,3.404,0},
    };
    PetscScalar nk_table_4 [][3] = {
      {0.206667,1.353,2.44},
      {0.210169,1.35,2.507},
      {0.213793,1.357,2.574},
      {0.217544,1.377,2.675},
      {0.221429,1.406,2.783},
      {0.225455,1.456,2.908},
      {0.22963,1.523,3.047},
      {0.233962,1.613,3.201},
      {0.238462,1.74,3.384},
      {0.243137,1.926,3.598},
      {0.248,2.234,3.822},
      {0.253061,2.695,3.937},
      {0.258333,3.2,3.909},
      {0.26383,3.733,3.646},
      {0.269565,4.054,3.157},
      {0.275556,4.103,2.691},
      {0.281818,4.014,2.363},
      {0.288372,3.897,2.161},
      {0.295238,3.794,2.04},
      {0.302439,3.719,1.971},
      {0.31,3.667,1.936},
      {0.317949,3.64,1.924},
      {0.326316,3.64,1.931},
      {0.335135,3.668,1.958},
      {0.344444,3.736,2.005},
      {0.354286,3.887,2.071},
      {0.364706,4.172,2.042},
      {0.375758,4.401,1.87},
      {0.3875,4.706,1.64},
      {0.4,4.778,1.119},
      {0.413333,4.605,0.786},
      {0.427586,4.43,0.596},
      {0.442857,4.28,0.472},
      {0.459259,4.154,0.385},
      {0.476923,4.047,0.319},
      {0.496,3.957,0.268},
      {0.516667,3.881,0.219},
      {0.53913,3.82,0.178},
      {0.563636,3.747,0.134},
      {0.590476,3.686,0.1},
      {0.62,3.664,0.059},
      {0.652632,3.559,0.003},
      {0.688889,3.479,0},
      {0.729412,3.422,0},
      {0.775,3.378,0},
      {0.826667,3.341,0},
    };
    PetscScalar nk_table_5 [][3] = {
      {0.206667,1.366,2.418},
      {0.210169,1.363,2.49},
      {0.213793,1.364,2.572},
      {0.217544,1.379,2.666},
      {0.221429,1.412,2.78},
      {0.225455,1.462,2.903},
      {0.22963,1.532,3.045},
      {0.233962,1.632,3.199},
      {0.238462,1.763,3.378},
      {0.243137,1.951,3.579},
      {0.248,2.25,3.787},
      {0.253061,2.686,3.904},
      {0.258333,3.187,3.899},
      {0.26383,3.731,3.645},
      {0.269565,4.072,3.147},
      {0.275556,4.107,2.668},
      {0.281818,4.009,2.351},
      {0.288372,3.894,2.159},
      {0.295238,3.789,2.046},
      {0.302439,3.73,1.98},
      {0.31,3.688,1.945},
      {0.317949,3.671,1.933},
      {0.326316,3.68,1.942},
      {0.335135,3.724,1.969},
      {0.344444,3.822,2.017},
      {0.354286,4.034,2.049},
      {0.364706,4.294,1.922},
      {0.375758,4.525,1.748},
      {0.3875,4.753,1.34},
      {0.4,4.654,0.926},
      {0.413333,4.483,0.684},
      {0.427586,4.328,0.534},
      {0.442857,4.195,0.429},
      {0.459259,4.081,0.355},
      {0.476923,3.985,0.292},
      {0.496,3.903,0.245},
      {0.516667,3.838,0.205},
      {0.53913,3.761,0.164},
      {0.563636,3.696,0.133},
      {0.590476,3.665,0.088},
      {0.62,3.558,0.002},
      {0.652632,3.477,0},
      {0.688889,3.417,0},
      {0.729412,3.368,0},
      {0.775,3.329,0},
      {0.826667,3.283,0},
    };
    PetscScalar nk_table_6 [][3] = {
      {0.206667,1.385,2.42},
      {0.210169,1.37,2.485},
      {0.213793,1.37,2.565},
      {0.217544,1.389,2.669},
      {0.221429,1.422,2.785},
      {0.225455,1.475,2.915},
      {0.22963,1.553,3.059},
      {0.233962,1.661,3.217},
      {0.238462,1.805,3.392},
      {0.243137,2.007,3.581},
      {0.248,2.304,3.772},
      {0.253061,2.74,3.881},
      {0.258333,3.221,3.866},
      {0.26383,3.762,3.617},
      {0.269565,4.12,3.107},
      {0.275556,4.127,2.616},
      {0.281818,4.015,2.318},
      {0.288372,3.903,2.142},
      {0.295238,3.815,2.038},
      {0.302439,3.758,1.978},
      {0.31,3.729,1.947},
      {0.317949,3.725,1.935},
      {0.326316,3.75,1.944},
      {0.335135,3.822,1.973},
      {0.344444,3.977,2.002},
      {0.354286,4.224,1.924},
      {0.364706,4.429,1.754},
      {0.375758,4.665,1.45},
      {0.3875,4.649,1.028},
      {0.4,4.497,0.754},
      {0.413333,4.343,0.584},
      {0.427586,4.208,0.468},
      {0.442857,4.092,0.384},
      {0.459259,3.992,0.317},
      {0.476923,3.909,0.262},
      {0.496,3.837,0.205},
      {0.516667,3.758,0.157},
      {0.53913,3.69,0.126},
      {0.563636,3.658,0.063},
      {0.590476,3.546,0.005},
      {0.62,3.467,0},
      {0.652632,3.405,0},
      {0.688889,3.354,0},
      {0.729412,3.313,0},
      {0.775,3.274,0},
      {0.826667,3.237,0},
    };
    PetscScalar nk_table_7 [][3] = {
      {0.206667,1.377,2.426},
      {0.210169,1.366,2.493},
      {0.213793,1.365,2.581},
      {0.217544,1.375,2.688},
      {0.221429,1.407,2.809},
      {0.225455,1.462,2.946},
      {0.22963,1.545,3.098},
      {0.233962,1.662,3.269},
      {0.238462,1.829,3.445},
      {0.243137,2.049,3.62},
      {0.248,2.354,3.788},
      {0.253061,2.777,3.873},
      {0.258333,3.214,3.853},
      {0.26383,3.758,3.637},
      {0.269565,4.144,3.15},
      {0.275556,4.142,2.645},
      {0.281818,4.028,2.365},
      {0.288372,3.932,2.206},
      {0.295238,3.868,2.111},
      {0.302439,3.835,2.055},
      {0.31,3.836,2.023},
      {0.317949,3.868,2.009},
      {0.326316,3.947,2.006},
      {0.335135,4.103,1.993},
      {0.344444,4.319,1.877},
      {0.354286,4.502,1.678},
      {0.364706,4.665,1.357},
      {0.375758,4.615,0.98},
      {0.3875,4.471,0.735},
      {0.4,4.325,0.574},
      {0.413333,4.196,0.46},
      {0.427586,4.084,0.374},
      {0.442857,3.987,0.307},
      {0.459259,3.906,0.245},
      {0.476923,3.823,0.184},
      {0.496,3.746,0.129},
      {0.516667,3.696,0.069},
      {0.53913,3.595,0.002},
      {0.563636,3.5,0},
      {0.590476,3.425,0},
      {0.62,3.361,0},
      {0.652632,3.306,0},
      {0.688889,3.261,0},
      {0.729412,3.225,0},
      {0.775,3.188,0},
      {0.826667,3.153,0},
    };
    PetscScalar nk_table_8 [][3] = {
      {0.206667,1.368,2.409},
      {0.210169,1.36,2.473},
      {0.213793,1.354,2.56},
      {0.217544,1.37,2.667},
      {0.221429,1.399,2.792},
      {0.225455,1.447,2.935},
      {0.22963,1.528,3.104},
      {0.233962,1.661,3.293},
      {0.238462,1.857,3.475},
      {0.243137,2.11,3.635},
      {0.248,2.426,3.763},
      {0.253061,2.833,3.815},
      {0.258333,3.233,3.765},
      {0.26383,3.751,3.582},
      {0.269565,4.107,3.128},
      {0.275556,4.112,2.639},
      {0.281818,4.004,2.389},
      {0.288372,3.928,2.256},
      {0.295238,3.893,2.183},
      {0.302439,3.904,2.144},
      {0.31,3.962,2.119},
      {0.317949,4.078,2.092},
      {0.326316,4.267,2.013},
      {0.335135,4.462,1.82},
      {0.344444,4.613,1.561},
      {0.354286,4.667,1.199},
      {0.364706,4.562,0.89},
      {0.375758,4.413,0.685},
      {0.3875,4.277,0.541},
      {0.4,4.155,0.437},
      {0.413333,4.05,0.353},
      {0.427586,3.961,0.276},
      {0.442857,3.872,0.205},
      {0.459259,3.787,0.161},
      {0.476923,3.738,0.104},
      {0.496,3.635,0.013},
      {0.516667,3.519,0.004},
      {0.53913,3.44,0.003},
      {0.563636,3.378,0},
      {0.590476,3.322,0},
      {0.62,3.277,0},
      {0.652632,3.236,0},
      {0.688889,3.202,0},
      {0.729412,3.173,0},
      {0.775,3.147,0},
      {0.826667,3.124,0},
    };



    molex.resize(9);
    molex[0] = 0.0;
    molex[1] = 0.099;
    molex[2] = 0.198;
    molex[3] = 0.315;
    molex[4] = 0.419;
    molex[5] = 0.491;
    molex[6] = 0.590;
    molex[7] = 0.7;
    molex[8] = 0.804;
    
    nk_molex.resize(9);
    
    for(unsigned int i=0; i<46; i++)
    {
      nk_molex[0].add_nk_sorted(nk_table_0[i][0]*um, nk_table_0[i][1], nk_table_0[i][2]);
      nk_molex[1].add_nk_sorted(nk_table_1[i][0]*um, nk_table_1[i][1], nk_table_1[i][2]);
      nk_molex[2].add_nk_sorted(nk_table_2[i][0]*um, nk_table_2[i][1], nk_table_2[i][2]);
      nk_molex[3].add_nk_sorted(nk_table_3[i][0]*um, nk_table_3[i][1], nk_table_3[i][2]);
      nk_molex[4].add_nk_sorted(nk_table_4[i][0]*um, nk_table_4[i][1], nk_table_4[i][2]);
      nk_molex[5].add_nk_sorted(nk_table_5[i][0]*um, nk_table_5[i][1], nk_table_5[i][2]);
      nk_molex[6].add_nk_sorted(nk_table_6[i][0]*um, nk_table_6[i][1], nk_table_6[i][2]);
      nk_molex[7].add_nk_sorted(nk_table_7[i][0]*um, nk_table_7[i][1], nk_table_7[i][2]);
      nk_molex[8].add_nk_sorted(nk_table_8[i][0]*um, nk_table_8[i][1], nk_table_8[i][2]);
    }
    for(unsigned int i=0; i<9; i++)
      nk_molex[i].build();
  }

public:

  std::complex<PetscScalar> RefractionIndex(PetscScalar lambda, PetscScalar Tl, PetscScalar Eg=0) const
  {
    PetscScalar mole_x = this->ReadxMoleFraction();
    if( mole_x <= molex.front() )
      return nk_molex.front().nk(lambda);
    if( mole_x >= molex.back() )
      return nk_molex.back().nk(lambda);

    unsigned int intp1=molex.size(), intp2=molex.size();
    PetscScalar a1=0.0, a2=0.0;
    for(unsigned int n=0; n<molex.size()-1; ++n)
    {
      if( mole_x >= molex[n] && mole_x <= molex[n+1])
      {
        intp1 = n;
        intp2 = n+1;
        a1 = (molex[n+1]-mole_x)/(molex[n+1]-molex[n]);
        a2 = (mole_x-molex[n])/(molex[n+1]-molex[n]);
      }
    }

    assert(intp1!=molex.size());
    assert(intp2!=molex.size());

    return a1*nk_molex[intp1].nk(lambda) + a2*nk_molex[intp2].nk(lambda);
  }

  // constructions
public:

  GSS_AlGaAs_Optical(const PMIS_Environment &env):PMIS_Optical(env)
  {
    init_wave_table();
  }

  ~GSS_AlGaAs_Optical() {}

};



extern "C"
{
  DLL_EXPORT_DECLARE  PMIS_Optical*  PMIS_AlGaAs_Optical_Default (const PMIS_Environment& env)
  {
    return new GSS_AlGaAs_Optical(env);
  }
}

