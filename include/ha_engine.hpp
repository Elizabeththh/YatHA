#ifndef HAENGINE_H
#define HAENGINE_H

#include "../third_party/cppjieba/Jieba.hpp"
#include "../include/stop_words_manager.hpp"
#include <iostream>
#include <fstream>
#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <iomanip>
#include <string>


using Tword = std::pair<int, std::string>;
class HaEngine
{
    private:
        std::queue<Tword> historyQueue;                            // 记录当前时间窗口的词语
        std::unordered_map<std::string, int> freqMap;              // 记录当前时间窗口词语的频次
        std::set<Tword> rankingSet;                                // 按词语出现频次升序排列集合

        std::vector<std::string> lines;
        std::vector<Tword> words;
        StopWordsManager swManager;
        
        
        const std::string inputFile{};
        const std::string outputFile{}; 
        std::ofstream out;                                          // 在构造函数中打开文件，而不是在 countTopKwords中打开，避免内容覆盖；
        int maxWindowSize{};                                      
        int currTime = -1;                                         // 输入文件时间戳从0秒开始，初始时间戳设为 -1
        int curWindowSize{};
        int topK{};
        
        public:
        cppjieba::Jieba jieba;
        
        HaEngine(const std::string& dictPath, const std::string& hmmPath, const std::string& userDictPath, 
                 const std::string& idfPath, const std::string& stopWordDictPath, int window, int k, 
                 const std::string &i, const std::string &o);
        void cutWordsTest();
        void cutWord();
        void writeOutput();
        bool readUtf8Lines(std::vector<std::string>& lines);
        void testOutput();
        void removeOutdatedWords();
        void countTopKWords(std::ofstream& out);

};

#endif 
