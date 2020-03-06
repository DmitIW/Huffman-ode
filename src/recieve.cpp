#include <iostream>
#include "SimplePocoHandler.h"

int main()
{
    SimplePocoHandler handler("localhost", 5672);

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");

    AMQP::Channel channel(&connection);
    channel.declareQueue("hello");
    channel.consume("hello", AMQP::noack).onReceived(
            [](const AMQP::Message &message,
               uint64_t,
               bool)
            {

                std::cout <<" [x] Received "<< message.contentEncoding() << std::endl;
            });

    std::cout << " [*] Waiting for messages. To exit press CTRL-C\n";
    handler.loop();
    return 0;
}