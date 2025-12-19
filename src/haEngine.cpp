#include "../include/haEngine.hpp"
#include "../include/stopWordsManager.hpp"
// 初始化 jieba 和 swmanager
HaEngine::HaEngine(int window, int k, const std::string &i, const std::string &o) : 
jieba("../data/dict/jieba.dict.utf8",
      "../data/dict/hmm_model.utf8",
      "../data/dict/user.dict.utf8",
      "../data/dict/idf.utf8",
      "../data/dict/stop_words.utf8"),
swManager("../data/dict/stop_words.utf8"),
max_window_size(window), top_k(k), inputFile(i), outputFile(o) { out.open(outputFile, std::ios::binary); }

void HaEngine::cutWord()
{
    if (!ReadUtf8Lines(lines))
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
            int timeSec = (sentence[1] - '0') * 3600 + stoi(sentence.substr(3, 2)) * 60 + stoi(sentence.substr(6, 2));

            std::vector<std::string> word;
            jieba.Cut(sentence.substr(10), word, true);

            if (cur_window_size < max_window_size && curr_time != timeSec)
            {
                curr_time = timeSec;
                ++cur_window_size;
            }
            else
            {
                if(curr_time != timeSec)
                {
                    curr_time = timeSec; // 保持当前时间戳与输入时间戳一致；
                    remove_outdate_words();
                }
            }
            size_t word_size = word.size();
            for (size_t i{}; i < word_size; i++)
            {
                if (!swManager.is_stopWord(word[i]))
                {
                    history_queue.push(std::make_pair(timeSec, word[i]));
                    if (freq_map.count(word[i]))
                    {
                        int old_word_count = freq_map[word[i]];
                        ranking_set.erase({old_word_count, word[i]});
                        ranking_set.insert({old_word_count + 1, word[i]});
                        ++freq_map[word[i]];
                    }
                    else
                    {
                        freq_map[word[i]] = 1;
                        ranking_set.insert({1, word[i]});
                    }
                }
            }
        }

        else
        {
            top_k = stoi(sentence.substr(17));
            countTopKWords(out);
        }
    }
}

void HaEngine::remove_outdate_words()
{
    if (!history_queue.empty())
    {
        int oldest_time = history_queue.front().first;
        while (!history_queue.empty() && history_queue.front().first == oldest_time)
        {
            auto oldword = history_queue.front();
            history_queue.pop();

            std::string word = oldword.second;
            int old_freq = freq_map[word];

            ranking_set.erase({old_freq, word});
            if (freq_map[word] == 1)
                freq_map.erase(word);
            else
            {
                --freq_map[word];
                ranking_set.insert({old_freq - 1, word});
            }
        }
    }
}

void HaEngine::countTopKWords(std::ofstream &out)
{

    out << std::setfill('0') << '[' << std::setw(2) << curr_time / 3600 << ':' << std::setw(2) << (curr_time % 3600) / 60 << ':' << std::setw(2) << curr_time % 60 << "]\n";
    auto it = ranking_set.rbegin();
    for (int i{1}; i <= top_k && it != ranking_set.rend(); ++i, ++it)
    {
        std::string word = it->second;
        int freq = it->first;
        out << i << ". " << "“" << word << "”" << "出现" << freq << "次\n";
    }
    out << "\n";
}

void HaEngine::cutWordsTest()
{
    if (!ReadUtf8Lines(lines))
    {
        std::cerr << "[ERROR] cannot open input file: " << inputFile << std::endl;
        std::cerr << "[HINT ] create a UTF-8 file named '" << inputFile << "' with Chinese sentences." << std::endl;
        exit;
    }
    for (size_t idx = 0; idx < lines.size(); ++idx)
    {
        const std::string &sentence = lines[idx];
        int timeSec = (sentence[1] - '0') * 3600 + stoi(sentence.substr(3, 2)) * 60 + stoi(sentence.substr(6, 2));

        std::vector<std::string> word;
        jieba.Cut(sentence.substr(10), word, true);
        for (size_t i{}; i < word.size(); i++)
        {
            if (!swManager.is_stopWord(word[i]))
                words.emplace_back(std::make_pair(timeSec, word[i]));
        }
    }
}

bool HaEngine::ReadUtf8Lines(std::vector<std::string> &lines)
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

void HaEngine::testOutput(std::ofstream &out)
{
    for (size_t i{}; i < words.size(); i++)
        out << "Timestamp: " << words[i].first << "s: " << words[i].second << "\n";
}
