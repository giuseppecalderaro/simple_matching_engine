#include "engine.hpp"
#include <iostream>
#include <string>
#include <vector>

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

int main(int argc, char **argv)
{
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

    // *** DEBUG
    for (auto &item : result)
        std::cout << " " << item << std::endl;
}
