#include <iostream>
#include <string>
#include <sstream>
#include <vector>


enum action {
    BID,
    ASK
}
;
class Order {

private:
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
}

    std::string getUserId() const { return user_id; }
    action getUserAction() const { return user_action; }
    float getPrice() const { return price; }
    int getQuantity() const { return quantity; }



};
int main() {

    while (true) {
        std::cout << "Place Your order : (format : user_id;BID / ASK;price;quantity)" << std::endl;
        std::string user_input;
        if (!std::getline(std::cin, user_input))
            break;
        if (user_input.empty())
            continue;

        auto split = [](const std::string text, char delim) {
            std::vector<std::string> result;
            std::string token;
            std::stringstream ss(text);
            while (std::getline(ss, token, delim)) {
                result.push_back(token);
            }
            return result;
        };

        std::vector<std::string> parts = split(user_input, ';');

        if (parts.size() != 4) {
            std::cout << "Invalid format. Expected 4 fields separated by ';'." << std::endl;
            continue;
        }

        std::string user_id = parts[0];
        std::string action_str = parts[1];
        action user_action;
        if (action_str == "BID") user_action = BID;
        else if (action_str == "ASK") user_action = ASK;
        else {
            std::cout << "Invalid action. Use BID or ASK." << std::endl;
            continue;
        }

        try {
            float price = std::stof(parts[2]);
            int quantity = std::stoi(parts[3]);

            Order new_order(user_id, user_action, price, quantity);

            std::cout << "User: " << new_order.getUserId()
                    << ", Action: " << (new_order.getUserAction() == BID ? "BID" : "ASK")
                    << ", Price: " << new_order.getPrice()
                    << ", Quantity: " << new_order.getQuantity()
                    << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
}