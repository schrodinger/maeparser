#include "Reader.hpp"

#include <fstream>
#include <iostream>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>


using boost::algorithm::ends_with;
using std::ifstream;
using std::istream;
using std::make_shared;

namespace schrodinger
{
namespace mae
{

Reader::Reader(std::string fname, size_t buffer_size)
{
    auto stream = make_shared<ifstream>(fname);
    if(ends_with(fname, ".mae")) {
        m_mae_parser = make_shared<MaeParser>(stream, buffer_size);
    } else if (ends_with(fname, ".maegz") || ends_with(fname, ".mae.gz")) {
        m_pregzip_stream = stream;  // Store it since maeparser won't
        m_gzip_stream = make_shared<boost::iostreams::filtering_streambuf<boost::iostreams::input> >();
        m_gzip_stream->push(boost::iostreams::gzip_decompressor());
        m_gzip_stream->push(*stream);
        auto decompressed_stream = make_shared<istream>(m_gzip_stream.get());
        m_mae_parser = make_shared<MaeParser>(decompressed_stream, buffer_size);
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
}
}
