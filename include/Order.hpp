#pragma once

#include <string>
#include <chrono>
#include <stdexcept>
#include "Types.hpp"

class Order {

private:
    static long next_id;
    long order_id;
    long timestamp;
    std::string user_id;
    Action user_action;
    float price;
    int quantity;

public:

explicit Order(const std::string& id, Action act, float p, int q)
    : user_id(id), user_action(act), price(p), quantity(q)
{
    if (p < 0) throw std::invalid_argument("Price cannot be negative");
    if (q <= 0) throw std::invalid_argument("Quantity must be positive");

    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch());
    timestamp = duration.count();
    order_id = next_id++;
    
}

    std::string getUserId() const { return user_id; }
    Action getUserAction() const { return user_action; }
    float getPrice() const { return price; }
    int getQuantity() const { return quantity; }
    void reduceQuantity(int amount) {
        if (amount < 0 || amount > quantity)
            throw std::invalid_argument("Invalid quantity reduction");
        quantity -= amount;
    }
    long getTimestamp() const {return timestamp;}
    long getOrderId() const {return order_id;}

};