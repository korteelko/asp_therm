/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "gas_ng_gost_defines.h"

#include "asp_utils/Logging.h"
#include "atherm_common.h"
#include "gas_defines.h"

#include <stdint.h>
#include <limits>

#define GET_ARRAY_SIZE(M) (sizeof(M) / sizeof(M[0]))

// clang-format off
/* check 07_11_19 */
// Параметры бинарного взаимодействия
// const size_t gases_count = 13;
/* checked 17_11_19 by ISO 20765-1:2005(E)
       excel with coefs look in Documents/studing */
const component_characteristics gases[] = {
// gas                  M        E           K          G         Q     F    S    W
#ifdef ISO_20765
  {GAS_TYPE_HEPTANE,    100.204,  427.72263, 0.7525189, 0.337542, 0.0,  0.0, 0.0, 0.0},
  {GAS_TYPE_OCTANE,     114.231,  450.32502, 0.784955,  0.383381, 0.0,  0.0, 0.0, 0.0},
  {GAS_TYPE_NONANE,     128.258,  470.84089, 0.8152731, 0.427354, 0.0,  0.0, 0.0, 0.0},
  {GAS_TYPE_DECANE,     142.285,  489.55837, 0.8437826, 0.469659, 0.0,  0.0, 0.0, 0.0},
  {GAS_TYPE_CARBON_MONOXIDE,
                        28.01,     105.5348, 0.4533894, 0.038953, 0.0,  0.0, 0.0, 0.0},
  {GAS_TYPE_WATER,      18.0153,   514.0156, 0.3825868, 0.3325, 1.06775, 0.0, 1.5822, 1.0},
  {GAS_TYPE_OXYGEN,     31.9988,   122.7667, 0.4186954, 0.021,    0.0,  0.0, 0.0, 0.0},
  {GAS_TYPE_HYDROGEN_SULFIDE,
                        34.082,  296.355,    0.4618263, 0.0885,0.633276, 0.0, 0.39, 0.0},
  {GAS_TYPE_ARGON,      39.948,  119.6299,   0.4216551, 0.0,      0.0, 0.0, 0.0, 0.0},
#endif  // ISO_20765
  {GAS_TYPE_METHANE,    16.0430, 151.318300, 0.4619255, 0.0,      0.0,  0.0, 0.0, 0.0},
  {GAS_TYPE_ETHANE,     30.0700, 244.166700, 0.5279209, 0.0793,   0.0,  0.0, 0.0, 0.0},
  {GAS_TYPE_PROPANE,    44.0970, 298.118300, 0.5837490, 0.141239, 0.0,  0.0, 0.0, 0.0},
  {GAS_TYPE_ISO_BUTANE, 58.1230, 324.068900, 0.6406937, 0.256692, 0.0,  0.0, 0.0, 0.0},
  {GAS_TYPE_N_BUTANE,   58.1230, 337.638900, 0.6341423, 0.281835, 0.0,  0.0, 0.0, 0.0},
  {GAS_TYPE_ISO_PENTANE,72.1500, 365.599900, 0.6738577, 0.332267, 0.0,  0.0, 0.0, 0.0},
  {GAS_TYPE_N_PENTANE,  72.1500, 370.682300, 0.6798307, 0.366911, 0.0,  0.0, 0.0, 0.0},
  {GAS_TYPE_HEXANE,     86.1770, 402.636293, 0.7175118, 0.289731, 0.0,  0.0, 0.0, 0.0},
  {GAS_TYPE_NITROGEN,   28.0135,  99.737780, 0.4479153, 0.027815, 0.0,  0.0, 0.0, 0.0},
  {GAS_TYPE_CARBON_DIOXIDE,
                        44.0100, 241.960600, 0.4557489, 0.189065, 0.69, 0.0, 0.0, 0.0},
  {GAS_TYPE_HELIUM,      4.0026,   2.610111, 0.3589888, 0.000000, 0.0,  0.0, 0.0, 0.0},
  {GAS_TYPE_HYDROGEN,    2.0159,  26.957940, 0.3514916, 0.034369, 0.0,  0.0, 0.0, 0.0}
};

const component_characteristics *get_characteristics(gas_t gas_name) {
  return get_coefs(gases, GET_ARRAY_SIZE(gases), gas_name);
}

// const size_t binary_associate_count = 38;
/* checked 17_11_19 by ISO 20765-1:2005(E)
       excel with coefs look in Documents/studing */
const binary_associate_coef gases_coef[] = {
// gas i                gas j                     E         V         K         G
#ifdef ISO_20765
  {GAS_TYPE_METHANE,  GAS_TYPE_HEPTANE,           0.88088,  1.191904, 0.983565, 1.0},
  {GAS_TYPE_METHANE,  GAS_TYPE_OCTANE,            0.880973, 1.205769, 0.982707, 1.0},
  {GAS_TYPE_METHANE,  GAS_TYPE_NONANE,            0.881067, 1.219634, 0.981849, 1.0},
  {GAS_TYPE_METHANE,  GAS_TYPE_DECANE,            0.881161, 1.233498, 0.980991, 1.0},
  {GAS_TYPE_METHANE,  GAS_TYPE_CARBON_MONOXIDE,   0.990126, 1.000000, 1.000000, 1.0},
  {GAS_TYPE_METHANE,  GAS_TYPE_WATER,             0.708218, 1.000000, 1.000000, 1.0},
  {GAS_TYPE_METHANE,  GAS_TYPE_HYDROGEN_SULFIDE,  0.931484, 0.736833, 1.000080, 1.0},

  {GAS_TYPE_ETHANE,  GAS_TYPE_WATER,              0.693168, 1.000000, 1.000000, 1.0},
  {GAS_TYPE_ETHANE,  GAS_TYPE_HYDROGEN_SULFIDE,   0.946871, 0.971926, 0.999969, 1.0},

  {GAS_TYPE_NITROGEN,  GAS_TYPE_OXYGEN,           1.021,    1.000000, 1.000000, 1.0},
  {GAS_TYPE_NITROGEN,  GAS_TYPE_CARBON_MONOXIDE,  1.00571,  1.000000, 1.000000, 1.0},
  {GAS_TYPE_NITROGEN,  GAS_TYPE_WATER,            0.746954, 1.000000, 1.000000, 1.0},
  {GAS_TYPE_NITROGEN,  GAS_TYPE_HYDROGEN_SULFIDE, 0.902271, 0.993476, 0.942596, 1.0},

  {GAS_TYPE_CARBON_DIOXIDE,  GAS_TYPE_HEPTANE,    0.831229, 1.077634, 0.895362, 1.0},
  {GAS_TYPE_CARBON_DIOXIDE,  GAS_TYPE_OCTANE,     0.80831,  1.088178, 0.881152, 1.0},
  {GAS_TYPE_CARBON_DIOXIDE,  GAS_TYPE_NONANE,     0.786323, 1.098291, 0.867520, 1.0},
  {GAS_TYPE_CARBON_DIOXIDE,  GAS_TYPE_DECANE,     0.765171, 1.108021, 0.854406, 1.0},
  {GAS_TYPE_CARBON_DIOXIDE,  GAS_TYPE_CARBON_MONOXIDE,1.5,  0.900000, 1.000000, 1.0},
  {GAS_TYPE_CARBON_DIOXIDE,  GAS_TYPE_WATER,      0.849408, 1.000000, 1.000000, 1.67309},
  {GAS_TYPE_CARBON_DIOXIDE,  GAS_TYPE_HYDROGEN_SULFIDE,0.955052, 1.04529, 1.00779, 1.0},

  {GAS_TYPE_HEXANE,    GAS_TYPE_HYDROGEN_SULFIDE, 1.008692, 1.028973, 0.968130, 1.0},
  {GAS_TYPE_HEPTANE,   GAS_TYPE_HYDROGEN_SULFIDE, 1.010126, 1.033754, 0.962870, 1.0},
  {GAS_TYPE_OCTANE,    GAS_TYPE_HYDROGEN_SULFIDE, 1.011501, 1.038338, 0.957828, 1.0},
  {GAS_TYPE_NONANE,    GAS_TYPE_HYDROGEN_SULFIDE, 1.012821, 1.042735, 0.952441, 1.0},
  {GAS_TYPE_DECANE,    GAS_TYPE_HYDROGEN_SULFIDE, 1.014089, 1.046966, 0.948338, 1.0},
  {GAS_TYPE_HYDROGEN,  GAS_TYPE_CARBON_MONOXIDE,  1.100000, 1.000000, 1.000000, 1.0},
#endif  // ISO_20765

  {GAS_TYPE_METHANE,    GAS_TYPE_PROPANE,         0.994635, 0.990877, 1.007619, 1.0},
  {GAS_TYPE_METHANE,    GAS_TYPE_ISO_BUTANE,      1.019530, 1.000000, 1.000000, 1.0},
  {GAS_TYPE_METHANE,    GAS_TYPE_N_BUTANE,        0.989844, 0.992291, 0.997596, 1.0},
  {GAS_TYPE_METHANE,    GAS_TYPE_ISO_PENTANE,     1.002350, 1.000000, 1.000000, 1.0},
  {GAS_TYPE_METHANE,    GAS_TYPE_N_PENTANE,       0.999268, 1.003670, 1.002529, 1.0},
  {GAS_TYPE_METHANE,    GAS_TYPE_HEXANE,          1.107274, 1.302576, 0.982962, 1.0},
  {GAS_TYPE_METHANE,    GAS_TYPE_NITROGEN,        0.971640, 0.886106, 1.003630, 1.0},
  {GAS_TYPE_METHANE,    GAS_TYPE_CARBON_DIOXIDE,  0.960644, 0.963827, 0.995933, 0.807653},
  {GAS_TYPE_METHANE,    GAS_TYPE_HYDROGEN,        1.170520, 1.156390, 1.023260, 1.957310},

  {GAS_TYPE_ETHANE,     GAS_TYPE_PROPANE,         1.022560, 1.065173, 0.986893, 1.0},
  {GAS_TYPE_ETHANE,     GAS_TYPE_ISO_BUTANE,      1.000000, 1.250000, 1.000000, 1.0},
  {GAS_TYPE_ETHANE,     GAS_TYPE_N_BUTANE,        1.013060, 1.250000, 1.000000, 1.0},
  {GAS_TYPE_ETHANE,     GAS_TYPE_ISO_PENTANE,     1.000000, 1.250000, 1.000000, 1.0},
  {GAS_TYPE_ETHANE,     GAS_TYPE_N_PENTANE,       1.005320, 1.250000, 1.000000, 1.0},
  {GAS_TYPE_ETHANE,     GAS_TYPE_NITROGEN,        0.970120, 0.816431, 1.007960, 1.0},
  {GAS_TYPE_ETHANE,     GAS_TYPE_CARBON_DIOXIDE,  0.925053, 0.969870, 1.008510, 0.370296},
  {GAS_TYPE_ETHANE,     GAS_TYPE_HYDROGEN,        1.164460, 1.616660, 1.020340, 1.0},

  {GAS_TYPE_PROPANE,    GAS_TYPE_N_BUTANE,        1.004900, 1.000000, 1.000000, 1.0},
  {GAS_TYPE_PROPANE,    GAS_TYPE_NITROGEN,        0.945939, 0.915502, 1.000000, 1.0},
  {GAS_TYPE_PROPANE,    GAS_TYPE_CARBON_DIOXIDE,  0.960237, 1.000000, 1.000000, 1.0},
  {GAS_TYPE_PROPANE,    GAS_TYPE_HYDROGEN,        1.034787, 1.000000, 1.000000, 1.0},

  {GAS_TYPE_ISO_BUTANE,   GAS_TYPE_NITROGEN,      0.946914, 1.000000, 1.000000, 1.0},
  {GAS_TYPE_ISO_BUTANE,   GAS_TYPE_CARBON_DIOXIDE,0.906849, 1.000000, 1.000000, 1.0},
  {GAS_TYPE_ISO_BUTANE,   GAS_TYPE_HYDROGEN,      1.300000, 1.000000, 1.000000, 1.0},

  {GAS_TYPE_N_BUTANE,   GAS_TYPE_NITROGEN,        0.973384, 0.993556, 1.000000, 1.0},
  {GAS_TYPE_N_BUTANE,   GAS_TYPE_CARBON_DIOXIDE,  0.897362, 1.000000, 1.000000, 1.0},
  {GAS_TYPE_N_BUTANE,   GAS_TYPE_HYDROGEN,        1.300000, 1.000000, 1.000000, 1.0},

  {GAS_TYPE_ISO_PENTANE,  GAS_TYPE_NITROGEN,      0.959340, 1.000000, 1.000000, 1.0},
  {GAS_TYPE_ISO_PENTANE,  GAS_TYPE_CARBON_DIOXIDE,0.726255, 1.000000, 1.000000, 1.0},

  {GAS_TYPE_N_PENTANE,  GAS_TYPE_NITROGEN,        0.945520, 1.000000, 1.000000, 1.0},
  {GAS_TYPE_N_PENTANE,  GAS_TYPE_CARBON_DIOXIDE,  0.859764, 1.000000, 1.000000, 1.0},

  {GAS_TYPE_HEXANE,     GAS_TYPE_CARBON_DIOXIDE,  0.855134, 1.066638, 0.910183, 1.0},

  {GAS_TYPE_NITROGEN,   GAS_TYPE_CARBON_DIOXIDE,  1.022740, 0.835058, 0.982361, 0.982746},
  {GAS_TYPE_NITROGEN,   GAS_TYPE_HYDROGEN,        1.086320, 0.408838, 1.032270, 1.0},

  {GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_HYDROGEN,    1.281790, 1.000000, 1.000000, 1.0},

  {GAS_TYPE_UNDEFINED,  GAS_TYPE_UNDEFINED,       1.000000, 1.000000, 1.000000, 1.0}
};

const binary_associate_coef *get_binary_associate_coefs(gas_t i, gas_t j) {
  if (j > i)
    std::swap(i, j);
  size_t bin_coef_count = sizeof(gases_coef) / sizeof(gases_coef[0]);
  size_t z = 0;
  for (; z < bin_coef_count; ++z)
    if (gases_coef[z].i == i) 
      break;
  // undefined gas or gas isn't in list
  if (z >= bin_coef_count - 1)
    return &(gases_coef[bin_coef_count - 1]);
  while (gases_coef[z].i == i) {
    if (gases_coef[z].j == j)
      return &(gases_coef[z]);
    ++z;
  }
  return &(gases_coef[bin_coef_count - 1]);
}

/* checked 08_11_19 */
/* checked 17_11_19 by ISO 20765-1:2005(E)
       excel with coefs look in Documents/studing */
const size_t A0_3_coefs_count = 58;
const A0_3_coef A0_3_coefs[] = {
// a              b    c    k     u    g    q    f    s    w
  { 0.153832600,  1.0, 0.0, 0.0,  0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 1
  { 1.341953000,  1.0, 0.0, 0.0,  0.5, 0.0, 0.0, 0.0, 0.0, 0.0},  // 2
  {-2.998583000,  1.0, 0.0, 0.0,  1.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 3
  {-0.048312280,  1.0, 0.0, 0.0,  3.5, 0.0, 0.0, 0.0, 0.0, 0.0},  // 4
  { 0.375796500,  1.0, 0.0, 0.0, -0.5, 1.0, 0.0, 0.0, 0.0, 0.0},  // 5
  {-1.589575000,  1.0, 0.0, 0.0,  4.5, 1.0, 0.0, 0.0, 0.0, 0.0},  // 6
  {-0.053588470,  1.0, 0.0, 0.0,  0.5, 0.0, 1.0, 0.0, 0.0, 0.0},  // 7
  { 0.886594630,  1.0, 0.0, 0.0,  7.5, 0.0, 0.0, 0.0, 1.0, 0.0},  // 8
  {-0.710237040,  1.0, 0.0, 0.0,  9.5, 0.0, 0.0, 0.0, 1.0, 0.0},  // 9
  {-1.471722000,  1.0, 0.0, 0.0,  6.0, 0.0, 0.0, 0.0, 0.0, 1.0},  // 10
  { 1.321850350,  1.0, 0.0, 0.0, 12.0, 0.0, 0.0, 0.0, 0.0, 1.0},  // 11
  {-0.786659250,  1.0, 0.0, 0.0, 12.5, 0.0, 0.0, 0.0, 0.0, 1.0},  // 12
  { 2.291290e-9,  1.0, 1.0, 3.0, -6.0, 0.0, 0.0, 1.0, 0.0, 0.0},  // 13
  { 0.157672400,  1.0, 1.0, 2.0,  2.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 14
  {-0.436386400,  1.0, 1.0, 2.0,  3.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 15
  {-0.044081590,  1.0, 1.0, 2.0,  2.0, 0.0, 1.0, 0.0, 0.0, 0.0},  // 16
  {-0.003433888,  1.0, 1.0, 4.0,  2.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 17
  { 0.032059050,  1.0, 1.0, 4.0, 11.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 18
  { 0.024873550,  2.0, 0.0, 0.0, -0.5, 0.0, 0.0, 0.0, 0.0, 0.0},  // 19
  { 0.073322790,  2.0, 0.0, 0.0,  0.5, 0.0, 0.0, 0.0, 0.0, 0.0},  // 20
  {-0.001600573,  2.0, 1.0, 2.0,  0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 21
  { 0.642470600,  2.0, 1.0, 2.0,  4.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 22
  {-0.416260100,  2.0, 1.0, 2.0,  6.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 23
  {-0.066899570,  2.0, 1.0, 4.0, 21.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 24
  { 0.279179500,  2.0, 1.0, 4.0, 23.0, 1.0, 0.0, 0.0, 0.0, 0.0},  // 25
  {-0.696605100,  2.0, 1.0, 4.0, 22.0, 0.0, 1.0, 0.0, 0.0, 0.0},  // 26
  {-0.002860589,  2.0, 1.0, 4.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0},  // 27
  {-0.008098836,  3.0, 0.0, 0.0, -0.5, 0.0, 1.0, 0.0, 0.0, 0.0},  // 28
  { 3.150547000,  3.0, 1.0, 1.0,  7.0, 1.0, 0.0, 0.0, 0.0, 0.0},  // 29
  { 0.007224479,  3.0, 1.0, 1.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0},  // 30
  {-0.705752900,  3.0, 1.0, 2.0,  6.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 31
  { 0.534979200,  3.0, 1.0, 2.0,  4.0, 1.0, 0.0, 0.0, 0.0, 0.0},  // 32
  {-0.079314910,  3.0, 1.0, 3.0,  1.0, 1.0, 0.0, 0.0, 0.0, 0.0},  // 33
  {-1.418465000,  3.0, 1.0, 3.0,  9.0, 1.0, 0.0, 0.0, 0.0, 0.0},  // 34
  {-5.99905e-17,  3.0, 1.0, 4.0,-13.0, 0.0, 0.0, 1.0, 0.0, 0.0},  // 35
  { 0.105840200,  3.0, 1.0, 4.0, 21.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 36
  { 0.034317290,  3.0, 1.0, 4.0,  8.0, 0.0, 1.0, 0.0, 0.0, 0.0},  // 37
  {-0.007022847,  4.0, 0.0, 0.0, -0.5, 0.0, 0.0, 0.0, 0.0, 0.0},  // 38
  { 0.024955870,  4.0, 0.0, 0.0,  0.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 39
  { 0.042968180,  4.0, 1.0, 2.0,  2.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 40
  { 0.746545300,  4.0, 1.0, 2.0,  7.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 41
  {-0.291961300,  4.0, 1.0, 2.0,  9.0, 0.0, 1.0, 0.0, 0.0, 0.0},  // 42
  { 7.294616000,  4.0, 1.0, 4.0, 22.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 43
  {-9.936757000,  4.0, 1.0, 4.0, 23.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 44
  {-0.005399808,  5.0, 0.0, 0.0,  1.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 45
  {-0.243256700,  5.0, 1.0, 2.0,  9.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 46
  { 0.049870160,  5.0, 1.0, 2.0,  3.0, 0.0, 1.0, 0.0, 0.0, 0.0},  // 47
  { 0.003733797,  5.0, 1.0, 4.0,  8.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 48
  { 1.874951000,  5.0, 1.0, 4.0, 23.0, 0.0, 1.0, 0.0, 0.0, 0.0},  // 49
  { 0.002168144,  6.0, 0.0, 0.0,  1.5, 0.0, 0.0, 0.0, 0.0, 0.0},  // 50
  {-0.658716400,  6.0, 1.0, 2.0,  5.0, 1.0, 0.0, 0.0, 0.0, 0.0},  // 51
  { 0.000205518,  7.0, 0.0, 0.0, -0.5, 0.0, 1.0, 0.0, 0.0, 0.0},  // 52
  { 0.009776195,  7.0, 1.0, 2.0,  4.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 53
  {-0.020487080,  8.0, 1.0, 1.0,  7.0, 1.0, 0.0, 0.0, 0.0, 0.0},  // 54
  { 0.015573220,  8.0, 1.0, 2.0,  3.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 55
  { 0.006862415,  8.0, 1.0, 2.0,  0.0, 1.0, 0.0, 0.0, 0.0, 0.0},  // 56
  {-0.001226752,  9.0, 1.0, 2.0,  1.0, 0.0, 0.0, 0.0, 0.0, 0.0},  // 57
  { 0.002850908,  9.0, 1.0, 2.0,  0.0, 0.0, 1.0, 0.0, 0.0, 0.0}   // 58
};

/* checked 08_11_19 */
/* checked 18_11_19 by ISO 20765-1:2005(E)
       excel with coefs look in Documents/studing */
const A4_coef A4_coefs[] = {
#ifdef ISO_20765
  {GAS_TYPE_HEPTANE,         57.77391, -57104.81056, 4.00000, 13.7266, 169.7890, 30.4707, 836.195, 43.55610, 1760.46,  0.00000,   0.000},
  {GAS_TYPE_OCTANE,          62.95591, -60546.76385, 4.00000, 15.6865, 158.9220, 33.8029, 815.064, 48.17310, 1693.07,  0.00000,   0.000},
  {GAS_TYPE_NONANE,          67.79407, -66600.12837, 4.00000, 18.0241, 156.8540, 38.1235, 814.882, 53.34150, 1693.79,  0.00000,   0.000},
  {GAS_TYPE_DECANE,          71.63669, -74131.45483, 4.00000, 21.0069, 164.9470, 43.4931, 836.264, 58.36570, 1750.24,  0.00000,   0.000},
  {GAS_TYPE_CARBON_MONOXIDE, 23.15547, - 2635.24412, 3.50055, 1.02865, 1550.450, 0.00493, 704.525, 0.000000, 0.00000,  0.00000,   0.000},
  {GAS_TYPE_WATER,           27.27642, - 7766.73308, 4.00392, 0.01059, 268.7950, 0.98763, 1141.41, 3.069040, 2507.37,  0.00000,   0.000},
  {GAS_TYPE_HYDROGEN_SULFIDE,27.28069, - 6069.03587, 4.00000, 3.11942, 1833.630, 1.00243, 847.181, 0.000000, 0.00000,  0.00000,   0.000},
  {GAS_TYPE_ARGON,           15.74399,   -745.37500, 2.50000, 0.00000,   0.0000, 0.00000,   0.000, 0.000000,   0.000,  0.00000,   0.000},
#endif  // ISO_20765
  {GAS_TYPE_METHANE,         35.53603, -15999.69151, 4.00088, 0.76315, 820.6590, 0.00460, 178.410, 8.744320, 1062.82, -4.46921, 1090.53},
  {GAS_TYPE_ETHANE,          42.42766, -23639.65301, 4.00263, 4.33939, 559.3140, 1.23722, 223.284, 13.19740, 1031.38, -6.01989, 1071.29},
  {GAS_TYPE_PROPANE,         50.40669, -31236.63551, 4.02939, 6.60569, 479.8560, 3.19700, 200.893, 19.19210, 955.312, -8.37267, 1027.29},
  {GAS_TYPE_N_BUTANE,        42.22997, -38957.80933, 4.33944, 9.44893, 468.2700, 6.89406, 183.636, 24.46180, 1914.10, 14.78240, 903.185},
  {GAS_TYPE_ISO_BUTANE,      39.99940, -38525.50276, 4.06714, 8.97575, 438.2700, 5.25156, 198.018, 25.14230, 1905.02, 16.13880, 893.765},
  {GAS_TYPE_N_PENTANE,       48.37597, -45215.83000, 4.00000, 8.95043, 178.6700, 21.8360, 840.538, 33.40320, 1774.25,  0.00000,   0.000},
  {GAS_TYPE_ISO_PENTANE,     48.86978, -51198.30946, 4.00000, 11.7618, 292.5030, 20.1101, 910.237, 33.16880, 1919.37,  0.00000,   0.000},
  {GAS_TYPE_HEXANE,          52.69477, -52746.83318, 4.00000, 11.6977, 182.3260, 26.8142, 859.207, 38.61640, 1826.59,  0.00000,   0.000},
  {GAS_TYPE_OXYGEN,          22.49931, - 2318.32269, 3.50146, 1.07558, 2235.710, 1.01334, 1116.69, 0.000000, 0.00000,  0.00000,   0.000},
  {GAS_TYPE_NITROGEN,        23.26530, - 2801.72907, 3.50031, 0.13732, 662.7380, -0.1466, 680.562, 0.900660, 1740.06,  0.00000,   0.000},
  {GAS_TYPE_CARBON_DIOXIDE,  26.35604, - 4902.17152, 3.50002, 2.04452, 919.3060,-1.06044, 865.070, 2.033660, 483.553,  0.01393, 341.109},
  {GAS_TYPE_HELIUM,          15.74399,   -745.37500, 2.50000, 0.00000,   0.0000, 0.00000,   0.000, 0.000000,   0.000,  0.00000,   0.000},
  {GAS_TYPE_HYDROGEN,        18.77280, - 5836.94370, 2.47906, 0.95806,  228.734, 0.45444, 326.843, 1.560390, 1651.71,  -1.3756, 1671.69},
};

const A4_coef *get_A4_coefs(gas_t gas_name) {
  return get_coefs(A4_coefs, GET_ARRAY_SIZE(A4_coefs), gas_name);
}

/* checked 08_11_19 */
const critical_params A5_critical_params[] = {
  {GAS_TYPE_METHANE,       190.564, 162.66, 0.064294},
  {GAS_TYPE_ETHANE,        305.320, 206.58, 0.109580},
  {GAS_TYPE_PROPANE,       369.825, 220.49, 0.184260},
  {GAS_TYPE_ISO_BUTANE,    407.850, 224.36, 0.161570},
  {GAS_TYPE_N_BUTANE,      425.160, 227.85, 0.213400},
  {GAS_TYPE_ISO_PENTANE,   460.390, 236.00, 0.261960},
  {GAS_TYPE_N_PENTANE,     469.650, 232.00, 0.295560},
  {GAS_TYPE_HEXANE,        507.850, 233.60, 0.299650},
  {GAS_TYPE_NITROGEN,      126.200, 313.10, 0.013592},
  {GAS_TYPE_CARBON_DIOXIDE,304.200, 468.00, 0.206250},
  {GAS_TYPE_HELIUM,        5.19000,  69.64, -0.14949},
  {GAS_TYPE_HYDROGEN,      32.9380,  31.36, -0.12916}
};

const critical_params *get_critical_params(gas_t gas_name) {
  return get_coefs(A5_critical_params, GET_ARRAY_SIZE(A5_critical_params), gas_name);
}

/* not used 11_08_2019 */
const A6_coef A6_coefs[] = {
  {GAS_TYPE_HYDROGEN,      -0.279070091, 7.81221301,  -0.699863421, 0.0378831186},
  {GAS_TYPE_CARBON_DIOXIDE,-0.468233636, 5.37907799,  -0.034963335, -0.0126198032},
  {GAS_TYPE_METHANE,       -0.838029104, 4.88406903,  -0.344504244, 0.0151593109},
  {GAS_TYPE_ETHANE,        -1.219244900, 4.05145591,  -0.200150993, 0.00662746099},
  {GAS_TYPE_PROPANE,        0.254518256, 2.54779249,  0.0683095277, -0.0114348793},
  {GAS_TYPE_N_BUTANE,      -0.524058048, 2.81260308, -0.0496574363, 0.0},
  {GAS_TYPE_ISO_BUTANE,     1.042738430, 1.69220741,  0.1940774190, -0.0159867334},
  {GAS_TYPE_N_PENTANE,      0.452603096, 1.79775689,  0.1570027760, -0.0158057627},
  {GAS_TYPE_ISO_PENTANE,    0.550744125, 1.75702204,  0.1733634560, -0.0167839786},
  {GAS_TYPE_HEXANE,         0.658064311, 1.50818329,  0.1782800270, -0.0161050134},
  {GAS_TYPE_HELIUM,         2.959298170, 7.17751320, -0.6411919460,  0.0451852767},
  {GAS_TYPE_HYDROGEN,       1.424108950, 3.03739469, -0.2030487370,  0.0106137856}
};

const A6_coef *get_A6_coefs(gas_t gas_name) {
  return get_coefs(A6_coefs, GET_ARRAY_SIZE(A6_coefs), gas_name);
}

const A7_coef A7_coefs[] = {
  {3.063313020,   1, 1},  // 1
  {-8.64573627,   1, 2},  // 2
  { 8.96123185,   1, 3},  // 3
  {-3.00860053,   1, 4},  // 4
  { 1.27196662,   2, 1},  // 5
  {-0.875183697,  2, 2},  // 6
  {-0.0577055575, 3, 1},  // 7
  { 0.0352272638, 5, 1}   // 8
};

const int A8_sigmas[] = {1, 1, 0, 1, 0, 1};

const A8_coef A8_coefs[] = {
  {GAS_TYPE_NITROGEN,       -0.005352690, 0.09101896, 0.01501200,
                             0.26406420,  -0.1032012, -0.1078872},
  {GAS_TYPE_CARBON_DIOXIDE, -0.034682020, 0.11304980, 0.05811886,
                             0.05767935, -0.1814105, -0.5971794},
  {GAS_TYPE_METHANE,         0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
  {GAS_TYPE_ETHANE,          0.041569310,        0.0, 0.06408111,
                             0.04763455, -0.1889656,  0.1533738},
  {GAS_TYPE_PROPANE,         0.039765380, 0.08375624, 0.17471800,
                             1.25027200, -0.5283498,  0.2458511},
  {GAS_TYPE_N_BUTANE,       -0.066677750, 0.21001740, 0.06330205,
                             0.31826600,  0.1474434, -1.1139350},
  {GAS_TYPE_ISO_BUTANE,      0.072349270, 0.00943521,-0.03673568,
                             0.45167220, -0.3272680, -0.6135352},
  {GAS_TYPE_N_PENTANE,              0.0, 0.16511560,-0.07126922,
                             0.06698673, -0.5283166, -0.7803174},
  {GAS_TYPE_ISO_PENTANE,     0.02229787, 0.08380246, 0.04639638,
                            -0.14505830, 0.03725585, -0.4106772},
  {GAS_TYPE_HEXANE,          0.17535290, -0.08018375,-0.03543316,
                            -0.09677546, -0.2015218, -1.2065620},
  {GAS_TYPE_HELIUM,          0.29924900, -0.14909410,-0.15773290,
                            -0.22532400, -0.2731058, -0.8827831},
  {GAS_TYPE_HYDROGEN,       -0.03937273,  0.01532106,-0.03423876,
                            -0.13992090, -0.06955475,-1.0490550}
};

const A8_coef *get_A8_coefs(gas_t gas_name) {
  return get_coefs(A8_coefs, GET_ARRAY_SIZE(A8_coefs), gas_name);
}

#ifndef ISO_20765
const A9_molar_mass A9_molar_masses[] = {
  {GAS_TYPE_OXYGEN,  31.9988},
  {GAS_TYPE_ARGON,   39.9480},
  {GAS_TYPE_HEPTANE, 100.204},
  {GAS_TYPE_OCTANE,  114.231}
};

const A9_molar_mass *get_molar_mass(gas_t gas_name) {
  return get_coefs(A9_molar_masses, GET_ARRAY_SIZE(A9_molar_masses), gas_name);
}
#endif  // !ISO_20765

const A2_SPG_coef A2_SPG_coefs[] = {
  {GAS_TYPE_METHANE, GAS_TYPE_ETHANE,      0.9939062, 0.9932865},
  {GAS_TYPE_METHANE, GAS_TYPE_PROPANE,     1.010338,  0.9964106},
  {GAS_TYPE_METHANE, GAS_TYPE_ISO_BUTANE,  1.029222,  0.9798303},
  {GAS_TYPE_METHANE, GAS_TYPE_N_BUTANE,    1.049264,  0.9709773},
  {GAS_TYPE_METHANE, GAS_TYPE_ISO_PENTANE, 1.339956,  0.8788424},
  {GAS_TYPE_METHANE, GAS_TYPE_N_PENTANE,   1.174340,  0.9302709},
  {GAS_TYPE_METHANE, GAS_TYPE_NITROGEN,    1.007886,  0.9417593},
  {GAS_TYPE_UNDEFINED, GAS_TYPE_UNDEFINED, 1.0, 1.0},
};

// todo: it is a copy of get_binary_associate_coefs
const A2_SPG_coef *get_A2_SPG_coefs(gas_t i, gas_t j) {
  size_t a2spg_coef_count = sizeof(A2_SPG_coefs) / sizeof(A2_SPG_coefs[0]);
  if ((i != GAS_TYPE_METHANE) && (j != GAS_TYPE_METHANE))
    return &(A2_SPG_coefs[a2spg_coef_count - 1]);
  if (i != GAS_TYPE_METHANE)
    std::swap(i, j);
  // now i = GAS_TYPE_METHANE
  size_t z = 0;
  while (A2_SPG_coefs[z].i == i) {
    if (A2_SPG_coefs[z].j == j)
      return &(A2_SPG_coefs[z]);
    ++z;
  }
  return &(A2_SPG_coefs[a2spg_coef_count - 1]);
}

#define d_inf std::numeric_limits<double>::infinity()

const A3_SPG_coef A3_SPG_coefs[40] = {
  {0.04367901028, 1, -0.5, 0, 0, d_inf, d_inf, d_inf, d_inf}, // 1
  {0.67092361990, 1, 0.5, 0, 0, d_inf, d_inf, d_inf, d_inf}, // 2
  {-1.7655778590, 1, 1.0, 0, 0, d_inf, d_inf, d_inf, d_inf}, // 3
  {0.85823302410, 2, 0.5, 0, 0, d_inf, d_inf, d_inf, d_inf}, // 4
  {-1.2065130520, 2, 1.0, 0, 0, d_inf, d_inf, d_inf, d_inf}, // 5
  {0.51204672200, 2, 1.5, 0, 0, d_inf, d_inf, d_inf, d_inf}, // 6
  {-4.000010791e-4, 2.0, 4.5, 0, 0, d_inf, d_inf, d_inf, d_inf}, // 7
  {-0.01247842423, 3, 0.0, 0, 0, d_inf, d_inf, d_inf, d_inf}, // 8
  {0.03100269701, 4, 1.0, 0, 0, d_inf, d_inf, d_inf, d_inf}, // 9
  {1.754748522e-3, 4.0, 3.0, 0, 0, d_inf, d_inf, d_inf, d_inf}, // 10
  {-3.171921605e-6, 8.0, 1.0, 0, 0, d_inf, d_inf, d_inf, d_inf}, // 11
  {-2.24034684e-6, 9.0, 3.0, 0, 0, d_inf, d_inf, d_inf, d_inf}, // 12
  {2.947056156e-7, 1.0, 3.0, 0, 0, d_inf, d_inf, d_inf, d_inf}, // 13
  {0.1830487909, 1, 0.0, -1, 1, d_inf, d_inf, d_inf, d_inf}, // 14
  {0.1511883679, 1, 1.0, -1, 1, d_inf, d_inf, d_inf, d_inf}, // 15
  {-0.4289363877, 1, 2.0, -1, 1, d_inf, d_inf, d_inf, d_inf}, // 16
  {0.06894002446, 2, 0.0, -1, 1, d_inf, d_inf, d_inf, d_inf}, // 17
  {-0.01408313996, 4, 0.0, -1, 1, d_inf, d_inf, d_inf, d_inf}, // 18
  {-0.0306305483, 5, 2.0, -1, 1, d_inf, d_inf, d_inf, d_inf}, // 19
  {-0.02969906708, 6, 2.0, -1, 1, d_inf, d_inf, d_inf, d_inf}, // 20
  {-0.01932040831, 1, 5.0, -1, 2, d_inf, d_inf, d_inf, d_inf}, // 21
  {-0.1105739959, 2, 5.0, -1, 2, d_inf, d_inf, d_inf, d_inf}, // 22
  {0.09952548995, 3, 5.0, -1, 2, d_inf, d_inf, d_inf, d_inf}, // 23
  {8.548437825e-3, 4, 2.0, -1, 2, d_inf, d_inf, d_inf, d_inf}, // 24
  {-0.06150555662, 4, 4.0, -1, 2, d_inf, d_inf, d_inf, d_inf}, // 25
  {-0.04291792423, 3, 12.0, -1, 3, d_inf, d_inf, d_inf, d_inf}, // 26
  {-0.0181320729, 5, 8.0, -1, 3, d_inf, d_inf, d_inf, d_inf}, // 27
  {0.0344590476, 5, 10.0, -1, 3, d_inf, d_inf, d_inf, d_inf}, // 28
  {-2.38591945e-3, 8, 10.0, -1, 3, d_inf, d_inf, d_inf, d_inf}, // 29
  {-0.01159094939, 2, 10.0, -1, 4, d_inf, d_inf, d_inf, d_inf}, // 30
  {0.06641693602, 3, 14.0, -1, 4, d_inf, d_inf, d_inf, d_inf}, // 31
  {-0.0237154959, 4, 12.0, -1, 4, d_inf, d_inf, d_inf, d_inf}, // 32
  {-0.03961624905, 4, 18.0, -1, 4, d_inf, d_inf, d_inf, d_inf}, // 33
  {-0.01387292044, 4, 22.0, -1, 4, d_inf, d_inf, d_inf, d_inf}, // 34
  {0.03389489599, 5, 18.0, -1, 4, d_inf, d_inf, d_inf, d_inf}, // 35
  {-2.927378753e-3, 6, 14.0, -1, 4, d_inf, d_inf, d_inf, d_inf}, // 36
  {9.324799946e-5, 2, 2.0, d_inf, d_inf, -20, -200, 1, 1.07}, // 37
  {-6.287171518, 0, 0.0, d_inf, d_inf, -40, -250, 1, 1.11}, // 38
  {12.71069467, 0, 1.0, d_inf, d_inf, -40, -250, 1, 1.11}, // 39
  {-6.423953466, 0, 2.0, d_inf, d_inf, -40, -250, 1, 1.11}, // 40
};


int A4_SPG_coef::delta[A4_SPG_coef::num] = {1, 1, 0, 1, 0, 1};

A4_SPG_coef A4_SPG_coefs[] = {
  {.gas_name = GAS_TYPE_METHANE, .coefs = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
  {.gas_name = GAS_TYPE_ETHANE, .coefs =
     {-0.05499404, 0.07132088, 0.03411748, 0.3463844, -0.1756987, 0.01181235}},
  {.gas_name = GAS_TYPE_PROPANE, .coefs =
     {-0.1033802, 0.1256433, 0.05515581, 0.3877078, -0.18687, 0.0509911}},
  {.gas_name = GAS_TYPE_ISO_BUTANE, .coefs =
     {-0.1446201, 0.1891534, 0.007255968, 0.3843276, -0.1778766, 0.07948337}},
  {.gas_name = GAS_TYPE_N_BUTANE, .coefs =
     {-0.1330569, 0.1515016, 0.06703781, 0.3101680, -0.1428283, 0.1022543}},
  {.gas_name = GAS_TYPE_ISO_PENTANE, .coefs =
     {-0.1344984, 0.1757778, 0.07751344, 0.4160334, -0.1988925, 0.09967660}},
  {.gas_name = GAS_TYPE_N_PENTANE, .coefs =
     {-0.1500247, 0.1765188, 0.08076395, 0.3802554, -0.1789241, 0.1206911}},
  {.gas_name = GAS_TYPE_NITROGEN, .coefs =
     {-0.0110658, 0.01395339, 0.01517371, 0.04907672, -0.02492141, 0.007076269}},
  {.gas_name = GAS_TYPE_CARBON_DIOXIDE, .coefs = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
};

A5_SPG_coef A5_SPG_coefs[] = {
  {.gas_name = GAS_TYPE_METHANE, .coefs =
     {3.98591747, 0.0944817883, -0.184059518, 0.121670883, 0.0}},
  {.gas_name = GAS_TYPE_ETHANE, .coefs =
     {4.04494534, -2.88738414, 20.4420998, -36.3289167, 24.1231231}},
  {.gas_name = GAS_TYPE_PROPANE, .coefs =
     {3.59984779, -4.14713461, 68.4776240, -163.469780, 133.087884}},
  {.gas_name = GAS_TYPE_ISO_BUTANE, .coefs =
     {3.27383299, -4.49009735, 114.587546, -290.175169, 249.508274}},
  {.gas_name = GAS_TYPE_N_BUTANE, .coefs =
     {1.10821140, 26.764665, 18.9823524, -194.636448, 240.749363}},
  {.gas_name = GAS_TYPE_ISO_PENTANE, .coefs =
     {10.1905588, -104.660203, 586.666061, -1150.48022, 817.341735}},
  {.gas_name = GAS_TYPE_N_PENTANE, .coefs =
     {1.30150258, 7.42798405, 241.151953, -857.021831, 901.466209}},
  {.gas_name = GAS_TYPE_NITROGEN, .coefs =
     {3.50000066, 0.0003858466241, 0.0000744623688, 0.0, 0.0}},
  {.gas_name = GAS_TYPE_CARBON_DIOXIDE, .coefs =
     {3.26743307, 3.04166057, -14.4322345, 28.2801767, -17.1064968}},
};
// clang-format on

molar_parameters calculate_molar_data(ng_gost_mix components) {
  molar_parameters mp;
  mp.mass = 0.0;
  const component_characteristics* xi_ch = nullptr;
  for (size_t i = 0; i < components.size(); ++i) {
    xi_ch = get_characteristics(components[i].first);
    if (xi_ch != nullptr) {
      mp.mass += components[i].second * xi_ch->M;
    } else {
      // свойства компонента газа неизвестны,
      Logging::Append(ERROR_INIT_T,
                      "Ошибка расчёта молярной массы смеси. "
                      "Данные компонента неизвестны.\n"
                      "Функция calculate_molar_data, component"
                          + hex2str(components[i].first));
      mp.mass = 0.0;
      break;
    }
  }
  if (is_above0(mp.mass))
    mp.Rm = 1000.0 * GAS_CONSTANT / mp.mass;
  return mp;
}
