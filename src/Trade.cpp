#include<iostream>
#include <algorithm>
#include "Trade.hpp"

void Trade::match_bid(Order& order, OrderBook &book) {
    if (order.getQuantity() <= 0) {
        std::cout << "Order quantity must be positive\n";
        return;
    }

    if (book.best_ask() == 0.0f || order.getPrice() < book.best_ask()) {
        std::cout << "Matching will not happen\n";
        return;
    }

    while (order.getQuantity() > 0 && book.best_ask() > 0.0f && order.getPrice() >= book.best_ask()) {
        Order* ask = book.best_ask_order();
        if (!ask)
            break;

        int traded_qty = std::min(order.getQuantity(), ask->getQuantity());
        buy_order_id = order.getOrderId();
        sell_order_id = ask->getOrderId();
        quantity = traded_qty;
        price = ask->getPrice();

        std::cout << "Trade executed: buy=" << buy_order_id
                  << " sell=" << sell_order_id
                  << " qty=" << quantity
                  << " price=" << price << "\n";

        order.reduceQuantity(traded_qty);
        ask->reduceQuantity(traded_qty);

        if (ask->getQuantity() == 0) {
            book.cancel(sell_order_id);
        }
    }

    if (order.getQuantity() > 0) {
        std::cout << "Remaining bid quantity: " << order.getQuantity() << " at price " << order.getPrice() << "\n";
    }
}

void Trade::match_ask(Order& order, OrderBook &book) {
    if (order.getQuantity() <= 0) {
        std::cout << "Order quantity must be positive\n";
        return;
    }

    if (book.best_bid() == 0.0f || order.getPrice() > book.best_bid()) {
        std::cout << "Matching will not happen\n";
        return;
    }

    while (order.getQuantity() > 0 && book.best_bid() > 0.0f && order.getPrice() <= book.best_bid()) {
        Order* bid = book.best_bid_order();
        if (!bid)
            break;

        int traded_qty = std::min(order.getQuantity(), bid->getQuantity());
        buy_order_id = bid->getOrderId();
        sell_order_id = order.getOrderId();
        quantity = traded_qty;
        price = bid->getPrice();

        std::cout << "Trade executed: buy=" << buy_order_id
                  << " sell=" << sell_order_id
                  << " qty=" << quantity
                  << " price=" << price << "\n";

        order.reduceQuantity(traded_qty);
        bid->reduceQuantity(traded_qty);

        if (bid->getQuantity() == 0) {
            book.cancel(buy_order_id);
        }
    }

    if (order.getQuantity() > 0) {
        std::cout << "Remaining ask quantity: " << order.getQuantity() << " at price " << order.getPrice() << "\n";
    }
}