#include "../include/ha_engine_sse.h"
#include "../third_party/json.hpp"

using json = nlohmann::json;

HaEngineSSE::HaEngineSSE(
    const std::string &dictPath, const std::string &hmmPath, const std::string &userDictPath,
    const std::string &idfPath, const std::string &stopWordDictPath, int window, int k, std::unordered_set<std::string> &ftr,
    std::unordered_set<std::string> &csr, const std::string &i, const std::string &o, int step) : HaEngine(dictPath, hmmPath, userDictPath, idfPath, stopWordDictPath, window, k, ftr, csr, i, o), step(step)
{
    // 读取文件内容到lines成员变量
    if (!readUtf8Lines(lines))
    {
        std::cerr << "[错误] 无法读取输入文件: " << inputFile << std::endl;
        exit(1);
    }
    std::cout << "[HaEngineSSE] 已读取 " << lines.size() << " 行数据" << std::endl;
}

bool HaEngineSSE::cutWord()
{
    size_t line_size = lines.size();
    if (lineIndex >= line_size)
        return false;
    const std::string &sentence = lines[lineIndex];

    int initTimeSec = (sentence[1] - '0') * 3600 + std::stoi(sentence.substr(3, 2)) * 60 + std::stoi(sentence.substr(6, 2));

    int threshold = initTimeSec + step;

    while (TWManager.getCurrentTime() < threshold && lineIndex < line_size)
    {
        const std::string s = lines[lineIndex++];
        int timeSec = (s[1] - '0') * 3600 + std::stoi(s.substr(3, 2)) * 60 + std::stoi(s.substr(6, 2));

        std::vector<std::string> word;
        jieba.Cut(s.substr(10), word, true);

        if (TWManager.shouldRemoveOld(timeSec))
            removeOutdatedWords();

        size_t word_size = word.size();
        for (size_t i{}; i < word_size; i++)
        {
            if (!SWManager.isStopWord(word[i]))
            {
                // 添加到时间窗口
                TWManager.addWord(timeSec, word[i]);

                // 更新词频排名
                ranker.addWord(word[i]);
            }
        }
    }
    countTopKWords(out);
    return true;
}

bool HaEngineSSE::cutWordFilter()
{
    size_t line_size = lines.size();
    if (lineIndex >= line_size)
        return false;

    const std::string &sentence = lines[lineIndex];

    int initTimeSec = (sentence[1] - '0') * 3600 + std::stoi(sentence.substr(3, 2)) * 60 + std::stoi(sentence.substr(6, 2));

    int threshold = initTimeSec + step;

    while (TWManager.getCurrentTime() < threshold && lineIndex < line_size)
    {
        const std::string s = lines[lineIndex++];
        int timeSec = (s[1] - '0') * 3600 + std::stoi(s.substr(3, 2)) * 60 + std::stoi(s.substr(6, 2));

        std::vector<std::pair<std::string, std::string>> wordWithCls;
        jieba.Tag(s.substr(10), wordWithCls);

        if (TWManager.shouldRemoveOld(timeSec))
            removeOutdatedWords();

        size_t word_size = wordWithCls.size();
        for (size_t i{}; i < word_size; i++)
        {
            if (!SWManager.isStopWord(wordWithCls[i].first) && filter.find(wordWithCls[i].second) == filter.end())
            {
                // 添加到时间窗口
                TWManager.addWord(timeSec, wordWithCls[i].first);

                // 更新词频排名
                ranker.addWord(wordWithCls[i].first);
            }
        }
    }
    countTopKWords(out);
    return true;
}

bool HaEngineSSE::cutWordChooser()
{
    size_t line_size = lines.size();
    if (lineIndex >= line_size)
        return false;

    const std::string &sentence = lines[lineIndex];

    int initTimeSec = (sentence[1] - '0') * 3600 + std::stoi(sentence.substr(3, 2)) * 60 + std::stoi(sentence.substr(6, 2));

    int threshold = initTimeSec + step;

    while (TWManager.getCurrentTime() < threshold && lineIndex < line_size)
    {
        const std::string s = lines[lineIndex++];
        int timeSec = (s[1] - '0') * 3600 + std::stoi(s.substr(3, 2)) * 60 + std::stoi(s.substr(6, 2));

        std::vector<std::pair<std::string, std::string>> wordWithCls;
        jieba.Tag(s.substr(10), wordWithCls);

        if (TWManager.shouldRemoveOld(timeSec))
            removeOutdatedWords();

        size_t word_size = wordWithCls.size();
        for (size_t i{}; i < word_size; i++)
        {
            if (!SWManager.isStopWord(wordWithCls[i].first) && chooser.find(wordWithCls[i].second) != chooser.end())
            {
                // 添加到时间窗口
                TWManager.addWord(timeSec, wordWithCls[i].first);

                // 更新词频排名
                ranker.addWord(wordWithCls[i].first);
            }
        }
    }
    countTopKWords(out);
    return true;
}

void HaEngineSSE::countTopKWords(std::ofstream &out)
{
    // 创建本地流对象，每次都覆盖文件
    std::ofstream localOut(outputFile, std::ios::out | std::ios::trunc);

    if (!localOut.is_open())
    {
        std::cerr << "[错误] 无法打开输出文件: " << outputFile << std::endl;
        return;
    }

    auto it = ranker.getRankingSet().rbegin();
    json output;
    int count = 0;
    for (int i{1}; i <= topK && it != ranker.getRankingSet().rend(); ++i, ++it)
    {
        std::string word = it->second;
        int freq = it->first;
        output[word] = freq;
        count++;
    }

    std::string jsonStr = output.dump(4);
    localOut << jsonStr << std::endl;
    localOut.flush(); // 确保刷新缓冲区
    localOut.close(); // 关闭文件

    std::cout << "[countTopKWords] 已写入 " << count << " 个词，JSON长度: " << jsonStr.length() << std::endl;
    std::cout << "[调试] JSON内容: " << jsonStr << std::endl;
}