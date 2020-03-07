//
// Created by dmitri on 06.03.2020.
//

#ifndef HUFFMANCODE_QUEUEConnector_H
#define HUFFMANCODE_QUEUEConnector_H

#include <amqp.h>
#include <amqp_tcp_socket.h>

#include <memory>
#include <string>
#include <cctype>

unsigned long long operator "" _passive(unsigned long long value);
unsigned long long operator "" _durable(unsigned long long value);
unsigned long long operator "" _autodelete(unsigned long long value);
unsigned long long operator "" _internal(unsigned long long value);
unsigned long long operator "" _exclusive(unsigned long long value);

namespace UTILITY {
    struct Address {
        std::string hostname;
        std::size_t port;
    };

    const std::size_t MAX_FRAME_SIZE = 131072;
    const std::string DEFAULT_EXCHANGE = "amq.direct";
    const std::string DEFAULT_EXCHANGE_TYPE = "direct";
    const std::string DEFAULT_LOGIN = "guest";
    const std::string DEFAULT_PASSWORD = "guest";
    const std::string DEFAULT_BINDING_KEY = "queue";

    void die(const char *fmt, ...);
    void die_on_amqp_error(amqp_rpc_reply_t x, char const *context);
    void die_on_error(int x, char const *context);

    void amqp_dump(void const *buffer, size_t len);
}

namespace AMQP {
    using Socket = amqp_socket_t*;

    class ConnectionBuilder;
    class ConsumeAdapter;
    class SpeakAdapter;

    class ConsumeHandler;
    class SpeakHandler;

    class Connector {
        friend class ConnectionBuilder;
        friend class ConsumeHandler;
        friend class SpeakHandler;
        friend class ConsumeAdapter;
        friend class SpeakAdapter;
    private:
        amqp_connection_state_t conn_;
        Socket socket_;
        UTILITY::Address address_;
        int socket_status_;
        int channel_num_;
        std::string exchange_;
        std::string queue_key_;

    public:
        int GetSocketStatus() const;

        ConsumeHandler CreateConsumer(ConsumeAdapter adapter);
        SpeakHandler CreateSpeaker(SpeakAdapter adapter);

        Connector(const Connector&) = delete;
        Connector& operator=(const Connector&) = delete;

        Connector(Connector&&) = default;
        Connector& operator=(Connector&&) = default;

        ~Connector();
    private:
        Connector(UTILITY::Address address, Socket socket, amqp_connection_state_t conn,
                int socket_status, int channel_num, std::string exchange, std::string queue_key);

    };

    class ConsumeHandler {
        friend class ConsumeAdapter;
    private:
        const Connector* connection_;
        amqp_bytes_t queue_name_;
        ConsumeHandler(const Connector* c, amqp_bytes_t queue_name);
    public:
        ConsumeHandler(const ConsumeHandler&) = delete;
        ConsumeHandler& operator=(const ConsumeHandler&) = delete;
        ~ConsumeHandler();
        template<class Callback>
        void Consume(Callback callback) {
            amqp_rpc_reply_t res;
            amqp_envelope_t envelope;
            amqp_maybe_release_buffers(connection_->conn_);
            res = amqp_consume_message(connection_->conn_, &envelope, NULL, 0);
            callback(res, envelope);
            amqp_destroy_envelope(&envelope);
        }
        template <class Callback>
        void ConsumeLoop(Callback callback) {
            while(true)
                Consume(callback);
        }
    };

    class SpeakHandler {
        friend class SpeakAdapter;
    private:
        const Connector* conn_;
        amqp_basic_properties_t props_;

        SpeakHandler(const Connector* c, amqp_basic_properties_t props);
    public:
        void Publish(const std::string& message);
    };

    class ConsumeAdapter {
    private:
        const Connector* conn_;
        uint16_t queue_declare_flags;
        amqp_bytes_t queue_name_;
    public:
        ConsumeAdapter();
        ConsumeAdapter& SetConnector(const Connector* c);
        ConsumeAdapter& SetQueueFlags(uint16_t p, uint16_t d, uint16_t e, uint16_t ad);
        ConsumeHandler Build();
    };

    class SpeakAdapter {
    private:
        const Connector* conn_;
        amqp_basic_properties_t props_;
    public:
        SpeakAdapter();
        SpeakAdapter& SetConnector(const Connector* c);
        SpeakHandler Build();
    };

    class ConnectionBuilder {
    private:
        amqp_connection_state_t conn;
        Socket socket;
        UTILITY::Address address;
        int socket_status;
        std::string vhost;
        size_t chanel_max;
        size_t frame_max;
        size_t heartbeat;
        amqp_sasl_method_enum sasl_method;
        std::string login;
        std::string password;
        int channel_num;
        std::string exchange;
        std::string exchange_type;
        uint16_t exchange_declare_flags;
        std::string bindingKey;
        amqp_bytes_t queue_name;
    public:
        ConnectionBuilder();
        ConnectionBuilder& SetHostname(std::string host);
        ConnectionBuilder& SetPort(std::size_t port);
        ConnectionBuilder& SetVhost(std::string vHost);
        ConnectionBuilder& SetChannelMax(std::size_t cmax);
        ConnectionBuilder& SetFrameMax(std::size_t fmax);
        ConnectionBuilder& SetHeartbeat(std::size_t h);
        ConnectionBuilder& SetSASLMethod(amqp_sasl_method_enum method);
        ConnectionBuilder& SetLogin(std::string l);
        ConnectionBuilder& SetPassword(std::string p);
        ConnectionBuilder& SetChannelNum(std::size_t n);
        ConnectionBuilder& SetExchange(std::string exch);
        ConnectionBuilder& SetExchangeType(std::string exch_t);
        ConnectionBuilder& SetExchangeFlags(uint16_t p, uint16_t d, uint16_t ad, uint16_t i);
        ConnectionBuilder& SetBindingKey(std::string binding_key);
        Connector Build();
    };
}



#endif //HUFFMANCODE_QUEUEConnector_H
