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

/**
 * These are the prefixes used to store stereochemical properties in Maestro
 * files. Each of those properties will be associated to one of these prefixes
 * plus an integer (integers don't have any particular meaning, they just
 * prevent the property names from clashing), e.g.:
 *
 * s_st_Chirality_1 :        2_R_1_3_5_4
 * s_st_AtomNumChirality_4:  6_ANR_1_5_7
 *
 * Syntax of chirality and pseudochirality properties is the same:
 *  1. Index of the chiral/pseudochiral atom.
 *  2. Chiral/pseudochiral label (R/S or ANR/ANS).
 *  3. Sequence of atoms around the chiral one, ordered by decreasing
 *     Cahn-Ingold-Prelog rank.
 *  Elements are separeted by underscores. For cases in which only 3 neighboring
 *  indexes are specified, an implicit Hydrogen atom needs to be considered.
 *
 * s_st_EZ_2 :               1_2_3_4_E
 *
 * Syntax for bond stereochemistry properties is similar:
 *  1. Highest C-I-P ranked stereogenical atom at one side of the group.
 *  2. List of intermediate atoms in the stereochemical group. These are
 *     typically 2 atoms bonded by a double bond, but can be more atoms and not
 *     necessarily bonded by double bonds (e.g. in case of allenes or
 *     allene-like structures)
 *  3. Highest C-I-P ranked stereogenical atom at the other side of the group.
 *  4. Stereochemical label for the bond (E/Z)
 *  As in chiral/pseudochiral properties, values are separeted by underscores.
 */
const std::string CT_CHIRALITY_PROP_PREFIX = "s_st_Chirality_";
const std::string CT_PSEUDOCHIRALITY_PROP_PREFIX = "s_st_AtomNumChirality_";
const std::string CT_EZ_PROP_PREFIX = "s_st_EZ_";

} // End namespace mae
} // End namespace schrodinger
