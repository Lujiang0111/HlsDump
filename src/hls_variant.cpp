#include "hls_variant.hpp"

#include <stdlib.h>
#include <fstream>
#include "los/files.h"

enum class LineTypes
{
    kNone = 0,
    kVariant,
    kSegment,
};

extern bool is_app_running;

HlsVariant::HlsVariant(const std::string &root_dir)
{
    root_dir_ = root_dir;
}

HlsVariant::~HlsVariant()
{

}

int HlsVariant::Download(const std::string &url, CURL *curl_hdl)
{
    std::string save_path = root_dir_ + "/" + GetSavePath(url);
    if (!los::files::CreateDir(save_path.c_str(), true))
    {
        return 5000;
    }

    HttpSyncDownload(curl_hdl, url, save_path);

    std::ifstream fin(save_path.c_str());
    if (!fin.is_open())
    {
        return 5000;
    }

    std::string variant_record_path = save_path + "-record";
    std::ofstream f_variant_record(variant_record_path.c_str(), std::ios::app);

    LineTypes line_type = LineTypes::kNone;
    size_t variant_cnt = 0;
    size_t segment_cnt = 0;
    int next_update_ms = 0;
    std::string last_extinf;
    std::string line;
    while ((is_app_running) && (std::getline(fin, line)))
    {
        // 去除空格
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
        if (0 == line.length())
        {
            continue;
        }

        if (0 == line.find("#EXT-X-STREAM-INF"))
        {
            line_type = LineTypes::kVariant;
        }
        else if (0 == line.find("#EXTINF"))
        {
            line_type = LineTypes::kSegment;
            last_extinf = line;
        }
        else if ('#' != line[0])
        {
            switch (line_type)
            {
            case LineTypes::kSegment:
            {
                ++segment_cnt;
                std::string segment_url = MakeAbsoluteUrl(url, line);
                std::string segment_save_path = root_dir_ + "/" + GetSavePath(segment_url);

                // 读第一个切片时，更新一下切片列表
                if (1 == segment_cnt)
                {
                    while ((!segments_.empty()) && (segments_.front() != segment_save_path))
                    {
                        segments_.pop_front();
                    }
                }

                bool is_new_segment = true;
                for (auto &&segment : segments_)
                {
                    if (segment == segment_save_path)
                    {
                        is_new_segment = false;
                        break;
                    }
                }

                if (is_new_segment)
                {
                    if (f_variant_record.is_open())
                    {
                        f_variant_record << last_extinf << std::endl << segment_save_path << std::endl;
                    }

                    last_extinf.erase(0, last_extinf.find_first_of("0123456789"));
                    last_extinf.erase(last_extinf.find_last_of("0123456789") + 1);
                    next_update_ms = static_cast<int>(atof(last_extinf.c_str()) * 1000);

                    segments_.emplace_back(segment_save_path);
                    los::files::CreateDir(segment_save_path.c_str(), true);
                    HttpSyncDownload(curl_hdl, segment_url, segment_save_path);
                }
                break;
            }

            case LineTypes::kVariant:
            {
                ++variant_cnt;
                if (variants_.size() < variant_cnt)
                {
                    auto variant = std::make_shared<HlsVariant>(root_dir_);
                    variants_.emplace_back(std::move(variant));
                }

                std::string variant_url = MakeAbsoluteUrl(url, line);
                auto variant = variants_[variant_cnt - 1].get();
                int ret_val = variant->Download(variant_url, curl_hdl);
                if ((0 == next_update_ms) || (ret_val < next_update_ms))
                {
                    next_update_ms = ret_val;
                }
                break;
            }

            default:
                continue;
            }

            line_type = LineTypes::kNone;
        }
    }

    return (next_update_ms > 0) ? next_update_ms : 1000;
}
