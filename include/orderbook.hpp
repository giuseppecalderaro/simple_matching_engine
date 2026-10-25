#pragma once
#include "order.hpp"
#include "types.hpp"
#include <algorithm>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace SimpleMatchingEngine {
    class Orderbook final
    {
        public:
            explicit Orderbook(OrdersMapType *orders_by_id_);

            void process_insert_order(const Order &order, std::vector<std::string> &trades);
            void process_pull_order(const Order &order);

            std::vector<std::string> print_levels() const;

        private:
            template <typename BookSideType>
            void insert_order(const Order &order, BookSideType &book_side)
            {
                book_side[order.get_price()].push_back(order.get_order_id());
            }

            template <typename BookSideType>
            void pull_order(const Order &order, BookSideType &book_side)
            {
                auto level_iter = book_side.find(order.get_price());
                if (level_iter == book_side.end())
                    return;

                auto &orders = level_iter->second;
                auto order_iter = std::find(orders.begin(), orders.end(), order.get_order_id());
                if (order_iter == orders.end())
                    return;

                orders.erase(order_iter);

                if (orders.empty())
                    book_side.erase(level_iter);
            }

            // Removes the order at the front of the best level from the book and from the pool of orders
            template <typename BookSideType>
            void remove_filled_order(BookSideType &book_side)
            {
                auto level_iter = book_side.begin();
                auto &orders = level_iter->second;

                orders_by_id_->erase(orders.front());
                orders.erase(orders.begin());

                if (orders.empty())
                    book_side.erase(level_iter);
            }

            template <typename LevelType>
            VolumeType level_volume(const LevelType &level) const
            {
                VolumeType ret = 0;
                for (const auto order_id : level.second)
                    ret += get_order(order_id).get_volume();

                return ret;
            }

            void uncross_book(std::vector<std::string> &trades);
            Order &get_order(OrderIdType order_id) const;
            static std::string publish_trade(const Order &bid, const Order &ask, VolumeType volume);

        private:
            // Using ordered maps as I will need to be able to iterate in order
            std::map<PriceType, std::vector<OrderIdType>, std::greater<PriceType>> bids_;
            std::map<PriceType, std::vector<OrderIdType>> asks_;
            OrdersMapType *orders_by_id_;
    };
}
