#include "order.hpp"
#include "enums.hpp"
#include <algorithm>
#include <cctype>
#include <charconv>
#include <sstream>
#include <stdexcept>

namespace SimpleMatchingEngine {
    namespace {
        // Integer part digits allowed in a price, keeps the fixed point value far from overflowing
        constexpr std::size_t MAX_PRICE_INTEGER_DIGITS = 12;

        bool all_digits(const std::string &value)
        {
            return std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isdigit(c); });
        }
    }

    Order Order::insert(const std::vector<std::string> &tokens, SequenceType sequence)
    {
        validate_token_count(tokens, 6);

        if (tokens[2].empty())
            throw std::runtime_error("Symbol cannot be empty");

        auto volume = parse_volume(tokens[5]);
        if (volume == 0)
            throw std::runtime_error("Volume of a new order cannot be zero");

        return Order(parse_int(tokens[1], "order id"),
                     tokens[2], // symbol
                     Order::side_from_string(tokens[3]),
                     parse_price(tokens[4]),
                     volume,
                     sequence);
    }

    Order Order::amend(const std::vector<std::string> &tokens, SequenceType sequence)
    {
        validate_token_count(tokens, 4);

        return Order(parse_int(tokens[1], "order id"),
                     "", // symbol
                     Side::UNKNOWN,
                     parse_price(tokens[2]),
                     parse_volume(tokens[3]),
                     sequence);
    }

    Order Order::pull(const std::vector<std::string> &tokens, SequenceType sequence)
    {
        validate_token_count(tokens, 2);

        return Order(parse_int(tokens[1], "order id"),
                     "", // symbol
                     Side::UNKNOWN,
                     0, // price
                     0, // volume
                     sequence);
    }

    Order::Order(OrderIdType order_id, SymbolType symbol, Side side, PriceType price, VolumeType volume, SequenceType sequence)
        : order_id_(order_id), symbol_(symbol), side_(side), price_(price), volume_(volume), sequence_(sequence)
    {}

    void Order::validate_token_count(const std::vector<std::string> &tokens, std::size_t expected)
    {
        if (tokens.size() != expected) {
            std::ostringstream oss;
            oss << tokens[0] << " expects " << expected << " fields, got " << tokens.size();
            throw std::runtime_error(oss.str());
        }
    }

    int Order::parse_int(const std::string &value, const char *field)
    {
        int ret = 0;
        auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), ret);
        if (value.empty() || ec != std::errc() || ptr != value.data() + value.size()) {
            std::ostringstream oss;
            oss << "Cannot decode " << field << ": " << value;
            throw std::runtime_error(oss.str());
        }

        return ret;
    }

    PriceType Order::parse_price(const std::string &price)
    {
        const auto dot = price.find('.');
        const auto integer_part = price.substr(0, dot);
        const auto fractional_part = dot == std::string::npos ? std::string() : price.substr(dot + 1);

        if (integer_part.empty() || integer_part.size() > MAX_PRICE_INTEGER_DIGITS || !all_digits(integer_part)
            || (dot != std::string::npos && fractional_part.empty())
            || fractional_part.size() > PRICE_DECIMALS || !all_digits(fractional_part)) {
            std::ostringstream oss;
            oss << "Price: " << price << " is not in a valid format";
            throw std::runtime_error(oss.str());
        }

        auto padded_fractional = fractional_part;
        padded_fractional.append(PRICE_DECIMALS - fractional_part.size(), '0');

        const PriceType ret = std::stoll(integer_part) * PRICE_SCALE + std::stoll(padded_fractional);
        if (ret == 0) {
            std::ostringstream oss;
            oss << "Price: " << price << " must be positive";
            throw std::runtime_error(oss.str());
        }

        return ret;
    }

    VolumeType Order::parse_volume(const std::string &vol_string)
    {
        VolumeType volume = parse_int(vol_string, "volume");
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
        oss << "Cannot decode side: " << side;
        throw std::runtime_error(oss.str());
    }
}
