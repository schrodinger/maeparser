///
// Demonstrate reading a schrodinger/Maestro formatted file and gleaning the
// bonding information, coordinates, and atomic number.
//
// Note that Maestro format uses 1-based indices when referring to indexed
// data in blocks. (for example, the atom numbers in bond blocks are 1 indexed.)
//
// Maestro "structures" may contain multiple non-bonded molecules in a
// coherent environment. For instance, both a ligand and a receptor may exist
// in a single f_m_ct block.
//
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <array>
#include <vector>
#include <unordered_map>

#include "Reader.hpp"

#include <boost/test/unit_test.hpp>


// These classes are not intended for production use. The are merely intended
// to illustrate where data is stored in the "block" data structures.
class Bond
{
public:
    Bond (int atom0, int atom1, int bond_order): atom0(atom0), atom1(atom1), bond_order(bond_order) {}

    const int atom0;
    const int atom1;
    const int bond_order;
};

class Structure
{
public:
    std::string title;
    std::vector<int> atomic_numbers;
    std::vector<std::array<double, 3>> coordinates;

    std::vector<Bond> bonds;

    // A "property" that some atoms have (others may not have this property)
    std::unordered_map<int, int> demo_property;
};


BOOST_AUTO_TEST_SUITE(DemoSuite)

// Reads all atom and bond information from test.mae, which is a standard
// mae formatted file. Only accesses properties that are gauranteed to
// exist in every f_m_ct block.
BOOST_AUTO_TEST_CASE(maeBlock)
{
    std::ifstream ss("test.mae");
    schrodinger::mae::Reader r(ss);

    std::vector<std::shared_ptr<Structure>> structures;
    std::shared_ptr<schrodinger::mae::Block> b;
    while ((b = r.next("f_m_ct")) != nullptr) {
        auto st = std::make_shared<Structure>();
        st->title = b->getStringProperty("s_m_title");

        // Atom data is in the m_atom indexed block
        {
            const auto atom_data = b->getIndexedBlock("m_atom");
            // All atoms are gauranteed to have these three field names:
            const auto atomic_numbers = atom_data->getIntProperty("i_m_atomic_number");
            const auto xs = atom_data->getRealProperty("r_m_x_coord");
            const auto ys = atom_data->getRealProperty("r_m_y_coord");
            const auto zs = atom_data->getRealProperty("r_m_z_coord");
            const auto size = atomic_numbers->size();
            BOOST_REQUIRE_EQUAL(size, xs->size());
            BOOST_REQUIRE_EQUAL(size, ys->size());
            BOOST_REQUIRE_EQUAL(size, zs->size());

            // atomic numbers, and x, y, and z coordinates
            for (size_t i=0; i<size; ++i) {
                st->atomic_numbers.push_back(atomic_numbers->at(i));
                st->coordinates.push_back({{xs->at(i), ys->at(i), zs->at(i)}});
            }

            // Other properties could fail, because not all properties have
            // values for all atoms. The last atom of the first structure does
            // not have the "i_m_template_index" property.
            const auto template_indices = atom_data->getIntProperty("i_m_template_index");
            for (size_t i=0; i<size; ++i) {
                if (template_indices->isDefined(i)) {
                    st->demo_property[i] = template_indices->at(i);
                }
            }
        }

        // Bond data is in the m_bond indexed block
        {
            const auto bond_data = b->getIndexedBlock("m_bond");
            // All bonds are gauranteed to have these three field names:
            auto from_atoms = bond_data->getIntProperty("i_m_from");
            auto to_atoms = bond_data->getIntProperty("i_m_to");
            auto orders = bond_data->getIntProperty("i_m_order");
            const auto size = from_atoms->size();

            for (size_t i=0; i<size; ++i) {
                // Atom indices in the bond data structure are 1 indexed!
                const auto from_atom = from_atoms->at(i) - 1;
                const auto to_atom = to_atoms->at(i) - 1;
                const auto order = orders->at(i);

                // Only one direction of the bond is recorded in the file
                st->bonds.emplace_back(from_atom, to_atom, order);
                st->bonds.emplace_back(to_atom, from_atom, order);
            }
        }

        structures.push_back(st);
    }

    // Check that all three f_m_ct blocks were read.
    BOOST_CHECK_EQUAL(structures.size(), 3u);
}

BOOST_AUTO_TEST_SUITE_END()
