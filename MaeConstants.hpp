#pragma once
#include <string>

namespace schrodinger
{
namespace mae
{

constexpr const char* MAE_FORMAT_VERSION = "s_m_m2io_version";
constexpr const char* CT_BLOCK = "f_m_ct";
constexpr const char* CT_TITLE = "s_m_title";
constexpr const char* ATOM_BLOCK = "m_atom";
constexpr const char* ATOM_ATOMIC_NUM = "i_m_atomic_number";
constexpr const char* ATOM_X_COORD = "r_m_x_coord";
constexpr const char* ATOM_Y_COORD = "r_m_y_coord";
constexpr const char* ATOM_Z_COORD = "r_m_z_coord";
constexpr const char* ATOM_FORMAL_CHARGE = "i_m_formal_charge";
constexpr const char* ATOM_PARTIAL_CHARGE = "r_m_charge1";
constexpr const char* BOND_BLOCK = "m_bond";
constexpr const char* BOND_ATOM_1 = "i_m_from";
constexpr const char* BOND_ATOM_2 = "i_m_to";
constexpr const char* BOND_ORDER = "i_m_order";

} // End namespace mae
} // End namespace schrodinger
