#include "../src/pc.hpp"


int main(int argc, char* argv[])
{
    PC pc;

    pc.on_command([&pc](SteeringCommand command)
    {
        pc.steering({0.1f, 1.2f, true, false});
    });

    while(true)
    {
        pc.update();
    }

    return 0;
}