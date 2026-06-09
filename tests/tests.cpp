#include <iostream>
#include <stdexcept>
#include "Order.hpp"
#include "OrderBook.hpp"
#define EXPECT_TRUE(x) if(!(x)) { std::cout << "FAILED: " << #x << "\n"; exit(1);}
#define EXPECT_FALSE(x) if(x) { std::cout << "FAILED: " << #x << "\n"; exit(1);}

void test_empty_book()
{
    OrderBook book;

    EXPECT_TRUE(book.best_bid() == 0.0f);
    EXPECT_TRUE(book.best_ask() == 0.0f);
    EXPECT_TRUE(book.spread() == 0.0f);

    std::cout << "✓ test_empty_book\n";
}

void test_single_bid()
{
    OrderBook book;
    Order order("alice", Action::BID, 100, 10);
    long id = order.getOrderId();

    book.add_order(order);

    EXPECT_TRUE(book.best_bid() == 100);
    EXPECT_TRUE(book.best_ask() == 0.0f);
    EXPECT_TRUE(book.get_order(id) != nullptr);
    EXPECT_TRUE(book.get_order(id)->getPrice() == 100);

    std::cout << "✓ test_single_bid\n";
}

void test_single_ask()
{
    OrderBook book;
    Order order("bob", Action::ASK, 105, 20);
    long id = order.getOrderId();

    book.add_order(order);

    EXPECT_TRUE(book.best_ask() == 105);
    EXPECT_TRUE(book.best_bid() == 0.0f);
    EXPECT_TRUE(book.get_order(id) != nullptr);
    EXPECT_TRUE(book.get_order(id)->getPrice() == 105);

    std::cout << "✓ test_single_ask\n";
}

void test_best_bid()
{
    OrderBook book;
    book.add_order(Order("u1", Action::BID, 100, 10));
    book.add_order(Order("u2", Action::BID, 105, 10));

    EXPECT_TRUE(book.best_bid() == 105);
    std::cout << "✓ test_best_bid\n";
}

void test_best_ask()
{
    OrderBook book;
    book.add_order(Order("u1", Action::ASK, 110, 10));
    book.add_order(Order("u2", Action::ASK, 108, 10));

    EXPECT_TRUE(book.best_ask() == 108);
    std::cout << "✓ test_best_ask\n";
}

void test_same_price_level()
{
    OrderBook book;
    Order first("u1", Action::BID, 100, 10);
    Order second("u2", Action::BID, 100, 15);

    book.add_order(first);
    book.add_order(second);

    EXPECT_TRUE(book.best_bid() == 100);
    EXPECT_TRUE(book.get_order(first.getOrderId()) != nullptr);
    EXPECT_TRUE(book.get_order(second.getOrderId()) != nullptr);

    std::cout << "✓ test_same_price_level\n";
}

void test_spread()
{
    OrderBook book;
    book.add_order(Order("u1", Action::BID, 100, 10));
    book.add_order(Order("u2", Action::ASK, 108, 10));

    EXPECT_TRUE(book.spread() == 8);
    std::cout << "✓ test_spread\n";
}

void test_spread_empty_side()
{
    OrderBook book;
    book.add_order(Order("u1", Action::BID, 100, 10));

    EXPECT_TRUE(book.spread() == 0.0f);
    std::cout << "✓ test_spread_empty_side\n";
}

void test_cancel()
{
    OrderBook book;
    Order order("alice", Action::BID, 100, 10);
    long id = order.getOrderId();

    book.add_order(order);
    book.cancel(id);

    EXPECT_TRUE(book.best_bid() == 0.0f);
    EXPECT_TRUE(book.get_order(id) == nullptr);
    EXPECT_TRUE(book.best_ask() == 0.0f);

    std::cout << "✓ test_cancel\n";
}

void test_cancel_nonexistent()
{
    OrderBook book;
    bool threw = false;

    try {
        book.cancel(999999);
    } catch (const std::out_of_range&) {
        threw = true;
    }

    EXPECT_TRUE(threw);
    std::cout << "✓ test_cancel_nonexistent\n";
}

void test_modify_order_price_and_quantity()
{
    OrderBook book;
    Order order("alice", Action::BID, 100, 10);
    long id = order.getOrderId();

    book.add_order(order);
    book.modify(book.get_order(id), 110.0f, 25);

    EXPECT_TRUE(book.best_bid() == 110.0f);
    EXPECT_TRUE(book.get_order(id) != nullptr);
    EXPECT_TRUE(book.get_order(id)->getPrice() == 110.0f);
    EXPECT_TRUE(book.get_order(id)->getQuantity() == 25);

    std::cout << "✓ test_modify_order_price_and_quantity\n";
}

int main()
{
    test_empty_book();
    test_single_bid();
    test_single_ask();
    test_best_bid();
    test_best_ask();
    test_same_price_level();
    test_spread();
    test_spread_empty_side();
    test_cancel();
    test_cancel_nonexistent();
    test_modify_order_price_and_quantity();

    std::cout << "\nAll tests passed\n";
}
