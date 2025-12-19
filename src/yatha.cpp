#include "../third_party/cppjieba/Jieba.hpp"
#include "../include/haEngine.hpp"
#include <iostream>

using namespace std;



int main()
{
    std::string input = "../test_sentences.txt";
    std::string output = "output.txt";
    int topk = 3;
    int windowsize = 600;

    std::ofstream out(output, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "[ERROR] cannot open output file: " << output << std::endl;
        return EXIT_FAILURE;
    }

    HaEngine ha(windowsize, topk, input, output);
    ha.cutWord();
}