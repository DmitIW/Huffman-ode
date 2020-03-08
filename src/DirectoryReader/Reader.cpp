//
// Created by dmitri on 08.03.2020.
//

#include "Reader.h"

#include <iostream>
#include <fstream>

Reader::Iterator::Iterator(const Reader::Iterator::iterator &t_i): true_iterator(t_i) {}
std::string Reader::Iterator::operator*() const { return true_iterator->path().string(); }
bool Reader::Iterator::operator!=(const Iterator &other) const { return true_iterator != other.true_iterator; }
bool Reader::Iterator::operator==(const Iterator &other) const { return true_iterator != other.true_iterator; }
Reader::Iterator& Reader::Iterator::operator++() { true_iterator++; return *this; }

Reader::Reader(std::string path_to):
    path_(move(path_to)),
    begin_(Iterator::iterator(path_)),
    end_(Iterator::iterator()) {}
Reader::Iterator Reader::begin() { return begin_; }
Reader::Iterator Reader::end() { return end_; }
std::string Reader::ReadFromFile(const std::string& path_to) {
    std::ifstream is(path_to);
    return {std::istreambuf_iterator<char>(is),
            std::istreambuf_iterator<char>()};
}
std::string Reader::GetExtension(const Iterator& it) {
    return it.true_iterator->path().extension();
}