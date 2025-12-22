#include "ha_engine.h"

class HaEngineSSE: public HaEngine
{
    private:
        int step;
        int lineIndex{};            // 永远指向需要读取的行
    public:
        HaEngineSSE(const std::string& dictPath, const std::string& hmmPath, const std::string& userDictPath, 
                    const std::string& idfPath, const std::string& stopWordDictPath, int window, int k, std::unordered_set<std::string>& ftr,
                    std::unordered_set<std::string>& chsr, const std::string &i, const std::string &o, int step);
        bool cutWord() override;
        void countTopKWords(std::ofstream& out) override;
        bool cutWordFilter() override;
        bool cutWordChooser() override;
        
        // 本来想实现实时更新这两个参数的功能，但是由于 SSE 只能由后端向前端单向推送数据作罢
        // void updateStep(int newStep) { step = newStep; }
        // void updateTopK(int newTopK) { topK = newTopK; }
};