/*

file: serial.cpp
author: osklu414, juska933
created: 2019-11-14

Wrapper class for serial I/O.

*/

#include <stdint.h>
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

#include <iostream>

#include "logging.hpp"
#include "serial.hpp"



Serial::Serial() : fd(-1), file() {}

Serial::~Serial() {}


void Serial::open(const std::string& file)
{
    //TRACE("serial open: ", file);
    fd = ::open(file.data(), O_RDWR | O_NOCTTY);
    if(fd < 0) {
	ERROR("serial open ", file);
	return;
    }
    
    //TRACE("setting options");
    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, BAUD);
	cfsetospeed(&options, BAUD);

	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~CSIZE; /* Mask the character size bits */
	options.c_cflag |= CS8; /* Select 8 data bits */
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /* Raw input */
	options.c_oflag &= ~OPOST; /* Raw output */

	tcsetattr(fd, TCSANOW, &options);

    if(!set_blocking(false))
    {
	return;
    }
    
    this->file = file;
}

void Serial::close()
{
    //TRACE("serial close");
    ::close(fd);
    this->file = "";
    this->fd = -1;
}


int Serial::write(const uint8_t* bytes, unsigned int size)
{
    //TRACE("serial write ", size, " bytes to ", get_file());
    int written = ::write(fd, bytes, size);
    if(written < size) WARN("wrote ", written, '/', size, " bytes");
    return written;
}

int Serial::read(uint8_t* bytes, unsigned int size)
{
    //TRACE("serial read ", size, " bytes from ", get_file());
    int read = ::read(fd, bytes, size);
    if(read < size); //WARN("read ", read, '/', size, " bytes");
    if(read < 0) return 0;
    return read;
}


bool Serial::set_blocking(bool block)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
	WARN("serial set_blocking: unable to get flags");
	return false;
    }

    if (block)
        flags &= ~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;
    
    if(fcntl(fd, F_SETFL, flags) == -1)
    {
	WARN("serial set_blocking: unable to set blocking");
	return false;
    }
    
    return true;
}


int Serial::get_fd()
{
    return fd;
}

std::string Serial::get_file()
{
    return file;
}
