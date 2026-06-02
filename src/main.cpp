#include <iostream>
#include <vector>
#include "OrderBook.hpp"
#include <sstream>
#include "HttpServer.hpp"

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
    HttpServer server(book);
    server.start(8080);
    return 0;

}