#pragma once

#include <memory>
#include <string>
#include <vector>

#include <curl/curl.h>

namespace agile {
namespace https {

/**
 *@brief Https请求结果
 */
struct HttpsResult
{
    // 错误码
    CURLcode code;
    // 内容
    std::string body;
};

/**
 *@brief https客户端
 */
class HttpsClient
{
   public:
    // Https请求进度
    using HttpsProgress = std::function<void(uint64_t len, uint64_t total)>;

    // Https请求数据回调
    using HttpsOnData = std::function<void(char* buffer, size_t size)>;

    HttpsClient() {}

    /**
     *@brief HttpsClient构造函数
     *@param host 域名或者ip:port
     *@param client_cert_path cert文件路径
     *@param client_key_path key文件路径
     */
    HttpsClient(const std::string& host, const std::string& client_cert_path, const std::string& client_key_path)
        : host_(host), client_cert_path_(client_cert_path), client_key_path_(client_key_path)
    {
    }

    virtual ~HttpsClient();

    /**
     *@brief Https Get 请求
     *@param path 请求路径
     *@param headers 包头数据
     *@param body 包体数据
     *@param on_data 数据回调函数
     *@return https 请求结果
     */
    std::shared_ptr<HttpsResult> Get(const std::string& path, const std::vector<std::string>& headers,
                                     const std::string& body);

    /**
     *@brief Https Get 请求
     *@param path 请求路径
     *@param headers 包头数据
     *@param body 包体数据
     *@param on_data 数据回调函数
     *@param progress 请求数据进度
     *@return https 请求结果
     */
    std::shared_ptr<HttpsResult> Get(const std::string& path, const std::vector<std::string>& headers,
                                     const std::string& body, const HttpsOnData& on_data,
                                     const HttpsProgress& progress);

    /**
     *@brief Https Post 请求
     *@param path 请求路径
     *@param headers 包头数据
     *@param body 包体数据
     *@param on_data 数据回调函数
     *@return https 请求结果
     */
    std::shared_ptr<HttpsResult> Post(const std::string& path, const std::vector<std::string>& headers,
                                      const std::string& body);

    /**
     *@brief 设置https timeout
     *@param connect_timeout 连接timeout
     *@param recv_data_timeout 接收数据超时
     */
    void SetTimeout(uint32_t connect_timeout, uint32_t recv_data_timeout)
    {
        if (connect_timeout > 0)
        {
            connect_timeout_ = connect_timeout;
        }
        if (recv_data_timeout > 0)
        {
            recv_data_timeout_ = recv_data_timeout;
        }
    }

   private:
    /**
     *@brief Https 请求
     *@param req_type 请求类型GET POST
     *@param path 请求路径
     *@param headers 包头数据
     *@param body 包体数据
     *@param on_data 数据回调函数
     *@param progress 请求数据进度
     *@return https 请求结果
     */
    std::shared_ptr<HttpsResult> DoReq(const std::string& req_type, const std::string& path,
                                       const std::vector<std::string>& headers, const std::string& body,
                                       const HttpsOnData& on_data, const HttpsProgress& progress);
    /**
     *@brief Https 请求(处理exception)
     *@param req_type 请求类型GET POST
     *@param path 请求路径
     *@param headers 包头数据
     *@param body 包体数据
     *@param on_data 数据回调函数
     *@param progress 请求数据进度
     *@return https 请求结果
     */
    std::shared_ptr<HttpsResult> DoReqWithException(const std::string& req_type, const std::string& path,
                                                    const std::vector<std::string>& headers, const std::string& body,
                                                    const HttpsOnData& on_data, const HttpsProgress& progress);

   private:
    // https 接收数据超时(单位秒)
    static constexpr uint32_t kHttpsRecvTimeoutSec = 30;
    // https 连接超时(单位秒)
    static constexpr uint32_t kHttpsConnectTimeoutSec = 10;

    // get标志
    bool get_flag_ = false;
    // curl
    CURL* pcurl_ = nullptr;
    // header
    curl_slist* header_ = nullptr;
    // 数据回调函数
    HttpsOnData on_data_ = nullptr;
    // 进度回调函数
    HttpsProgress progress_ = nullptr;
    // 域名或者ip:port
    std::string host_;
    // cert文件路径
    std::string client_cert_path_;
    // key文件路径
    std::string client_key_path_;
    // 接收数据timeout
    uint32_t recv_data_timeout_ = kHttpsRecvTimeoutSec;
    // 连接timeout
    uint32_t connect_timeout_ = kHttpsConnectTimeoutSec;
};

}  // namespace https
}  // namespace agile