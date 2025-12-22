#include "../third_party/cppjieba/Jieba.hpp"
#include "../third_party/cppjieba/limonp/ArgvContext.hpp"
#include "../include/ha_engine.h"
#include "../include/ha_engine_sse.h"
#include "../third_party/cpp_httplib/httplib.h"
#include "../third_party/json.hpp"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
using json = nlohmann::json;

const std::string DICT_PATH = "dict/jieba.dict.utf8";
const std::string HMM_PATH = "dict/hmm_model.utf8";
const std::string USER_DICT_PATH = "dict/user.dict.utf8";
const std::string IDF_PATH = "dict/idf.utf8";
const std::string STOP_WORD_DICT_PATH = "dict/stop_words.utf8";
// const std::string SENSITIVE_WORD_PATH      =     "dict/sensitive_words.utf8";

void runWebServer();

int main(int argc, char *argv[])
{
    std::string inputFile{};
    std::string outputFile{};

    // 默认参数
    int topK = 3;
    int windowSize = 600;
    std::unordered_set<std::string> filter{};
    std::unordered_set<std::string> chooser{};
    bool isServer = false;

    limonp::ArgvContext arg(argc, argv);
    arg.ReadArgv(inputFile, outputFile, windowSize, topK, filter, chooser, isServer);

    if (isServer)
    {
        runWebServer();
        return 0;
    }

    std::ofstream out(outputFile, std::ios::binary);
    if (!out.is_open())
    {
        std::cerr << "[错误]：无法读取输出文件 " << outputFile << std::endl;
        return EXIT_FAILURE;
    }

    HaEngine ha(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_DICT_PATH, windowSize, topK, filter, chooser, inputFile, outputFile);
    if (filter.size() == 0 && chooser.size() == 0)
        ha.cutWord();
    else if (filter.size() != 0)
        ha.cutWordFilter();
    else if (chooser.size() != 0)
        ha.cutWordChooser();
    else
        ha.cutWord();
    // ha.cutWordsTest();
    // ha.testOutput();
}

// 开启服务器
void runWebServer()
{
    httplib::Server svr;

    svr.set_mount_point("/", "../web");
    svr.set_mount_point("/img", "../img");

    svr.Post("/api/analyze", [](const httplib::Request &req, httplib::Response &res)
    {
        std::string inputContent = req.body;

        std::string tempInput = "temp_input.txt";
        std::string tempOutput = "temp_output.txt";

        {
            std::ofstream ofs(tempInput);
            ofs << inputContent;
        }

        {
            std::unordered_set<std::string> filter;
            std::unordered_set<std::string> chooser;
            HaEngine ha(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_DICT_PATH, 600, 10, filter, chooser, tempInput, tempOutput);
            ha.cutWord();
        } 

        std::ifstream ifs(tempOutput);
        std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
        
        res.set_content(content, "text/plain"); 
    });

    svr.Post("/api/analyze-filter", [](const httplib::Request &req, httplib::Response &res)
    {
        try {
            json j = json::parse(req.body);

            std::string inputContent;
            if(j.contains("content"))
                inputContent = j["content"];
            else
            {
                res.set_content("JSON数据未提供\"content\"字段", "text/plain");
                return;
            }
            std::unordered_set<std::string> filter;
            if(j.contains("pos"))
                filter = j["pos"].get<std::unordered_set<std::string>>();
            else
            {
                res.set_content("JSON数据未提供\"pos\"字段", "text/plain");
                return;
            }
            // 保存 content 到临时文件
            std::string tempInput = "temp_filter_input.txt";
            std::string tempOutput = "temp_filter_output.txt";
            {
                std::ofstream ofs(tempInput);
                ofs << inputContent;
            }
            
            // 创建 HaEngine 并执行过滤分析
            {
                std::unordered_set<std::string> chooser;
                HaEngine ha(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_DICT_PATH, 
                           600, 10, filter, chooser, tempInput, tempOutput);
                ha.cutWordFilter();
            }
            
            // 读取结果并返回
            std::ifstream ifs(tempOutput);
            std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
            res.set_content(content, "text/plain");
        }
        catch (const std::exception& e) 
        {
            res.set_content(std::string("服务器错误: ") + e.what(), "text/plain");
        }

    });

    svr.Post("/api/analyze-chooser", [](const httplib::Request &req, httplib::Response &res)
    {

        try {
            json j = json::parse(req.body);

            std::string inputContent;
            if(j.contains("content"))
                inputContent = j["content"];
            else
            {
                res.set_content("JSON数据未提供\"content\"字段", "text/plain");
                return;
            }
            std::unordered_set<std::string> chooser;
            if(j.contains("pos"))
                chooser = j["pos"].get<std::unordered_set<std::string>>();
            else
            {
                res.set_content("JSON数据未提供\"pos\"字段", "text/plain");
                return;
            }
            // 保存 content 到临时文件
            std::string tempInput = "temp_chooser_input.txt";
            std::string tempOutput = "temp_chooser_output.txt";
            {
                std::ofstream ofs(tempInput);
                ofs << inputContent;
            }
            
            // 创建 HaEngine 并执行放行分析
            {
                std::unordered_set<std::string> filter;
                HaEngine ha(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_DICT_PATH, 
                           600, 10, filter, chooser, tempInput, tempOutput);
                ha.cutWordChooser();
            }
            
            // 读取结果并返回
            std::ifstream ifs(tempOutput);
            std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
            res.set_content(content, "text/plain");
        }
        catch (const std::exception& e) 
        {
            res.set_content(std::string("服务器错误: ") + e.what(), "text/plain");
        }
    });

    svr.Post("/api/stream-analyze", [](const httplib::Request& req, httplib::Response& res)
    {
        std::cout << "[INFO] 收到滚动分析请求" << std::endl;
        
        try
        {
            // 解析JSON请求
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
            
            // 保存临时文件（使用唯一文件名避免并发冲突）
            auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
            std::string tempInput = "stream_temp_input_" + std::to_string(timestamp) + ".txt";
            std::string tempOutput = "stream_temp_output_" + std::to_string(timestamp) + ".txt";
            
            {
                std::ofstream ofs(tempInput);
                ofs << content;
            }
            std::cout << "[文件] 已保存到: " << tempInput << std::endl;

            // 设置SSE响应头
            res.set_header("Content-Type", "text/event-stream");
            res.set_header("Cache-Control", "no-cache");
            res.set_header("Connection", "keep-alive");
            res.set_header("Access-Control-Allow-Origin", "*");
            
            // SSE 推送
            res.set_content_provider(
                "text/event-stream",
                [window, topk, filter, chooser, tempInput, tempOutput, speed, mode]
                (size_t offset, httplib::DataSink &sink) mutable {
                    
                    std::cout << "[SSE] 开始流式推送" << std::endl;
                    
                    // 创建HaEngineSSE对象，step从前端传入
                    int step = static_cast<int>(speed);  
                    HaEngineSSE engine(
                        DICT_PATH, HMM_PATH, USER_DICT_PATH, 
                        IDF_PATH, STOP_WORD_DICT_PATH,
                        window, topk, 
                        const_cast<std::unordered_set<std::string>&>(filter),
                        const_cast<std::unordered_set<std::string>&>(chooser),
                        tempInput, tempOutput, step
                    );
                    
                    std::cout << "[SSE] HaEngineSSE已创建，step=" << step << std::endl;
                    
                    // 每秒推送一次 
                    bool hasMoreData = true;
                    int pushCount = 0;
                    
                    while (hasMoreData) {
                        // 检查连接是否已断开
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
                        
                        // 如果没有更多数据，退出循环
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
                        
                        // 读取整个JSON内容（多行）
                        std::stringstream buffer;
                        buffer << resultFile.rdbuf();
                        std::string jsonContent = buffer.str();
                        resultFile.close();
                        
                        std::cout << "[调试] 读取的JSON长度: " << jsonContent.length() << std::endl;
                        
                        // 如果JSON为空，跳过这次推送
                        if (jsonContent.empty() || jsonContent == "{}") 
                        {
                            std::cout << "[警告] JSON为空，跳过这次推送" << std::endl;
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                            continue;
                        }
                        
                        // 解析JSON以提取词频数据
                        try 
                        {
                            json wordsJson = json::parse(jsonContent);
                            
                            // 获取当前时间戳（用pushCount估算）
                            int currentTimestamp = pushCount * step;
                            
                            // 构造SSE消息
                            json output;
                            output["timestamp"] = currentTimestamp;
                            output["words"] = wordsJson;
                            
                            std::string sseMessage = "data: " + output.dump() + "\n\n";
                            
                            // 推送数据，检查写入是否成功
                            if (!sink.write(sseMessage.c_str(), sseMessage.size())) {
                                std::cout << "[SSE] 写入失败，客户端可能已断开" << std::endl;
                                break;
                            }
                            
                            pushCount++;
                            std::cout << "[SSE] 第" << pushCount << "次推送: timestamp=" 
                                      << currentTimestamp << std::endl;
                            
                        } 
                        catch (const std::exception& e) 
                        {
                            std::cerr << "[错误] JSON解析失败: " << e.what() << std::endl;
                        }
                        
                        // 每秒更新一次图表 
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
                    
                    // 关闭连接
                    sink.done();
                    return true;
                }
            );
            
        }
        catch(const std::exception& e)
        {
            std::cerr << "[错误] " << e.what() << std::endl;
            res.status = 400;
            res.set_content(std::string("错误: ") + e.what(), "text/plain");
        }
    });    

    std::cout << "服务已在 http://localhost:8080 启动" << std::endl;
    svr.listen("0.0.0.0", 8080);
}