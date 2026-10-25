#pragma once
#include <cstdint>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace SimpleMatchingEngine {
    class Order;

    template<typename T>
    using Unqualified = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

    using OrderIdType = int;
    using PriceType = std::int64_t; // fixed point, in ticks of 1 / PRICE_SCALE
    using SymbolType = const std::string &;
    using SequenceType = std::uint64_t;
    using VolumeType = int;

    using OrdersMapType = std::unordered_map<OrderIdType, Order>;

    // Prices are accepted with up to 4 decimals
    inline constexpr std::size_t PRICE_DECIMALS = 4;
    inline constexpr PriceType PRICE_SCALE = 10000;

    // Prints the shortest decimal representation, e.g. 122000 -> "12.2", 460000 -> "46"
    inline std::string format_price(PriceType price)
    {
        auto ret = std::to_string(price / PRICE_SCALE);
        const auto fractional = price % PRICE_SCALE;
        if (fractional == 0)
            return ret;

        auto digits = std::to_string(fractional);
        digits.insert(0, PRICE_DECIMALS - digits.size(), '0');
        digits.erase(digits.find_last_not_of('0') + 1);
        return ret + "." + digits;
    }
}
