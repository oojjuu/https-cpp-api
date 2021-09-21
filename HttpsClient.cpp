#include "HttpsClient.h"

namespace agile {
namespace https {

HttpsClient::~HttpsClient()
{
    if (pcurl_)
    {
        curl_easy_cleanup(pcurl_);
    }
    if (header_)
    {
        curl_slist_free_all(header_);
    }
}

static size_t OnCurlString(char* buffer, size_t size, size_t nmemb, std::string* stream)
{
    size_t size_val = size * nmemb;
    stream->append(buffer, size_val);
    return size_val;
}

static size_t OnCurlData(char* buffer, size_t size, size_t nmemb, void* on_data_cb)
{
    size_t size_val = size * nmemb;
    HttpsClient::HttpsOnData* cb = (HttpsClient::HttpsOnData*)on_data_cb;
    (*cb)(buffer, size_val);
    return size_val;
}

static int OnCurlProgress(void* progress_cb, double dltotal, double dlnow, double ultotal, double ulnow)
{
    if (dltotal > 0 && dlnow > 0)
    {
        HttpsClient::HttpsProgress* cb = (HttpsClient::HttpsProgress*)progress_cb;
        (*cb)(dlnow, dltotal);
    }
    return 0;
}

std::shared_ptr<HttpsResult> HttpsClient::Get(const std::string& path, const std::vector<std::string>& headers,
                                              const std::string& body)
{
    return Get(path, headers, body, nullptr, nullptr);
}

std::shared_ptr<HttpsResult> HttpsClient::Get(const std::string& path, const std::vector<std::string>& headers,
                                              const std::string& body, const HttpsOnData& on_data,
                                              const HttpsProgress& progress)
{
    return DoReqWithException("GET", path, headers, body, on_data, progress);
}

std::shared_ptr<HttpsResult> HttpsClient::Post(const std::string& path, const std::vector<std::string>& headers,
                                               const std::string& body)
{
    return DoReqWithException("POST", path, headers, body, nullptr, nullptr);
}

std::shared_ptr<HttpsResult> HttpsClient::DoReqWithException(const std::string& req_type, const std::string& path,
                                                             const std::vector<std::string>& headers,
                                                             const std::string& body, const HttpsOnData& on_data,
                                                             const HttpsProgress& progress)
{
    try
    {
        return DoReq(req_type, path, headers, body, on_data, progress);
    }
    catch (std::exception& ex)
    {
        std::cout << "https fail to DoReq path:" << path << " ex:" << ex.what() << std::endl;
    }
    return nullptr;
}

std::shared_ptr<HttpsResult> HttpsClient::DoReq(const std::string& req_type, const std::string& path,
                                                const std::vector<std::string>& headers, const std::string& body,
                                                const HttpsOnData& on_data, const HttpsProgress& progress)
{
    if (get_flag_)
    {
        std::cout << "https get already" << std::endl;
        return nullptr;
    }

    get_flag_ = true;

    pcurl_ = curl_easy_init();
    if (!pcurl_)
    {
        std::cout << "https fail to curl_easy_init" << std::endl;
        return nullptr;
    }

    std::shared_ptr<HttpsResult> res = std::make_shared<HttpsResult>();

    if (!headers.empty())
    {
        for (auto& it : headers)
        {
            header_ = curl_slist_append(header_, it.c_str());
        }

        res->code = curl_easy_setopt(pcurl_, CURLOPT_HTTPHEADER, header_);
        if (res->code != CURLE_OK)
        {
            std::cout << "fail to CURLOPT_HTTPHEADER code:" << (int)res->code << std::endl;
            return res;
        }
    }

    res->code = curl_easy_setopt(pcurl_, CURLOPT_URL, path.c_str());
    if (res->code != CURLE_OK)
    {
        std::cout << "fail to CURLOPT_URL code:" << (int)res->code << ", path:" << path << std::endl;
        return res;
    }

    progress_ = progress;
    if (progress_)
    {
        curl_easy_setopt(pcurl_, CURLOPT_PROGRESSFUNCTION, OnCurlProgress);
        curl_easy_setopt(pcurl_, CURLOPT_PROGRESSDATA, &progress_);
        curl_easy_setopt(pcurl_, CURLOPT_NOPROGRESS, 0L);
    }

    on_data_ = on_data;
    if (on_data_)
    {
        curl_easy_setopt(pcurl_, CURLOPT_WRITEFUNCTION, OnCurlData);
        curl_easy_setopt(pcurl_, CURLOPT_WRITEDATA, &on_data_);
    }
    else
    {
        curl_easy_setopt(pcurl_, CURLOPT_WRITEFUNCTION, OnCurlString);
        curl_easy_setopt(pcurl_, CURLOPT_WRITEDATA, &res->body);
    }

    curl_easy_setopt(pcurl_, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(pcurl_, CURLOPT_SSL_VERIFYHOST, 0);

    if (!body.empty())
    {
        curl_easy_setopt(pcurl_, CURLOPT_POSTFIELDS, body.c_str());
    }

    curl_easy_setopt(pcurl_, CURLOPT_CUSTOMREQUEST, req_type.c_str());
    curl_easy_setopt(pcurl_, CURLOPT_CONNECTTIMEOUT, connect_timeout_);
    curl_easy_setopt(pcurl_, CURLOPT_TIMEOUT, recv_data_timeout_);
    res->code = curl_easy_perform(pcurl_);

    curl_easy_cleanup(pcurl_);
    pcurl_ = nullptr;
    if (header_)
    {
        curl_slist_free_all(header_);
        header_ = nullptr;
    }

    if (res->code != CURLE_OK)
    {
        std::cout << "https fail to curl_easy_perform code:" << (int)res->code << ", body:" << res->body
                             << ", path:" << path << std::endl;
    }
    return res;
}

}  // namespace https
}  // namespace agile
