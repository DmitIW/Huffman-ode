//
// Created by dmitri on 08.03.2020.
//

#ifndef HUFFMANCODE_UTILITY_HPP
#define HUFFMANCODE_UTILITY_HPP

#include <string>
#include <string_view>
#include <optional>
#include <memory>
#include <vector>
#include <set>
#include <tuple>

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

        template<class Symbol>
        struct HuffmanNode {
            HuffmanNode() = default;
            HuffmanNode(Symbol s, int c):
                symbol(s),
                count(c),
                left(nullptr),
                right(nullptr) {}
            HuffmanNode(int c, std::unique_ptr<HuffmanNode<Symbol>> l,
                        std::unique_ptr<HuffmanNode<Symbol>> r):
                        symbol(std::nullopt),
                        count(c),
                        left(move(l)), right(move(r)) {}

            std::optional<Symbol> symbol;
            std::size_t count;
            std::unique_ptr<HuffmanNode<Symbol>> left;
            std::unique_ptr<HuffmanNode<Symbol>> right;
            bool operator<(const HuffmanNode<Symbol>& other) const {
                return std::tie(count, symbol) < std::tie(other.count, other.symbol);
            }
        };
        template <class Symbol>
        struct UniquePtrCompare {
            using Obj = HuffmanNode<Symbol>;
            using is_transparent = void;
            bool operator()(const std::unique_ptr<Obj>& lhs, const std::unique_ptr<Obj>& rhs) const {
                return *lhs < *rhs;
            }
            bool operator()(const std::unique_ptr<Obj>& lhs, const Obj* rhs) const {
                return *lhs < *rhs;
            }
            bool operator()(const Obj* lhs, const std::unique_ptr<Obj>& rhs) const {
                return *lhs < *rhs;
            }
        };

        template<class Symbol>
        std::unique_ptr<HuffmanNode<Symbol>> HuffmanCodeConstruction(
                const std::vector<std::pair<Symbol, int>>& symbols)
        {
            std::set<std::unique_ptr<HuffmanNode<Symbol>>,
                    UniquePtrCompare<Symbol>> HuffmanCodeTree;

            for (const auto& symbol : symbols)
                HuffmanCodeTree.emplace(
                    std::make_unique<HuffmanNode<Symbol>>(symbol.first, symbol.second));

            while (HuffmanCodeTree.size() > 1) {
                auto it1 = HuffmanCodeTree.begin();
                auto it2 = std::next(it1);
                auto node1 = HuffmanCodeTree.extract(it1);
                auto node2 = HuffmanCodeTree.extract(it2);
                HuffmanCodeTree.emplace(
                    std::make_unique<HuffmanNode<Symbol>>(node1.value()->count + node2.value()->count,
                                                          move(node1.value()), move(node2.value()))
                );
            }

            if (HuffmanCodeTree.empty())
                return {nullptr};
            return std::move(HuffmanCodeTree.extract(HuffmanCodeTree.begin()).value());
        }
}


#endif //HUFFMANCODE_UTILITY_HPP
