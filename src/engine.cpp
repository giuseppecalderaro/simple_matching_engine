#include "engine.hpp"
#include <boost/algorithm/string.hpp>

namespace SimpleMatchingEngine {
    MatchingEngine::MatchingEngine()
        : orders_by_id_(std::make_unique<OrdersMapType>())
    {
        update_timestamp();
    }

    void MatchingEngine::process(const std::string &wire, std::vector<std::string> &trades)
    {
        std::vector<std::string> tokens;
        boost::split(tokens, wire, boost::is_any_of(","), boost::token_compress_on);
        Command cmd = command_from_string(tokens[0]);

        switch (cmd) {
            case Command::INSERT:
            {
                const auto &order = Order::insert(tokens, timestamp_);
                process_insert_order(order, trades);
                break;
            }
            case Command::AMEND:
            {
                const auto &order = Order::amend(tokens, timestamp_);
                // If amend volume is 0 we pull the order
                order.get_volume() != 0 ? process_amend_order(order, trades) : process_pull_order(order);
                break;
            }
            case Command::PULL:
            {
                const auto &order = Order::pull(tokens, timestamp_);
                process_pull_order(order);
                break;
            }
            default:
            {
                // This should not happen, will throw not to silently fail
                std::ostringstream oss;
                oss << "Unknown command: " << cmd << std::endl;
                throw std::runtime_error(oss.str());
            }
        }

        // Time passes...
        update_timestamp();
    }

    std::vector<std::string> MatchingEngine::publish_books() const noexcept {
        std::vector<std::string> ret;

        for (auto &[symbol, book] : orderbooks_) {
            std::ostringstream oss;
            oss << "===" << symbol << "===";
            ret.push_back(oss.str());

            auto levels = book.print_levels();
            ret.insert(ret.end(), levels.begin(), levels.end());
        }

        return ret;
    }

    void MatchingEngine::process_insert_order(const Order &order, std::vector<std::string> &trades) noexcept
    {
        // Add the order to the pool of orders
        orders_by_id_->insert(std::make_pair(order.get_order_id(), order));

        // Check if we already have an orderbook for the symbol.
        // If we don't, let's add it.
        auto iter = orderbooks_.find(order.get_symbol());
        if (iter == orderbooks_.end()) {
            auto inserted = orderbooks_.insert(std::make_pair(order.get_symbol(), Orderbook(orders_by_id_.get(), trades)));
            iter = inserted.first;
        }

        auto &orderbook = iter->second;
        orderbook.process_insert_order(order);
    }

    void MatchingEngine::process_amend_order(const Order &order, std::vector<std::string> &trades)
    {
        auto order_iter = orders_by_id_->find(order.get_order_id());
        if (order_iter == orders_by_id_->end()) {
            // The order doesn't exist. Maybe it's been matched or cancelled before we could amend it ?
            return;
        }

        auto &existing = order_iter->second;
        auto &orderbook = retrieve_orderbook(existing);
        if (order.get_price() == existing.get_price() && order.get_volume() <= existing.get_volume()) {
            // If we are not changing the price and the volume is not increasing the order keeps its priority
            existing.set_volume(order.get_volume());
            existing.set_timestamp(timestamp_);
            orderbook.process_amend_order(existing);
        } else {
            // Otherwise it ends at the bottom of the queue.
            orderbook.process_pull_order(existing);
            existing.set_price(order.get_price());
            existing.set_volume(order.get_volume());
            existing.set_timestamp(timestamp_);
            orderbook.process_insert_order(existing);
        }
    }

    void MatchingEngine::process_pull_order(const Order &order)
    {
        auto order_iter = orders_by_id_->find(order.get_order_id());
        if (order_iter == orders_by_id_->end()) {
            // The order doesn't exist. Maybe it's been matched before we could cancel it ?
            return;
        }

        const auto &existing = order_iter->second;

        auto &orderbook = retrieve_orderbook(existing);
        orderbook.process_pull_order(existing);

        orders_by_id_->erase(order_iter);
    }

    void MatchingEngine::update_timestamp() noexcept
    {
        timestamp_ = std::chrono::steady_clock::now();
    }

    Orderbook &MatchingEngine::retrieve_orderbook(const Order &order) {
        auto orderbook_iter = orderbooks_.find(order.get_symbol());
        if (orderbook_iter == orderbooks_.end()) {
            // We have the order but we cannot find the associated orderbook?
            // Something must be seriously wrong. Throwing.
            std::ostringstream oss;
            oss << "Cannot find orderbook for symbol: " << order.get_symbol() << " cannot amend order";
            throw std::runtime_error(oss.str());
        }

        return orderbook_iter->second;
    }

    Command MatchingEngine::command_from_string(const std::string &cmd)
    {
        if (cmd == "INSERT")
            return Command::INSERT;

        if (cmd == "AMEND")
            return Command::AMEND;

        if (cmd == "PULL")
            return Command::PULL;

        std::ostringstream oss;
        oss << "Cannot decode command: " << cmd << std::endl;
        throw std::runtime_error(oss.str());
    }
}
