#include <iostream>
#include "OrderBook.hpp"


void OrderBook::add_order(const Order& order) {
    if(order.getUserAction() == Action::BID) {
        bids[order.getPrice()].push_back(order);
    } else {
        asks[order.getPrice()].push_back(order);
    }
    order_lookup.insert_or_assign(order.getOrderId(), order);
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

void OrderBook::cancel(long order_id)
{
    auto lookup_it = order_lookup.find(order_id);

    if(lookup_it == order_lookup.end())
    {
        std::cout << "Order not found\n";
        return;
    }

    Order order = lookup_it->second;

    if(order.getUserAction() == Action::BID)
    {
        auto& level = bids[order.getPrice()];

        for(auto it = level.begin();it != level.end();++it)
        {
            if(it->getOrderId() == order_id)
            {
                level.erase(it);
                break;
            }
        }

        if(level.empty())
            bids.erase(order.getPrice());
    }
    else
    {
        auto& level = asks[order.getPrice()];

        for(auto it = level.begin();it != level.end();++it)
        {
            if(it->getOrderId() == order_id)
            {
                level.erase(it);
                break;
            }
        }

        if(level.empty())
            asks.erase(order.getPrice());
    }


    order_lookup.erase(order_id);

    std::cout << "Cancelled order "<< order_id<< "\n";
}
void OrderBook::display() const{
    std::cout << "\nASKS\n";
    for (const auto& [price,orders]: asks) {
        int totalQty = 0;
        for(const auto& order : orders){
            totalQty += order.getQuantity();
        }
        std::cout << price << ":" << totalQty << " ( " << orders.size() << " )"<<"\n";
    }

        std::cout << "\nBIDS\n";
    for (const auto& [price,orders]: bids) {
        int totalQty = 0;
        for(const auto& order : orders){
            totalQty += order.getQuantity();
        }
        std::cout << price << ":" << totalQty << " ( " << orders.size() << " )"<<"\n";
    }

    std::cout<<"Best Bid :"<<best_bid()<<"\n";
    std::cout<<"Best Ask :"<<best_ask()<<"\n";
    std::cout<<"Spread :"<<spread()<<"\n";
}