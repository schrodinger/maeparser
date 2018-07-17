#include <iostream>
#include <stdexcept>

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "MaeBlock.hpp"

using namespace schrodinger;

BOOST_AUTO_TEST_SUITE(MaeBlockSuite)

BOOST_AUTO_TEST_CASE(maeBlock)
{
    double tolerance = std::numeric_limits<double>::epsilon();

    mae::Block b("dummy");
    b.setRealProperty("a", 1.0);
    BOOST_REQUIRE(b.hasRealProperty("a"));
    BOOST_REQUIRE(!b.hasRealProperty("b"));
    BOOST_REQUIRE_CLOSE(b.getRealProperty("a"), 1.0, tolerance);
    BOOST_REQUIRE_THROW(b.getRealProperty("b"), std::out_of_range);

    b.setIntProperty("a", 3);
    BOOST_REQUIRE(b.hasIntProperty("a"));
    BOOST_REQUIRE(!b.hasIntProperty("b"));
    BOOST_REQUIRE_EQUAL(b.getIntProperty("a"), 3);
    BOOST_REQUIRE_THROW(b.getIntProperty("b"), std::out_of_range);

    b.setBoolProperty("a", true);
    BOOST_REQUIRE(b.hasBoolProperty("a"));
    BOOST_REQUIRE(!b.hasBoolProperty("b"));
    BOOST_REQUIRE(b.getBoolProperty("a"));
    BOOST_REQUIRE_THROW(b.getBoolProperty("b"), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(maeIndexedRealProperty)
{
    double tolerance = std::numeric_limits<double>::epsilon();
    {
        auto dv = std::make_shared<std::vector<double>>();
        dv->push_back(1.0);
        dv->push_back(2.0);
        dv->push_back(3.0);
        mae::IndexedRealProperty irp(*dv);
        BOOST_REQUIRE_CLOSE(irp[0], 1.0, tolerance);
        BOOST_REQUIRE_CLOSE(irp[1], 2.0, tolerance);
        BOOST_REQUIRE_CLOSE(irp[2], 3.0, tolerance);
    }
    {
        std::vector<double> dv;
        dv.push_back(1.0);
        dv.push_back(2.0);
        dv.push_back(3.0);
        mae::IndexedRealProperty irp(dv);
        BOOST_REQUIRE_CLOSE(irp[0], 1.0, tolerance);
        BOOST_REQUIRE_CLOSE(irp[1], 2.0, tolerance);
        BOOST_REQUIRE_CLOSE(irp[2], 3.0, tolerance);
    }
    {
        auto dv = std::make_shared<std::vector<double>>();
        boost::dynamic_bitset<>* bs = new boost::dynamic_bitset<>(3);
        bs->set(1);

        dv->push_back(1.0);
        dv->push_back(0.0);
        dv->push_back(3.0);
        mae::IndexedRealProperty irp(*dv, bs);
        BOOST_REQUIRE(irp.isDefined(0));
        BOOST_REQUIRE_CLOSE(irp[0], 1.0, tolerance);
        BOOST_REQUIRE(!irp.isDefined(1));
        BOOST_REQUIRE_THROW(irp[1], std::runtime_error);
        BOOST_REQUIRE(irp.isDefined(2));
        BOOST_REQUIRE_CLOSE(irp[2], 3.0, tolerance);
    }
    {
        std::vector<double> dv;
        boost::dynamic_bitset<>* bs = new boost::dynamic_bitset<>(3);
        bs->set(1);

        dv.push_back(1.0);
        dv.push_back(0.0);
        dv.push_back(3.0);
        mae::IndexedRealProperty irp(dv, bs);
        BOOST_REQUIRE(irp.isDefined(0));
        BOOST_REQUIRE_CLOSE(irp[0], 1.0, tolerance);
        BOOST_REQUIRE(!irp.isDefined(1));
        BOOST_REQUIRE_THROW(irp[1], std::runtime_error);
        BOOST_REQUIRE(irp.isDefined(2));
        BOOST_REQUIRE_CLOSE(irp[2], 3.0, tolerance);
    }
}

BOOST_AUTO_TEST_CASE(maeIndexedBlock)
{
    using namespace mae;
    double tolerance = std::numeric_limits<double>::epsilon();
    {
        std::vector<double> dv;
        boost::dynamic_bitset<>* bs = new boost::dynamic_bitset<>(3);
        bs->set(1);

        dv.push_back(1.0);
        dv.push_back(0.0);
        dv.push_back(3.0);
        IndexedBlock ib("m_atom");
        BOOST_REQUIRE(!ib.hasRealProperty("r_m_float"));
        auto irps = std::shared_ptr<IndexedRealProperty>(
            new IndexedRealProperty(dv, bs));

        ib.setRealProperty("r_m_float", irps);
        BOOST_REQUIRE(ib.hasRealProperty("r_m_float"));

        auto irpg = ib.getRealProperty("r_m_float");
        IndexedRealProperty& irp = *irpg;
        BOOST_REQUIRE(irp.isDefined(0));
        BOOST_REQUIRE_CLOSE(irp[0], 1.0, tolerance);
        BOOST_REQUIRE_CLOSE(irp.at(0, 999.0), 1.0, tolerance);
        BOOST_REQUIRE(!irp.isDefined(1));
        BOOST_REQUIRE_THROW(irp[1], std::runtime_error);
        BOOST_REQUIRE_CLOSE(irp.at(1, 999.0), 999.0, tolerance);
        BOOST_REQUIRE(irp.isDefined(2));
        BOOST_REQUIRE_CLOSE(irp[2], 3.0, tolerance);
        BOOST_REQUIRE_CLOSE(irp.at(2, 999.0), 3.0, tolerance);
    }
}

BOOST_AUTO_TEST_CASE(maeIndexedBlockBool)
{
    using namespace mae;
    {
        std::vector<BoolProperty> dv;
        boost::dynamic_bitset<>* bs = new boost::dynamic_bitset<>(3);
        bs->set(1);

        dv.push_back(true);
        dv.push_back(false);
        dv.push_back(true);
        IndexedBlock ib("m_atom");
        BOOST_REQUIRE(!ib.hasBoolProperty("b_m_bool"));
        auto ibps = std::shared_ptr<IndexedBoolProperty>(
            new IndexedBoolProperty(dv, bs));

        ib.setBoolProperty("b_m_bool", ibps);
        BOOST_REQUIRE(ib.hasBoolProperty("b_m_bool"));

        auto ibpg = ib.getBoolProperty("b_m_bool");
        IndexedBoolProperty& ibp = *ibpg;
        BOOST_REQUIRE(ibp.isDefined(0));
        BOOST_REQUIRE_EQUAL(ibp[0], true);
        BOOST_REQUIRE(!ibp.isDefined(1));
        BOOST_REQUIRE_THROW(ibp[1], std::runtime_error);
        BOOST_REQUIRE(ibp.isDefined(2));
        BOOST_REQUIRE_EQUAL(ibp[2], true);
    }
}

BOOST_AUTO_TEST_SUITE_END()
