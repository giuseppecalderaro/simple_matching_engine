#include "order.hpp"
#include "enums.hpp"
#include <memory>
#include <sstream>
#include <boost/algorithm/string.hpp>

namespace SimpleMatchingEngine {
    Order Order::insert(const std::vector<std::string> &tokens, TimeStampType timestamp)
    {
        auto price = validate_price(tokens[4]);
        auto volume = validate_volume(tokens[5]);
        return Order(std::stoi(tokens[1]), // Order id
                     tokens[2], // symbol
                     Order::side_from_string(tokens[3]),
                     price,
                     volume,
                     timestamp);
    }

    Order Order::amend(const std::vector<std::string> &tokens, TimeStampType timestamp)
    {
        auto price = validate_price(tokens[2]);
        auto volume = validate_volume(tokens[3]);
        return Order(std::stoi(tokens[1]), // Order id
                     "", // symbol
                     Side::UNKNOWN,
                     price,
                     volume,
                     timestamp);
    }

    Order Order::pull(const std::vector<std::string> &tokens, TimeStampType timestamp)
    {
        return Order(std::stoi(tokens[1]), // Order id
                     "", // symbol
                     Side::UNKNOWN,
                     convert_price<PriceType>("0"), // price
                     0, // volume
                     timestamp);
    }

    Order::Order(OrderIdType order_id, SymbolType symbol, Side side, PriceType price, VolumeType volume, TimeStampType timestamp)
        : order_id_(order_id), symbol_(symbol), side_(side), price_(price), volume_(volume), timestamp_(timestamp)
    {}

    PriceType Order::validate_price(const std::string &price)
    {
        auto found = price.find_last_of(".");
        if (found != std::string::npos && price.substr(found + 1).size() > 4) {
            std::ostringstream oss;
            oss << "Price: " << price << " is not in a valid format";
            throw std::runtime_error(oss.str());
        }

        return convert_price<PriceType>(price);
    }

    VolumeType Order::validate_volume(const std::string &vol_string)
    {
        VolumeType volume = std::stoi(vol_string);
        if (volume < 0) {
            std::ostringstream oss;
            oss << "Volume: " << volume << " cannot be negative";
            throw std::runtime_error(oss.str());
        }

        return volume;
    }

    Side Order::side_from_string(const std::string &side)
    {
        if (side == "BUY")
            return Side::BUY;

        if (side == "SELL")
            return Side::SELL;

        std::ostringstream oss;
        oss << "Cannot decode side: " << side << std::endl;
        throw std::runtime_error(oss.str());
    }
}
