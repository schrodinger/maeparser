#include "Writer.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <fstream>
#include <iostream>

#include "MaeBlock.hpp"

using namespace std;
using boost::algorithm::ends_with;

namespace schrodinger
{
namespace mae
{

Writer::Writer(std::shared_ptr<ostream> stream)
{
    m_out = stream;
    write_opening_block();
}

Writer::Writer(std::string fname)
{
    auto pregzip_stream = new ofstream();
    pregzip_stream->open(fname, std::ios_base::out | std::ios_base::binary);

    if (ends_with(fname, ".maegz") || ends_with(fname, ".mae.gz")) {
        m_gzip_stream =
            std::make_shared<boost::iostreams::filtering_ostreambuf>();
        m_gzip_compressor = make_shared<boost::iostreams::gzip_compressor>();
        m_gzip_stream->push(*m_gzip_compressor);
        m_gzip_stream->push(*pregzip_stream);
        m_out = std::make_shared<std::ostream>(m_gzip_stream.get());
    } else {
        m_out.reset(dynamic_cast<ostream*>(pregzip_stream));
    }
    write_opening_block();
}

void Writer::write(const std::shared_ptr<Block>& block)
{
    block->write(*m_out);
}

void Writer::write_opening_block()
{
    shared_ptr<Block> b = make_shared<Block>("");
    b->setStringProperty("s_m_m2io_version", "2.0.0");
    write(b);
}

} // namespace mae
} // namespace schrodinger
