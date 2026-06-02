#pragma once

#include "OrderBook.hpp"

class HttpServer
{
private:

    OrderBook& book;

public:

    explicit HttpServer(
        OrderBook& book);

    void start(int port);
};