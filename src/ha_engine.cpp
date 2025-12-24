#include "../include/ha_engine.h"
#include "../include/stop_words_manager.h"
// 初始化 jieba 和 SWmanager
HaEngine::HaEngine
(
    const std::string &dictPath, const std::string &hmmPath, const std::string &userDictPath,
    const std::string &idfPath, const std::string &stopWordDictPath, int window, int k, std::unordered_set<std::string> &ftr,
    std::unordered_set<std::string> &csr, const std::string &i, const std::string &o
) : 
    jieba(dictPath, hmmPath, userDictPath, idfPath, stopWordDictPath),
    SWManager(stopWordDictPath),
    TWManager(window),
    topK(k), filter(ftr), chooser(csr), inputFile(i), outputFile(o) 
    { out.open(outputFile, std::ios::binary); }

// 将新到的句子分词，然后更新时间窗口、词频和词排行榜
bool HaEngine::cutWord()
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

            // 使用新的时间窗口管理器判断是否需要移除过期词
            if (TWManager.shouldRemoveOld(timeSec))
                removeOutdatedWords();

            // 添加新词
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

        else
        {
            topK = std::stoi(sentence.substr(17));
            countTopKWords(out);
        }
    }
    return true;
}

// 分词的同时过滤掉不需要的词性
bool HaEngine::cutWordFilter()
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

            // 使用新的时间窗口管理器
            if (TWManager.shouldRemoveOld(timeSec))
                removeOutdatedWords();

            size_t word_size = wordWithCls.size();
            for (size_t i{}; i < word_size; i++)
            {
                if (!SWManager.isStopWord(wordWithCls[i].first) && filter.find(wordWithCls[i].second) == filter.end())
                {
                    TWManager.addWord(timeSec, wordWithCls[i].first);
                    ranker.addWord(wordWithCls[i].first);
                }
            }
        }

        else
        {
            topK = std::stoi(sentence.substr(17));
            countTopKWords(out);
        }
    }
    return true;
}

// 选择用户所需的词
bool HaEngine::cutWordChooser()
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

            // 使用新的时间窗口管理器
            if (TWManager.shouldRemoveOld(timeSec))
                removeOutdatedWords();

            size_t word_size = wordWithCls.size();
            for (size_t i{}; i < word_size; i++)
            {
                if (!SWManager.isStopWord(wordWithCls[i].first) && chooser.find(wordWithCls[i].second) != chooser.end())
                {
                    TWManager.addWord(timeSec, wordWithCls[i].first);
                    ranker.addWord(wordWithCls[i].first);
                }
            }
        }

        else
        {
            topK = std::stoi(sentence.substr(17));
            countTopKWords(out);
        }
    }
    return true;
}

// 删去已经离开时间窗口的词
void HaEngine::removeOutdatedWords()
{
    // 从时间窗口管理器获取过期的词列表
    auto outdatedWords = TWManager.getAndRemoveOutdatedWords();

    // 遍历过期词，使用 WordRanker 更新词频和排名
    // 这里可以用c++17标准引入的结构化绑定特性，所以xmake.lua文件要规定c++标准为c++17
    for (const auto &[timestamp, word] : outdatedWords)
        ranker.removeWord(word);
}

// 计算当前时间窗口的 TopK 词汇
void HaEngine::countTopKWords(std::ofstream &out)
{
    out << std::setfill('0') << '[' << std::setw(2) << TWManager.getCurrentTime() / 3600 << ':' << std::setw(2) << (TWManager.getCurrentTime() % 3600) / 60 << ':' << std::setw(2) << TWManager.getCurrentTime() % 60 << "]\n";
    auto it = ranker.getRankingSet().rbegin();
    for (int i{1}; i <= topK && it != ranker.getRankingSet().rend(); ++i, ++it)
    {
        std::string word = it->second;
        int freq = it->first;
        out << i << ". " << "“" << word << "”" << "出现" << freq << "次\n";
    }
    out << "\n";
}

// 分词测试
// void HaEngine::cutWordsTest()
// {
//     if (!readUtf8Lines(lines))
//     {
//         std::cerr << "[ERROR] cannot open input file: " << inputFile << std::endl;
//         std::cerr << "[HINT ] create a UTF-8 file named '" << inputFile << "' with Chinese sentences." << std::endl;
//         exit(1);
//     }
//     for (size_t idx = 0; idx < lines.size(); ++idx)
//     {
//         const std::string &sentence = lines[idx];
//         if(sentence[1] != 'A')
//         {
//             int timeSec = (sentence[1] - '0') * 3600 + (std::stoi(sentence.substr(3, 2)) % 3600) * 60 + std::stoi(sentence.substr(6, 2)) % 3600;

//             std::vector<std::string> word;
//             jieba.Cut(sentence.substr(10), word, true);
//             for (size_t i{}; i < word.size(); i++)
//             {
//                 if (!SWManager.isStopWord(word[i]))
//                     words.emplace_back(std::make_pair(timeSec, word[i]));
//             }
//         }
//     }
// }

// 读取输入文件
bool HaEngine::readUtf8Lines(std::vector<std::string> &lines)
{
    std::ifstream ifs(inputFile, std::ios::binary);
    if (!ifs.is_open())
        return false;
    std::string line;
    while (std::getline(ifs, line))
    {
        // 移除所有的 \r 字符（不只是行尾的）
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        if (!line.empty())
            lines.push_back(line);
    }
    return true;
}

// 输出测试分词结果
// void HaEngine::testOutput()
// {
//     for (size_t i{}; i < words.size(); i++)
//         out << "Timestamp: " << words[i].first << "s: " << words[i].second << "\n";
// }
