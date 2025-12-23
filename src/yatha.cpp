#include "../third_party/cppjieba/Jieba.hpp"
#include "../third_party/cppjieba/limonp/ArgvContext.hpp"
#include "../include/ha_engine.h"
#include "../include/web_server.h"
#include "../include/constants.h"
#include <iostream>
#include <sstream>

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
        WebServer server;
        server.setupRoutes();
        server.start();
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
