#include "../include/ha_engine.hpp"
#include "../include/stop_words_manager.hpp"
// 初始化 jieba 和 swmanager
HaEngine::HaEngine(const std::string& dictPath, const std::string& hmmPath, const std::string& userDictPath, 
                   const std::string& idfPath, const std::string& stopWordDictPath, int window, int k, 
                   const std::string &i, const std::string &o) : 
jieba(dictPath, hmmPath, userDictPath, idfPath, stopWordDictPath),
swManager(stopWordDictPath),
maxWindowSize(window), topK(k), inputFile(i), outputFile(o) { out.open(outputFile, std::ios::binary); }

void HaEngine::cutWord()
{
    if (!readUtf8Lines(lines))
    {
        std::cerr << "[ERROR] cannot open input file: " << inputFile << std::endl;
        std::cerr << "[HINT ] create a UTF-8 file named '" << inputFile << "' with Chinese sentences." << std::endl;
        exit;
    }

    size_t line_size = lines.size();
    for (size_t idx{}; idx < line_size; ++idx)
    {
        const std::string &sentence = lines[idx];
        if (sentence[1] != 'A')
        {
            int timeSec = (sentence[1] - '0') * 3600 + std::stoi(sentence.substr(3, 2)) * 60 + std::stoi(sentence.substr(6, 2));

            std::vector<std::string> word;
            jieba.Cut(sentence.substr(10), word, true);

            if (curWindowSize < maxWindowSize && currTime != timeSec)
            {
                currTime = timeSec;
                ++curWindowSize;
            }
            else
            {
                if(currTime != timeSec)
                {
                    currTime = timeSec; // 保持当前时间戳与输入时间戳一致；
                    removeOutdatedWords();
                }
            }
            size_t word_size = word.size();
            for (size_t i{}; i < word_size; i++)
            {
                if (!swManager.isStopWord(word[i]))
                {
                    historyQueue.push(std::make_pair(timeSec, word[i]));
                    if (freqMap.count(word[i]))
                    {
                        int old_word_count = freqMap[word[i]];
                        rankingSet.erase({old_word_count, word[i]});
                        rankingSet.insert({old_word_count + 1, word[i]});
                        ++freqMap[word[i]];
                    }
                    else
                    {
                        freqMap[word[i]] = 1;
                        rankingSet.insert({1, word[i]});
                    }
                }
            }
        }

        else
        {
            topK = std::stoi(sentence.substr(17));
            countTopKWords(out);
        }
    }
}

void HaEngine::removeOutdatedWords()
{
    if (!historyQueue.empty())
    {
        int oldest_time = historyQueue.front().first;
        while (!historyQueue.empty() && historyQueue.front().first == oldest_time)
        {
            auto oldword = historyQueue.front();
            historyQueue.pop();

            std::string word = oldword.second;
            int old_freq = freqMap[word];

            rankingSet.erase({old_freq, word});
            if (freqMap[word] == 1)
                freqMap.erase(word);
            else
            {
                --freqMap[word];
                rankingSet.insert({old_freq - 1, word});
            }
        }
    }
}

void HaEngine::countTopKWords(std::ofstream &out)
{

    out << std::setfill('0') << '[' << std::setw(2) << currTime / 3600 << ':' << std::setw(2) << (currTime % 3600) / 60 << ':' << std::setw(2) << currTime % 60 << "]\n";
    auto it = rankingSet.rbegin();
    for (int i{1}; i <= topK && it != rankingSet.rend(); ++i, ++it)
    {
        std::string word = it->second;
        int freq = it->first;
        out << i << ". " << "“" << word << "”" << "出现" << freq << "次\n";
    }
    out << "\n";
}

void HaEngine::cutWordsTest()
{
    if (!readUtf8Lines(lines))
    {
        std::cerr << "[ERROR] cannot open input file: " << inputFile << std::endl;
        std::cerr << "[HINT ] create a UTF-8 file named '" << inputFile << "' with Chinese sentences." << std::endl;
        exit;
    }
    for (size_t idx = 0; idx < lines.size(); ++idx)
    {
        const std::string &sentence = lines[idx];
        if(sentence[1] != 'A')
        {
            int timeSec = (sentence[1] - '0') * 3600 + (std::stoi(sentence.substr(3, 2)) % 3600) * 60 + std::stoi(sentence.substr(6, 2)) % 3600;

            std::vector<std::string> word;
            jieba.Cut(sentence.substr(10), word, true);
            for (size_t i{}; i < word.size(); i++)
            {
                if (!swManager.isStopWord(word[i]))
                    words.emplace_back(std::make_pair(timeSec, word[i]));
            }
        }
    }
}

bool HaEngine::readUtf8Lines(std::vector<std::string> &lines)
{
    std::ifstream ifs(inputFile, std::ios::binary);
    if (!ifs.is_open())
        return false;
    std::string line;
    while (std::getline(ifs, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (!line.empty())
            lines.push_back(line);
    }
    return true;
}

void HaEngine::testOutput()
{
    for (size_t i{}; i < words.size(); i++)
        out << "Timestamp: " << words[i].first << "s: " << words[i].second << "\n";
}
