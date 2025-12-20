#include "../third_party/cppjieba/Jieba.hpp"
#include "../include/ha_engine.hpp"
#include <iostream>


const std::string DICT_PATH             =     "data/dict/jieba.dict.utf8";
const std::string HMM_PATH              =     "data/dict/hmm_model.utf8";
const std::string USER_DICT_PATH        =     "data/dict/user.dict.utf8";
const std::string IDF_PATH              =     "data/dict/idf.utf8";
const std::string STOP_WORD_DICT_PATH   =     "data/dict/stop_words.utf8";



int main(int argc, char* argv[])
{
    std::string inputFile{};
    std::string outputFile{};
    
    // 默认参数
    int topK = 3;
    int windowSize = 600;
    if(argc == 3)
    {
        inputFile = argv[1];
        outputFile = argv[2];
    }
    else if(argc == 4)
    {
        inputFile = argv[1];
        outputFile = argv[2];
        windowSize = std::stoi(argv[3]);
    }
    else
    {
        std::cout << "请提供正确的输入输出文件\n";
        std::cout << "用法：./yatha <输入文件路径> <输出文件路径> （可选，默认时间窗口长度为600s）<时间窗口长度（单位：秒）>\n";
    }
    

    std::ofstream out(outputFile, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "[ERROR] cannot open output file: " << outputFile << std::endl;
        return EXIT_FAILURE;
    }

    HaEngine ha(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_DICT_PATH, windowSize, topK, inputFile, outputFile);
    ha.jieba.DeleteUserWord("技术创新");
    ha.cutWord();
    // ha.cutWordsTest();
    // ha.testOutput();
}