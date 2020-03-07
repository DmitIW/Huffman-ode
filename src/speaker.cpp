//
// Created by dmitri on 07.03.2020.
//

#include "QueueConnectionHandler.h"

int main() {
    using AMQP::ConnectionHandler;
    using AMQP::ConnectionBuilder;
    ConnectionHandler handler = ConnectionBuilder().Build();

    handler.SetPublisherMode();
    handler.Publish("First message: Hello, World!");
    handler.Publish("Second message: I'm still alive;)");
    handler.Publish("Third message: Tired with all these, for restful death I cry:\n"
                    "As to behold desert a beggar born,\n"
                    "And needy nothing trimmed in jollity,\n"
                    "And purest faith unhappily forsworn,\n"
                    "And gilded honour shamefully misplaced,\n"
                    "And maiden virtue rudely strumpeted,\n"
                    "And right perfection wrongfully disgraced,\n"
                    "And strength by limping sway disabld,\n"
                    "And art made tongue-tied by authority,\n"
                    "And folly (doctor-like) controlling skill,\n"
                    "And simple truth miscalled simplicity,\n"
                    "And captive good attending captain10 ill:\n"
                    "Tired with all these, from these would I be gone,\n"
                    "Save that, to die, I leave my love alone.");
    handler.Publish("Fourth message: It was Shakespear sonnet");
}