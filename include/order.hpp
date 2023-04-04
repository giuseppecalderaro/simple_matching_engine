#pragma once
#include "enums.hpp"
#include "types.hpp"
#include <memory>
#include <string>
#include <vector>

namespace SimpleMatchingEngine {
    class Order final
    {
    public:
        static Order insert(const std::vector<std::string> &tokens, TimeStampType timestamp);
        static Order amend(const std::vector<std::string> &tokens, TimeStampType timestamp);
        static Order pull(const std::vector<std::string> &tokens, TimeStampType timestamp);
        Order(OrderIdType order_id, SymbolType symbol, Side side, PriceType price, VolumeType volume, TimeStampType timestamp);

        OrderIdType get_order_id() const noexcept {
            return order_id_;
        }

        SymbolType get_symbol() const noexcept {
            return symbol_;
        }

        Side get_side() const noexcept {
            return side_;
        }

        bool is_buy() const noexcept {
            return side_ == Side::BUY;
        }

        bool is_sell() const noexcept {
            return side_ == Side::SELL;
        }

        PriceType get_price() const noexcept {
            return price_;
        }

        void set_price(PriceType new_price) noexcept {
            price_ = new_price;
        }

        VolumeType get_volume() const noexcept {
            return volume_;
        }

        void set_volume(VolumeType new_volume) noexcept {
            volume_ = new_volume;
        }

        void reduce_volume(VolumeType other)
        {
            volume_ -= other;
        }

        void set_timestamp(TimeStampType new_timestamp) noexcept {
            timestamp_ = new_timestamp;
        }

        bool is_aggressor(const Order &other) const noexcept {
            if (timestamp_ > other.timestamp_)
                return true;

            return false;
        }

    private:
        static PriceType validate_price(const std::string &price);
        static VolumeType validate_volume(const std::string &volume);
        static Side side_from_string(const std::string &side);

    private:
        OrderIdType order_id_;
        Unqualified<SymbolType> symbol_;
        Side side_;
        Unqualified<PriceType> price_;
        VolumeType volume_;
        TimeStampType timestamp_;
    };
}
