#pragma once
#include "Order.hpp"
#include "OrderBook.hpp"
class Trade 
{
    private:
        long buy_order_id;
        long sell_order_id;
        int quantity;
        float price;
    public:
        void process_order(Order& order);
        void match_bid(Order& order,OrderBook &book);
        void match_ask(Order& order,OrderBook &book);

};