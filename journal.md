## v0.1 完成基础滑动窗口热词统计功能
1. **输出文件每次都被覆盖的问题**
```cpp
        std::ofstream out(outputFile, std::ios::binary);
        countTopKWords(out);
```
   - **现象**：`countTopKWords()` 函数在每次执行 ACTION 命令时都会创建新的 `std::ofstream` 对象，导致文件被覆盖，只能看到最后一次的输出
   - **原因**：在 `cutWord()` 中每次遇到 ACTION 时都执行 `std::ofstream out(outputFile, std::ios::binary);`，默认模式为覆盖模式
   查阅资料得到以下解决方案：
   - **解决方案**：
     - 方案1：使用追加模式 `std::ios::binary | std::ios::app`
     - 方案2：将 `ofstream` 作为类成员变量，在构造函数中打开，析构函数中关闭
     - 方案3：使用静态变量标记首次写入，首次覆盖，后续追加
   - **方案分析**：
        - 方案1：如果output.txt已存在，那么每次运行程序后output.txt都会变长，显然不是我们想要的结果
        - 方案2：将文件流作为类成员，在构造函数中打开一次，整个对象生命周期内保持打开状态，符合面向对象设计原则，且确保每次运行都是新文件
        - 方案3：需要额外的静态变量维护状态，增加了复杂度，且如果创建多个HaEngine对象会出现问题
        故选择方案2
```cpp
class haEngine
{
    private:
    ...
    std::ofstream out;
};

// 初始化 jieba 和 swmanager
HaEngine::HaEngine(int window, int k, const std::string &i, const std::string &o) : 
jieba("../data/dict/jieba.dict.utf8",
      "../data/dict/hmm_model.utf8",
      "../data/dict/user.dict.utf8",
      "../data/dict/idf.utf8",
      "../data/dict/stop_words.utf8"),
swManager("../data/dict/stop_words.utf8"),
max_window_size(window), top_k(k), inputFile(i), outputFile(o) { out.open(outputFile, std::ios::binary); }
```

2. **同一时间戳多条数据触发重复删除导致词频异常**
```cpp
// 错误代码
else
{
    curr_time = timeSec;        // 更新当前时间
    remove_outdate_words();     // 每条弹幕都删除一次
}
```

   - **现象**：程序运行后，后期查询的热词频率都变成了1次，热词统计失效
   - **原因**：当窗口满了（达到600秒）后，同一时间戳的多条弹幕会重复触发删除逻辑
   - **问题分析**：
     - 假设第600秒有100条弹幕（同一时间戳）
     - 第1条弹幕：`timeSec = 600`，`curr_time` 从599更新为600，调用 `remove_outdate_words()` 删除第0秒的所有词 
     - 第2条弹幕：`timeSec = 600`，但 `curr_time` 已经是600，`cur_window_size >= max_window_size` 仍然成立，再次进入 `else` 分支
     - 又调用 `remove_outdate_words()`，这次删除第1秒的所有词
     - 第3-100条弹幕：继续删除第2秒、第3秒...的词
     - 结果：窗口被疯狂缩小，大量高频词被误删，导致剩余词频率都变成1
   - **解决方案**：只在时间戳变化时触发删除操作

```cpp
// 修正后代码
if (cur_window_size < max_window_size)
{
    if (curr_time != timeSec)
    {
        curr_time = timeSec;
        ++cur_window_size;
    }
}
else
{
    if (curr_time != timeSec)  // 增加时间戳变化判断
    {
        curr_time = timeSec;
        remove_outdate_words();
    }
}
```

   - **核心思想**：滑动窗口的删除操作应该以时间戳为单位，而不是以单条数据为单位只有当新的时间戳到来时，才删除最早时间戳的所有数据，保证窗口始终维持600秒的长度

## v0.2 规范化命名，引入 Xmake 支持跨平台编译，支持基本 CLI 
### 人生苦短，我选 Xmake
`xmake.lua`：
```lua
add_rules("mode.debug", "mode.release")


target("yatha")
    set_kind("binary")
    add_files("src/*.cpp")

    add_includedirs("include")
    add_includedirs("third_party/cppjieba")

    set_targetdir("bin")
    set_rundir("$(projectdir)")
    set_runargs("input1.txt", "output.txt")
    
    if is_plat("linux", "macosx") then 
        add_syslinks("pthread", "m")
    end
```
完成同等功能所需的 `CmakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.10)

project(yatha CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)


file(GLOB SOURCES "src/*.cpp")


add_executable(yatha ${SOURCES})

target_include_directories(yatha PRIVATE
    include
    third_party/cppjieba
)

if(UNIX OR APPLE)
    # 查找线程库 (比直接写 "pthread" 更规范)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)

    target_link_libraries(yatha PRIVATE
        Threads::Threads 
        m               
    )
endif()

add_custom_target(run
    COMMAND yatha input1.txt output.txt
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} # 对应 set_rundir("$(projectdir)")
    DEPENDS yatha
    COMMENT "Running yatha with default arguments..."
)
```
1. 可读性强
lua 是一门脚本语言，语法简洁直观比起 `CmakeLists.txt` 中一会大写一会小写，各种何意味的宏定义，xmake 的语法绝对是**小葱拌豆腐——一清二白了**
2. 构建流程简单
Cmake 通常需要
```shell
mkdir build 
cd build
cmake ..
make
```
这一套繁琐的流程才能开始构建项目

但是对于 xmake，只要写好配置文件`xmake.lua`（如上面所示，并不繁琐）不管在项目的哪一个目录，构建项目只需执行`xmake`，运行可执行文件只需`xmake run`（若任何依赖文件有更新会重新构建）~~对于正在焦头烂额赶大作业 DDL 的我来说这确实是 make my life easier了~~

### Windows下使用 Xmake 遇到的编码问题
![alt text](/img/image.png)
查阅资料可知：Windows 默认代码页（操作系统中用于表示文本文件中字符的编码）通常是 GBK，这时需要为 MSVC 指定 `/utf8` 参数
```lua
if is_plat("windows") then 
        add_cxflags("/utf-8", {tools = "cl"})
        add_cxflags("-finput-charset=UTF-8", "-fexec-charset=UTF-8", {tools = {"gcc", "clang"}})
end
```

### 扩展命令行参数选项，实现词性筛选功能
1. 发现 cppjieba 库的作者在 `limonp` 文件夹中偷偷实现了很多很好用的工具函数

比如 `ArgvContext.hpp` 就是一个用来处理命令行参数的类

该类的构造函数：
```cpp
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
```
于是在该类中添加一个`public`函数`ReadArgv()`实现命令行参数的读取
```cpp
void ReadArgv(std::string& Input, std::string& Output, int& Windows, int& TopK, std::unordered_set<std::string>& ftr, std::unordered_set<std::string>& csr)
  {
    if(this->HasKey("-h"))
    {
      PrintHelp();
      exit(0);
    }
    if(this->HasKey("-wc"))
    {
      PrintPOSHelp();
      exit(0);
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
  }
```
2. 查阅cppjieba库资料得到词性对照表，根据用户输入来选择**过滤/放行**属于某种词性的词语
（敏感词过滤功能因为测试文本**不方便**生成，所以改做词性筛选）
由于cppjieba库本身实现对某些词语的识别就不太准确，如“刘备”竟然被归为音译人名所以**是否精准地筛选/放行某类词性的词语不在考虑范围内**，这里只注重算法设计

在 `HaEngin` 类里分别维护无序集 `filter`, `chooser` 用来确定需要过滤/放行的词性，再对原来的 `cutWords()`函数稍加修改即可实现功能

## v0.3 实现词性过滤/放行功能
在 `HaEngine` 类中增加了 `cutWordFilter` 和 `cutWordChooser` 两个成员函数
- **原理**：
  使用 `jieba.Tag()` 替代普通的 `jieba.Cut()`，这样分词结果会包含词性信息（如 `n` 表示名词，`v` 表示动词）
- **实现逻辑**：
  - `cutWordFilter`：遍历分词结果，如果某词的词性**不在** `filter` 集合中，且不是停用词，则统计该词
  - `cutWordChooser`：遍历分词结果，如果某词的词性**在** `chooser` 集合中，且不是停用词，则统计该词

```cpp
if (!swManager.isStopWord(wordWithCls[i].first) && filter.find(wordWithCls[i].second) == filter.end())
{
    // 加入统计...
}
```
**同时增添命令行参数选项 `-wc`，打印词性标识符对应的词性，方便用户进行选择**


## v0.7 实现 Web GUI 界面
使用一个轻量级的 C++ HTTP 库 `cpp-httplib` 来搭建 Web 服务器（引用一个头文件就能启动服务器，非常方便），为用户提供一个 Web GUI 界面。

### 1. 后端实现
- **集成 httplib**：在 `yatha.cpp` 中引入 `httplib.h`
- **新增服务器模式**：
  - 修改 `ArgvContext` 类，增加 `-s` 参数解析，用来启动 HTTP 服务器
  - 当用户输入 `./yatha -s` 时，启动 HTTP 服务器监听 8080 端口
- **API 设计**：
  - 提供 `/api/analyze` POST 接口
  - 接收前端上传的文本内容，写入临时文件 `temp_input.txt`
  - 利用 cpp 的 RAII 机制控制 `HaEngine` 生命周期，确保结果写入磁盘后再读取返回

```cpp
// RAII 确保文件流正确关闭
{
    HaEngine ha(..., tempInput, tempOutput);
    ha.cutWord();
} // ha 离开域，自动析构，确保文件正确关闭并保存
```

### 2. 前端实现
- **界面设计**：创建一个简洁的 HTML 页面，包含文件上传区域，顺便给实验室打一个小小的广告🤭
- **交互逻辑**：
  - 支持点击上传和拖拽上传 `.txt` 文件
  - 使用 `fetch` API 将文件内容发送给后端
  - 收到响应后实时在页面上显示热词分析结果，无需页面跳转

### 3. 遇到的坑与解决方案
- **问题1：网页端分析结果显示为空**
  - 原因：`HaEngine` 还在运行（文件流未关闭）时就尝试读取输出文件，导致读到空内容
  - 解决办法：将 `HaEngine` 的实例化放入独立作用域 `{}` 中，强制其在读取前析构并刷新文件流
- **问题2：文件覆盖风险**
  - 原因：最初使用 `input1.txt` 作为临时文件，容易覆盖用户数据
  - 解决办法：改用 `temp_input.txt` 和 `temp_output.txt`
- **问题3：文件挂载问题**
  - 原因：前端访问静态文件的请求需要处理之后返回
  如
  ```cpp
  svr.Get("/index.html", [](auto& req, auto& res){
    // 1. 打开 web/index.html
    // 2. 读取内容
    // 3. 设置 Content-Type 为 text/html
    // 4. 发送
  });

  svr.Get("/style.css", [](auto& req, auto& res){
    // 1. 打开 web/style.css
    // ... 重复
  });
  ```

  - 解决方案，使用 `svr.set_mount_point()` 函数，一行代码即可实现静态目录的挂载，无需手动处理每个文件的请求。
  ```cpp
    // Serve static files from web directory (assuming running from data/ directory)
    svr.set_mount_point("/", "../web");
    svr.set_mount_point("/img", "../img");
  ```

## v0.8 代码重构
当我准备在Web GUI 中新增滚动查询功能的时候，发现 `HaEngine` 非常臃肿，几乎所有功能都是在这个类中实现的。为了保证代码的可读性和减少未来不必要的调试麻烦，同时减少编译~~坐牢~~时间（如果函数都实现在一个类中，那么每一次小变动都会导致项目主要文件的重新编译，会**极大地提升不幸福感**，大作业要赶不完了），决定重构代码！

### 原来的 `HaEngine` 类：
```cpp
class HaEngine
{
    private:
        std::queue<Tword> historyQueue;                            // 记录当前时间窗口的词语
        std::unordered_map<std::string, int> freqMap;              // 记录当前时间窗口词语的频次
        std::set<Tword> rankingSet;                                // 按词语出现频次升序排列集合

        std::vector<std::string> lines;
        std::vector<Tword> words;
        StopWordsManager swManager;
        
        
        const std::string inputFile{};
        const std::string outputFile{}; 
        std::ofstream out;                                          // 在构造函数中打开文件，而不是在 countTopKwords中打开，避免内容覆盖；
        int maxWindowSize{};                                      
        int currTime = -1;                                          // 输入文件时间戳从0秒开始，初始时间戳设为 -1
        int curWindowSize{};
        int topK{};
        std::unordered_set<std::string> filter{};        
        std::unordered_set<std::string> chooser{};                   
        
        public:
        cppjieba::Jieba jieba;
        
        HaEngine(const std::string& dictPath, const std::string& hmmPath, const std::string& userDictPath, 
                 const std::string& idfPath, const std::string& stopWordDictPath, int window, int k, std::unordered_set<std::string>& ftr,
                 std::unordered_set<std::string>& chsr, const std::string &i, const std::string &o);
        void cutWordsTest();
        void cutWord();
        void cutWordFilter();
        void cutWordChooser();
        void writeOutput();
        bool readUtf8Lines(std::vector<std::string>& lines);
        void testOutput();
        void removeOutdatedWords();
        void countTopKWords(std::ofstream& out);

};
```
可以看到包括1）时间窗口管理，2）TopK 排行榜管理等功能的有关变量和函数统统挤在了这个类里，看得头晕。

### 重构代码！
`HaEngine` 作为总调度器，将原来的时间窗口管理、TopK排行榜管理分别分离到`TimeWindowManager`类和`WordRanker`类中
```cpp
class WordRanker
{
    private:
        std::unordered_map<std::string, int> freqMap; // 词频映射
        std::set<Tword> rankingSet;                   // 按频次排序的集合 (freq, word)

    public:
        WordRanker() = default;

        // 添加一个词（词频+1）
        void addWord(const std::string &word);

        // 移除一个词（词频-1，如果为0则完全删除）
        void removeWord(const std::string &word);

        // 获取TopK词汇
        std::vector<std::pair<std::string, int>> getTopK(int k) const;

        // 获取排名集合（用于输出格式化）
        const std::set<Tword> &getRankingSet() const { return rankingSet; }
};
```

```cpp
class TimeWindowManager
{
    private:
        std::queue<Tword> historyQueue;  // 存储所有在窗口内的词
        int maxWindowSize;               // 最大窗口大小
        int currTime = -1;               // 当前时间戳
        int curWindowSize = 0;           // 当前窗口大小

    public:
        TimeWindowManager(int windowSize);
        
        // 判断是否需要移除过期词
        bool shouldRemoveOld(int newTime);
        
        // 获取并移除过期的词（返回需要删除的词列表）
        std::vector<Tword> getAndRemoveOutdatedWords();
        
        // 添加新词到窗口
        void addWord(int timestamp, const std::string& word);
        
        // Getter方法
        int getCurrentTime() const { return currTime; }
        int getCurrentWindowSize() const { return curWindowSize; }
        bool isEmpty() const { return historyQueue.empty(); }
};
```
重构之后的好处：
1. **单一职责**
每个类只负责一件事：`TimeWindowManager` 管理时间窗口，`WordRanker` 管理词频排名，`HaEngine` 负责协调调度。代码职责清晰，可读性更强了，易于维护

2. **降低耦合度**
各模块相对独立，接口明确。且修改一个模块的内部实现不会影响其他模块

3. **减少编译时间**（主要！！）
假如想修改 `WordRanker` 的实现时，只需重新编译 `word_ranker.cpp` 及依赖它的文件，而不会触发整个项目的重新编译，节省时间（天生打工圣体，太好了剩余价值又能被充分压榨了）

4. **便于功能扩展**
