#include <cstdio>
#include <fstream>
#include <iostream>

#include "MaeBlock.hpp"
#include "Reader.hpp"
#include "Writer.hpp"
#include "MaeConstants.hpp"

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

using namespace schrodinger::mae;
using std::shared_ptr;

BOOST_AUTO_TEST_SUITE(WriterSuite)

BOOST_AUTO_TEST_CASE(Writer0)
{
    Reader r("test.mae");
    auto w = new Writer("test_write.mae");
    std::vector<std::shared_ptr<Block> > input;

    std::shared_ptr<Block> b;
    while ((b = r.next(CT_BLOCK)) != nullptr) {
        input.push_back(b);
        w->write(b);
    }
    delete w;

    Reader output_r("test_write.mae");
    int input_num = 0;
    while ((b = output_r.next(CT_BLOCK)) != nullptr) {
        BOOST_CHECK(*b == *(input[input_num++]));
    }

}

BOOST_AUTO_TEST_CASE(Writer1)
{
    Reader r("test.mae");
    auto w = new Writer("test_write.maegz");
    std::vector<std::shared_ptr<Block> > input;

    std::shared_ptr<Block> b;
    while ((b = r.next(CT_BLOCK)) != nullptr) {
        input.push_back(b);
        w->write(b);
    }
    delete w;

    Reader output_r("test_write.maegz");
    int input_num = 0;
    while ((b = output_r.next(CT_BLOCK)) != nullptr) {
        BOOST_CHECK(*b == *(input[input_num++]));
    }

}

BOOST_AUTO_TEST_SUITE_END()
