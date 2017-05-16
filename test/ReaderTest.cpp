#include <cstdio>
#include <fstream>
#include <iostream>

#include <boost/test/unit_test.hpp>

#include "Reader.hpp"
#include "MaeBlock.hpp"

using namespace schrodinger::mae;

BOOST_AUTO_TEST_SUITE(ReaderSuite)

BOOST_AUTO_TEST_CASE(Reader0)
{
    std::stringstream ss;
    ss << "\n"
       << "{"
          "\n"
       << "  s_m_m2io_version"
          "\n"
       << "  :::"
          "\n"
       << "  1.1.0 "
          "\n"
       << "}"
          "\n";

    Reader r(ss);

    auto b = r.next("");
    BOOST_REQUIRE(b);
    BOOST_REQUIRE_EQUAL(b->getStringProperty("s_m_m2io_version"), "1.1.0");
}

BOOST_AUTO_TEST_CASE(NamedBlock0)
{
    std::stringstream ss;
    ss << "\n"
       << "\n"
       << "f_m_ct {"
          "\n"
       << "  s_m_prop"
          "\n"
       << "  :::"
          "\n"
       << "  1.1.0 "
          "\n"
       << "}"
          "\n";

    Reader r(ss);

    auto b = r.next("f_m_ct");
    BOOST_REQUIRE(b);
    BOOST_REQUIRE_EQUAL(b->getStringProperty("s_m_prop"), "1.1.0");
}

BOOST_AUTO_TEST_CASE(NamedBlock1)
{
    std::stringstream ss;
    ss << "{"
          "\n"
       << "  s_m_m2io_version"
          "\n"
       << "  :::"
          "\n"
       << "  1.1.0 "
          "\n"
       << "}"
          "\n"
       << "\n"
       << "f_m_ct {"
          "\n"
       << "  s_m_prop"
          "\n"
       << "  :::"
          "\n"
       << "  1.1.0 "
          "\n"
       << "}"
          "\n";

    Reader r(ss);

    auto b = r.next("f_m_ct");
    BOOST_REQUIRE(b);
    BOOST_REQUIRE_EQUAL(b->getStringProperty("s_m_prop"), "1.1.0");

    b = r.next("f_m_ct");
    BOOST_REQUIRE(b == nullptr);
}

BOOST_AUTO_TEST_CASE(NestedBlock)
{
    std::stringstream ss;
    ss << "{"
          "\n"
       << "  s_m_m2io_version"
          "\n"
       << "  :::"
          "\n"
       << "  1.1.0 "
          "\n"
       << "}"
          "\n"
       << "\n"
       << "f_m_ct {"
          "\n"
       << "  s_m_prop"
          "\n"
       << "  :::"
          "\n"
       << "  1.1.0 "
          "\n"
       << "  m_nested {"
          "\n"
       << "    s_m_prop"
          "\n"
       << "    :::"
          "\n"
       << "    1.1.0 "
          "\n"
       << "  }"
          "\n"
       << "}"
          "\n";

    Reader r(ss);

    auto b = r.next("f_m_ct");
    BOOST_REQUIRE(b);
    BOOST_REQUIRE_EQUAL(b->getStringProperty("s_m_prop"), "1.1.0");
    BOOST_REQUIRE(b->hasBlock("m_nested"));
    BOOST_REQUIRE_EQUAL(b->getBlock("m_nested")->getStringProperty("s_m_prop"),
                        "1.1.0");
}

BOOST_AUTO_TEST_CASE(NestedIndexedBlock)
{
    std::stringstream ss;
    ss << "{"
          "\n" // 1
       << "  s_m_m2io_version"
          "\n" // 2
       << "  :::"
          "\n" // 3
       << "  1.1.0 "
          "\n" // 4
       << "}"
          "\n" // 5
       << "\n" // 6
       << "f_m_ct {"
          "\n" // 7
       << "  s_m_prop"
          "\n" // 8
       << "  :::"
          "\n" // 9
       << "  1.1.0 "
          "\n" // 10
       << "  m_nested[2] {"
          "\n" // 11
       << "    s_m_prop"
          "\n" // 12
       << "    :::"
          "\n" // 13
       << "    1 1.1.0 "
          "\n" // 14
       << "    2 1.1.0 "
          "\n" // 15
       << "    :::"
          "\n" // 16
       << "  }"
          "\n" // 17
       << "  m_bond[2] {"
          "\n" // 18
       << "    s_m_prop"
          "\n" // 19
       << "    :::"
          "\n" // 20
       << "    1 1.1.0 "
          "\n" // 21
       << "    2 1.1.0 "
          "\n" // 22
       << "    :::"
          "\n" // 23
       << "  }"
          "\n" // 24
       << "  m_dependencies {"
          "\n" // 25
       << "    s_m_prop"
          "\n" // 26
       << "    :::"
          "\n" // 27
       << "    1.1.0 "
          "\n" // 28
       << "  }"
          "\n" // 29
       << "}"
          "\n"; // 30

    Reader r(ss);

    auto b = r.next("f_m_ct");
    BOOST_REQUIRE(b);
    std::shared_ptr<IndexedBlock> ibn = b->getIndexedBlock("m_nested");
    auto prop = ibn->getStringProperty("s_m_prop");
    BOOST_REQUIRE_EQUAL((*prop)[0], "1.1.0");
    BOOST_REQUIRE_EQUAL((*prop)[1], "1.1.0");
}

BOOST_AUTO_TEST_CASE(BufferedReader)
{
    std::ifstream ss("test.mae");

    Reader r(ss);

    size_t count = 0;
    std::shared_ptr<Block> b;
    while ((b = r.next("f_m_ct")) != nullptr) {
        std::shared_ptr<IndexedBlock> iba = b->getIndexedBlock("m_atom");
        std::shared_ptr<IndexedBlock> ibb = b->getIndexedBlock("m_bond");
        count++;
    }
    BOOST_REQUIRE_EQUAL(count, 3u);
}

BOOST_AUTO_TEST_CASE(BufferedFileReader)
{
    FILE* f = fopen("test.mae", "r");

    Reader r(f);

    size_t count = 0;
    std::shared_ptr<Block> b;
    while ((b = r.next("f_m_ct")) != nullptr) {
        std::shared_ptr<IndexedBlock> iba = b->getIndexedBlock("m_atom");
        std::shared_ptr<IndexedBlock> ibb = b->getIndexedBlock("m_bond");
        count++;
    }
    fclose(f);
    BOOST_REQUIRE_EQUAL(count, 3u);
}

BOOST_AUTO_TEST_CASE(TextReader)
{
    std::ifstream ss("test.mae");

    Reader r(ss);

    size_t count = 0;
    std::shared_ptr<Block> b;
    while ((b = r.next("f_m_ct")) != nullptr) {
        count++;
    }
    BOOST_REQUIRE_EQUAL(count, 3u);
}

BOOST_AUTO_TEST_CASE(TextFileReader)
{
    FILE* f = fopen("test.mae", "r");

    Reader r(f);

    size_t count = 0;
    std::shared_ptr<Block> b;
    while ((b = r.next("f_m_ct")) != nullptr) {
        count++;
    }
    fclose(f);
    BOOST_REQUIRE_EQUAL(count, 3u);
}

BOOST_AUTO_TEST_CASE(DirectReader)
{
    FILE* f = fopen("test.mae", "r");
    DirectMaeParser mae_parser(f);

    Reader r(&mae_parser);

    size_t count = 0;
    std::shared_ptr<Block> b;
    while ((b = r.next("f_m_ct")) != nullptr) {
        std::shared_ptr<IndexedBlock> iba = b->getIndexedBlock("m_atom");
        std::shared_ptr<IndexedBlock> ibb = b->getIndexedBlock("m_bond");
        count++;
    }
    fclose(f);
    BOOST_REQUIRE_EQUAL(count, 3u);
}
BOOST_AUTO_TEST_SUITE_END()
