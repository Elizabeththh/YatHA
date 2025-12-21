/************************************
 * file enc : ascii
 * author   : wuyanyi09@gmail.com
 ************************************/

#ifndef LIMONP_ARGV_FUNCTS_H
#define LIMONP_ARGV_FUNCTS_H

#include <set>
#include <sstream>
#include "StringUtil.hpp"

namespace limonp {

using namespace std;

class ArgvContext {
 public :
  ArgvContext(int argc, const char* const * argv) {
    for(int i = 0; i < argc; i++) {
      if(StartsWith(argv[i], "-")) {
        if(i + 1 < argc && !StartsWith(argv[i + 1], "-")) {
          mpss_[argv[i]] = argv[i+1];
          i++;
        } else {
          sset_.insert(argv[i]);
        }
      } else {
        args_.push_back(argv[i]);
      }
    }
  }
  ~ArgvContext() {
  }

  friend ostream& operator << (ostream& os, const ArgvContext& args);
  string operator [](size_t i) const {
    if(i < args_.size()) {
      return args_[i];
    }
    return "";
  }
  string operator [](const string& key) const {
    map<string, string>::const_iterator it = mpss_.find(key);
    if(it != mpss_.end()) {
      return it->second;
    }
    return "";
  }

  bool HasKey(const string& key) const {
    if(mpss_.find(key) != mpss_.end() || sset_.find(key) != sset_.end()) {
      return true;
    }
    return false;
  }

  void ReadArgv(std::string& Input, std::string& Output, int& Windows, int& TopK, std::unordered_set<std::string>& ftr, std::unordered_set<std::string>& csr, bool& isServer)
  {
    if(this->HasKey("-h"))
    {
      PrintHelp();
      exit(0);
    }
    if(this->HasKey("-s"))
    {
      isServer = true;
      return;
    }
    if(this->HasKey("-wc"))
    {
      PrintPOSHelp();
      exit(0);
    }
    if(this->HasKey("-s"))
    {
      isServer = true;
      return;
    }
    if(this->HasKey("-i"))
    {
        Input = mpss_["-i"];
        if(args_[0] == ".\\yatha.exe")
          std::cout << "输入文件：" << "data\\" << Input << "\n";
        else
          std::cout << "输入文件：" << "data/" << Input << "\n";
    }
    else
    {
        std::cout << "请指定输入文件\n";
        exit(1);
    }
    if(this->HasKey("-o"))
    {
        Output = mpss_["-o"]; 
        if(args_[0] == ".\\yatha.exe")
          std::cout << "输出文件：" << "data\\" << Output << "\n";
        else
          std::cout << "输出文件：" << "data/" << Output << "\n";
    }
    else
    {
        std::cout << "请指定输出文件\n";
        exit(1);
    }
    if(this->HasKey("-t"))
    {
      Windows = std::stoi(mpss_["-t"]);
      std::cout << "时间窗口大小：" << Windows << "s\n";
    }
    if(this->HasKey("-k"))
    {
      TopK = std::stoi(mpss_["-k"]);
      std::cout << "TopK: " << TopK << "\n";
    }
    if(this->HasKey("-f"))
    {
      std::stringstream CharCls(mpss_["-f"]);
      std::string Char;
      while(std::getline(CharCls, Char, ','))
        ftr.insert(Char);
      std::cout << "过滤词: ";

      for(const auto it: ftr)
      {
          // 呼，真是太辛苦了
          if(it == "n") std::cout << "名词 ";
          else if(it == "nr") std::cout << "人名 ";
          else if(it == "ns") std::cout << "地名 ";
          else if(it == "nt") std::cout << "机构团体 ";
          else if(it == "nz") std::cout << "其他专名 ";
          else if(it == "nrt") std::cout << "音译名 ";
          else if(it == "nrfg") std::cout << "音译人名 ";
          else if(it == "ng") std::cout << "名词性语素 ";
          else if(it == "v") std::cout << "动词 ";
          else if(it == "vg") std::cout << "动词性语素 ";
          else if(it == "vn") std::cout << "名动词 ";
          else if(it == "vd") std::cout << "副动词 ";
          else if(it == "vq") std::cout << "动量词 ";
          else if(it == "a") std::cout << "形容词 ";
          else if(it == "ag") std::cout << "形容词性语素 ";
          else if(it == "ad") std::cout << "副形词 ";
          else if(it == "an") std::cout << "名形词 ";
          else if(it == "d") std::cout << "副词 ";
          else if(it == "dg") std::cout << "副词性语素 ";
          else if(it == "m") std::cout << "数词 ";
          else if(it == "mg") std::cout << "数词性语素 ";
          else if(it == "mq") std::cout << "数量词 ";
          else if(it == "q") std::cout << "量词 ";
          else if(it == "qg") std::cout << "量词性语素 ";
          else if(it == "r") std::cout << "代词 ";
          else if(it == "rg") std::cout << "代词性语素 ";
          else if(it == "p") std::cout << "介词 ";
          else if(it == "c") std::cout << "连词 ";
          else if(it == "u") std::cout << "助词 ";
          else if(it == "ug") std::cout << "助词性语素 ";
          else if(it == "uj") std::cout << "助词性语素 ";
          else if(it == "f") std::cout << "方位词 ";
          else if(it == "b") std::cout << "区别词 ";
          else if(it == "bg") std::cout << "区别语素 ";
          else if(it == "t") std::cout << "时间词 ";
          else if(it == "tg") std::cout << "时间词性语素 ";
          else if(it == "s") std::cout << "处所词 ";
          else if(it == "x") std::cout << "非语素字/标点 ";
          else if(it == "e") std::cout << "叹词 ";
          else if(it == "o") std::cout << "拟声词 ";
          else if(it == "y") std::cout << "语气词 ";
          else if(it == "i") std::cout << "成语 ";
          else if(it == "l") std::cout << "习用语 ";
          else if(it == "j") std::cout << "简称 ";
          else if(it == "jn") std::cout << "简称名词 ";
          else if(it == "z") std::cout << "状态词 ";
          else if(it == "zg") std::cout << "状态词性语素 ";
          else if(it == "k") std::cout << "后接成分 ";
          else if(it == "g") std::cout << "语素 ";
          else if(it == "eng") std::cout << "英文/外语 ";
          else std::cout << it;
      }
    }
    else if(this->HasKey("-c"))
    {
      std::stringstream CharCls(mpss_["-c"]);
      std::string Char;
      while(getline(CharCls, Char, ','))
        csr.insert(Char);
      std::cout << "选择词：";

      for(const auto it: csr)
      {
        // 呼，真是太辛苦了
        if(it == "n") std::cout << "名词 ";
        else if(it == "nr") std::cout << "人名 ";
        else if(it == "ns") std::cout << "地名 ";
        else if(it == "nt") std::cout << "机构团体 ";
        else if(it == "nz") std::cout << "其他专名 ";
        else if(it == "nrt") std::cout << "音译名 ";
        else if(it == "nrfg") std::cout << "音译人名 ";
        else if(it == "ng") std::cout << "名词性语素 ";
        else if(it == "v") std::cout << "动词 ";
        else if(it == "vg") std::cout << "动词性语素 ";
        else if(it == "vn") std::cout << "名动词 ";
        else if(it == "vd") std::cout << "副动词 ";
        else if(it == "vq") std::cout << "动量词 ";
        else if(it == "a") std::cout << "形容词 ";
        else if(it == "ag") std::cout << "形容词性语素 ";
        else if(it == "ad") std::cout << "副形词 ";
        else if(it == "an") std::cout << "名形词 ";
        else if(it == "d") std::cout << "副词 ";
        else if(it == "dg") std::cout << "副词性语素 ";
        else if(it == "m") std::cout << "数词 ";
        else if(it == "mg") std::cout << "数词性语素 ";
        else if(it == "mq") std::cout << "数量词 ";
        else if(it == "q") std::cout << "量词 ";
        else if(it == "qg") std::cout << "量词性语素 ";
        else if(it == "r") std::cout << "代词 ";
        else if(it == "rg") std::cout << "代词性语素 ";
        else if(it == "p") std::cout << "介词 ";
        else if(it == "c") std::cout << "连词 ";
        else if(it == "u") std::cout << "助词 ";
        else if(it == "ug") std::cout << "助词性语素 ";
        else if(it == "uj") std::cout << "助词性语素 ";
        else if(it == "f") std::cout << "方位词 ";
        else if(it == "b") std::cout << "区别词 ";
        else if(it == "bg") std::cout << "区别语素 ";
        else if(it == "t") std::cout << "时间词 ";
        else if(it == "tg") std::cout << "时间词性语素 ";
        else if(it == "s") std::cout << "处所词 ";
        else if(it == "x") std::cout << "非语素字/标点 ";
        else if(it == "e") std::cout << "叹词 ";
        else if(it == "o") std::cout << "拟声词 ";
        else if(it == "y") std::cout << "语气词 ";
        else if(it == "i") std::cout << "成语 ";
        else if(it == "l") std::cout << "习用语 ";
        else if(it == "j") std::cout << "简称 ";
        else if(it == "jn") std::cout << "简称名词 ";
        else if(it == "z") std::cout << "状态词 ";
        else if(it == "zg") std::cout << "状态词性语素 ";
        else if(it == "k") std::cout << "后接成分 ";
        else if(it == "g") std::cout << "语素 ";
        else if(it == "eng") std::cout << "英文/外语 ";
        else std::cout << it;
      }
      std::cout << "\n";
    }
  }
  void PrintHelp()
  {
    std::cout << "用法: yatha -i <input_file> -o <output_file> [options]\n"
              << "选项:\n"
              << "  -i <file>       输入文件路径 (必须)\n"
              << "  -o <file>       输出文件路径 (必须)\n"
              << "  -t <seconds>    时间窗口大小 (默认: 600s)\n"
              << "  -k <number>     TopK 热词数量 (默认: 3)\n"
              << "  -f <pos>        词性过滤 (例如: -f n,v,eng)\n"
              << "  -wc             显示详细词性列表说明\n"
              << "  -h              显示帮助信息\n";
  }
  void PrintPOSHelp()
  {
      std::cout << "jieba分词词性标记说明:\n"
                << "  名词 (n-series):\n"
                << "    n:名词, nr:人名, ns:地名, nt:机构团体, nz:其他专名\n"
                << "    nrt:音译名, nrfg:音译人名, ng:名词性语素\n"
                << "  动词 (v-series):\n"
                << "    v:动词, vg:动词性语素, vn:名动词, vd:副动词, vq:动量词\n"
                << "  形容词 (a-series):\n"
                << "    a:形容词, ag:形容词性语素, ad:副形词, an:名形词\n"
                << "  副词 (d-series):\n"
                << "    d:副词, dg:副词性语素\n"
                << "  数词与量词:\n"
                << "    m:数词, mg:数词性语素, mq:数量词, q:量词, qg:量词性语素\n"
                << "  代词 (r-series):\n"
                << "    r:代词, rg:代词性语素\n"
                << "  虚词:\n"
                << "    p:介词, c:连词, u:助词, ug/uj:助词性语素\n"
                << "    f:方位词, b:区别词, bg:区别语素\n"
                << "  时空:\n"
                << "    t:时间词, tg:时间词性语素, s:处所词\n"
                << "  其他:\n"
                << "    x:非语素字/标点, e:叹词, o:拟声词, y:语气词\n"
                << "    i:成语, l:习用语, j:简称, jn:简称名词\n"
                << "    z:状态词, zg:状态词性语素, k:后接成分, g:语素\n"
                << "  特殊:\n"
                << "    eng:英文/外语\n";
  }
 private:
  vector<string> args_;
  map<string, string> mpss_;
  set<string> sset_;
}; // class ArgvContext

inline ostream& operator << (ostream& os, const ArgvContext& args) {
  return os<<args.args_<<args.mpss_<<args.sset_;
}

} // namespace limonp

#endif
