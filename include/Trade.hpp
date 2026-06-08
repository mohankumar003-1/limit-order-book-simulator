#pragma once
#include <vector>
#include "Order.hpp"
#include "OrderBook.hpp"

class Trade 
{
    private:
        long buy_order_id;
        long sell_order_id;
        int quantity;
        float price;
        std::vector<Trade> trades;
    public:
        long getBuyOrderId() const { return buy_order_id; }
        void setBuyOrderId(long id) { buy_order_id = id; }

        long getSellOrderId() const { return sell_order_id; }
        void setSellOrderId(long id) { sell_order_id = id; }

        int getQuantity() const { return quantity; }
        void setQuantity(int qty) { quantity = qty; }

        float getPrice() const { return price; }
        void setPrice(float p) { price = p; }

        void process_order(Order& order);
        void match_bid(Order& order, OrderBook &book);
        void match_ask(Order& order, OrderBook &book);
};