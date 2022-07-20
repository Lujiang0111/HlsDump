#ifndef HLS_DUMP_INCLUDE_HLS_HPP_
#define HLS_DUMP_INCLUDE_HLS_HPP_

#include <string>

#include "hls_variant.hpp"

class Hls
{
public:
    Hls() = delete;
    Hls(const Hls &) = delete;
    Hls &operator=(const Hls &) = delete;

    explicit Hls(const std::string &url);
    ~Hls();

    void StartDownload();

private:
    std::string url_;
    std::string root_dir_;
    CURL *curl_hdl_;
    std::shared_ptr<HlsVariant> variant_;
};

#endif // !HLS_DUMP_INCLUDE_HLS_HPP_
