#include "hls.hpp"

#include <thread>

extern bool is_app_running;

Hls::Hls(const std::string &url)
{
    url_ = url;

    auto start_time = std::chrono::system_clock::now();
    time_t tt = std::chrono::system_clock::to_time_t(start_time);
    struct tm localTm;
#if defined (WIN32) || defined (_WINDLL)
    localtime_s(&localTm, &tt);
#else
    localtime_r(&tt, &localTm);
#endif

    char root_dir[256];
    sprintf(root_dir, "%04d%02d%02d-%02d%02d%02d",
        localTm.tm_year + 1900, localTm.tm_mon + 1, localTm.tm_mday,
        localTm.tm_hour, localTm.tm_min, localTm.tm_sec);
    root_dir_ = root_dir;
    curl_hdl_ = curl_easy_init();
    variant_ = std::make_shared<HlsVariant>(root_dir_);
}

Hls::~Hls()
{
    curl_easy_cleanup(curl_hdl_);
}

void Hls::StartDownload()
{
    std::chrono::time_point<std::chrono::steady_clock> nowTime = std::chrono::steady_clock::now();
    std::chrono::time_point<std::chrono::steady_clock> nextTime = nowTime;
    while (is_app_running)
    {
        nowTime = std::chrono::steady_clock::now();
        if (nextTime > nowTime)
        {
            std::this_thread::sleep_for(nextTime - nowTime);
        }
        else
        {
            nextTime = nowTime;
        }

        nextTime += std::chrono::milliseconds(variant_->Download(url_, curl_hdl_));
    }
}
