//
// Created by dmitri on 08.03.2020.
//

#ifndef HUFFMANCODE_READER_H
#define HUFFMANCODE_READER_H

#include <filesystem>
#include <string>

class Reader {
    class Iterator {
        using iterator = std::filesystem::directory_iterator;
        friend class Reader;
    private:
        iterator true_iterator;
    public:
        explicit Iterator(const iterator& t_i);

        bool operator!=(const Iterator& other) const;
        bool operator==(const Iterator& other) const;
        Iterator& operator++();
        std::string operator*() const;
    };
    using Path = std::filesystem::directory_iterator;
private:
    Path path_;
    Iterator begin_;
    Iterator end_;
public:
    explicit Reader(std::string path_to);
    Iterator begin();
    Iterator end();
    static std::string ReadFromFile(const std::string& path_to);
    static std::string GetExtension(const Iterator& it);
};

#endif //HUFFMANCODE_READER_H
