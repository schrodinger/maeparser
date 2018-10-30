#pragma once

#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <cstdio>
#include <string>

#include "MaeParserConfig.hpp"


namespace schrodinger
{
namespace mae
{

class Block;

class EXPORT_MAEPARSER Writer
{
  private:
    std::shared_ptr<boost::iostreams::filtering_ostreambuf> m_gzip_stream;
    std::shared_ptr<boost::iostreams::gzip_compressor> m_gzip_compressor;
    std::shared_ptr<std::ostream> m_out;

    void write_opening_block();

  public:
    Writer(std::shared_ptr<std::ostream> stream);
    Writer(std::string fname);

    void write(const std::shared_ptr<Block>& block);
};

} // namespace mae
} // namespace schrodinger
