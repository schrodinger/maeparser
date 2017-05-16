#pragma once

#include <cstdio>

#include "Buffer.hpp"
#include "MaeBlock.hpp"
#include "MaeParser.hpp"

namespace schrodinger
{
namespace mae
{

class Reader
{
  private:
    MaeParser* m_mae_parser;

  public:
    Reader(FILE* file, size_t buffer_size = BufferLoader::DEFAULT_SIZE)
        : m_mae_parser(nullptr)
    {
        m_mae_parser = new MaeParser(file, buffer_size);
    }

    Reader(std::istream& stream,
           size_t buffer_size = BufferLoader::DEFAULT_SIZE)
        : m_mae_parser(nullptr)
    {
        m_mae_parser = new MaeParser(stream, buffer_size);
    }

    // Should be made private if we conclude there's no need for the
    // DirectParser. The only current purpose of allowing construction from a
    // MaeParser is to allow direct/buffered behavior difference.
    Reader(MaeParser* mae_parser) : m_mae_parser(mae_parser) {}

    std::shared_ptr<Block> next(const std::string& outer_block_name);
};

} // namespace mae
} // namespace schrodinger
