#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <map>
#include <deque>
#include <unordered_map>

enum class action {
    BID,
    ASK
};

class Order {

private:
    static long next_id;
    long order_id;
    long timestamp;
    std::string user_id;
    action user_action;
    float price;
    int quantity;

public:

explicit Order(const std::string& id, action act, float p, int q)
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
    action getUserAction() const { return user_action; }
    float getPrice() const { return price; }
    int getQuantity() const { return quantity; }
    long getTimestamp() const {return timestamp;}
    long getOrderId() const {return order_id;}

};

long Order::next_id = 1;

class OrderBook {

    private :
    
    std::map<float,std::deque<Order>,std::greater<float>> bids;
    std::map<float,std::deque<Order>> asks;
    std::unordered_map<long, Order> order_lookup;

    public:

    void add_order(const Order& order);
    void display() const;
    float best_bid() const;
    float best_ask() const;
    float spread() const;
    void cancel(long order_id);
};

void OrderBook::add_order(const Order& order) {
    if(order.getUserAction() == action::BID) {
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

    if(order.getUserAction() == action::BID)
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

std::vector<std::string> split(const std::string& text, char delim) {
    std::vector<std::string> result;
    std::string token;
    std::stringstream ss(text);
    while (std::getline(ss, token, delim)) {
        result.push_back(token);
    }
    return result;
}

int main() {

    OrderBook book;

    while (true) {
        std::cout << "Place Your order : (format : user_id;BID / ASK;price;quantity)" << std::endl;
        std::string user_input;
        if (!std::getline(std::cin, user_input))
            break;
        if (user_input.empty())
            continue;

        std::vector<std::string> parts = split(user_input, ';');


        if(user_input.starts_with("CANCEL;"))
        {
            auto parts = split(user_input, ';');

            if(parts.size() == 2)
            {
                long id = std::stol(parts[1]);

                book.cancel(id);

                book.display();
            }

            continue;
        }
        if (parts.size() != 4) {
            std::cout << "Invalid format. Expected 4 fields separated by ';'." << std::endl;
            continue;
        }

        std::string user_id = parts[0];
        std::string action_str = parts[1];
        action user_action;
        if (action_str == "BID") user_action = action::BID;
        else if (action_str == "ASK") user_action = action::ASK;
        else {
            std::cout << "Invalid action. Use BID or ASK." << std::endl;
            continue;
        }

        try {
            float price = std::stof(parts[2]);
            int quantity = std::stoi(parts[3]);

            Order new_order(user_id, user_action, price, quantity);

            book.add_order(new_order);
            std::cout<< "Order accepted. ID="<< new_order.getOrderId()<< "\n";
            book.display();
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
}