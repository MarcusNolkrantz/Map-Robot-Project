/*

file: serial.cpp
author: osklu414, juska933
created: 2019-11-14

Wrapper class for serial I/O.

*/

#ifndef SERIAL_HPP
#define SERIAL_HPP

#include <string>

#define BAUD B38400


class Serial
{
public:
    Serial();
    ~Serial();

    void open(const std::string& file);
    void close();

    int write(const uint8_t* bytes, unsigned int size);
    int read(uint8_t* bytes, unsigned int size);
    
    bool set_blocking(bool block);

    int get_fd();
    std::string get_file();

private:
    int fd;
    std::string file;
};

#endif // SERIAL_HPP
