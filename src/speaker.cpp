//
// Created by dmitri on 07.03.2020.
//

#include "QueueConnectionHandler.h"
#include "utility.hpp"
#include "Reader.h"

#include <iostream>

void WalkyTalky(AMQP::SpeakHandler& speaker) {
    speaker.Publish(UTILITY::attach_prefix(UTILITY::ON_PROCESSING_PREFIX, UTILITY::PREFIX_DELIMITER,
                                           "First message: Hello, World!"));
    speaker.Publish(UTILITY::attach_prefix(UTILITY::ON_PROCESSING_PREFIX, UTILITY::PREFIX_DELIMITER,
                                           "Second message: I'm still alive;)"));
    speaker.Publish(UTILITY::attach_prefix(UTILITY::ON_PROCESSING_PREFIX, UTILITY::PREFIX_DELIMITER,
                                           "Third message: Tired with all these, for restful death I cry:\n"
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
                                           "Save that, to die, I leave my love alone."));
    speaker.Publish(UTILITY::attach_prefix(UTILITY::ON_PROCESSING_PREFIX, UTILITY::PREFIX_DELIMITER,
                                           "Fourth message: It was Shakespear sonnet"));
}

int main() {
    using Builder = AMQP::ConnectionBuilder;
    using Connector = AMQP::Connector;
    using Speaker = AMQP::SpeakHandler;

    Connector connector = Builder().Build();
    Speaker speaker = connector.CreateSpeaker(AMQP::SpeakAdapter());

    try {
        Reader r{"/home/dmitri/package_with_txt_files"};
        for (auto it = r.begin(); it != r.end(); ++it)
            if (Reader::GetExtension(it) == ".txt")
                speaker.Publish(UTILITY::attach_prefix(UTILITY::ON_PROCESSING_PREFIX,
                                                       UTILITY::PREFIX_DELIMITER,
                                                       Reader::ReadFromFile(*it)));
        speaker.Publish(UTILITY::attach_prefix(UTILITY::ON_END_PREFIX, UTILITY::PREFIX_DELIMITER, "END"));
    } catch (const std::filesystem::filesystem_error& exc) {
        std::cout << "ERROR::" << exc.what() << std::endl;
    } catch (...) {
        throw;
    }
}