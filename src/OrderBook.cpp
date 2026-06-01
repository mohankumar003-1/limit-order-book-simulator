#include <iostream>
#include "OrderBook.hpp"

// forward declaration of helper used below
template <typename Map>
static void remove_from_level(Map& map, const float price, long order_id);


void OrderBook::add_order(const Order& order) {
    
    auto [it, inserted] = order_lookup.emplace(order.getOrderId(), std::move(order));
    if (!inserted) return; 

    Order* ptr = &it->second;

    if (order.getUserAction() == Action::BID)
        bids[order.getPrice()].push_back(ptr);
    else
        asks[order.getPrice()].push_back(ptr);
}

float OrderBook::best_bid() const{
    return bids.empty() ? 0.0 : bids.begin()->first;
}

float OrderBook::best_ask() const{
    return asks.empty() ? 0.0 : asks.begin()->first;
}

float OrderBook::spread() const{
    if (bids.empty() || asks.empty())
        return 0.0f;

    return best_ask() - best_bid(); // No abs because in valid market always ask >= bid
}

void OrderBook::cancel(long order_id) {
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

void OrderBook::display() const {
    std::cout << "\nASKS\n";
    for (const auto& [price,orders]: asks) {
        int totalQty = 0;
        for(const auto& order : orders){
            totalQty += order->getQuantity();
        }
        std::cout << price << ":" << totalQty << " ( " << orders.size() << " )"<<"\n";
    }

        std::cout << "\nBIDS\n";
    for (const auto& [price,orders]: bids) {
        int totalQty = 0;
        for(const auto& order : orders){
            totalQty += order->getQuantity();
        }
        std::cout << price << ":" << totalQty << " ( " << orders.size() << " )"<<"\n";
    }

    std::cout<<"Best Bid :"<<best_bid()<<"\n";
    std::cout<<"Best Ask :"<<best_ask()<<"\n";
    std::cout<<"Spread :"<<spread()<<"\n";
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