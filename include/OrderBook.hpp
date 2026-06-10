#pragma once
#include <map>
#include <deque>
#include <unordered_map>
#include <mutex>
#include <optional>

#include "Order.hpp"

class OrderBook {

    private :
    
    std::map<float,std::deque<Order*>,std::greater<float>> bids;
    std::map<float,std::deque<Order*>> asks;
    std::unordered_map<long, Order> order_lookup;
    mutable std::mutex mutex;

    public:

    void add_order(const Order& order);
    void display() const;
    float best_bid() const;
    float best_ask() const;
    float spread() const;
    void cancel(long order_id);
    void modify(long order_id, float price, long quantity);
    std::optional<Order> get_order_copy(long order_id) const;
    const Order* get_order(long order_id) const;
    Order* get_order(long order_id);
    Order* best_ask_order();
    Order* best_bid_order();
};