#ifndef STOPWORDSMANAGER_H
#define STOPWORDSMANAGER_H

#include "../third_party/cppjieba/Jieba.hpp"
#include <unordered_set>
#include <fstream>

class StopWordsManager
{
    private:
        std::unordered_set<std::string> stop_words;

    public:
        StopWordsManager(const std::string& stop_wordsDict);
        bool is_stopWord(std::string& word);
};

#endif 