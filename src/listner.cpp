//
// Created by dmitri on 07.03.2020.
//

#include "QueueConnectionHandler.h"
#include "Worker.h"

#include <iostream>
#include <chrono>
#include <thread>

using namespace std;

class PrintBody {
private:
  struct synchronize {
      synchronize(mutex& mutex, vector<string>& obj):
        locker(mutex),
        ref_to_object(obj) {}
      lock_guard<mutex> locker;
      vector<string>& ref_to_object;
  };
  vector<string> strs;
  mutex reader;
public:
    PrintBody(): strs(), reader() {
        strs.reserve(1000);
    }
    PrintBody(PrintBody&& other):
        strs(std::move(other.strs)),
        reader() {}
    PrintBody(const PrintBody& other):
        strs(other.strs),
        reader() {}
    void operator()(string msg) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        synchronize(reader, strs).ref_to_object.emplace_back(move(msg));
    }
    void Print() const {
        for (const auto& str : strs)
            cout << str << endl;
    }
};

int main() {
    using Builder = AMQP::ConnectionBuilder;
    using Connector = AMQP::Connector;
    using Consumer = AMQP::ConsumeHandler;
    using Callback = PrintBody;

    Connector connector = Builder().Build();
    Consumer consumer = connector.CreateConsumer(AMQP::ConsumeAdapter());
    WorkersPool workers{Callback()};

    bool on_processing = true;
    while (on_processing)
        consumer.Consume([&](const amqp_rpc_reply_t&, const amqp_envelope_t& envelope){
            auto [prefix, message] = UTILITY::detach_prefix({(char*)envelope.message.body.bytes,
                                                             envelope.message.body.len}
                                  , UTILITY::PREFIX_DELIMITER);
            cout << "Prefix: " << prefix.value_or("") << endl;
            cout << "Message: " << message << endl << endl;
            if (prefix.value_or("") == UTILITY::ON_PROCESSING_PREFIX) {
                workers.Processing(move(message));
            } else if (prefix.value_or("") == UTILITY::ON_END_PREFIX) {
                on_processing = false;
                workers.Wait();
            }
        });
    workers.InnerProcessor().Print();
}