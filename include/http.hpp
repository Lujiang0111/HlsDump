#ifndef HLS_DUMP_INCLUDE_HTTP_HPP_
#define HLS_DUMP_INCLUDE_HTTP_HPP_

#include <curl/curl.h>
#include <string>

void HttpInit();

void HttpDeinit();

bool HttpSyncDownload(CURL *curl_hdl, const std::string &url, const std::string &save_path);

std::string MakeAbsoluteUrl(const std::string &base, const std::string &rel);

std::string GetSavePath(const std::string &url);

#endif // !HLS_DUMP_INCLUDE_HTTP_HPP_
