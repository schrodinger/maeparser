#pragma once

#include <boost/dynamic_bitset.hpp>
#include <cassert>
#include <map>
#include <stdexcept>
#include <cstring>
#include <string>

namespace schrodinger
{
namespace mae
{

template <typename T>
inline const T& get_property(const std::map<std::string, T>& map,
                             const std::string& name)
{
    auto iter = map.find(name);
    if (iter == map.end()) {
        throw std::out_of_range("Key not found: " + name);
    } else {
        return iter->second;
    }
}

// Forward declaration.
class IndexedBlockBuffer;
class IndexedBlock;

class IndexedBlockMapI
{
  public:
    virtual ~IndexedBlockMapI(){};
    virtual bool hasIndexedBlock(const std::string& name) = 0;
    virtual std::shared_ptr<IndexedBlock>
    getIndexedBlock(const std::string& name) = 0;
};

class IndexedBlockMap : public IndexedBlockMapI
{
    std::map<std::string, std::shared_ptr<IndexedBlock>> m_indexed_block;

  public:
    virtual bool hasIndexedBlock(const std::string& name);

    virtual std::shared_ptr<IndexedBlock>
    getIndexedBlock(const std::string& name);

    /**
     * Add an IndexedBlock to the map.
     */
    void addIndexedBlock(const std::string& name,
                         std::shared_ptr<IndexedBlock> indexed_block)
    {
        m_indexed_block[name] = indexed_block;
    }
};

class BufferedIndexedBlockMap : public IndexedBlockMapI
{
  private:
    std::map<std::string, std::shared_ptr<IndexedBlock>> m_indexed_block;
    std::map<std::string, std::shared_ptr<IndexedBlockBuffer>> m_indexed_buffer;

  public:
    virtual bool hasIndexedBlock(const std::string& name);

    virtual std::shared_ptr<IndexedBlock>
    getIndexedBlock(const std::string& name);

    /**
     * Add an IndexedBlockBuffer to the map, which can be used to retrieve an
     * IndexedBlock.
     */
    void addIndexedBlockBuffer(const std::string& name,
                               std::shared_ptr<IndexedBlockBuffer> block_buffer)
    {
        m_indexed_buffer[name] = block_buffer;
    }
};

class Block
{
  private:
    std::string m_name;

    std::map<std::string, bool> m_bmap;
    std::map<std::string, double> m_rmap;
    std::map<std::string, int> m_imap;
    std::map<std::string, std::string> m_smap;
    std::map<std::string, std::shared_ptr<Block>> m_sub_block;
    std::shared_ptr<IndexedBlockMapI> m_indexed_block_map;

    // Prevent copying.
    Block(const Block&);
    Block& operator=(const Block&);

  public:
    Block(std::string name)
        : m_name(name), m_bmap(), m_rmap(), m_imap(), m_smap(),
          m_indexed_block_map(nullptr)
    {
    }

    ~Block();

    const std::string& getName() const { return m_name; }

    void setIndexedBlockMap(std::shared_ptr<IndexedBlockMapI> indexed_block_map)
    {
        m_indexed_block_map = indexed_block_map;
    }

    bool hasIndexedBlock(const std::string& name)
    {
        return m_indexed_block_map->hasIndexedBlock(name);
    }

    std::shared_ptr<IndexedBlock> getIndexedBlock(const std::string& name);

    void addBlock(std::shared_ptr<Block> b) { m_sub_block[b->getName()] = b; }

    /**
     * Check whether this block has a sub-block of the provided name.
     */
    bool hasBlock(const std::string& name)
    {
        std::map<std::string, std::shared_ptr<Block>>::const_iterator iter =
            m_sub_block.find(name);
        return (iter != m_sub_block.end());
    }

    /**
     * Retrieve a shared pointer to the named sub-block.
     */
    std::shared_ptr<Block> getBlock(const std::string& name)
    {
        std::map<std::string, std::shared_ptr<Block>>::const_iterator iter =
            m_sub_block.find(name);
        if (iter == m_sub_block.end()) {
            throw std::out_of_range("Sub-block not found: " + name);
        } else {
            return iter->second;
        }
    }

    bool hasRealProperty(const std::string& name) const
    {
        return (m_rmap.find(name) != m_rmap.end());
    }

    double getRealProperty(const std::string& name) const
    {
        return get_property<double>(m_rmap, name);
    }

    void setRealProperty(const std::string& name, double value)
    {
        m_rmap[name] = value;
    }

    bool hasIntProperty(const std::string& name) const
    {
        return (m_imap.find(name) != m_imap.end());
    }

    int getIntProperty(const std::string& name) const
    {
        return get_property<int>(m_imap, name);
    }

    void setIntProperty(const std::string& name, int value)
    {
        m_imap[name] = value;
    }

    bool hasBoolProperty(const std::string& name) const
    {
        return (m_bmap.find(name) != m_bmap.end());
    }

    bool getBoolProperty(const std::string& name) const
    {
        return get_property<bool>(m_bmap, name);
    }

    void setBoolProperty(const std::string& name, bool value)
    {
        m_bmap[name] = value;
    }

    bool hasStringProperty(const std::string& name) const
    {
        return (m_smap.find(name) != m_smap.end());
    }

    const std::string& getStringProperty(const std::string& name) const
    {
        return get_property<std::string>(m_smap, name);
    }

    void setStringProperty(const std::string& name, std::string value)
    {
        m_smap[name] = value;
    }
};

template <typename T> class IndexedProperty
{
  private:
    std::vector<T> m_data;
    boost::dynamic_bitset<>* m_is_null;

    // Prevent copying.
    IndexedProperty<T>(const IndexedProperty<T>&);
    IndexedProperty<T>& operator=(const IndexedProperty<T>&);

  public:
    typedef typename std::vector<T>::size_type size_type;

    /**
     * Construct an IndexedProperty from a reference to a vector of data.
     * This swaps out the data of the input vector.
     *
     * The optional boost::dynamic_bitset is owned by the created object.
     */
    explicit IndexedProperty<T>(std::vector<T>& data,
                                boost::dynamic_bitset<>* is_null = nullptr)
        : m_data(), m_is_null(is_null)
    {
        m_data.swap(data);
    }

    ~IndexedProperty<T>()
    {
        if (m_is_null != nullptr) {
            delete m_is_null;
        }
    }

    size_type size() const { return m_data.size(); }

    bool hasUndefinedValues() const
    {
        return (m_is_null != NULL && m_is_null->any());
    }

    bool isDefined(size_type index) const
    {
        if (m_is_null == nullptr) {
            // Use of assert matches out-of-bounds behavior for dynamic_bitset.
            assert(index < m_data.size());
            return true;
        } else {
            return !(*m_is_null)[index];
        }
    }

    void undefine(size_type index)
    {
        if (m_is_null == NULL) {
            m_is_null = new boost::dynamic_bitset<>(m_data.size());
        }
        m_is_null->set(index);
    }

    inline T& operator[](size_type index)
    {
        if (m_is_null && (*m_is_null)[index]) {
            throw std::runtime_error("Indexed property value undefined.");
        }
        return m_data[index];
    }

    inline const T& operator[](size_type index) const
    {
        if (m_is_null && (*m_is_null)[index]) {
            throw std::runtime_error("Indexed property value undefined.");
        }
        return m_data[index];
    }

    inline T& at(size_type index)
    {
        return operator[](index);
    }

    inline const T& at(size_type index) const
    {
        return operator[](index);
    }

    void set(size_type index, const T& value)
    {
        m_data[index] = value;
        if (m_is_null != NULL && (*m_is_null)[index]) {
            m_is_null->reset(index);
        }
    }
};

typedef IndexedProperty<double> IndexedRealProperty;
typedef IndexedProperty<int> IndexedIntProperty;
typedef IndexedProperty<bool> IndexedBoolProperty;
typedef IndexedProperty<std::string> IndexedStringProperty;

template <typename T>
inline std::shared_ptr<T>
get_indexed_property(const std::map<std::string, std::shared_ptr<T>>& map,
                     const std::string& name)
{
    auto iter = map.find(name);
    if (iter == map.end()) {
        throw std::out_of_range("Key not found: " + name);
    } else {
        return iter->second;
    }
}

template <typename T>
inline void set_indexed_property(std::map<std::string, std::shared_ptr<T>>& map,
                                 const std::string& name,
                                 const std::shared_ptr<T> value)

{
    auto iter = map.find(name);
    if (iter != map.end()) {
        map.erase(iter);
    }
    map.insert(std::pair<std::string, std::shared_ptr<T>>(name, value));
}

class IndexedBlock
{
  private:
    const std::string m_name;

    std::map<std::string, std::shared_ptr<IndexedBoolProperty>> m_bmap;
    std::map<std::string, std::shared_ptr<IndexedIntProperty>> m_imap;
    std::map<std::string, std::shared_ptr<IndexedRealProperty>> m_rmap;
    std::map<std::string, std::shared_ptr<IndexedStringProperty>> m_smap;

    // Prevent copying.
    IndexedBlock(const IndexedBlock&);
    IndexedBlock& operator=(const IndexedBlock&);

  public:
    /**
     * Create an indexed block.
     */
    IndexedBlock(const std::string& name)
        : m_name(name), m_bmap(), m_imap(), m_rmap(), m_smap()
    {
    }

    // Default destructor. ~IndexedBlock()

    template <typename T>
    void setProperty(const std::string& name,
                     std::shared_ptr<IndexedProperty<T>> value);

    std::shared_ptr<IndexedBoolProperty>
    getBoolProperty(const std::string& name) const
    {
        return get_indexed_property<IndexedBoolProperty>(m_bmap, name);
    }

    void setBoolProperty(const std::string& name,
                         const std::shared_ptr<IndexedBoolProperty> value)
    {
        set_indexed_property<IndexedBoolProperty>(m_bmap, name, value);
    }

    std::shared_ptr<IndexedIntProperty>
    getIntProperty(const std::string& name) const
    {
        return get_indexed_property<IndexedIntProperty>(m_imap, name);
    }

    void setIntProperty(const std::string& name,
                        const std::shared_ptr<IndexedIntProperty> value)
    {
        set_indexed_property<IndexedIntProperty>(m_imap, name, value);
    }

    std::shared_ptr<IndexedRealProperty>
    getRealProperty(const std::string& name) const
    {
        return get_indexed_property<IndexedRealProperty>(m_rmap, name);
    }

    void setRealProperty(const std::string& name,
                         const std::shared_ptr<IndexedRealProperty> value)
    {
        set_indexed_property<IndexedRealProperty>(m_rmap, name, value);
    }

    std::shared_ptr<IndexedStringProperty>
    getStringProperty(const std::string& name) const
    {
        return get_indexed_property<IndexedStringProperty>(m_smap, name);
    }

    void setStringProperty(const std::string& name,
                           const std::shared_ptr<IndexedStringProperty> value)
    {
        set_indexed_property<IndexedStringProperty>(m_smap, name, value);
    }
};

} // namespace mae
} // namespace schrodinger
