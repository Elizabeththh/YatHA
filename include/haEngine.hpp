#include "../third_party/cppjieba/Jieba.hpp"
#include "../include/stopWordsManager.hpp"
#include <iostream>
#include <fstream>
#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <iomanip>


using Tword = std::pair<int, std::string>;
class HaEngine
{
    private:
        std::queue<Tword> history_queue;                            // 记录当前时间窗口的词语
        std::unordered_map<std::string, int> freq_map;              // 记录当前时间窗口词语的频次
        std::set<Tword> ranking_set;                                // 按词语出现频次升序排列集合

        std::vector<std::string> lines;
        std::vector<Tword> words;
        cppjieba::Jieba jieba;
        StopWordsManager swManager;
        
        
        const std::string inputFile{};
        const std::string outputFile{}; 
        std::ofstream out;                                          // 在构造函数中打开文件，而不是在 countTopKwords中打开，避免内容覆盖；
        int max_window_size{};                                      
        int curr_time = -1;                                         // 输入文件时间戳从0秒开始，初始时间戳设为 -1
        int cur_window_size{};
        int top_k{};

    public:
        HaEngine(int window, int k, const std::string& i, const std::string& o);
        void cutWordsTest();
        void cutWord();
        void writeOutput();
        bool ReadUtf8Lines(std::vector<std::string>& lines);
        void testOutput(std::ofstream& out);
        void remove_outdate_words();
        void countTopKWords(std::ofstream& out);

};
