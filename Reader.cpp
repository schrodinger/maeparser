#include "Reader.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <fstream>
#include <iostream>

using boost::algorithm::ends_with;

namespace schrodinger
{
namespace mae
{

Reader::Reader(std::string fname, size_t buffer_size)
{
    std::unique_ptr<std::ifstream> stream(new std::ifstream( fname, std::ios_base::in | std::ios_base::binary));
    if (ends_with(fname, ".mae")) {
        m_mae_parser.reset(new MaeParser(*stream, buffer_size));
    } else if (ends_with(fname, ".maegz") || ends_with(fname, ".mae.gz")) {
        m_gzip_stream_filter.reset(new boost::iostreams::filtering_streambuf<boost::iostreams::input>());
        m_gzip_stream_filter->push(boost::iostreams::gzip_decompressor());
        m_gzip_stream_filter->push(*stream);
        m_stream.reset(new std::istream(m_gzip_stream_filter.get()));
        m_mae_parser.reset(new MaeParser(*m_stream, buffer_size));
    }
}

std::shared_ptr<Block> Reader::next(const std::string& outer_block_name)
{
    while (true) {
        m_mae_parser->whitespace();
        auto block = m_mae_parser->outerBlock();
        if (block == nullptr) {
            return nullptr;
        } else if (block->getName() == outer_block_name) {
            return block;
        }
    }
}
} // namespace mae
} // namespace schrodinger
