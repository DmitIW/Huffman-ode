//
// Created by dmitri on 08.03.2020.
//

#ifndef HUFFMANCODE_UTILITY_HPP
#define HUFFMANCODE_UTILITY_HPP

#include <string>
#include <string_view>
#include <optional>

#include <amqp.h>
#include <amqp_tcp_socket.h>

namespace UTILITY {
        struct Address {
            std::string hostname;
            std::size_t port;
        };
}

namespace UTILITY {

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

        std::string attach_prefix(std::string prefix, std::string prefix_delimiter, std::string str);
        std::pair<std::optional<std::string>, std::string>
        detach_prefix(std::string_view str, std::string_view prefix_delimiter);

        const std::string ON_PROCESSING_PREFIX = "on_processing";
        const std::string ON_END_PREFIX = "on_end";
        const std::string PREFIX_DELIMITER = ":";
}


#endif //HUFFMANCODE_UTILITY_HPP
