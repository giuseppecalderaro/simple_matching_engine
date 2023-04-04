#pragma once
#include <chrono>
#include <concepts>
#include <string>
#include <unordered_map>

namespace SimpleMatchingEngine {
    class Order;

    template<typename T>
    using Unqualified = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

    using OrderIdType = int;
    using PriceType = const std::string &; // you can change this to "double" if required
    using SymbolType = const std::string &;
    using TimeStampType = std::chrono::time_point<std::chrono::steady_clock>;
    using VolumeType = int;

    using OrdersMapType = std::unordered_map<OrderIdType, Order>;

    template <typename T>
    const std::string &convert_price(const std::string &price) {
        return price;
    }

    template <typename T>
    requires std::floating_point<T>
    double convert_price(const std::string &price) {
        return std::stod(price);
    }
}
