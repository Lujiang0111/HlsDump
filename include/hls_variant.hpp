#ifndef HLS_DUMP_INCLUDE_HLS_VARIANT_HPP_
#define HLS_DUMP_INCLUDE_HLS_VARIANT_HPP_

#include <vector>
#include <deque>
#include <memory>

#include "http.hpp"

class HlsVariant
{
public:
    HlsVariant() = delete;
    HlsVariant(const HlsVariant &) = delete;
    HlsVariant &operator=(const HlsVariant &) = delete;

    explicit HlsVariant(const std::string &root_dir);
    ~HlsVariant();

    // 下载Variant下的所有内容，返回下一次更新Variant的等待时间（单位毫秒）
    int Download(const std::string &url, CURL *curl_hdl);

private:
    std::string root_dir_;
    std::vector<std::shared_ptr<HlsVariant>> variants_;
    std::deque<std::string> segments_;
};

#endif // !HLS_DUMP_INCLUDE_HLS_VARIANT_HPP_
