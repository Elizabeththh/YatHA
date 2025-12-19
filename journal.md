## v0.1 完成基础滑动窗口热词统计功能
### 遇到的问题
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

   - **核心思想**：滑动窗口的删除操作应该以时间戳为单位，而不是以单条数据为单位。只有当新的时间戳到来时，才删除最早时间戳的所有数据，保证窗口始终维持600秒的长度