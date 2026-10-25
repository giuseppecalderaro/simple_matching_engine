#include "engine.hpp"
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <stdexcept>

namespace SimpleMatchingEngine {
    MatchingEngine::MatchingEngine()
        : orders_by_id_(std::make_unique<OrdersMapType>()), sequence_(0)
    {}

    void MatchingEngine::process(const std::string &wire, std::vector<std::string> &trades)
    {
        // Empty fields are kept so that they can be rejected rather than shifting the other fields
        std::vector<std::string> tokens;
        boost::split(tokens, wire, boost::is_any_of(","));
        Command cmd = command_from_string(tokens[0]);

        switch (cmd) {
            case Command::INSERT:
            {
                const auto &order = Order::insert(tokens, sequence_);
                process_insert_order(order, trades);
                break;
            }
            case Command::AMEND:
            {
                const auto &order = Order::amend(tokens, sequence_);
                // If amend volume is 0 we pull the order
                order.get_volume() != 0 ? process_amend_order(order, trades) : process_pull_order(order);
                break;
            }
            case Command::PULL:
            {
                const auto &order = Order::pull(tokens, sequence_);
                process_pull_order(order);
                break;
            }
            default:
            {
                // This should not happen, will throw not to silently fail
                std::ostringstream oss;
                oss << "Unknown command: " << cmd;
                throw std::runtime_error(oss.str());
            }
        }

        ++sequence_;
    }

    std::vector<std::string> MatchingEngine::publish_books() const
    {
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

    void MatchingEngine::process_insert_order(const Order &order, std::vector<std::string> &trades)
    {
        // Add the order to the pool of orders
        auto [order_iter, inserted] = orders_by_id_->emplace(order.get_order_id(), order);
        if (!inserted) {
            std::ostringstream oss;
            oss << "Duplicate order id: " << order.get_order_id();
            throw std::runtime_error(oss.str());
        }

        // Add an orderbook for the symbol if we don't have one yet
        auto orderbook_iter = orderbooks_.try_emplace(order.get_symbol(), orders_by_id_.get()).first;
        orderbook_iter->second.process_insert_order(order_iter->second, trades);
    }

    void MatchingEngine::process_amend_order(const Order &order, std::vector<std::string> &trades)
    {
        auto order_iter = orders_by_id_->find(order.get_order_id());
        if (order_iter == orders_by_id_->end()) {
            // The order doesn't exist. Maybe it's been matched or cancelled before we could amend it ?
            return;
        }

        auto &existing = order_iter->second;
        if (order.get_price() == existing.get_price() && order.get_volume() <= existing.get_volume()) {
            // If we are not changing the price and the volume is not increasing the order keeps its priority.
            // The book only holds order ids, so there is nothing to update there.
            existing.set_volume(order.get_volume());
        } else {
            // Otherwise it ends at the bottom of the queue.
            auto &orderbook = retrieve_orderbook(existing);
            orderbook.process_pull_order(existing);
            existing.set_price(order.get_price());
            existing.set_volume(order.get_volume());
            existing.set_sequence(sequence_);
            orderbook.process_insert_order(existing, trades);
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

    Orderbook &MatchingEngine::retrieve_orderbook(const Order &order) {
        auto orderbook_iter = orderbooks_.find(order.get_symbol());
        if (orderbook_iter == orderbooks_.end()) {
            // We have the order but we cannot find the associated orderbook?
            // Something must be seriously wrong. Throwing.
            std::ostringstream oss;
            oss << "Cannot find orderbook for symbol: " << order.get_symbol();
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
        oss << "Cannot decode command: " << cmd;
        throw std::runtime_error(oss.str());
    }
}
