//
// Created by dmitri on 06.03.2020.
//

#include "QueueConnectionHandler.h"

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <iostream>

using namespace std;



AMQP::ConnectionHandler::ConnectionHandler(UTILITY::Address address, AMQP::Socket socket, amqp_connection_state_t conn,
                                           int socket_status, amqp_bytes_t queue_name, int channel_num, string exchange,
                                           string queue_key):
    conn_(conn),
    socket_(move(socket)),
    address_(move(address)),
    socket_status_(socket_status),
    channel_num_(channel_num),
    queue_name_(move(queue_name)),
    exchange_(move(exchange)),
    queue_key_(move(queue_key)),
    props_() {}

int AMQP::ConnectionHandler::GetSocketStatus() const { return socket_status_; }

void AMQP::ConnectionHandler::SetPublisherMode() {
    props_._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
    props_.content_type = amqp_cstring_bytes("text/plain");
    props_.delivery_mode = 2; /* persistent delivery mode */
}

void AMQP::ConnectionHandler::SetConsumerMode() {
    amqp_basic_consume(conn_, channel_num_,
            queue_name_, amqp_empty_bytes, 0, 1, 0,
                       amqp_empty_table);
    UTILITY::die_on_amqp_error(amqp_get_rpc_reply(conn_), "Consuming");
}



void AMQP::ConnectionHandler::Publish(const std::string &message) {
    UTILITY::die_on_error(amqp_basic_publish(conn_, channel_num_, amqp_cstring_bytes(exchange_.c_str()),
                                    amqp_cstring_bytes(queue_key_.c_str()), 0, 0,
                                    &props_, amqp_cstring_bytes(message.c_str())),
                 "Publishing");
}

AMQP::ConnectionHandler::~ConnectionHandler() {
    amqp_channel_close(conn_, channel_num_, AMQP_REPLY_SUCCESS);
    amqp_connection_close(conn_, AMQP_REPLY_SUCCESS);
    amqp_destroy_connection(conn_);
}

//
//
//

#define JOIN(A, B) A ## B
#define CONNECTION_BUILDER_SETTER_MOVE(FuncSuffix, TYPE, INTO_VALUE, LOCAL_VALUE) \
AMQP::ConnectionBuilder& AMQP::ConnectionBuilder::JOIN(Set, FuncSuffix)(TYPE INTO_VALUE) { LOCAL_VALUE = move(INTO_VALUE); return *this; }
#define CONNECTION_BUILDER_SETTER(FuncSuffix, TYPE, INTO_VALUE, LOCAL_VALUE) \
AMQP::ConnectionBuilder& AMQP::ConnectionBuilder::JOIN(Set, FuncSuffix)(TYPE INTO_VALUE) { LOCAL_VALUE = INTO_VALUE; return *this; }

CONNECTION_BUILDER_SETTER_MOVE(Hostname, std::string, host, address.hostname)
CONNECTION_BUILDER_SETTER(Port, std::size_t, port, address.port)
CONNECTION_BUILDER_SETTER_MOVE(Vhost, std::string, vHost, vhost)
CONNECTION_BUILDER_SETTER(ChannelMax, std::size_t, cmax, chanel_max)
CONNECTION_BUILDER_SETTER(FrameMax, std::size_t, fmax, frame_max)
CONNECTION_BUILDER_SETTER(Heartbeat, std::size_t, h, heartbeat)
CONNECTION_BUILDER_SETTER(SASLMethod, amqp_sasl_method_enum, method, sasl_method)
CONNECTION_BUILDER_SETTER_MOVE(Login, std::string, l, login)
CONNECTION_BUILDER_SETTER_MOVE(Password, std::string, p, password)
CONNECTION_BUILDER_SETTER(ChannelNum, std::size_t, n, channel_num)
CONNECTION_BUILDER_SETTER_MOVE(Exchange, std::string, exch, exchange)
CONNECTION_BUILDER_SETTER_MOVE(ExchangeType, std::string, exch_t, exchange_type)
CONNECTION_BUILDER_SETTER_MOVE(BindingKey, std::string, binding_key, bindingKey)

#define PASSIVE 0x08u
#define DURABLE 0x04u
#define AUTO_DELETE_EXCHANGE 0x02u
#define INTERNAL 0x01u
#define EXCLUSIVE 0x02u
#define AUTO_DELETE_QUEUE 0x01u

AMQP::ConnectionBuilder & AMQP::ConnectionBuilder::SetExchangeFlags(uint16_t p, uint16_t d, uint16_t ad, uint16_t i) {
    exchange_declare_flags = (p << 3u) + (d << 2u) + (ad << 1u) + (i);
    return *this;
}
AMQP::ConnectionBuilder & AMQP::ConnectionBuilder::SetQueueFlags(uint16_t p, uint16_t d, uint16_t e, uint16_t ad) {
    queue_declare_flags = (p << 3u) + (d << 2u) + (e << 1u) + (ad);
    return *this;
}
AMQP::ConnectionBuilder::ConnectionBuilder():
    conn(),
    socket(nullptr),
    address({"127.0.0.1", 5672}),
    socket_status(0),
    vhost("/"),
    chanel_max(0),
    frame_max(UTILITY::MAX_FRAME_SIZE),
    heartbeat(0),
    sasl_method(AMQP_SASL_METHOD_PLAIN),
    login(UTILITY::DEFAULT_LOGIN),
    password(UTILITY::DEFAULT_PASSWORD),
    channel_num(1),
    exchange(UTILITY::DEFAULT_EXCHANGE),
    exchange_type(UTILITY::DEFAULT_EXCHANGE_TYPE),
    exchange_declare_flags(0x04u),
    queue_declare_flags(0x04u),
    bindingKey(UTILITY::DEFAULT_BINDING_KEY),
    queue_name() {}

AMQP::ConnectionHandler AMQP::ConnectionBuilder::Build() {
    conn = amqp_new_connection();

    socket = amqp_tcp_socket_new(conn);
    if (!socket)
        UTILITY::die("creating TCP socket");
    cerr << "New socket created\n";

    socket_status = amqp_socket_open(socket, address.hostname.c_str(), address.port);
    if (socket_status)
        UTILITY::die("opening TCP socket");
    cerr << "Socket open\n";

    UTILITY::die_on_amqp_error(
            amqp_login(conn, vhost.c_str(), chanel_max, frame_max,
                    heartbeat, sasl_method,
                    login.c_str(), password.c_str()),
                      "Logging in");
    cerr << "Login completed\n";

    amqp_channel_open(conn, channel_num);
    UTILITY::die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");
    cerr << "Channel open\n";
    {
        amqp_queue_declare_ok_t *r = amqp_queue_declare(
                conn, channel_num,
                amqp_empty_bytes,
                queue_declare_flags & PASSIVE,
                queue_declare_flags & DURABLE,
                queue_declare_flags & EXCLUSIVE,
                queue_declare_flags & AUTO_DELETE_QUEUE,
                amqp_empty_table);
        UTILITY::die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring queue");
        queue_name = amqp_bytes_malloc_dup(r->queue);
        if (queue_name.bytes == nullptr) {
            fprintf(stderr, "Out of memory while copying queue name");
            exit(1);
        }
    }
    amqp_queue_bind(conn, channel_num, queue_name, amqp_cstring_bytes(exchange.c_str()),
                    amqp_cstring_bytes(bindingKey.c_str()), amqp_empty_table);
    UTILITY::die_on_amqp_error(amqp_get_rpc_reply(conn), "Binding queue");
    cerr << "Queue binding over\n";
    return AMQP::ConnectionHandler(
            address, move(socket), conn, socket_status, queue_name, channel_num, move(exchange), move(bindingKey)
            );
}