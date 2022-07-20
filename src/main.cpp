#include <signal.h>
#include <iostream>
#include <thread>

#include "hls.hpp"

bool is_app_running = true;
static void SigIntHandler(int sig_num)
{
    signal(SIGINT, SigIntHandler);
    is_app_running = false;
}

int main(int argc, char **argv)
{
    signal(SIGINT, SigIntHandler);

    HttpInit();

    std::string url;
    if (argc >= 2)
    {
        url = argv[1];
    }
    else
    {
        std::cout << "Input hls url:";
        std::cin >> url;
    }
    
    auto hls = std::make_shared<Hls>(url);
    hls->StartDownload();

    HttpDeinit();
    return 0;
}
