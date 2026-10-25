#include "orderbook.hpp"
#include "order.hpp"
#include "types.hpp"
#include <sstream>
#include <stdexcept>

namespace SimpleMatchingEngine {
    Orderbook::Orderbook(OrdersMapType *orders_by_id_)
      : orders_by_id_(orders_by_id_)
    {
        if (!orders_by_id_)
            throw std::runtime_error("orders_by_id_ cannot be nullptr");
    }

    void Orderbook::process_insert_order(const Order &order, std::vector<std::string> &trades)
    {
        order.is_buy() ? insert_order(order, bids_) : insert_order(order, asks_);
        uncross_book(trades);
    }

    void Orderbook::process_pull_order(const Order &order)
    {
        order.is_buy() ? pull_order(order, bids_) : pull_order(order, asks_);
        // No need to uncross the book when pulling an order
    }

    std::vector<std::string> Orderbook::print_levels() const
    {
        std::vector<std::string> ret;

        for (auto bid_iter = bids_.begin(), ask_iter = asks_.begin(); bid_iter != bids_.end() || ask_iter != asks_.end(); ) {
            std::ostringstream oss;

            if (bid_iter != bids_.end()) {
                oss << format_price(bid_iter->first) << "," << level_volume(*bid_iter);
                bid_iter = std::next(bid_iter);
            } else {
                oss << ",";
            }

            oss << ",";

            if (ask_iter != asks_.end()) {
                oss << format_price(ask_iter->first) << "," << level_volume(*ask_iter);
                ask_iter = std::next(ask_iter);
            } else {
                oss << ",";
            }

            ret.push_back(oss.str());
        }

        return ret;
    }

    void Orderbook::uncross_book(std::vector<std::string> &trades)
    {
        // Match the front orders of the best levels until one side is empty or the book is uncrossed
        while (!bids_.empty() && !asks_.empty()) {
            const auto &bid_level = *bids_.begin();
            const auto &ask_level = *asks_.begin();

            if (bid_level.first < ask_level.first)
                return;

            auto &bid_order = get_order(bid_level.second.front());
            auto &ask_order = get_order(ask_level.second.front());
            const auto traded_volume = std::min(bid_order.get_volume(), ask_order.get_volume());

            trades.push_back(publish_trade(bid_order, ask_order, traded_volume));

            bid_order.reduce_volume(traded_volume);
            ask_order.reduce_volume(traded_volume);

            // Check both before removing anything, removing an order invalidates its reference
            const bool bid_filled = bid_order.get_volume() == 0;
            const bool ask_filled = ask_order.get_volume() == 0;

            if (bid_filled)
                remove_filled_order(bids_);

            if (ask_filled)
                remove_filled_order(asks_);
        }
    }

    Order &Orderbook::get_order(OrderIdType order_id) const
    {
        auto order_iter = orders_by_id_->find(order_id);
        if (order_iter != orders_by_id_->end()) [[likely]]
            return order_iter->second;

        std::ostringstream oss;
        oss << "Cannot find order with id: " << order_id;
        throw std::runtime_error(oss.str());
    }

    std::string Orderbook::publish_trade(const Order &bid, const Order &ask, VolumeType volume)
    {
        const auto &aggressor = bid.is_aggressor(ask) ? bid : ask;
        const auto &passive = bid.is_aggressor(ask) ? ask : bid;

        std::ostringstream oss;
        oss << passive.get_symbol() << "," << format_price(passive.get_price()) << "," << volume << "," << aggressor.get_order_id() << "," << passive.get_order_id();
        return oss.str();
    }
}
