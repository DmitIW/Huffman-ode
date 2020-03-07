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
        cout << string_view{(char*)envelope.message.body.bytes, envelope.message.body.len} << endl;
        printf("\n");
        printf("Length: %lu\n", envelope.message.body.len);
        printf("\n\n\n");
    }
};

int main() {
    using Builder = AMQP::ConnectionBuilder;
    using Connector = AMQP::Connector;
    using Consumer = AMQP::ConsumeHandler;

    Connector connector = Builder().Build();
    Consumer consumer = connector.CreateConsumer(AMQP::ConsumeAdapter());
    consumer.ConsumeLoop(MessagePrint());
}