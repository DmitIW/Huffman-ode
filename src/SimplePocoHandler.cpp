//
// Created by dmitri on 05.03.2020.
//
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <optional>
#include <Poco/Net/StreamSocket.h>

#include "SimplePocoHandler.h"

using namespace std;

namespace utility {
    class Buffer {
    private:
        vector<char> m_data;
        size_t m_use;

        inline bool is_full() const { return m_use >= m_data.size(); }
    public:
        explicit Buffer(size_t size): m_data(size, 0), m_use(0) {
            cout << "Buffer cont\n";
        }

        size_t write(const char* data, size_t size) {
            if (is_full())
                return 0;

            const size_t write = (size + m_use) < m_data.size() ? size : m_data.size() - m_use;
            copy(data, data + write, m_data.begin());
            return write;
        }

        void drain() { m_use = 0; }
        size_t available() const { return m_use; }
        const char* cbegin() const { return &m_data.front(); }
        const char* cend() const { return (&m_data.back()) + 1; }
        char* begin() { return &m_data.front(); }
        char* end() { return (&m_data.back()) + 1; }
        size_t size() const { return m_data.size(); }
        void resize(size_t new_size, char value = 0) { m_data.resize(new_size, value); }
        void shl(size_t count) {
            if (count >= m_use)
                throw runtime_error("Buffer:: Shift count wrong");

            move(m_data.data() + count, m_data.data() + (m_data.size() - count),
                    m_data.begin());
            m_use -= count;
        }
    };
}

class SimplePocoHandlerImpl {
public:

    Poco::Net::StreamSocket socket;

    bool connected;
    bool quit;

    AMQP::Connection* connection;

    utility::Buffer inputBuffer;
    utility::Buffer outputBuffer;
    utility::Buffer tmpBuffer;

    SimplePocoHandlerImpl():
        socket(),
        connected(false),
        quit(false),
        connection(nullptr),
        inputBuffer(SimplePocoHandler::BUFFER_SIZE),
        outputBuffer(SimplePocoHandler::BUFFER_SIZE),
        tmpBuffer(SimplePocoHandler::TEMP_BUFFER_SIZE) {}
};

SimplePocoHandler::SimplePocoHandler(const string& host, uint16_t port):
    m_impl(make_unique<SimplePocoHandlerImpl>()) {
    m_impl->socket.connect(Poco::Net::SocketAddress(host, port));
    m_impl->socket.setKeepAlive(true);
}
SimplePocoHandler::~SimplePocoHandler() { close(); }

void SimplePocoHandler::loop() {
    try {
        cout << "SimplePocoHandler::loop\n";
        while(!m_impl->quit) {
            int socket_avail = m_impl->socket.available();
            size_t input_buf_avail = m_impl->inputBuffer.available();

            if (socket_avail > 0) {

                if (static_cast<int64_t>(m_impl->tmpBuffer.size()) < socket_avail)
                    m_impl->tmpBuffer.resize(socket_avail);

                m_impl->socket.receiveBytes(m_impl->tmpBuffer.begin(), socket_avail);
                m_impl->inputBuffer.write(m_impl->tmpBuffer.begin(), socket_avail);

            } else if (socket_avail < 0) {
                throw Poco::Exception("Problem with socket.");
            }

            if (m_impl->connection && input_buf_avail != 0) {
                size_t count = m_impl->connection->parse(m_impl->inputBuffer.cbegin(), input_buf_avail);

                if (count == input_buf_avail)
                    m_impl->inputBuffer.drain();
                else if (count > 0)
                    m_impl->inputBuffer.shl(count);
            }

            sendDataFromBuffer();
            this_thread::sleep_for(chrono::milliseconds(10));
        }

        if (m_impl->outputBuffer.available() != 0)
            sendDataFromBuffer();

        cout << "SimplePocoHandler::loop end\n";

    } catch (const Poco::Exception& exc) {
        cerr << "SimplePocoHandler::loop :: POCO exception: " << exc.displayText() << "\n";
    } catch (...) {
        cerr << "SimplePocoHandler::loop :: Exception: \n";
        throw;
    }
}

void SimplePocoHandler::quit() { m_impl->quit = true; }
void SimplePocoHandler::close() { m_impl->socket.close(); }
void SimplePocoHandler::onData(AMQP::Connection *connection, const char *data, size_t size) {
    cout << "SimplePocoHandler::onData\n";
    m_impl->connection = connection;
    const size_t writen = m_impl->outputBuffer.write(data, size);
    if (writen != size) {
        sendDataFromBuffer();
        m_impl->outputBuffer.write(data + writen, size - writen);
    }
}
void SimplePocoHandler::onReady(AMQP::Connection*) { m_impl->connected = true; }
void SimplePocoHandler::onError(AMQP::Connection*, const char *message) { cerr << message << "\n"; }
void SimplePocoHandler::onClosed(AMQP::Connection*) {
    cout << "Close connection \n";
    quit();
}
bool SimplePocoHandler::connected() const { return m_impl->connected; }
void SimplePocoHandler::sendDataFromBuffer() {
    const size_t available = m_impl->outputBuffer.available();
    if (available != 0) {
        m_impl->socket.sendBytes(m_impl->outputBuffer.cbegin(), available);
        m_impl->outputBuffer.drain();
    }
}