#include "Reader.hpp"

namespace schrodinger
{
namespace mae
{

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
