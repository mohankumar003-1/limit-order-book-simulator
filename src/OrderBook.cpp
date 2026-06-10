#include <algorithm>
#include <iostream>
#include <mutex>
#include "OrderBook.hpp"

// forward declaration of helper used below
template <typename Map>
static void remove_from_level(Map& map, const float price, long order_id);


void OrderBook::add_order(const Order& order) {
    std::lock_guard<std::mutex> lock(mutex);

    auto [it, inserted] = order_lookup.emplace(order.getOrderId(), std::move(order));
    if (!inserted)
        return;

    Order* ptr = &it->second;
    if (order.getUserAction() == Action::BID)
        bids[order.getPrice()].push_back(ptr);
    else
        asks[order.getPrice()].push_back(ptr);
}

float OrderBook::best_bid() const {
    std::lock_guard<std::mutex> lock(mutex);
    return bids.empty() ? 0.0f : bids.begin()->first;
}

float OrderBook::best_ask() const {
    std::lock_guard<std::mutex> lock(mutex);
    return asks.empty() ? 0.0f : asks.begin()->first;
}

float OrderBook::spread() const {
    std::lock_guard<std::mutex> lock(mutex);
    if (bids.empty() || asks.empty())
        return 0.0f;

    return asks.begin()->first - bids.begin()->first;
}

void OrderBook::cancel(long order_id) {
    std::lock_guard<std::mutex> lock(mutex);
    auto lookup_it = order_lookup.find(order_id);
    if (lookup_it == order_lookup.end())
        throw std::out_of_range("cancel: order not found");

    const Order& order = lookup_it->second;
    if (order.getUserAction() == Action::BID)
        remove_from_level(bids, order.getPrice(), order_id);
    else
        remove_from_level(asks, order.getPrice(), order_id);

    order_lookup.erase(lookup_it);
}

const Order* OrderBook::get_order(long order_id) const {
    std::lock_guard<std::mutex> lock(mutex);
    auto lookup_it = order_lookup.find(order_id);
    return lookup_it == order_lookup.end() ? nullptr : &lookup_it->second;
}

Order* OrderBook::get_order(long order_id) {
    std::lock_guard<std::mutex> lock(mutex);
    auto lookup_it = order_lookup.find(order_id);
    return lookup_it == order_lookup.end() ? nullptr : &lookup_it->second;
}

std::optional<Order> OrderBook::get_order_copy(long order_id) const {
    std::lock_guard<std::mutex> lock(mutex);
    auto lookup_it = order_lookup.find(order_id);
    if (lookup_it == order_lookup.end())
        return std::nullopt;
    return lookup_it->second;
}

Order* OrderBook::best_ask_order() {
    std::lock_guard<std::mutex> lock(mutex);
    if (asks.empty())
        return nullptr;
    auto &level = asks.begin()->second;
    return level.empty() ? nullptr : level.front();
}

Order* OrderBook::best_bid_order() {
    std::lock_guard<std::mutex> lock(mutex);
    if (bids.empty())
        return nullptr;
    auto &level = bids.begin()->second;
    return level.empty() ? nullptr : level.front();
}

void OrderBook::display() const {
    std::lock_guard<std::mutex> lock(mutex);
    std::cout << "\nASKS\n";
    for (const auto& [price, orders] : asks) {
        int totalQty = 0;
        for (const auto& order : orders) {
            totalQty += order->getQuantity();
        }
        std::cout << price << ":" << totalQty << " ( " << orders.size() << " )" << "\n";
    }

    std::cout << "\nBIDS\n";
    for (const auto& [price, orders] : bids) {
        int totalQty = 0;
        for (const auto& order : orders) {
            totalQty += order->getQuantity();
        }
        std::cout << price << ":" << totalQty << " ( " << orders.size() << " )" << "\n";
    }

    float bestBid = bids.empty() ? 0.0f : bids.begin()->first;
    float bestAsk = asks.empty() ? 0.0f : asks.begin()->first;
    float spreadVal = (bids.empty() || asks.empty()) ? 0.0f : bestAsk - bestBid;

    std::cout << "Best Bid :" << bestBid << "\n";
    std::cout << "Best Ask :" << bestAsk << "\n";
    std::cout << "Spread :" << spreadVal << "\n";
}

void OrderBook::modify(long order_id, float price, long quantity) {
    std::lock_guard<std::mutex> lock(mutex);
    if (price < 0.0f)
        throw std::invalid_argument("Price cannot be negative");
    if (quantity <= 0)
        throw std::invalid_argument("Quantity must be positive");

    auto lookup_it = order_lookup.find(order_id);
    if (lookup_it == order_lookup.end())
        throw std::out_of_range("modify: order not found");

    Order& order = lookup_it->second;
    const float oldPrice = order.getPrice();
    auto action = order.getUserAction();

    if (oldPrice != price) {
        if (action == Action::BID)
            remove_from_level(bids, oldPrice, order_id);
        else
            remove_from_level(asks, oldPrice, order_id);
    }

    order.setPrice(price);
    order.setQuantity(quantity);

    if (oldPrice != price) {
        if (action == Action::BID)
            bids[price].push_back(&order);
        else
            asks[price].push_back(&order);
    }
}

//Helpers

template <typename Map>
static void remove_from_level(Map& map,const float price, long order_id) {
    auto level_it = map.find(price);
    if (level_it == map.end()) return;

    auto& dq = level_it->second;
    auto  it  = std::find_if(dq.begin(), dq.end(),
                    [order_id](const Order* o) {
                        return o->getOrderId() == order_id;
                    });

    if (it != dq.end()) dq.erase(it);
    if (dq.empty())     map.erase(level_it);
}