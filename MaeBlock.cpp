#include "MaeBlock.hpp"
#include "MaeParser.hpp"

namespace schrodinger
{
namespace mae
{

Block::~Block()
{
    // TODO: check with valgrind whether some destructor is needed
}

std::shared_ptr<IndexedBlock> Block::getIndexedBlock(const std::string& name)
{
    return m_indexed_block_map->getIndexedBlock(name);
}

bool IndexedBlockMap::hasIndexedBlock(const std::string& name)
{
    return m_indexed_block.find(name) != m_indexed_block.end();
}

std::shared_ptr<IndexedBlock>
IndexedBlockMap::getIndexedBlock(const std::string& name)
{
    std::map<std::string, std::shared_ptr<IndexedBlock>>::const_iterator itb =
        m_indexed_block.find(name);
    if (itb != m_indexed_block.end()) {
        return itb->second;
    } else {
        throw std::out_of_range("Indexed block not found: " + name);
    }
}

bool BufferedIndexedBlockMap::hasIndexedBlock(const std::string& name)
{
    if (m_indexed_buffer.find(name) != m_indexed_buffer.end()) {
        return true;
    } else if (m_indexed_block.find(name) != m_indexed_block.end()) {
        return true;
    } else {
        return false;
    }
}

std::shared_ptr<IndexedBlock>
BufferedIndexedBlockMap::getIndexedBlock(const std::string& name)
{
    std::map<std::string, std::shared_ptr<IndexedBlock>>::const_iterator itb =
        m_indexed_block.find(name);
    if (itb != m_indexed_block.end()) {
        return itb->second;
    }

    auto itbb = m_indexed_buffer.find(name);
    if (itbb == m_indexed_buffer.end()) {
        throw std::out_of_range("Indexed block not found: " + name);
    } else {
        std::shared_ptr<IndexedBlock> ib(itbb->second->getIndexedBlock());
        m_indexed_buffer.erase(itbb);
        return ib;
    }
}

template <>
void IndexedBlock::setProperty<bool>(
    const std::string& name, std::shared_ptr<IndexedProperty<bool>> value)
{
    set_indexed_property<IndexedProperty<bool>>(m_bmap, name, value);
}

template <>
void IndexedBlock::setProperty<double>(
    const std::string& name, std::shared_ptr<IndexedProperty<double>> value)
{
    set_indexed_property<IndexedProperty<double>>(m_rmap, name, value);
}

template <>
void IndexedBlock::setProperty<int>(const std::string& name,
                                    std::shared_ptr<IndexedProperty<int>> value)
{
    set_indexed_property<IndexedProperty<int>>(m_imap, name, value);
}

template <>
void IndexedBlock::setProperty<std::string>(
    const std::string& name,
    std::shared_ptr<IndexedProperty<std::string>> value)
{
    set_indexed_property<IndexedProperty<std::string>>(m_smap, name, value);
}

} // namespace mae
} // namespace schrodinger
