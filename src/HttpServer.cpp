#include "HttpServer.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <sstream>
#include <optional>
#include <cctype>

#pragma comment(lib, "ws2_32.lib")

static std::string escapeJson(const std::string &value)
{
    std::string escaped;
    for (char c : value)
    {
        switch (c)
        {
        case '"':
            escaped += "\\\"";
            break;
        case '\\':
            escaped += "\\\\";
            break;
        case '\b':
            escaped += "\\b";
            break;
        case '\f':
            escaped += "\\f";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped.push_back(c);
            break;
        }
    }
    return escaped;
}

static std::string actionToString(Action action)
{
    return action == Action::BID ? "BID" : "ASK";
}

static std::pair<std::string, std::string> splitPathAndQuery(const std::string &fullPath)
{
    size_t queryPos = fullPath.find('?');
    if (queryPos == std::string::npos)
        return {fullPath, ""};
    return {fullPath.substr(0, queryPos), fullPath.substr(queryPos + 1)};
}

static std::optional<long> extractOrderId(const std::string &path, const std::string &query)
{
    if (path.rfind("/order", 0) != 0)
        return std::nullopt;

    if (path.size() > 6 && path[6] == '/')
    {
        std::string idString = path.substr(7);
        if (idString.empty())
            return std::nullopt;
        try
        {
            return std::stol(idString);
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    if (!query.empty())
    {
        std::istringstream queryStream(query);
        std::string part;
        while (std::getline(queryStream, part, '&'))
        {
            size_t eq = part.find('=');
            if (eq == std::string::npos)
                continue;
            std::string key = part.substr(0, eq);
            std::string value = part.substr(eq + 1);
            if (key == "id")
            {
                try
                {
                    return std::stol(value);
                }
                catch (...)
                {
                    return std::nullopt;
                }
            }
        }
    }

    return std::nullopt;
}

static std::string getRequestBody(const std::string &request)
{
    const std::string separator = "\r\n\r\n";
    size_t pos = request.find(separator);
    if (pos == std::string::npos)
        return {};
    return request.substr(pos + separator.size());
}

static std::optional<std::string> parseJsonStringField(const std::string &json, const std::string &field)
{
    std::string key = "\"" + field + "\"";
    size_t pos = json.find(key);
    if (pos == std::string::npos)
        return std::nullopt;
    pos = json.find(':', pos);
    if (pos == std::string::npos)
        return std::nullopt;
    pos = json.find('"', pos);
    if (pos == std::string::npos)
        return std::nullopt;
    size_t end = json.find('"', pos + 1);
    if (end == std::string::npos)
        return std::nullopt;
    return json.substr(pos + 1, end - pos - 1);
}

static std::optional<float> parseJsonFloatField(const std::string &json, const std::string &field)
{
    std::string key = "\"" + field + "\"";
    size_t pos = json.find(key);
    if (pos == std::string::npos)
        return std::nullopt;
    pos = json.find(':', pos);
    if (pos == std::string::npos)
        return std::nullopt;
    size_t start = json.find_first_not_of(" \t\n\r", pos + 1);
    if (start == std::string::npos)
        return std::nullopt;
    size_t end = start;
    while (end < json.size() && (std::isdigit(json[end]) || json[end] == '.' || json[end] == '-' || json[end] == '+'))
        ++end;
    try
    {
        return std::stof(json.substr(start, end - start));
    }
    catch (...)
    {
        return std::nullopt;
    }
}

static std::optional<int> parseJsonIntField(const std::string &json, const std::string &field)
{
    auto value = parseJsonFloatField(json, field);
    if (!value)
        return std::nullopt;
    return static_cast<int>(*value);
}

static std::optional<Action> parseJsonActionField(const std::string &json, const std::string &field)
{
    auto actionStr = parseJsonStringField(json, field);
    if (!actionStr)
        return std::nullopt;
    if (*actionStr == "BID")
        return Action::BID;
    if (*actionStr == "ASK")
        return Action::ASK;
    return std::nullopt;
}

static std::string makeJsonResponse(int statusCode, const std::string &statusText, const std::string &body)
{
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    response << "Content-Type: application/json\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Connection: close\r\n\r\n";
    response << body;
    return response.str();
}

HttpServer::HttpServer(OrderBook &book) : book(book)
{
}

void HttpServer::start(int port)
{

    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa))
    {
        std::cout << "WSAStartup failed\n";
        return;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in serverAddr{};

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr));

    listen(serverSocket, SOMAXCONN);

    std::cout << "Listening on port " << port << "\n";

    while (true)
    {
        SOCKET client = accept(serverSocket, nullptr, nullptr);

        char buffer[4096] = {0};

        int bytes = recv(client, buffer, sizeof(buffer) - 1, 0);

        if (bytes <= 0)
        {
            closesocket(client);
            continue;
        }

        std::string request(buffer);

        std::cout << "\nREQUEST:\n"
                  << request << "\n";

        std::istringstream requestStream(request);
        std::string method;
        std::string fullPath;
        requestStream >> method >> fullPath;

        auto [path, query] = splitPathAndQuery(fullPath);

        if (method == "GET" && path == "/top")
        {
            std::ostringstream body;
            body << "{";
            body << "\"best_bid\":" << book.best_bid() << ",";
            body << "\"best_ask\":" << book.best_ask() << ",";
            body << "\"spread\":" << book.spread();
            body << "}";

            std::string response = makeJsonResponse(200, "OK", body.str());
            send(client, response.c_str(), static_cast<int>(response.size()), 0);
        }
        else if (method == "GET" && path == "/health")
        {
            std::string body = "{\"status\":\"healthy\"}";
            std::string response = makeJsonResponse(200, "OK", body);
            send(client, response.c_str(), static_cast<int>(response.size()), 0);
        }
        else if (method == "POST" && path == "/order")
        {
            std::string bodyText = getRequestBody(request);
            if (bodyText.empty())
            {
                std::string body = "{\"error\":\"missing request body\"}";
                std::string response = makeJsonResponse(400, "Bad Request", body);
                send(client, response.c_str(), static_cast<int>(response.size()), 0);
            }
            else
            {
                auto userId = parseJsonStringField(bodyText, "user_id");
                auto action = parseJsonActionField(bodyText, "action");
                auto price = parseJsonFloatField(bodyText, "price");
                auto quantity = parseJsonIntField(bodyText, "quantity");

                if (!userId || !action || !price || !quantity)
                {
                    std::string body = "{\"error\":\"invalid order payload\"}";
                    std::string response = makeJsonResponse(400, "Bad Request", body);
                    send(client, response.c_str(), static_cast<int>(response.size()), 0);
                }
                else
                {
                    try
                    {
                        Order newOrder(*userId, *action, *price, *quantity);
                        book.add_order(newOrder);

                        std::ostringstream body;
                        body << "{";
                        body << "\"status\":\"created\",";
                        body << "\"order_id\":" << newOrder.getOrderId() << ",";
                        body << "\"user_id\":\"" << escapeJson(newOrder.getUserId()) << "\",";
                        body << "\"action\":\"" << actionToString(newOrder.getUserAction()) << "\",";
                        body << "\"price\":" << newOrder.getPrice() << ",";
                        body << "\"quantity\":" << newOrder.getQuantity();
                        body << "}";

                        std::string response = makeJsonResponse(201, "Created", body.str());
                        send(client, response.c_str(), static_cast<int>(response.size()), 0);
                    }
                    catch (const std::exception &)
                    {
                        std::string body = "{\"error\":\"invalid order payload\"}";
                        std::string response = makeJsonResponse(400, "Bad Request", body);
                        send(client, response.c_str(), static_cast<int>(response.size()), 0);
                    }
                }
            }
        }
        else if ((method == "GET" || method == "DELETE") && path.rfind("/order", 0) == 0)
        {
            auto orderId = extractOrderId(path, query);
            if (!orderId)
            {
                std::string body = "{\"error\":\"invalid order id\"}";
                std::string response = makeJsonResponse(400, "Bad Request", body);
                send(client, response.c_str(), static_cast<int>(response.size()), 0);
            }
            else if (method == "GET")
            {
                const Order *order = book.get_order(*orderId);
                if (!order)
                {
                    std::string body = "{\"error\":\"order not found\"}";
                    std::string response = makeJsonResponse(404, "Not Found", body);
                    send(client, response.c_str(), static_cast<int>(response.size()), 0);
                }
                else
                {
                    std::ostringstream body;
                    body << "{";
                    body << "\"order_id\":" << order->getOrderId() << ",";
                    body << "\"user_id\":\"" << escapeJson(order->getUserId()) << "\",";
                    body << "\"action\":\"" << actionToString(order->getUserAction()) << "\",";
                    body << "\"price\":" << order->getPrice() << ",";
                    body << "\"quantity\":" << order->getQuantity() << ",";
                    body << "\"timestamp\":" << order->getTimestamp();
                    body << "}";

                    std::string response = makeJsonResponse(200, "OK", body.str());
                    send(client, response.c_str(), static_cast<int>(response.size()), 0);
                }
            }
            else if (method == "DELETE")
            {
                try
                {
                    book.cancel(*orderId);
                    std::ostringstream body;
                    body << "{";
                    body << "\"status\":\"deleted\",";
                    body << "\"order_id\":" << *orderId;
                    body << "}";

                    std::string response = makeJsonResponse(200, "OK", body.str());
                    send(client, response.c_str(), static_cast<int>(response.size()), 0);
                }
                catch (const std::exception &)
                {
                    std::string body = "{\"error\":\"order not found\"}";
                    std::string response = makeJsonResponse(404, "Not Found", body);
                    send(client, response.c_str(), static_cast<int>(response.size()), 0);
                }
            }
        }
        else
        {
            std::string body = "{\"error\":\"route not found\"}";
            std::string response = makeJsonResponse(404, "Not Found", body);
            send(client, response.c_str(), static_cast<int>(response.size()), 0);
        }

        closesocket(client);
    }
}