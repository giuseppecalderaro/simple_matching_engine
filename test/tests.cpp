#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "../include/engine.hpp"
#include <string>

using namespace SimpleMatchingEngine;

std::vector<std::string> run(std::vector<std::string> const& input)
{
    std::vector<std::string> ret;
    MatchingEngine engine;

    for (auto &wire : input)
        engine.process(wire, ret);

    auto books = engine.publish_books();
    ret.insert(ret.end(), books.begin(), books.end());

    return ret;
}

TEST_CASE("insert") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,AAPL,BUY,12.2,5");

    auto result = run(input);

    REQUIRE(result.size() == 2);
    CHECK(result[0] == "===AAPL===");
    CHECK(result[1] == "12.2,5,,");
}

TEST_CASE("simple match sell is aggressive") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,AAPL,BUY,12.2,5");
    input.emplace_back("INSERT,2,AAPL,SELL,12.1,8");

    auto result = run(input);

    REQUIRE(result.size() == 3);
    CHECK(result[0] == "AAPL,12.2,5,2,1");
    CHECK(result[1] == "===AAPL===");
    CHECK(result[2] == ",,12.1,3");
}

TEST_CASE("simple match buy is aggressive") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,AAPL,SELL,12.1,8");
    input.emplace_back("INSERT,2,AAPL,BUY,12.2,5");

    auto result = run(input);

    REQUIRE(result.size() == 3);
    CHECK(result[0] == "AAPL,12.1,5,2,1");
    CHECK(result[1] == "===AAPL===");
    CHECK(result[2] == ",,12.1,3");
}

TEST_CASE("multi insert and multi match") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,8,AAPL,BUY,14.235,5");
    input.emplace_back("INSERT,6,AAPL,BUY,14.235,6");
    input.emplace_back("INSERT,7,AAPL,BUY,14.235,12");
    input.emplace_back("INSERT,2,AAPL,BUY,14.234,5");
    input.emplace_back("INSERT,1,AAPL,BUY,14.23,3");
    input.emplace_back("INSERT,5,AAPL,SELL,14.237,8");
    input.emplace_back("INSERT,3,AAPL,SELL,14.24,9");
    input.emplace_back("PULL,8");
    input.emplace_back("INSERT,4,AAPL,SELL,14.234,25");

    auto result = run(input);

    REQUIRE(result.size() == 7);
    CHECK(result[0] == "AAPL,14.235,6,4,6");
    CHECK(result[1] == "AAPL,14.235,12,4,7");
    CHECK(result[2] == "AAPL,14.234,5,4,2");
    CHECK(result[3] == "===AAPL===");
    CHECK(result[4] == "14.23,3,14.234,2");
    CHECK(result[5] == ",,14.237,8");
    CHECK(result[6] == ",,14.24,9");
}

TEST_CASE("multi symbol") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,TEST,BUY,0.3854,5");
    input.emplace_back("INSERT,2,TSLA,BUY,412,31");
    input.emplace_back("INSERT,3,TSLA,BUY,410.5,27");
    input.emplace_back("INSERT,4,AAPL,SELL,21,8");
    input.emplace_back("INSERT,11,TEST,SELL,0.3854,4");
    input.emplace_back("INSERT,13,TEST,SELL,0.3853,6");


    auto result = run(input);

    REQUIRE(result.size() == 9);
    CHECK(result[0] == "TEST,0.3854,4,11,1");
    CHECK(result[1] == "TEST,0.3854,1,13,1");
    CHECK(result[2] == "===AAPL===");
    CHECK(result[3] == ",,21,8");
    CHECK(result[4] == "===TEST===");
    CHECK(result[5] == ",,0.3853,5");
    CHECK(result[6] == "===TSLA===");
    CHECK(result[7] == "412,31,,");
    CHECK(result[8] == "410.5,27,,");
}

TEST_CASE("amend") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,TEST,BUY,45.95,5");
    input.emplace_back("INSERT,2,TEST,BUY,45.95,6");
    input.emplace_back("INSERT,3,TEST,BUY,45.95,12");
    input.emplace_back("INSERT,4,TEST,SELL,46,8");
    input.emplace_back("AMEND,2,46,3"); // 1st trade
    input.emplace_back("INSERT,5,TEST,SELL,45.95,1"); // 2nd trade
    input.emplace_back("AMEND,1,45.95,3");
    input.emplace_back("INSERT,6,TEST,SELL,45.95,1"); // 3rd trade, the order keeps the position in the queue
    input.emplace_back("AMEND,1,45.95,5");
    input.emplace_back("INSERT,7,TEST,SELL,45.95,1");

    auto result = run(input);

    REQUIRE(result.size() == 6);
    CHECK(result[0] == "TEST,46,3,2,4");
    CHECK(result[1] == "TEST,45.95,1,5,1");
    CHECK(result[2] == "TEST,45.95,1,6,1");
    CHECK(result[3] == "TEST,45.95,1,7,3");
    CHECK(result[4] == "===TEST===");
    CHECK(result[5] == "45.95,16,46,5");
}

TEST_CASE("mix of everything") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,DISC,BUY,1234.12,300");
    input.emplace_back("INSERT,2,DISC,SELL,1233,250");
    input.emplace_back("PULL,1");
    input.emplace_back("INSERT,1,AAPL,BUY,498,10");
    input.emplace_back("INSERT,2,AAPL,BUY,499,1");
    input.emplace_back("INSERT,3,AAPL,BUY,499,1");
    input.emplace_back("INSERT,4,AAPL,BUY,499,1");
    input.emplace_back("INSERT,5,AAPL,BUY,499,1");
    input.emplace_back("INSERT,6,AAPL,BUY,499,1");
    input.emplace_back("INSERT,7,AAPL,SELL,499,15");
    input.emplace_back("AMEND,1,499,10");
    input.emplace_back("PULL,1");
    input.emplace_back("PULL,7");

    auto result = run(input);

    REQUIRE(result.size() == 9);
    CHECK(result[0] == "DISC,1234.12,250,2,1");
    CHECK(result[1] == "AAPL,499,1,7,2");
    CHECK(result[2] == "AAPL,499,1,7,3");
    CHECK(result[3] == "AAPL,499,1,7,4");
    CHECK(result[4] == "AAPL,499,1,7,5");
    CHECK(result[5] == "AAPL,499,1,7,6");
    CHECK(result[6] == "AAPL,499,10,1,7");
    CHECK(result[7] == "===AAPL===");
    CHECK(result[8] == "===DISC===");
}

TEST_CASE("catch wrong price") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,DISC,BUY,1234.12345,300");

    REQUIRE_THROWS(run(input));
}

TEST_CASE("catch wrong volume") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,DISC,BUY,1234.12,-300");

    REQUIRE_THROWS(run(input));
}

TEST_CASE("catch wrong command") {
    auto input = std::vector<std::string>();

    input.emplace_back("WHATISTHIS,1,RACE,BUY,1234.12,300");

    REQUIRE_THROWS(run(input));
}

TEST_CASE("catch wrong side") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,RACE,IamNotBuying,1234.12,300");

    REQUIRE_THROWS(run(input));
}

TEST_CASE("prices are compared numerically") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,AAPL,BUY,100,5");
    input.emplace_back("INSERT,2,AAPL,SELL,99,5");

    auto result = run(input);

    REQUIRE(result.size() == 2);
    CHECK(result[0] == "AAPL,100,5,2,1");
    CHECK(result[1] == "===AAPL===");
}

TEST_CASE("price levels are sorted numerically") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,AAPL,BUY,9,1");
    input.emplace_back("INSERT,2,AAPL,BUY,10,1");
    input.emplace_back("INSERT,3,AAPL,SELL,100,1");
    input.emplace_back("INSERT,4,AAPL,SELL,11,1");

    auto result = run(input);

    REQUIRE(result.size() == 3);
    CHECK(result[0] == "===AAPL===");
    CHECK(result[1] == "10,1,11,1");
    CHECK(result[2] == "9,1,100,1");
}

TEST_CASE("equivalent prices match") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,AAPL,BUY,12.2,5");
    input.emplace_back("INSERT,2,AAPL,SELL,12.20,5");

    auto result = run(input);

    REQUIRE(result.size() == 2);
    CHECK(result[0] == "AAPL,12.2,5,2,1");
    CHECK(result[1] == "===AAPL===");
}

TEST_CASE("equivalent prices share a level") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,AAPL,BUY,12.2,5");
    input.emplace_back("INSERT,2,AAPL,BUY,12.2000,6");
    input.emplace_back("INSERT,3,AAPL,SELL,13.0,1");

    auto result = run(input);

    REQUIRE(result.size() == 2);
    CHECK(result[0] == "===AAPL===");
    CHECK(result[1] == "12.2,11,13,1");
}

TEST_CASE("prices are printed in their shortest form") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,AAPL,BUY,0.0001,1");
    input.emplace_back("INSERT,2,AAPL,SELL,1000.5,1");

    auto result = run(input);

    REQUIRE(result.size() == 2);
    CHECK(result[1] == "0.0001,1,1000.5,1");
}

TEST_CASE("pulling a filled order does not touch the rest of the level") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,AAPL,BUY,10,5");
    input.emplace_back("INSERT,2,AAPL,BUY,10,5");
    input.emplace_back("INSERT,3,AAPL,SELL,10,5");
    input.emplace_back("PULL,1");

    auto result = run(input);

    REQUIRE(result.size() == 3);
    CHECK(result[0] == "AAPL,10,5,3,1");
    CHECK(result[1] == "===AAPL===");
    CHECK(result[2] == "10,5,,");
}

TEST_CASE("pulling an order filled by a partial match") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,AAPL,BUY,10,3");
    input.emplace_back("INSERT,2,AAPL,BUY,10,5");
    input.emplace_back("INSERT,3,AAPL,SELL,10,4");
    input.emplace_back("PULL,1");

    auto result = run(input);

    REQUIRE(result.size() == 4);
    CHECK(result[0] == "AAPL,10,3,3,1");
    CHECK(result[1] == "AAPL,10,1,3,2");
    CHECK(result[2] == "===AAPL===");
    CHECK(result[3] == "10,4,,");
}

TEST_CASE("pulling a filled sell order does not touch the rest of the level") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,AAPL,SELL,10,5");
    input.emplace_back("INSERT,2,AAPL,SELL,10,5");
    input.emplace_back("INSERT,3,AAPL,BUY,10,5");
    input.emplace_back("PULL,1");

    auto result = run(input);

    REQUIRE(result.size() == 3);
    CHECK(result[0] == "AAPL,10,5,3,1");
    CHECK(result[1] == "===AAPL===");
    CHECK(result[2] == ",,10,5");
}

TEST_CASE("amending a filled order is ignored") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,AAPL,BUY,10,5");
    input.emplace_back("INSERT,2,AAPL,BUY,10,5");
    input.emplace_back("INSERT,3,AAPL,SELL,10,5");
    input.emplace_back("AMEND,1,11,5");

    auto result = run(input);

    REQUIRE(result.size() == 3);
    CHECK(result[0] == "AAPL,10,5,3,1");
    CHECK(result[1] == "===AAPL===");
    CHECK(result[2] == "10,5,,");
}

TEST_CASE("order id can be reused once the order is filled") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,AAPL,BUY,10,5");
    input.emplace_back("INSERT,2,AAPL,BUY,10,5");
    input.emplace_back("INSERT,3,AAPL,SELL,10,5");
    input.emplace_back("INSERT,1,AAPL,SELL,12,7");

    auto result = run(input);

    REQUIRE(result.size() == 3);
    CHECK(result[1] == "===AAPL===");
    CHECK(result[2] == "10,5,12,7");
}

TEST_CASE("amend to zero volume pulls the order") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,AAPL,BUY,10,5");
    input.emplace_back("AMEND,1,10,0");
    input.emplace_back("INSERT,2,AAPL,SELL,10,5");

    auto result = run(input);

    REQUIRE(result.size() == 2);
    CHECK(result[0] == "===AAPL===");
    CHECK(result[1] == ",,10,5");
}

TEST_CASE("amend keeping priority does not make the order aggressive") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,AAPL,BUY,10,5");
    input.emplace_back("INSERT,2,AAPL,SELL,11,5");
    input.emplace_back("AMEND,1,10,3");
    input.emplace_back("AMEND,2,10,5");

    auto result = run(input);

    REQUIRE(result.size() == 3);
    CHECK(result[0] == "AAPL,10,3,2,1");
    CHECK(result[1] == "===AAPL===");
    CHECK(result[2] == ",,10,2");
}

TEST_CASE("trades go to the vector passed to each call") {
    MatchingEngine engine;

    std::vector<std::string> first_trades;
    engine.process("INSERT,1,AAPL,BUY,10,5", first_trades);

    std::vector<std::string> second_trades;
    engine.process("INSERT,2,AAPL,SELL,10,5", second_trades);

    CHECK(first_trades.empty());
    REQUIRE(second_trades.size() == 1);
    CHECK(second_trades[0] == "AAPL,10,5,2,1");
}

TEST_CASE("catch duplicate order id") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,AAPL,BUY,10,5");
    input.emplace_back("INSERT,1,AAPL,SELL,20,5");

    REQUIRE_THROWS(run(input));
}

TEST_CASE("catch zero volume insert") {
    auto input = std::vector<std::string>();

    input.emplace_back("INSERT,1,AAPL,BUY,10,0");

    REQUIRE_THROWS(run(input));
}

TEST_CASE("catch malformed messages") {
    const auto message = GENERATE(as<std::string>{},
        "INSERT,1,AAPL",               // too few fields
        "INSERT,1,AAPL,BUY,10,5,7",    // too many fields
        "INSERT,1,,BUY,10,5",          // empty symbol
        "INSERT,1,AAPL,,10,5",         // empty side
        "INSERT,,AAPL,BUY,10,5",       // empty order id
        "INSERT,x1,AAPL,BUY,10,5",     // non numeric order id
        "INSERT,1,AAPL,BUY,10,5abc",   // trailing junk in volume
        "INSERT,1,AAPL,BUY,abc,5",     // non numeric price
        "INSERT,1,AAPL,BUY,-10,5",     // negative price
        "INSERT,1,AAPL,BUY,0,5",       // zero price
        "INSERT,1,AAPL,BUY,10.,5",     // missing decimals
        "INSERT,1,AAPL,BUY,.5,5",      // missing integer part
        "INSERT,1,AAPL,BUY,1.2.3,5",   // two dots
        "AMEND,1,10",                  // too few fields
        "AMEND,1,10,-1",               // negative volume
        "PULL",                        // missing order id
        "PULL,1,2");                   // too many fields

    CAPTURE(message);

    MatchingEngine engine;
    std::vector<std::string> trades;
    REQUIRE_THROWS(engine.process(message, trades));
}
