#include "EmptySample.h"
#include "core/networking.h"

int main(int argc, char *argv[])
{
    auto theApp = std::make_shared<kinski::EmptySample>();
    LOG_INFO<<"local ip: " << kinski::net::local_ip();
    return theApp->run();
}
