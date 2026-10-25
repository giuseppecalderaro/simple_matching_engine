#pragma once
#include "enums.hpp"
#include "types.hpp"
#include <string>
#include <vector>

namespace SimpleMatchingEngine {
    class Order final
    {
    public:
        static Order insert(const std::vector<std::string> &tokens, SequenceType sequence);
        static Order amend(const std::vector<std::string> &tokens, SequenceType sequence);
        static Order pull(const std::vector<std::string> &tokens, SequenceType sequence);
        Order(OrderIdType order_id, SymbolType symbol, Side side, PriceType price, VolumeType volume, SequenceType sequence);

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

        void reduce_volume(VolumeType other) noexcept {
            volume_ -= other;
        }

        void set_sequence(SequenceType new_sequence) noexcept {
            sequence_ = new_sequence;
        }

        // The order that reached the book last is the aggressor
        bool is_aggressor(const Order &other) const noexcept {
            return sequence_ > other.sequence_;
        }

    private:
        static void validate_token_count(const std::vector<std::string> &tokens, std::size_t expected);
        static int parse_int(const std::string &value, const char *field);
        static PriceType parse_price(const std::string &price);
        static VolumeType parse_volume(const std::string &volume);
        static Side side_from_string(const std::string &side);

    private:
        OrderIdType order_id_;
        Unqualified<SymbolType> symbol_;
        Side side_;
        PriceType price_;
        VolumeType volume_;
        SequenceType sequence_;
    };
}
