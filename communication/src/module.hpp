/*

file: module.hpp
author: osklu414
created: 2019-11-14

Base class for modules (steering and sensor).

*/


#ifndef MODULE_HPP
#define MODULE_HPP

#include <stdint.h>
#include <string>
#include <memory>

#include "serial.hpp"


class PC;


enum ModuleId : uint8_t
{
    SENSOR = 0,
    STEERING = 1
};


class Module
{
public:
    Module(const std::string& file);
    virtual ~Module();

    virtual void update() = 0;
    void set_pc(const std::shared_ptr<PC>& pc);

protected:
    Serial serial;
    std::shared_ptr<PC> pc;

private:

};

#endif // MODULE_HPP