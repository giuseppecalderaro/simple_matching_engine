#include "orderbook.hpp"
#include "order.hpp"
#include "types.hpp"
#include <numeric>

namespace SimpleMatchingEngine {
    Orderbook::Orderbook(OrdersMapType *orders_by_id_, std::vector<std::string> &trades)
      : orders_by_id_(orders_by_id_), trades_(trades)
    {
        if (!orders_by_id_)
            throw std::runtime_error("orders_by_id_ cannot be nullptr");
    }

    void Orderbook::process_insert_order(const Order &order)
    {
        order.is_buy() ? insert_order(order, bids_) : insert_order(order, asks_);
        uncross_book();
    }

    void Orderbook::process_amend_order(const Order &order)
    {
        uncross_book();
    }

    void Orderbook::process_pull_order(const Order &order)
    {
        order.is_buy() ? pull_order(order, bids_) : pull_order(order, asks_);
        // No need to uncross the book when pulling an order
    }

    std::vector<std::string> Orderbook::print_levels() const noexcept {
        std::vector<std::string> ret;

        for (auto bid_iter = bids_.begin(), ask_iter = asks_.begin(); bid_iter != bids_.end() || ask_iter != asks_.end(); ) {
            std::ostringstream oss;

            if (bid_iter != bids_.end()) {
                auto bid_total_volume = std::accumulate(bid_iter->second.begin(), bid_iter->second.end(), 0, [this](int sum, const OrderIdType &order_id) {
                    const auto &order = get_order(order_id);
                    return sum + order.get_volume();
                });
                oss << bid_iter->first << "," << bid_total_volume;
                bid_iter = std::next(bid_iter);
            } else {
                oss << ",";
            }

            oss << ",";

            if (ask_iter != asks_.end()) {
                auto ask_total_volume = std::accumulate(ask_iter->second.begin(), ask_iter->second.end(), 0, [this](int sum, const OrderIdType &order_id) {
                    const auto &order = get_order(order_id);
                    return sum + order.get_volume();
                });
                oss << ask_iter->first << "," << ask_total_volume;
                ask_iter = std::next(ask_iter);
            } else {
                oss << ",";
            }

            ret.push_back(oss.str());
        }

        return ret;
    }

    void Orderbook::uncross_book() noexcept
    {
        // Need to check if price levels are crossed
        auto bid_level = bids_.begin();
        auto ask_level = asks_.begin();

        while (true) {
            if (bid_level == bids_.end() || ask_level == asks_.end()) {
                // One of the two sides is empty, just return
                return;
            }

            const auto &bid_price = bid_level->first;
            const auto &ask_price = ask_level->first;

            if (bid_price < ask_price) {
                // The book is now uncrossed
                return;
            }

            if (bid_price >= ask_price) {
                auto bid_order_iter = bid_level->second.begin();
                auto ask_order_iter = ask_level->second.begin();

                while(true) {
                    auto &bid_order = get_order(*bid_order_iter);
                    const auto bid_order_volume = bid_order.get_volume();

                    auto &ask_order = get_order(*ask_order_iter);
                    const auto ask_order_volume = ask_order.get_volume();

                    // Publish a trade
                    trades_.push_back(publish_trade(bid_order, ask_order));

                    if (bid_order_volume > ask_order_volume) {
                        // We need to:
                        // 1. Reduce the available volume on the bid order
                        bid_order.reduce_volume(ask_order_volume);

                        // 2. Remove the ask order
                        ask_order_iter = ask_level->second.erase(ask_order_iter);
                        orders_by_id_->erase(ask_order.get_order_id());

                        // 3. Check the next ask order level
                        if (ask_level->second.empty()) {
                            ask_level = asks_.erase(ask_level);
                            break;
                        }
                    }

                    if (bid_order_volume == ask_order_volume) {
                        // We need to:
                        // 1. Remove the bid order
                        bid_order_iter = bid_level->second.erase(bid_order_iter);

                        // 2. Remove the ask order
                        ask_order_iter = ask_level->second.erase(ask_order_iter);

                        // 3. Check the next bid and ask order levels
                        bool need_to_break = false;
                        if (bid_level->second.empty()) {
                            bid_level = bids_.erase(bid_level);
                            orders_by_id_->erase(bid_order.get_order_id());
                            need_to_break = true;
                        }

                        if (ask_level->second.empty()) {
                            ask_level = asks_.erase(ask_level);
                            orders_by_id_->erase(ask_order.get_order_id());
                            need_to_break = true;
                        }

                        if (need_to_break)
                            break;
                    }

                    if (bid_order_volume < ask_order_volume) {
                        // We need to:
                        // 1. Reduce the available volume on the ask order
                        ask_order.reduce_volume(bid_order_volume);

                        // 2. Remove the bid order
                        bid_order_iter = bid_level->second.erase(bid_order_iter);

                        // 3. Check the next bid order level
                        if (bid_level->second.empty()) {
                            bid_level = bids_.erase(bid_level);
                            orders_by_id_->erase(bid_order.get_order_id());
                            break;
                        }
                    }
                }

            }
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

    std::string Orderbook::publish_trade(const Order &bid, const Order &ask) {
        const auto &aggressor = bid.is_aggressor(ask) ? bid : ask;
        const auto &passive = bid.is_aggressor(ask) ? ask : bid;

        std::ostringstream oss;
        oss << passive.get_symbol() << "," << passive.get_price() << "," << std::min(bid.get_volume(), ask.get_volume()) << "," << aggressor.get_order_id() << "," << passive.get_order_id();
        return oss.str();
    }
}
