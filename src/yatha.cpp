#include "../third_party/cppjieba/Jieba.hpp"
#include "../third_party/cppjieba/limonp/ArgvContext.hpp"
#include "../include/ha_engine.h"
#include <iostream>


const std::string DICT_PATH                =     "dict/jieba.dict.utf8";
const std::string HMM_PATH                 =     "dict/hmm_model.utf8";
const std::string USER_DICT_PATH           =     "dict/user.dict.utf8";
const std::string IDF_PATH                 =     "dict/idf.utf8";
const std::string STOP_WORD_DICT_PATH      =     "dict/stop_words.utf8";
// const std::string SENSITIVE_WORD_PATH      =     "dict/sensitive_words.utf8";



int main(int argc, char* argv[])
{
    std::string inputFile{};
    std::string outputFile{};
    
    // 默认参数
    int topK = 3;
    int windowSize = 600;
    std::unordered_set<std::string> filter{};
    std::unordered_set<std::string> chooser{};
    
    limonp::ArgvContext arg(argc, argv);
    arg.ReadArgv(inputFile, outputFile, windowSize, topK, filter, chooser);

    std::ofstream out(outputFile, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "[ERROR] cannot open output file: " << outputFile << std::endl;
        return EXIT_FAILURE;
    }

    HaEngine ha(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_DICT_PATH, windowSize, topK, filter, chooser, inputFile, outputFile);
    if(filter.size() == 0 && chooser.size() == 0)
        ha.cutWord();
    else if(filter.size() != 0)
        ha.cutWordFilter();
    else if(chooser.size() != 0)
        ha.cutWordChooser();
    else
        ha.cutWord();
    // ha.cutWordsTest();
    // ha.testOutput();
}