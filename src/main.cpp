#include <iostream>
#include <vector>
#include "OrderBook.hpp"
#include <sstream>

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
        Action user_action;
        if (action_str == "BID") user_action = Action::BID;
        else if (action_str == "ASK") user_action = Action::ASK;
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