/*

file: module.cpp
author: osklu414
created: 2019-11-14

Base class for modules (steering and sensor).

*/


#include "logging.hpp"
#include "module.hpp"


Module::Module(const std::string& file) : serial()
{
    serial.open(file);
}

Module::~Module()
{
    serial.close();
}


void
Module::set_pc(const std::shared_ptr<PC>& pc)
{
    this->pc = pc;
}