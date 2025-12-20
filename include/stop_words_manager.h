#ifndef STOPWORDSMANAGER_H
#define STOPWORDSMANAGER_H

#include "../third_party/cppjieba/Jieba.hpp"
#include <unordered_set>
#include <fstream>

class StopWordsManager
{
    private:
        std::unordered_set<std::string> stopWords;

    public:
        StopWordsManager(const std::string& stopWordsDict);
        bool isStopWord(std::string& word);
};

#endif 