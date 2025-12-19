#include "../include/stopWordsManager.hpp"

StopWordsManager::StopWordsManager(const std::string& stopWordsDict)
{
    std::ifstream ifs(stopWordsDict);
    if(!ifs.is_open())
    {
        std::cerr << "Error: Cannot open stop words file: " << stopWordsDict;
        exit;
    }
    std::string word;
    while(getline(ifs, word))
    {
        if(!word.empty() && word.back() == '\r')
            word.pop_back();
        if(!word.empty())
            stop_words.insert(word);
    }
    std::cout << "Loaded " << stop_words.size() << " stop words.\n";
}

bool StopWordsManager::is_stopWord(std::string& word)
{
    if(stop_words.count(word))
        return true;
    return false;
}