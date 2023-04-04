#pragma once
#include "enums.hpp"
#include "order.hpp"
#include "orderbook.hpp"
#include "types.hpp"
#include <memory>
#include <string>
#include <vector>

namespace SimpleMatchingEngine {
    class MatchingEngine final
    {
    public:
        MatchingEngine();
        void process(const std::string &wire, std::vector<std::string> &trades);
        std::vector<std::string> publish_books() const noexcept;

    private:
        void process_insert_order(const Order &order, std::vector<std::string> &trades) noexcept;
        void process_amend_order(const Order &order, std::vector<std::string> &trades);
        void process_pull_order(const Order &order);
        void update_timestamp() noexcept;

        Orderbook &retrieve_orderbook(const Order &order);
        static Command command_from_string(const std::string &cmd);

    private:
        std::unique_ptr<OrdersMapType> orders_by_id_;
        std::map<Unqualified<SymbolType>, Orderbook> orderbooks_;
        TimeStampType timestamp_;
    };
}
