//
// Created by dmitri on 09.03.2020.
//

//
// Принимает два или один аргумент на вход. Вычисляет Код Хаффмана по всем файлам с расширением .txt в директории
// Сохраняет результаты в два файла: SymbolStats.json - кол-во вхождений символов в корпус, HuffmanCode.json - json с результирующим кодом.
// Аргументы: первый (обязательный) - путь к директории с корпусом текстов, второй (опционально) - путь к директории, куда будут сохранены результаты
//

#include "utility.hpp"
#include "QueueConnectionHandler.h"
#include "Reader.h"
#include "Worker.h"
#include "HashTable.h"

#include <iostream>
#include <algorithm>
#include <string>
#include <string_view>
#include <thread>
#include <future>
#include <fstream>

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace std;

const string QUEUE_KEY = "HuffmanCodeQueue";

class HuffmanCodeConstructor {
private:
    ConcurrentHashTable<char, int> symbol_stat;
public:

    HuffmanCodeConstructor() = default;
    void operator()(string_view msg) {
        for (char symbol : msg)
            symbol_stat[symbol].ref_to_value++;
    }
    static void TreeObserver(const unique_ptr<UTILITY::HuffmanNode<char>>& root, string current_code_state,
            rapidjson::Writer<rapidjson::StringBuffer>& w) {
        if (!root)
            return;

        if (root->symbol.has_value()) {
            const string key{root->symbol.value()};
            w.Key(key.c_str(), 1, true);
            w.String(current_code_state.c_str());
        }
        else {
            TreeObserver(root->left, current_code_state + "0", w);
            TreeObserver(root->right, current_code_state + "1", w);
        }
    }
    void Construct(const string& output_path) const {
        using namespace rapidjson;
        string output_path_file_stat = output_path + "SymbolStat.json";
        cout << "Preparing file " << output_path_file_stat << "..." << endl;

        auto symbols = symbol_stat.GetDumpVec();
        sort(symbols.begin(), symbols.end(), [](const pair<char, int>& lhs, const pair<char, int>& rhs){
           return lhs.second > rhs.second;
        });

        StringBuffer s_stat;
        Writer<StringBuffer> writer_stat(s_stat);
        writer_stat.StartObject();
        for (const auto& symbol: symbols) {
            const string key{symbol.first};
            writer_stat.Key(key.c_str(), 1, true);
            writer_stat.Int(symbol.second);
        }
        writer_stat.EndObject();

        ofstream os_stat(output_path_file_stat, ios::out);
        os_stat << s_stat.GetString() << endl;
        os_stat.close();

        string output_path_file_huffman_code = output_path + "HuffmanCode.json";
        cout << "Preparing file " << output_path_file_huffman_code << "..." << endl;

        auto root = UTILITY::HuffmanCodeConstruction<char>(symbols);

        StringBuffer s_huffman_code;
        Writer<StringBuffer> writer_huffman_code(s_huffman_code);
        writer_huffman_code.StartObject();
        TreeObserver(root, "", writer_huffman_code);
        writer_huffman_code.EndObject();

        ofstream os_huffman_code(output_path_file_huffman_code, ios::out);
        os_huffman_code << s_huffman_code.GetString() << endl;
        os_huffman_code.close();
    }
};

void Consumer(const string& path_to) {
    AMQP::Connector connector = AMQP::ConnectionBuilder()
            .SetBindingKey(QUEUE_KEY)
            .Build();
    AMQP::ConsumeHandler consumer = connector.CreateConsumer(AMQP::ConsumeAdapter());
    size_t workers_num = thread::hardware_concurrency();
    cout << "Consumer: workers number = " << workers_num << endl;
    WorkersPool workers{HuffmanCodeConstructor(),
                        workers_num == 0 ? 4 : workers_num};

    bool on_processing = true;
    while (on_processing)
        consumer.Consume([&](const amqp_rpc_reply_t&, const amqp_envelope_t& envelope){
            auto [prefix_opt, message] = UTILITY::detach_prefix({(char*)envelope.message.body.bytes,
                                                             envelope.message.body.len},
                                                                     UTILITY::PREFIX_DELIMITER);
            string prefix = prefix_opt.value_or("");
            cout << "Prefix: " << prefix << endl;
            cout << "Message : " << message << endl << endl;
            if (prefix == UTILITY::ON_PROCESSING_PREFIX)
                workers.Processing(move(message));
            else if (prefix == UTILITY::ON_END_PREFIX) {
                workers.Wait();
                on_processing = false;
            }
        });
    workers.InnerProcessor().Construct(path_to);
}

void Speaker(const string& path_to) {
    AMQP::Connector connector = AMQP::ConnectionBuilder()
            .SetBindingKey(QUEUE_KEY)
            .Build();
    AMQP::SpeakHandler speaker = connector.CreateSpeaker(AMQP::SpeakAdapter());
    cout << "Path to directory with .txt: " << path_to << endl;
    try {
        Reader r{path_to};
        for (auto it = r.begin(); it != r.end(); ++it)
            if (Reader::GetExtension(it) == ".txt") {
                cout << "File: " << *it << endl;
                speaker.Publish(UTILITY::attach_prefix(UTILITY::ON_PROCESSING_PREFIX,
                                                       UTILITY::PREFIX_DELIMITER,
                                                       Reader::ReadFromFile(*it)));
            }
        speaker.Publish(UTILITY::attach_prefix(UTILITY::ON_END_PREFIX, UTILITY::PREFIX_DELIMITER, "END"));
    } catch (const std::filesystem::filesystem_error& exc) {
        std::cout << "ERROR::" << exc.what() << std::endl;
    } catch (...) {
        throw;
    }
}

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        cout << "Input path to directory with .txt files.\n";
        return 0;
    }

    future<void> consume_future = async(Consumer, argc == 2 ? "" : argv[2]);
    this_thread::sleep_for(chrono::milliseconds (200));
    Speaker(argv[1]);
}