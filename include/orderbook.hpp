#pragma once
#include "order.hpp"
#include "types.hpp"
#include <algorithm>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace SimpleMatchingEngine {
    class Orderbook final
    {
        public:
            Orderbook(OrdersMapType *orders_by_id_, std::vector<std::string> &trades);

            void process_insert_order(const Order &order);
            void process_amend_order(const Order &order);
            void process_pull_order(const Order &order);
            void uncross_book() noexcept;

            std::vector<std::string> print_levels() const noexcept;

        private:
            template <typename BookSideType>
            void insert_order(const Order &order, BookSideType &book_side)
            {
                const auto order_id = order.get_order_id();

                std::vector<OrderIdType> orders{ order_id };
                auto inserted = book_side.emplace(std::make_pair(order.get_price(), orders));
                if (inserted.second == false) {
                    auto &orders = inserted.first->second;
                    orders.push_back(order_id);
                }
            }

            template <typename BookSideType>
            void pull_order(const Order &order, BookSideType &book_side)
            {
                auto level_iter = book_side.find(order.get_price());
                if (level_iter == book_side.end())
                    return;

                auto &orders = level_iter->second;
                auto order_iter = std::find(orders.begin(), orders.end(), order.get_order_id());
                orders.erase(order_iter);

                if (level_iter->second.empty())
                    level_iter = book_side.erase(level_iter);
            }

            Order &get_order(OrderIdType order_id) const;
            static std::string publish_trade(const Order &bid, const Order &ask);

        private:
            // Using ordered maps as I will need to be able to iterate in order
            std::map<Unqualified<PriceType>, std::vector<OrderIdType>, std::greater<PriceType>> bids_;
            std::map<Unqualified<PriceType>, std::vector<OrderIdType>> asks_;
            OrdersMapType *orders_by_id_;
            std::vector<std::string> &trades_;
    };
}
