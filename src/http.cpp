#include "http.hpp"
#include <chrono>
#include <fstream>
#include "los/logs.h"

static std::string progress_bar[101];
static void ProgressBarInit()
{
    for (int i = 0; i <= 100; ++i)
    {
        progress_bar[i] = "[";
        int j = 0;
        for (j = 0; j < i / 2; ++j)
        {
            progress_bar[i] += "=";
        }
        for (; j < 50; ++j)
        {
            progress_bar[i] += " ";
        }
        progress_bar[i] += "][";
        progress_bar[i] += std::to_string(i);
        progress_bar[i] += "%]";
    }
}

void HttpInit()
{
    ProgressBarInit();
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

void HttpDeinit()
{
    curl_global_cleanup();
}

static size_t HttpSyncDownloadCb(char *buffer, size_t size, size_t nitems, void *outstream)
{
    std::fstream *p_fout = static_cast<std::fstream *>(outstream);
    p_fout->write(buffer, size * nitems);
    return size * nitems;
}

static int HttpSyncDownloadProgressCb(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    int rate = (dltotal > 0) ? static_cast<int>(dlnow / dltotal * 100) : 0;
    if (rate < 0)
    {
        rate = 0;
    }
    if (rate > 100)
    {
        rate = 100;
    }

    std::chrono::system_clock::time_point *p_start_time = static_cast<std::chrono::system_clock::time_point *>(clientp);
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - *p_start_time);
    los::logs::Printf("%s %lldms\r", progress_bar[rate].c_str(), duration_ms.count());
    return CURLE_OK;
}

bool HttpSyncDownload(CURL *curl_hdl, const std::string &url, const std::string &save_path)
{
    curl_easy_reset(curl_hdl);

    std::fstream fout(save_path.c_str(), std::ios::out | std::ios::binary);
    if (!fout.is_open())
    {
        return false;
    }

    curl_easy_setopt(curl_hdl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl_hdl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_hdl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_hdl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl_hdl, CURLOPT_WRITEFUNCTION, HttpSyncDownloadCb);
    curl_easy_setopt(curl_hdl, CURLOPT_WRITEDATA, &fout);

    auto start_time = std::chrono::system_clock::now();
    time_t tt = std::chrono::system_clock::to_time_t(start_time);
    struct tm localTm;
#if defined (WIN32) || defined (_WINDLL)
    localtime_s(&localTm, &tt);
#else
    localtime_r(&tt, &localTm);
#endif

    LOS_DEF_LOG(los::logs::kInfo, "Downloading %s to %s", url.c_str(), save_path.c_str());

    curl_easy_setopt(curl_hdl, CURLOPT_PROGRESSFUNCTION, HttpSyncDownloadProgressCb);
    curl_easy_setopt(curl_hdl, CURLOPT_PROGRESSDATA, &start_time);
    curl_easy_setopt(curl_hdl, CURLOPT_NOPROGRESS, 0L);

    CURLcode ret_code = curl_easy_perform(curl_hdl);
    los::logs::Printf("\n");

    if (CURLE_OK != ret_code)
    {
        LOS_DEF_LOG(los::logs::kError, "Download %s fail! error=%s", url.c_str(), curl_easy_strerror(ret_code));
        return false;
    }
    LOS_DEF_LOG(los::logs::kInfo, "Downloading %s done!", url.c_str());

    return true;
}

std::string MakeAbsoluteUrl(const std::string &base, const std::string &rel)
{
    if (0 == rel.length())
    {
        return base;
    }

    size_t sep = base.find("://");
    if ((std::string::npos != sep) && ('/' == rel[0]))
    {
        // "http://abc/de/f" + "//gh" = "http://gh"
        if ('/' == rel[1])
        {
            return base.substr(0, sep + 1) + rel;
        }

        sep = base.find('/', sep + 3);

        // "http://abc/de/f" + "/gh" = "http://abc/gh"
        if (std::string::npos != sep)
        {
            return base.substr(0, sep) + rel;
        }

        // "http://abc" + "/gh" = "http://abc/gh"
        return base + rel;
    }

    if (('/' == rel[0]) || (std::string::npos != rel.find("://")))
    {
        return rel;
    }

    // "http://abc/de/f" + "gh" = "http://abc/de/gh"
    sep = base.rfind('/');
    std::string ret = (std::string::npos != sep) ? base.substr(0, sep + 1) : "";
    ret += rel;

    return ret;
}

std::string GetSavePath(const std::string &url)
{
    std::string ret;
    do
    {
        size_t http_sep = url.find("://");
        if (std::string::npos == http_sep)
        {
            ret += url;
            break;
        }

        size_t slash_sep = url.find('/', http_sep + 3);
        if (std::string::npos == slash_sep)
        {
            ret += url.substr(http_sep + 3);
            break;
        }

        ret += url.substr(slash_sep + 1);
    } while (0);

    size_t query_sep = ret.find('?');
    if (std::string::npos != query_sep)
    {
        ret.erase(query_sep);
    }
    return ret;
}
