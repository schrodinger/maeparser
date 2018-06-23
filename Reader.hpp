#pragma once

#include <string>
#include <cstdio>
#include <boost/iostreams/filtering_streambuf.hpp>

#include "MaeParserConfig.hpp"
#include "Buffer.hpp"
#include "MaeBlock.hpp"
#include "MaeParser.hpp"

namespace schrodinger
{
namespace mae
{

class EXPORT_MAEPARSER Reader
{
  private:
    std::shared_ptr<MaeParser> m_mae_parser;
    std::shared_ptr<std::ifstream> m_pregzip_stream;
    std::shared_ptr<boost::iostreams::filtering_streambuf<boost::iostreams::input> > m_gzip_stream;

  public:
    Reader(FILE* file, size_t buffer_size = BufferLoader::DEFAULT_SIZE)
    {
        m_mae_parser.reset(new MaeParser(file, buffer_size));
    }

    Reader(std::shared_ptr<std::istream> stream,
           size_t buffer_size = BufferLoader::DEFAULT_SIZE)
    {
        m_mae_parser.reset(new MaeParser(stream, buffer_size));
    }

    Reader(std::string fname, size_t buffer_size = BufferLoader::DEFAULT_SIZE);

    // Should be made private if we conclude there's no need for the
    // DirectParser. The only current purpose of allowing construction from a
    // MaeParser is to allow direct/buffered behavior difference.
    Reader(std::shared_ptr<MaeParser> mae_parser) : m_mae_parser(mae_parser) {}

    std::shared_ptr<Block> next(const std::string& outer_block_name);
};

} // namespace mae
} // namespace schrodinger
