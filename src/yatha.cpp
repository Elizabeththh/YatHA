#include "../third_party/cppjieba/Jieba.hpp"
#include "../third_party/cppjieba/limonp/ArgvContext.hpp"
#include "../include/ha_engine.h"
#include "../third_party/cpp_httplib/httplib.h"
#include <iostream>

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

        std::unordered_set<std::string> filter;
        std::unordered_set<std::string> chooser;
        {
            HaEngine ha(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_DICT_PATH, 600, 10, filter, chooser, tempInput, tempOutput);
            ha.cutWord();
        } 

        std::ifstream ifs(tempOutput);
        std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
        
        res.set_content(content, "text/plain"); 
    });

    std::cout << "服务已在 http://localhost:8080 启动" << std::endl;
    svr.listen("0.0.0.0", 8080);
}