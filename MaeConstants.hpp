#pragma once
#include <string>

namespace schrodinger
{
namespace mae
{

const std::string MAE_FORMAT_VERSION = "s_m_m2io_version";
const std::string CT_BLOCK = "f_m_ct";
const std::string CT_TITLE = "s_m_title";
const std::string ATOM_CT_BLOCK = "m_atom";
const std::string ATOM_ATOMIC_NUM = "i_m_atomic_number";
const std::string ATOM_X_COORD = "r_m_x_coord";
const std::string ATOM_Y_COORD = "r_m_y_coord";
const std::string ATOM_Z_COORD = "r_m_z_coord";
const std::string ATOM_CHARGE = "i_m_formal_charge";
const std::string BOND_CT_BLOCK = "m_bond";
const std::string BOND_FROM_ATOM = "i_m_from";
const std::string BOND_TO_ATOM = "i_m_to";
const std::string BOND_ORDER = "i_m_order";

} // End namespace mae
} // End namespace schrodinger
