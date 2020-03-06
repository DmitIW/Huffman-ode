//
// Created by dmitri on 07.03.2020.
//

#include "QueueConnectionHandler.h"
#include <iostream>

using namespace std;

int main() {
    using AMQP::ConnectionHandler;
    using AMQP::ConnectionBuilder;
    ConnectionHandler handler = ConnectionBuilder().Build();

    cout << handler.GetSocketStatus() << endl;

    return 0;
}