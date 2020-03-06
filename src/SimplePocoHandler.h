//
// Created by dmitri on 05.03.2020.
//

#ifndef HUFFMANCODE_SIMPLEPOCOHANDLER_H
#define HUFFMANCODE_SIMPLEPOCOHANDLER_H

#include <include/amqpcpp.h>
#include <memory>

class SimplePocoHandlerImpl;
class SimplePocoHandler: public AMQP::ConnectionHandler {
private:
    std::unique_ptr<SimplePocoHandlerImpl> m_impl;
public:
    friend SimplePocoHandlerImpl;

    SimplePocoHandler(const std::string& host, uint16_t port);
    ~SimplePocoHandler() override;

    void loop();
    void quit();

    bool connected() const;

    SimplePocoHandler(const SimplePocoHandler&) = delete;
    SimplePocoHandler& operator=(const SimplePocoHandler&) = delete;

private:
    static constexpr size_t MB = 1024 * 1024;
    static constexpr size_t BUFFER_SIZE = 8 *MB;
    static constexpr size_t TEMP_BUFFER_SIZE = 1 *MB;

    void onData(AMQP::Connection* connection, const char* data, size_t size) override;
    void onReady(AMQP::Connection* connection) override;
    void onError(AMQP::Connection* connection, const char* message) override;
    void onClosed(AMQP::Connection* connection) override;

    void close();
    void sendDataFromBuffer();
};

#endif //HUFFMANCODE_SIMPLEPOCOHANDLER_H
