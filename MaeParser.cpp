#include <cstdio>
#include <cstdlib>

#include <boost/spirit/include/qi_parse_attr.hpp>
#include <boost/spirit/include/qi_numeric.hpp>

#include "MaeParser.hpp"
#include "MaeBlock.hpp"

#define WHITESPACE ' ' : case '\n' : case '\r' : case '\t'

namespace qi = boost::spirit::qi;

namespace schrodinger
{

namespace mae
{

static bool property_key_author_name(Buffer& buffer, char*& save);

static std::string outer_block_name(Buffer& buffer);

const int read_exception::BUFFER_SIZE;

void read_exception::format(int line_number, int column, const char* msg)
{
    snprintf(m_msg, BUFFER_SIZE, "Line %d, column %d: %s\n", line_number,
             column, msg);
    m_msg[BUFFER_SIZE - 1] = '\0';
}

// TODO: Not sure that newlines embedded in comments are allowed.
void comment(Buffer& buffer)
{
    ++buffer.current; // Step past initial '#'
    while (buffer.current < buffer.end || buffer.load()) {
        switch (*buffer.current) {
        case '#':
            return;
        case '\n':
            ++buffer.line_number;
        }
        ++buffer.current;
    }
    throw read_exception(buffer, "Unterminated comment.");
}

void whitespace(Buffer& buffer)
{
    while (buffer.current < buffer.end || buffer.load()) {
        switch (*buffer.current) {
        case '\n':
            ++buffer.line_number;
        case '\r':
        case ' ':
        case '\t':
            break;
        case '#':
            comment(buffer);
            break;
        default:
            return;
        }
        ++buffer.current;
    }
}

bool character(char c, Buffer& buffer)
{
    char* save = nullptr;
    return character(c, buffer, save);
}

bool character(char c, Buffer& buffer, char*& save)
{
    if (buffer.current >= buffer.end && !buffer.load(save)) {
        return false;
    } else if (*buffer.current != c) {
        return false;
    } else {
        ++buffer.current;
        return true;
    }
}

/**
 * Read an integer and return its value. An integer is terminated
 * either by whitespace or a ']'.
 */
template <> int parse_value<int>(Buffer& buffer)
{
    int value = 0;
    int sign = 1;

    char* save = buffer.current;
    while (buffer.current < buffer.end || buffer.load()) {
        switch (*buffer.current) {
        case ']':
        case WHITESPACE:
            if (save == buffer.current) {
                throw read_exception(buffer, "Missing integer.");
            }
            return value * sign;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            value = value * 10 + *buffer.current - '0';
            break;
        case '-':
            if (sign == -1 || value) {
                throw read_exception(buffer, "Unexpected '-'.");
            }
            sign = -1;
            break;
        default:
            throw read_exception(buffer, "Unexpected character.");
        }
        ++buffer.current;
    }
    return value * sign;
}

template <> double parse_value<double>(Buffer& buffer)
{
    char* save = buffer.current;
    while (buffer.current < buffer.end || buffer.load(save)) {
        switch (*buffer.current) {
        case '-':
        case '.':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 'e':
        case 'E':
            break;
        case WHITESPACE:
            goto done;
        default:
            throw read_exception(buffer,
                                 "Unexpected character in real number.");
        }
        ++buffer.current;
    }

done:
    if (save == buffer.current) {
        throw read_exception(buffer, "Missing real.");
    }

    double value = 0;
    if (!qi::parse(save, buffer.current, qi::double_, value) ||
        save != buffer.current) {
        // On error, save will have advanced to the point of the problem.
        // May differ on versions of boost
        throw read_exception(buffer.line_number, buffer.getColumn(save),
                             "Bad real number.");
    }
    return value;
}

template <> std::string parse_value<std::string>(Buffer& buffer)
{
    char* save = buffer.current;
    if (*buffer.current != '"') {
        while (buffer.current < buffer.end || buffer.load(save)) {
            switch (*buffer.current) {
            case WHITESPACE:
                return std::string(save, buffer.current);
            }
            ++buffer.current;
        }
        return std::string(save, buffer.current);
    } else {
        save = ++buffer.current;
        while (buffer.current < buffer.end || buffer.load(save)) {
            switch (*buffer.current) {
            case '"':
                return std::string(save, buffer.current++);
            case '\\':
                ++buffer.current;
                break;
            }
            ++buffer.current;
        }
        throw read_exception(buffer, "Unterminated quoted string at EOF.");
    }
}

template <> bool parse_value<bool>(Buffer& buffer)
{
    bool value = false;
    if (*buffer.current == '1') {
        value = true;
    } else if (*buffer.current == '0') {
        value = false;
    } else {
        throw read_exception(buffer, "Unexpected character for boolean value.");
    }
    ++buffer.current;

    if (buffer.current >= buffer.end) {
        if (!buffer.load()) {
            return value;
        }
    }

    switch (*buffer.current) {
    case WHITESPACE:
        return value;
    default:
        throw read_exception(buffer, "Unexpected character for boolean value.");
    }
}

std::string outer_block_beginning(Buffer& buffer)
{
    std::string name = outer_block_name(buffer);
    schrodinger::mae::whitespace(buffer);
    if (!character('{', buffer)) {
        throw read_exception(buffer, "Missing '{' for outer block.");
    }
    return name;
}

std::shared_ptr<Block> MaeParser::outerBlock()
{
    if (!m_buffer.load()) {
        return nullptr;
    }
    std::string name = outer_block_beginning(m_buffer);
    return blockBody(name);
}

std::string outer_block_name(Buffer& buffer)
{
    char* save = buffer.current;
    char c = *buffer.current;
    if (c == '{') {
        return std::string();
    } else if (c != 'f' && c != 'p') {
        goto bad_format;
    }
    ++buffer.current;

    if (!character('_', buffer, save)) {
        goto bad_format;
    }
    if (!property_key_author_name(buffer, save)) {
        goto bad_format;
    }

    return std::string(save, buffer.current - save);

bad_format:
    throw read_exception(buffer, "Bad format for outer block name; "
                                 "must be (f|p)_<author>_<name>.");
}

std::string MaeParser::blockBeginning(int* indexed)
{
    *indexed = 0;

    char* save = m_buffer.current;
    if (!property_key_author_name(m_buffer, save)) {
        throw read_exception(m_buffer, "Bad format for block name; "
                                       "must be <author>_<name>.");
    }
    std::string name(save, m_buffer.current - save);

    schrodinger::mae::whitespace(m_buffer);

    if (character('[', m_buffer)) {
        schrodinger::mae::whitespace(
            m_buffer); // TODO: is m_block[ 123 ] allowed?
        *indexed = parse_value<int>(m_buffer);
        schrodinger::mae::whitespace(m_buffer);
        if (!character(']', m_buffer)) {
            throw read_exception(m_buffer, "Bad block index; missing ']'.");
        }
        schrodinger::mae::whitespace(m_buffer);
    }

    if (character('{', m_buffer)) {
        return name;
    } else {
        throw read_exception(m_buffer, "Missing '{' for block.");
    }
}

std::shared_ptr<Block> MaeParser::blockBody(const std::string& name)
{
    auto block = std::make_shared<Block>(name);
    IndexedBlockParser* indexed_block_parser = getIndexedBlockParser();

    std::vector<std::shared_ptr<std::string>> property_names;
    schrodinger::mae::whitespace(m_buffer);
    properties(&property_names);

    std::vector<std::shared_ptr<std::string>>::const_iterator iter =
        property_names.begin();
    std::string s;
    for (; iter != property_names.end(); ++iter) {
        schrodinger::mae::whitespace(m_buffer);
        switch ((*(*iter))[0]) {
        case 'r':
            block->setRealProperty((*(*iter)), parse_value<double>(m_buffer));
            break;
        case 's':
            block->setStringProperty((*(*iter)),
                                     parse_value<std::string>(m_buffer));
            break;
        case 'i':
            block->setIntProperty((*(*iter)), parse_value<int>(m_buffer));
            break;
        case 'b':
            block->setBoolProperty((*(*iter)), parse_value<bool>(m_buffer));
            break;
        }
    }
    schrodinger::mae::whitespace(m_buffer);
    while (true) {
        if (!m_buffer.load()) {
            throw read_exception(m_buffer, "Missing '}' for block.");
        }

        if (*m_buffer.current == '}') {
            ++m_buffer.current;
            break;
        }

        int indexed = 0;
        std::string name = blockBeginning(&indexed);
        if (indexed) {
            indexed_block_parser->parse(name, indexed, m_buffer);
        } else {
            std::shared_ptr<Block> sub_block =
                std::shared_ptr<Block>(blockBody(name));
            block->addBlock(sub_block);
        }
        schrodinger::mae::whitespace(m_buffer);
    }
    block->setIndexedBlockMap(std::shared_ptr<IndexedBlockMapI>(
        indexed_block_parser->getIndexedBlockMap()));
    return block;
}

void MaeParser::properties(
    std::vector<std::shared_ptr<std::string>>* property_names)
{
    std::shared_ptr<std::string> property_name;
    while ((property_name = property_key(m_buffer)) != nullptr) {
        property_names->push_back(std::shared_ptr<std::string>(property_name));
        schrodinger::mae::whitespace(m_buffer);
    }
    triple_colon(m_buffer);
    return;
}

void triple_colon(Buffer& buffer)
{
    for (int i = 0; i < 3; ++i) {
        if (!character(':', buffer)) {
            throw read_exception(buffer, "Bad ':::' token.");
        }
    }
}

std::shared_ptr<std::string> MaeParser::property()
{
    return property_key(m_buffer);
}

std::shared_ptr<std::string> property_key(Buffer& buffer)
{
    if (!buffer.load()) {
        throw read_exception(buffer, "Missing property key.");
    }

    char* save = buffer.current;
    switch (*buffer.current) {
    case 'b':
    case 'i':
    case 'r':
    case 's':
        break;
    case ':':
        return nullptr;
    default:
        goto bad_format;
    }
    ++buffer.current;

    if (buffer.current >= buffer.end) {
        if (!buffer.load(save)) {
            goto bad_format;
        }
    }
    if (*buffer.current != '_') {
        goto bad_format;
    }
    ++buffer.current;

    if (!property_key_author_name(buffer, save)) {
        goto bad_format;
    }
    return std::make_shared<std::string>(save, buffer.current - save);

bad_format:
    throw read_exception(buffer, "Bad format for property; "
                                 "must be (b|i|r|s)_<author>_<name>.");
}

bool property_key_author_name(Buffer& buffer, char*& save)
{
    while (buffer.current < buffer.end || buffer.load(save)) {
        switch (*buffer.current) {
        case '_':
            ++buffer.current;
            goto name;
        default:
            if (!((*buffer.current >= 'a' && *buffer.current <= 'z') ||
                  (*buffer.current >= 'A' && *buffer.current <= 'Z'))) {
                return false;
            }
        }
        ++buffer.current;
    }

name:
    char* start = buffer.current;
    while (buffer.current < buffer.end || buffer.load(save)) {
        switch (*buffer.current) {
        case WHITESPACE:
        case ':':
        case '{':
        case '[':
            return buffer.current != start;
        }
        ++buffer.current;
    }
    return false;
}

void IndexedBlockBuffer::value(Buffer& buffer)
{
    char* save = buffer.current;

    if (buffer.current == buffer.end) {
        throw read_exception(buffer, "Unexpected EOF in indexed block values.");
    }

    if (*buffer.current != '"') {
        while (buffer.current < buffer.end || buffer.load(save)) {
            switch (*buffer.current) {
            case WHITESPACE:
                m_tokens_list.setTokenIndices(save - buffer.begin,
                                              buffer.current - buffer.begin);
                return;
            }
            ++buffer.current;
        }
        // If EOF is reached...
        m_tokens_list.setTokenIndices(save - buffer.begin,
                                      buffer.current - buffer.begin);
        return;
    } else {
        ++buffer.current;
        while (buffer.current < buffer.end || buffer.load(save)) {
            switch (*buffer.current) {
            case '"':
                if (*(buffer.current - 1) == '\\') {
                    break;
                }
                ++buffer.current;
                m_tokens_list.setTokenIndices(save - buffer.begin,
                                              buffer.current - buffer.begin);
                return;
            }
            ++buffer.current;
        }
        throw read_exception(buffer, "Unterminated quoted string at EOF.");
    }
}

DirectIndexedBlockParser::~DirectIndexedBlockParser()
{
}

void DirectIndexedBlockParser::parse(const std::string& name, size_t size,
                                     Buffer& buffer)
{
    if (m_indexed_block_map == nullptr) {
        m_indexed_block_map = std::make_shared<IndexedBlockMap>();
    }
    auto indexed_block = std::make_shared<IndexedBlock>(name);

    std::vector<std::string> property_keys;

    whitespace(buffer);
    std::shared_ptr<std::string> property_name;
    while ((property_name = property_key(buffer)) != nullptr) {
        property_keys.push_back(*property_name);
        whitespace(buffer);
    }
    triple_colon(buffer);

    std::vector<IndexedValueParser*> parsers;
    parsers.reserve(property_keys.size() + 1);
    IndexedValueParser* p = new IndexedValueCollector<int>("", size);
    parsers.push_back(p);
    for (auto& key : property_keys) {
        switch (key[0]) {
        case 'b':
            p = new IndexedValueCollector<bool>(key, size);
            break;
        case 'i':
            p = new IndexedValueCollector<int>(key, size);
            break;
        case 'r':
            p = new IndexedValueCollector<double>(key, size);
            break;
        case 's':
            p = new IndexedValueCollector<std::string>(key, size);
            break;
        default:
            throw std::out_of_range("An unexpected error was found.");
        }
        parsers.push_back(p);
    }

    for (int i = 0; i < size; ++i) {
        for (auto parser : parsers) {
            whitespace(buffer);
            parser->parse(buffer);
        }
    }
    whitespace(buffer);
    triple_colon(buffer);
    whitespace(buffer);
    if (!character('}', buffer)) {
        throw read_exception(buffer, "Missing '{' for outer block.");
    }

    for (auto parser : parsers) {
        parser->addToIndexedBlock(indexed_block.get());
        delete parser;
    }
    m_indexed_block_map->addIndexedBlock(name, indexed_block);
}

std::shared_ptr<IndexedBlockMapI> DirectIndexedBlockParser::getIndexedBlockMap()
{
    std::shared_ptr<IndexedBlockMapI> map(m_indexed_block_map);
    m_indexed_block_map = nullptr;
    return map;
}

void IndexedBlockBuffer::parse(Buffer& buffer)
{
    // Modifies buffer to use a loader that stores offsets and data in
    // m_tokens_list. Original loader restored at data_collector destruction.
    BufferDataCollector data_collector(&buffer, &m_tokens_list);

    size_t values = m_rows * (m_property_names.size() + 1);
    m_tokens_list.reserve(values);

    if (buffer.size() == 0) {
        if (!buffer.load()) {
            throw read_exception(buffer,
                                 "Unexpected EOF in indexed block scan.");
        }
    }
    m_tokens_list.appendBufferData(buffer.data());

    for (std::size_t ix = 0; ix < values; ix++) {
        // TODO: Another boost in performance can be had by avoiding the
        // function call overhead for value and whitespace, but simplying
        // marking the functions as inline doesn't seem to do the trick.
        whitespace(buffer);
        value(buffer);
    }
    whitespace(buffer);
}

/**
 * This function is measurably faster than strtol.
 *
 * The main reason for this is probably that it does not deal with alternate
 * bases for the integer.
 */
static long int simple_strtol(const char* ptr, const char* end)
{
    long int value = 0;
    long int sign = 1;

    while (ptr < end) {
        switch (*ptr) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            value = value * 10 + *ptr - '0';
            break;
        case '-':
            if (sign == -1 || value) {
                throw std::invalid_argument("Unexpected '-' in integer.");
            }
            sign = -1;
            break;
        default:
            throw std::invalid_argument("Unexpected character in integer.");
        }
        ++ptr;
    }
    return value * sign;
}

IndexedBlock* IndexedBlockBuffer::getIndexedBlock()
{
    IndexedBlock* iblock = new IndexedBlock(getName());

    std::vector<std::string>::const_iterator iter = m_property_names.begin();

    // Indexed blocks have row indexes explicitly mixed in as the first
    // value of each row. This is why a) prop_indx starts at 1, b)
    // col_count is prop_count + 1 instead of prop_count.
    //
    size_t prop_count = m_property_names.size();
    size_t col_count = prop_count + 1;
    size_t value_count = col_count * m_rows;
    boost::dynamic_bitset<>* is_null = nullptr;
    for (int prop_indx = 1; iter != m_property_names.end();
         ++iter, ++prop_indx) {
        char type = (*iter)[0];
        is_null = nullptr;
        const char* data;
        size_t len;
        switch (type) {
        case 'b': {
            std::vector<bool> bvalues;
            bvalues.reserve(m_rows);
            for (size_t ix = prop_indx; ix < value_count; ix += col_count) {
                getData(ix, &data, &len);
                if (data[0] == '<' && data[1] == '>') {
                    if (is_null == nullptr) {
                        is_null = new boost::dynamic_bitset<>(m_rows);
                    }
                    is_null->set(bvalues.size());
                    bvalues.push_back(false);
                } else if (data[0] == '1') {
                    bvalues.push_back(true);
                } else if (data[0] == '0') {
                    bvalues.push_back(false);
                } else {
                    throw std::out_of_range("Bogus bool.");
                }
            }
            std::shared_ptr<IndexedBoolProperty> ibp(
                new IndexedBoolProperty(bvalues, is_null));
            iblock->setBoolProperty(*iter, ibp);
        } break;
        case 'i': {
            std::vector<int> ivalues;
            ivalues.reserve(m_rows);
            for (size_t ix = prop_indx; ix < value_count; ix += col_count) {
                getData(ix, &data, &len);
                if (data[0] == '<' && data[1] == '>') {
                    if (is_null == nullptr) {
                        is_null = new boost::dynamic_bitset<>(m_rows);
                    }
                    is_null->set(ivalues.size());
                    ivalues.push_back(0);
                } else {
                    long int value = simple_strtol(data, data + len);
                    ivalues.push_back(value);
                }
            }
            std::shared_ptr<IndexedIntProperty> iip(
                new IndexedIntProperty(ivalues, is_null));
            iblock->setIntProperty(*iter, iip);
        } break;
        case 'r': {
            std::vector<double> dvalues;
            dvalues.reserve(m_rows);
            for (size_t ix = prop_indx; ix < value_count; ix += col_count) {
                getData(ix, &data, &len);
                if (data[0] == '<' && data[1] == '>') {
                    if (is_null == nullptr) {
                        is_null = new boost::dynamic_bitset<>(m_rows);
                    }
                    is_null->set(dvalues.size());
                    dvalues.push_back(0);
                } else {
                    double value = 0;
                    const char* end = data + len;
                    if (!qi::parse(data, end, qi::double_, value) ||
                        data != end) {
                        throw std::invalid_argument("Bad floating point "
                                                    "representation.");
                    }
                    dvalues.push_back(value);
                }
            }
            std::shared_ptr<IndexedRealProperty> irp(
                new IndexedRealProperty(dvalues, is_null));
            iblock->setRealProperty(*iter, irp);
        } break;
        case 's': {
            std::vector<std::string> svalues;
            svalues.reserve(m_rows);
            for (size_t ix = prop_indx; ix < value_count; ix += col_count) {
                getData(ix, &data, &len);
                if (data[0] == '<' && data[1] == '>') {
                    if (is_null == nullptr) {
                        is_null = new boost::dynamic_bitset<>(m_rows);
                    }
                    is_null->set(svalues.size());
                    svalues.emplace_back();
                } else {
                    svalues.emplace_back(data, len);
                }
            }
            std::shared_ptr<IndexedStringProperty> isp(
                new IndexedStringProperty(svalues, is_null));
            iblock->setStringProperty(*iter, isp);
        } break;
        }
    }
    return iblock;
}

BufferedIndexedBlockParser::BufferedIndexedBlockParser()
{
    m_indexed_block_map = std::make_shared<BufferedIndexedBlockMap>();
}

BufferedIndexedBlockParser::~BufferedIndexedBlockParser()
{
}

std::shared_ptr<IndexedBlockMapI>
BufferedIndexedBlockParser::getIndexedBlockMap()
{
    std::shared_ptr<IndexedBlockMapI> indexed_block_map(m_indexed_block_map);
    m_indexed_block_map = nullptr;
    return indexed_block_map;
}

void BufferedIndexedBlockParser::parse(const std::string& name, size_t size,
                                       Buffer& buffer)
{
    auto ibb = std::make_shared<IndexedBlockBuffer>(name, size);
    whitespace(buffer);
    std::shared_ptr<std::string> property_name;
    while ((property_name = property_key(buffer)) != nullptr) {
        ibb->addPropertyName(*property_name);
        whitespace(buffer);
    }
    triple_colon(buffer);
    ibb->parse(buffer);
    triple_colon(buffer);
    whitespace(buffer);

    if (!character('}', buffer)) {
        throw read_exception(buffer, "Missing closing '}' for "
                                     "indexed block.");
    }
    m_indexed_block_map->addIndexedBlockBuffer(name, ibb);
}

} // end of namespace mae

} // end of namespace schrodinger
