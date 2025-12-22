#ifndef HAENGINE_H
#define HAENGINE_H

#include "../third_party/cppjieba/Jieba.hpp"
#include "../include/stop_words_manager.h"
#include "../include/time_window_manager.h"
#include "../include/word_ranker.h"
#include <iostream>
#include <fstream>
#include <iomanip>



using Tword = std::pair<int, std::string>;
class HaEngine
{
    protected:
        TimeWindowManager TWManager;                           // 时间窗口管理器
        WordRanker ranker;                                     // 词频排名管理器
        StopWordsManager SWManager;

        std::vector<std::string> lines;
        std::vector<Tword> words;                              // 用于测试的词汇列表
        
        const std::string inputFile;
        const std::string outputFile; 
        std::ofstream out;
        int topK;
        std::unordered_set<std::string> filter;        
        std::unordered_set<std::string> chooser;                   
        
    public:
        cppjieba::Jieba jieba;
        
        HaEngine(const std::string& dictPath, const std::string& hmmPath, const std::string& userDictPath, 
                 const std::string& idfPath, const std::string& stopWordDictPath, int window, int k, std::unordered_set<std::string>& ftr,
                 std::unordered_set<std::string>& chsr, const std::string &i, const std::string &o);
        // void cutWordsTest();
        virtual bool cutWord();
        virtual bool cutWordFilter();
        virtual bool cutWordChooser();
        bool readUtf8Lines(std::vector<std::string>& lines);
        // void testOutput();
        void removeOutdatedWords();
        virtual void countTopKWords(std::ofstream& out);

};

#endif 
