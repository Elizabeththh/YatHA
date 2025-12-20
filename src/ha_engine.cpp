#include "../include/ha_engine.h"
#include "../include/stop_words_manager.h"
// 初始化 jieba 和 swmanager
HaEngine::HaEngine(const std::string& dictPath, const std::string& hmmPath, const std::string& userDictPath, 
                   const std::string& idfPath, const std::string& stopWordDictPath, int window, int k, std::unordered_set<std::string>& ftr,
                   std::unordered_set<std::string>& csr, const std::string &i, const std::string &o) : 
jieba(dictPath, hmmPath, userDictPath, idfPath, stopWordDictPath),
swManager(stopWordDictPath),
maxWindowSize(window), topK(k), filter(ftr), chooser(csr), inputFile(i), outputFile(o) { out.open(outputFile, std::ios::binary); }


// 将新到的句子分词，然后更新时间窗口、词频和词排行榜
void HaEngine::cutWord()
{
    if (!readUtf8Lines(lines))
    {
        std::cerr << "[错误] 无法打开输入文件: " << inputFile << std::endl;
        std::cerr << "[提示] 创建一个带有中文句子的输入文件 '" << inputFile << std::endl;
        exit(1);
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

// 分词的同时过滤掉不需要的词性
void HaEngine::cutWordFilter()
{
    if (!readUtf8Lines(lines))
    {
        std::cerr << "[错误] 无法打开输入文件: " << inputFile << std::endl;
        std::cerr << "[提示] 创建一个带有中文句子的输入文件 '" << inputFile << std::endl;
        exit(1);
    }

    size_t line_size = lines.size();
    for (size_t idx{}; idx < line_size; ++idx)
    {
        const std::string &sentence = lines[idx];
        if (sentence[1] != 'A')
        {
            int timeSec = (sentence[1] - '0') * 3600 + std::stoi(sentence.substr(3, 2)) * 60 + std::stoi(sentence.substr(6, 2));

            std::vector<std::pair<std::string, std::string>> wordWithCls;
            jieba.Tag(sentence.substr(10), wordWithCls);

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
            size_t word_size = wordWithCls.size();
            for (size_t i{}; i < word_size; i++)
            {
                if (!swManager.isStopWord(wordWithCls[i].first) && filter.find(wordWithCls[i].second) == filter.end())
                {
                    historyQueue.push(std::make_pair(timeSec, wordWithCls[i].first));
                    if (freqMap.count(wordWithCls[i].first))
                    {
                        int old_word_count = freqMap[wordWithCls[i].first];
                        rankingSet.erase({old_word_count, wordWithCls[i].first});
                        rankingSet.insert({old_word_count + 1, wordWithCls[i].first});
                        ++freqMap[wordWithCls[i].first];
                    }
                    else
                    {
                        freqMap[wordWithCls[i].first] = 1;
                        rankingSet.insert({1, wordWithCls[i].first});
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

// 选择用户所需的词
void HaEngine::cutWordChooser()
{
    if (!readUtf8Lines(lines))
    {
        std::cerr << "[错误] 无法打开输入文件: " << inputFile << std::endl;
        std::cerr << "[提示] 创建一个带有中文句子的输入文件 '" << inputFile << std::endl;
        exit(1);
    }

    size_t line_size = lines.size();
    for (size_t idx{}; idx < line_size; ++idx)
    {
        const std::string &sentence = lines[idx];
        if (sentence[1] != 'A')
        {
            int timeSec = (sentence[1] - '0') * 3600 + std::stoi(sentence.substr(3, 2)) * 60 + std::stoi(sentence.substr(6, 2));

            std::vector<std::pair<std::string, std::string>> wordWithCls;
            jieba.Tag(sentence.substr(10), wordWithCls);

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
            size_t word_size = wordWithCls.size();
            for (size_t i{}; i < word_size; i++)
            {
                if (!swManager.isStopWord(wordWithCls[i].first) && chooser.find(wordWithCls[i].second) != chooser.end())
                {
                    historyQueue.push(std::make_pair(timeSec, wordWithCls[i].first));
                    if (freqMap.count(wordWithCls[i].first))
                    {
                        int old_word_count = freqMap[wordWithCls[i].first];
                        rankingSet.erase({old_word_count, wordWithCls[i].first});
                        rankingSet.insert({old_word_count + 1, wordWithCls[i].first});
                        ++freqMap[wordWithCls[i].first];
                    }
                    else
                    {
                        freqMap[wordWithCls[i].first] = 1;
                        rankingSet.insert({1, wordWithCls[i].first});
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

// 删去已经离开时间窗口的词
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

// 计算当前时间窗口的 TopK 词汇
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

// 用来检测分词结果
void HaEngine::cutWordsTest()
{
    if (!readUtf8Lines(lines))
    {
        std::cerr << "[ERROR] cannot open input file: " << inputFile << std::endl;
        std::cerr << "[HINT ] create a UTF-8 file named '" << inputFile << "' with Chinese sentences." << std::endl;
        exit(1);
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


// 读取输入文件
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

// 输出测试分词结果
void HaEngine::testOutput()
{
    for (size_t i{}; i < words.size(); i++)
        out << "Timestamp: " << words[i].first << "s: " << words[i].second << "\n";
}
