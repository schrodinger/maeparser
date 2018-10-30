#include "MaeBlock.hpp"
#include "MaeParser.hpp"

using namespace std;

namespace schrodinger
{
namespace mae
{

static const double tolerance = 0.00001; // Tolerance to match string cutoff


// Wrap to-string to allow it to take strings and be a no-op
template <typename T>
static string local_to_string(T val)
{
    return to_string(val);
}

static string local_to_string(string val)
{
    if(val.length() == 0) return R"("")";
    // Create new string big enough to escape every character and add quotes
    int pos_in_old = 0;
    bool escaped_char = false;
    for(; pos_in_old<val.length(); pos_in_old++) {
        const char& c = val[pos_in_old];
        if(c == '"' || c == '\\' || c == ' ') {
            escaped_char = true;
            break;
        }
    }
    if(!escaped_char) return val;

    int pos_in_new = 1;
    string new_string(val.length()*2 + 2, '\"');
    for(pos_in_old = 0; pos_in_old<val.length(); pos_in_old++)
    {
        const char& c = val[pos_in_old];
        if(c == '"' || c == '\\') {
            new_string[pos_in_new++] = '\\';
        }
        new_string[pos_in_new++] = val[pos_in_old];
    }
    new_string.resize(pos_in_new+1);
    return new_string;
}

template <typename T>
static void output_property_names(ostream& out,
        const string& indentation, map<string, T> properties)
{
    for(const auto& p : properties) {
        out << indentation << p.first << "\n";
    }
}

template <typename T>
static void output_property_values(ostream& out,
        const string& indentation, map<string, T> properties)
{
    for(const auto& p : properties) {
        out << indentation << local_to_string(p.second) << "\n";
    }
}

Block::~Block()
{
    // TODO: check with valgrind whether some destructor is needed
}

void Block::write(ostream& out, unsigned int current_indentation) const
{

    string root_indentation = string(current_indentation, ' ');
    string indentation = string(current_indentation+2, ' ');

    out << root_indentation << getName() << " {\n";

    output_property_names(out, indentation, m_bmap);
    output_property_names(out, indentation, m_rmap);
    output_property_names(out, indentation, m_imap);
    output_property_names(out, indentation, m_smap);

    if(m_bmap.size() + m_rmap.size() + m_imap.size() + m_smap.size() > 0) {
        out << indentation + ":::\n";
    }

    output_property_values(out, indentation, m_bmap);
    output_property_values(out, indentation, m_rmap);
    output_property_values(out, indentation, m_imap);
    output_property_values(out, indentation, m_smap);

    if(hasIndexedBlockData()) {
        const auto block_names = m_indexed_block_map->getBlockNames();
        for(const auto& name : block_names) {
            const auto& indexed_block = m_indexed_block_map->getIndexedBlock(name);
            indexed_block->write(out, current_indentation+2);
        }
    }

    for(const auto& p : m_sub_block) {
        const auto& sub_block = p.second;
        sub_block->write(out, current_indentation+2);
    }


    out << root_indentation << "}\n\n";

    return;
}

string Block::toString() const
{
    ostringstream stream;
    write(stream);

    return stream.str();
}


shared_ptr<IndexedBlock> Block::getIndexedBlock(const string& name)
{
    if(!hasIndexedBlockData()) {
        throw out_of_range("Indexed block not found: " + name);
    }
    return m_indexed_block_map->getIndexedBlock(name);
}

bool rmap_equal(map<string, double> rmap1,
        map<string, double> rmap2)
{
    for(const auto& p : rmap1) {
        if(rmap2.count(p.first) != 1) return false;
        if(abs(p.second - rmap2[p.first]) > tolerance) return false;
    }

    return true;
}

bool Block::operator==(const Block& rhs) const
{
    if(m_bmap != rhs.m_bmap) return false;
    if(!rmap_equal(m_rmap, rhs.m_rmap)) return false;
    if(m_imap != rhs.m_imap) return false;
    if(m_smap != rhs.m_smap) return false;
    if(m_sub_block != rhs.m_sub_block) return false;
    if(!(*m_indexed_block_map == *(rhs.m_indexed_block_map))) return false;
    return true;
}


bool IndexedBlockMapI::operator==(const IndexedBlockMapI& rhs)
{
    const auto& block_names = getBlockNames();
    for(const auto& name : block_names) {
        if(!rhs.hasIndexedBlock(name)) return false;
        const auto& block1 = rhs.getIndexedBlock(name);
        const auto& block2 = getIndexedBlock(name);
        if(*block1 != *block2) return false;
    }
    return true;
}

bool IndexedBlockMap::hasIndexedBlock(const string& name) const
{
    return m_indexed_block.find(name) != m_indexed_block.end();
}

shared_ptr<IndexedBlock>
IndexedBlockMap::getIndexedBlock(const string& name) const
{
    map<string, shared_ptr<IndexedBlock>>::const_iterator itb =
        m_indexed_block.find(name);
    if (itb != m_indexed_block.end()) {
        return itb->second;
    } else {
        throw out_of_range("Indexed block not found: " + name);
    }
}

bool BufferedIndexedBlockMap::hasIndexedBlock(const string& name) const
{
    if (m_indexed_buffer.find(name) != m_indexed_buffer.end()) {
        return true;
    } else if (m_indexed_block.find(name) != m_indexed_block.end()) {
        return true;
    } else {
        return false;
    }
}

shared_ptr<IndexedBlock>
BufferedIndexedBlockMap::getIndexedBlock(const string& name) const
{
    map<string, shared_ptr<IndexedBlock>>::const_iterator itb =
        m_indexed_block.find(name);
    if (itb != m_indexed_block.end()) {
        return itb->second;
    }

    auto itbb = m_indexed_buffer.find(name);
    if (itbb == m_indexed_buffer.end()) {
        throw out_of_range("Indexed block not found: " + name);
    } else {
        shared_ptr<IndexedBlock> ib(itbb->second->getIndexedBlock());
        return ib;
    }
}

template <>
EXPORT_MAEPARSER void IndexedBlock::setProperty<BoolProperty>(
    const string& name, shared_ptr<IndexedBoolProperty> value)
{
    set_indexed_property<IndexedBoolProperty>(m_bmap, name, value);
}

template <>
EXPORT_MAEPARSER void IndexedBlock::setProperty<double>(
    const string& name, shared_ptr<IndexedProperty<double>> value)
{
    set_indexed_property<IndexedProperty<double>>(m_rmap, name, value);
}

template <>
EXPORT_MAEPARSER void
IndexedBlock::setProperty<int>(const string& name,
                               shared_ptr<IndexedProperty<int>> value)
{
    set_indexed_property<IndexedProperty<int>>(m_imap, name, value);
}

template <>
EXPORT_MAEPARSER void IndexedBlock::setProperty<string>(
    const string& name,
    shared_ptr<IndexedProperty<string>> value)
{
    set_indexed_property<IndexedProperty<string>>(m_smap, name, value);
}

size_t IndexedBlock::size() const
{
    size_t count = 0;
    for(const auto& p : m_bmap) count = max(p.second->size(), count);
    for(const auto& p : m_imap) count = max(p.second->size(), count);
    for(const auto& p : m_rmap) count = max(p.second->size(), count);
    for(const auto& p : m_smap) count = max(p.second->size(), count);

    return count;
}


template <typename T>
static void output_indexed_property_values(ostream& out,
        const string& indentation, map<string, T> properties,
        unsigned int index)
{
    for(const auto& p : properties) {
        const auto& property = p.second;
        if(property->isDefined(index)) {
            out << " " << local_to_string(property->at(index));
        } else {
            out << " <>";
        }
    }
}

void IndexedBlock::write(ostream& out, unsigned int current_indentation) const
{
    string root_indentation = string(current_indentation, ' ');
    string indentation = string(current_indentation+2, ' ');
    const bool has_data = m_bmap.size() + m_rmap.size() + m_imap.size() + m_smap.size() > 0;

    out << root_indentation << getName() << "[" << to_string((int)size()) << "] {\n";

    if(has_data) {
        out << indentation + "# First column is Index #\n";
    }

    output_property_names(out, indentation, m_bmap);
    output_property_names(out, indentation, m_rmap);
    output_property_names(out, indentation, m_imap);
    output_property_names(out, indentation, m_smap);

    if(has_data) {
        out << indentation + ":::\n";
    }

    for(unsigned int i=0; i<size(); i++) {
        out << indentation << i+1;
        output_indexed_property_values(out, indentation, m_bmap, i);
        output_indexed_property_values(out, indentation, m_rmap, i);
        output_indexed_property_values(out, indentation, m_imap, i);
        output_indexed_property_values(out, indentation, m_smap, i);
        out << endl;
    }

    if(has_data) {
        out << indentation + ":::\n";
    }

    out << root_indentation << "}\n";

    return;
}

string IndexedBlock::toString() const
{
    ostringstream stream;
    write(stream);

    return stream.str();
}

template <typename T>
bool IndexedProperty<T>::operator==(const IndexedProperty<T>& rhs) const
{
    if(m_is_null == nullptr || rhs.m_is_null == nullptr) {
        if((m_is_null == nullptr) != (rhs.m_is_null == nullptr)) return false;
    } else if(*m_is_null != *(rhs.m_is_null)) {
        return false;
    }
    if(m_data != rhs.m_data) return false;
    return true;
}


// For doubles we need to implement our own comparator for the vectors to
// take precision into account
template <>
bool IndexedProperty<double>::operator==(const IndexedProperty<double>& rhs) const
{
    if(m_is_null == nullptr || rhs.m_is_null == nullptr) {
        if((m_is_null == nullptr) != (rhs.m_is_null == nullptr)) return false;
    } else if(*m_is_null != *(rhs.m_is_null)) {
        return false;
    }
    for(int i=0; i<m_data.size(); i++)
        if(abs(m_data[i] - rhs.m_data[i]) > tolerance) return false;

    return true;
}

bool IndexedBlock::operator==(const IndexedBlock& rhs) const
{
    using namespace std;

    for(const auto& p : m_bmap) {
        if(rhs.m_bmap.count(p.first) != 1) return false;
        const auto& bmap2 = rhs.m_bmap.find(p.first);
        if(!(*(bmap2->second) == *(p.second))) return false;
    }

    for(const auto& p : m_imap) {
        if(rhs.m_imap.count(p.first) != 1) return false;
        const auto& imap2 = rhs.m_imap.find(p.first);
        if(!(*(imap2->second) == *(p.second))) return false;
    }

    for(const auto& p : m_rmap) {
        if(rhs.m_rmap.count(p.first) != 1) return false;
        const auto& rmap2 = rhs.m_rmap.find(p.first);
        if(!(*(rmap2->second) == *(p.second))) return false;
    }

    for(const auto& p : m_smap) {
        if(rhs.m_smap.count(p.first) != 1) return false;
        const auto& smap2 = rhs.m_smap.find(p.first);
        if(!(*(smap2->second) == *(p.second))) return false;
    }

    return true;
}

} // namespace mae
} // namespace schrodinger
