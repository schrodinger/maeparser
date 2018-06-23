#include "Reader.hpp"

#include <fstream>
#include <iostream>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>


using boost::algorithm::ends_with;

namespace schrodinger
{
namespace mae
{

Reader::Reader(std::string fname, size_t buffer_size)
{
    std::shared_ptr<std::ifstream> stream(new std::ifstream(fname));
    if(ends_with(fname, ".mae")) {
        m_mae_parser.reset(new MaeParser(stream, buffer_size));
    } else if (ends_with(fname, ".maegz") || ends_with(fname, ".mae.gz")) {
#ifdef ZLIB_AVAILABLE
        m_pregzip_stream = stream;  // Store it since maeparser won't
        m_gzip_stream.reset(new boost::iostreams::filtering_streambuf<boost::iostreams::input>());
        m_gzip_stream->push(boost::iostreams::gzip_decompressor());
        m_gzip_stream->push(*stream);
        std::shared_ptr<std::istream> decompressed_stream(new std::istream(m_gzip_stream.get()));
        m_mae_parser.reset(new MaeParser(decompressed_stream, buffer_size));
#else
        throw std::runtime_error("maeparser not compiled with gz support");
#endif
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
