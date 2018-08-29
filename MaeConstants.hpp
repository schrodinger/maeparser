#pragma once
#include <string>

namespace schrodinger
{
namespace mae
{

const char* const MAE_FORMAT_VERSION = "s_m_m2io_version";
const char* const CT_BLOCK = "f_m_ct";
const char* const CT_TITLE = "s_m_title";
const char* const ATOM_BLOCK = "m_atom";
const char* const ATOM_ATOMIC_NUM = "i_m_atomic_number";
const char* const ATOM_X_COORD = "r_m_x_coord";
const char* const ATOM_Y_COORD = "r_m_y_coord";
const char* const ATOM_Z_COORD = "r_m_z_coord";
const char* const ATOM_FORMAL_CHARGE = "i_m_formal_charge";
const char* const ATOM_PARTIAL_CHARGE = "r_m_charge1";
const char* const BOND_BLOCK = "m_bond";
const char* const BOND_ATOM_1 = "i_m_from";
const char* const BOND_ATOM_2 = "i_m_to";
const char* const BOND_ORDER = "i_m_order";

} // End namespace mae
} // End namespace schrodinger
