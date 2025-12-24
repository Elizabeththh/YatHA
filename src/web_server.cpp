#include "../include/web_server.h"
#include "../include/constants.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

// 构造函数
WebServer::WebServer() {}

// 生成唯一的临时文件名
std::string WebServer::generateTempFileName(const std::string &prefix)
{
    auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    return prefix + "_" + std::to_string(timestamp) + ".txt";
}

// 保存临时文件
void WebServer::saveTempFile(const std::string &filename, const std::string &content)
{
    std::ofstream ofs(filename);
    if (!ofs.is_open())
        throw std::runtime_error("无法创建临时文件: " + filename);
    ofs << content;
}

// 读取临时文件
std::string WebServer::readTempFile(const std::string &filename)
{
    std::ifstream ifs(filename);
    if (!ifs.is_open())
        throw std::runtime_error("无法打开文件: " + filename);
    return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

// 清理临时文件
void WebServer::cleanupTempFile(const std::string &filename)
{
    std::remove(filename.c_str());
}

// 处理直接分析请求
void WebServer::handleAnalyze(const httplib::Request &req, httplib::Response &res)
{
    try
    {
        std::string inputContent = req.body;
        std::string tempInput = "temp/temp_input.txt";
        std::string tempOutput = "temp/temp_output.txt";

        saveTempFile(tempInput, inputContent);

        {
            std::unordered_set<std::string> filter;
            std::unordered_set<std::string> chooser;
            HaEngine ha(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH,
                        STOP_WORD_DICT_PATH, 600, 10, filter, chooser,
                        tempInput, tempOutput);
            ha.cutWord();
        }

        std::string content = readTempFile(tempOutput);
        res.set_content(content, "text/plain");
    }
    catch (const std::exception &e)
    {
        res.status = 500;
        res.set_content(std::string("服务器错误: ") + e.what(), "text/plain");
    }
}

// 处理过滤分析请求
void WebServer::handleAnalyzeFilter(const httplib::Request &req, httplib::Response &res)
{
    try
    {
        json j = json::parse(req.body);

        if (!j.contains("content"))
        {
            res.status = 400;
            res.set_content("JSON数据未提供\"content\"字段", "text/plain");
            return;
        }

        if (!j.contains("pos"))
        {
            res.status = 400;
            res.set_content("JSON数据未提供\"pos\"字段", "text/plain");
            return;
        }

        std::string inputContent = j["content"];
        std::unordered_set<std::string> filter = j["pos"].get<std::unordered_set<std::string>>();

        std::string tempInput = "temp/temp_filter_input.txt";
        std::string tempOutput = "temp/temp_filter_output.txt";

        saveTempFile(tempInput, inputContent);

        {
            std::unordered_set<std::string> chooser;
            HaEngine ha(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH,
                        STOP_WORD_DICT_PATH, 600, 10, filter, chooser,
                        tempInput, tempOutput);
            ha.cutWordFilter();
        }

        std::string content = readTempFile(tempOutput);
        res.set_content(content, "text/plain");
    }
    catch (const json::parse_error &e)
    {
        res.status = 400;
        res.set_content(std::string("JSON解析错误: ") + e.what(), "text/plain");
    }
    catch (const std::exception &e)
    {
        res.status = 500;
        res.set_content(std::string("服务器错误: ") + e.what(), "text/plain");
    }
}

// 处理放行分析请求
void WebServer::handleAnalyzeChooser(const httplib::Request &req, httplib::Response &res)
{
    try
    {
        json j = json::parse(req.body);

        if (!j.contains("content"))
        {
            res.status = 400;
            res.set_content("JSON数据未提供\"content\"字段", "text/plain");
            return;
        }

        if (!j.contains("pos"))
        {
            res.status = 400;
            res.set_content("JSON数据未提供\"pos\"字段", "text/plain");
            return;
        }

        std::string inputContent = j["content"];
        std::unordered_set<std::string> chooser = j["pos"].get<std::unordered_set<std::string>>();

        std::string tempInput = "temp/temp_chooser_input.txt";
        std::string tempOutput = "temp/temp_chooser_output.txt";

        saveTempFile(tempInput, inputContent);

        {
            std::unordered_set<std::string> filter;
            HaEngine ha(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH,
                        STOP_WORD_DICT_PATH, 600, 10, filter, chooser,
                        tempInput, tempOutput);
            ha.cutWordChooser();
        }

        std::string content = readTempFile(tempOutput);
        res.set_content(content, "text/plain");
    }
    catch (const json::parse_error &e)
    {
        res.status = 400;
        res.set_content(std::string("JSON解析错误: ") + e.what(), "text/plain");
    }
    catch (const std::exception &e)
    {
        res.status = 500;
        res.set_content(std::string("服务器错误: ") + e.what(), "text/plain");
    }
}

// 处理流式分析请求
void WebServer::handleStreamAnalyze(const httplib::Request &req, httplib::Response &res)
{
    std::cout << "[INFO] 收到滚动分析请求" << std::endl;

    try
    {
        // 解析 JSON字符串
        json j = json::parse(req.body);

        std::string content = j["content"];
        int window = j["window"];
        int topk = j["topk"];
        double speed = j["speed"];
        std::string mode = j["mode"];

        std::cout << "[配置] window=" << window
                  << ", topk=" << topk
                  << ", speed=" << speed
                  << ", mode=" << mode << std::endl;

        // 构建词性过滤集合
        std::unordered_set<std::string> filter, chooser;
        if (mode == "filter")
        {
            filter = j["pos"].get<std::unordered_set<std::string>>();
            std::cout << "[过滤] 过滤模式，共" << filter.size() << "个词性" << std::endl;
        }
        else if (mode == "allow")
        {
            chooser = j["pos"].get<std::unordered_set<std::string>>();
            std::cout << "[过滤] 放行模式，共" << chooser.size() << "个词性" << std::endl;
        }

        // 生成唯一的临时文件名
        std::string tempInput = generateTempFileName("temp/sse/stream_temp_input");
        std::string tempOutput = generateTempFileName("temp/sse/stream_temp_output");

        saveTempFile(tempInput, content);
        std::cout << "[文件] 已保存到: " << tempInput << std::endl;

        // 设置SSE响应头
        res.set_header("Content-Type", "text/event-stream");
        res.set_header("Cache-Control", "no-cache");
        res.set_header("Connection", "keep-alive");
        res.set_header("Access-Control-Allow-Origin", "*");

        // SSE 推送
        res.set_content_provider(
            "text/event-stream",
            [window, topk, filter, chooser, tempInput, tempOutput, speed, mode](size_t offset, httplib::DataSink &sink) mutable
            {
                std::cout << "[SSE] 开始流式推送" << std::endl;

                int step = static_cast<int>(speed);
                HaEngineSSE engine(
                    DICT_PATH, HMM_PATH, USER_DICT_PATH,
                    IDF_PATH, STOP_WORD_DICT_PATH,
                    window, topk,
                    const_cast<std::unordered_set<std::string> &>(filter),
                    const_cast<std::unordered_set<std::string> &>(chooser),
                    tempInput, tempOutput, step);

                std::cout << "[SSE] HaEngineSSE已创建，step=" << step << std::endl;

                bool hasMoreData = true;
                int pushCount = 0;

                while (hasMoreData)
                {
                    if (!sink.is_writable())
                    {
                        std::cout << "[SSE] 客户端已断开连接，停止推送" << std::endl;
                        break;
                    }

                    // 根据模式调用不同的处理函数
                    if (mode == "none")
                        hasMoreData = engine.cutWord();
                    else if (mode == "filter")
                        hasMoreData = engine.cutWordFilter();
                    else if (mode == "allow")
                        hasMoreData = engine.cutWordChooser();
                    else
                        hasMoreData = engine.cutWord();

                    if (!hasMoreData)
                    {
                        std::cout << "[SSE] 数据处理完毕" << std::endl;
                        break;
                    }

                    // 读取临时输出文件的JSON结果
                    std::ifstream resultFile(tempOutput);
                    if (!resultFile.is_open())
                    {
                        std::cerr << "[错误] 无法打开输出文件" << std::endl;
                        break;
                    }

                    std::stringstream buffer;
                    buffer << resultFile.rdbuf();
                    std::string jsonContent = buffer.str();
                    resultFile.close();

                    std::cout << "[调试] 读取的JSON长度: " << jsonContent.length() << std::endl;

                    if (jsonContent.empty() || jsonContent == "{}")
                    {
                        std::cout << "[警告] JSON为空，跳过这次推送" << std::endl;
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        continue;
                    }

                    try
                    {
                        json wordsJson = json::parse(jsonContent);
                        int currentTimestamp = pushCount * step;

                        json output;
                        output["timestamp"] = currentTimestamp;
                        output["words"] = wordsJson;

                        std::string sseMessage = "data: " + output.dump() + "\n\n";

                        if (!sink.write(sseMessage.c_str(), sseMessage.size()))
                        {
                            std::cout << "[SSE] 写入失败，客户端可能已断开" << std::endl;
                            break;
                        }

                        pushCount++;
                        std::cout << "[SSE] 第" << pushCount << "次推送: timestamp="
                                  << currentTimestamp << std::endl;
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << "[错误] JSON解析失败: " << e.what() << std::endl;
                    }

                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }

                // 发送结束标记
                std::string endMessage = "data: {\"done\":true}\n\n";
                sink.write(endMessage.c_str(), endMessage.size());
                std::cout << "[SSE] 推送完成，共" << pushCount << "次" << std::endl;

                // 清理临时文件
                std::remove(tempInput.c_str());
                std::remove(tempOutput.c_str());
                std::cout << "[清理] 已删除临时文件" << std::endl;

                sink.done();
                return true;
            });
    }
    catch (const json::parse_error &e)
    {
        std::cerr << "[错误] JSON解析失败: " << e.what() << std::endl;
        res.status = 400;
        res.set_content(std::string("JSON解析错误: ") + e.what(), "text/plain");
    }
    catch (const std::exception &e)
    {
        std::cerr << "[错误] " << e.what() << std::endl;
        res.status = 500;
        res.set_content(std::string("错误: ") + e.what(), "text/plain");
    }
}

// 设置所有路由
void WebServer::setupRoutes()
{
    // 设置静态文件目录
    server.set_mount_point("/", WEB_PATH);
    server.set_mount_point("/img", IMG_PATH);

    // 暴露 API 接口
    server.Post("/api/analyze", [this](const httplib::Request &req, httplib::Response &res)
                 { handleAnalyze(req, res); });

    server.Post("/api/analyze-filter", [this](const httplib::Request &req, httplib::Response &res)
                 { handleAnalyzeFilter(req, res); });

    server.Post("/api/analyze-chooser", [this](const httplib::Request &req, httplib::Response &res)
                 { handleAnalyzeChooser(req, res); });

    server.Post("/api/stream-analyze", [this](const httplib::Request &req, httplib::Response &res)
                 { handleStreamAnalyze(req, res); });
}

// 启动服务器
void WebServer::start(const std::string &host, int port)
{
    int actualPort = port;
    int maxRetries = 10; // 尝试 10 个端口 (8080-8089)       
    bool started = false;

    server.set_read_timeout(5, 0);
    server.set_write_timeout(5, 0);

    for (int i = 0; i < maxRetries; i++)
    {
        actualPort = port + i;
        std::cout << "正在尝试启动服务器: http://" << host << ":" << actualPort << "..." << std::endl;
        
        
        std::atomic<bool> bindSuccess(false);
        std::thread listenThread([&]() {
            bindSuccess = server.listen(host.c_str(), actualPort);
        });

        // 等待一小段时间看服务器是否成功启动
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        if (server.is_running()) {
            std::cout << "\n服务器启动成功！" << std::endl;
            std::cout << "========================================" << std::endl;
            std::cout << "  访问地址: http://localhost:" << actualPort << std::endl;
            std::cout << "========================================" << std::endl;
            started = true;
            listenThread.join();
            return;
        }
        else
        {
            std::cout << "端口 " << actualPort << " 已被占用或无法绑定，尝试下一个端口..." << std::endl;
            server.stop();
            if (listenThread.joinable()) {
                listenThread.join();
            }
        }
    }

    if (!started)
    {
        std::cerr << "\n错误: 尝试端口 " << port << "-" << (port + maxRetries - 1) << " 后仍无法找到可用端口" << std::endl;
        std::cerr << "请检查是否有其他服务占用了这些端口。" << std::endl;
    }
}
