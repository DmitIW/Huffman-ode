//
// Created by dmitri on 07.03.2020.
//

#include "QueueConnectionHandler.h"
#include <iostream>

using namespace std;

struct MessagePrint {
    void operator()(const amqp_rpc_reply_t& res, const amqp_envelope_t& envelope) {
        if (AMQP_RESPONSE_NORMAL != res.reply_type) {
            exit(1);
        }

        printf("Delivery %u, exchange %.*s routingkey %.*s\n",
               (unsigned)envelope.delivery_tag, (int)envelope.exchange.len,
               (char *)envelope.exchange.bytes, (int)envelope.routing_key.len,
               (char *)envelope.routing_key.bytes);

        if (envelope.message.properties._flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {
            printf("Content-type: %.*s\n",
                   (int)envelope.message.properties.content_type.len,
                   (char *)envelope.message.properties.content_type.bytes);
        }
        printf("----\n");

        UTILITY::amqp_dump(envelope.message.body.bytes, envelope.message.body.len);
    }
};

int main() {
    using AMQP::ConnectionHandler;
    using AMQP::ConnectionBuilder;
    ConnectionHandler handler = ConnectionBuilder().Build();

    cout << handler.GetSocketStatus() << endl;

    handler.SetConsumerMode();
    for (size_t i = 0; i < 10u; i++) {
        handler.Consume(MessagePrint());
    }

}