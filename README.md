# Scientific Paper Crawler 🚀

一个基于Modern C++的高性能学术预印本爬虫系统，支持多线程、协程和进程三种并发模式，可同时爬取arXiv、bioRxiv和ChemRxiv的公开论文。

## 特性

- 🔥 **多并发模式**：支持线程、协程(C++20)、进程三种并发模型
- 📚 **多数据源**：同时爬取arXiv、bioRxiv和ChemRxiv论文数据
- ⚙️ **灵活配置**：基于TOML的配置文件，支持动态调整爬取参数
- 🏗️ **现代C++**：使用C++20特性，模块化设计，低耦合度架构
- 📊 **多种输出格式**：支持JSON、CSV、XML等多种数据存储格式
- 🎯 **智能分类**：按物理、材料、化学、生物学等学科关键词自动分类

## 环境要求

### 系统要求
- **操作系统**：Linux、macOS或Windows（推荐Linux）
- **编译器**：支持C++20的编译器（GCC 11+、Clang 12+、MSVC 2019+）
- **构建工具**：CMake 3.15+

### 依赖库
```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake libcurl4-openssl-dev

# CentOS/RHEL
sudo yum install gcc-c++ make cmake3 libcurl-devel

# macOS
brew install cmake curl
```

## 快速开始

### 1. 获取代码
```bash
git clone https://github.com/huangzsdy/crawlPaper.git
cd scientific-crawler
```

### 2. 构建项目
```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译（使用多核加速）
make -j$(nproc)
```

### 3. 配置爬虫
编辑配置文件 `config/config.toml`：

```toml
[crawler]
mode = "thread"  # 可选: thread, coroutine, process
max_connections = 10
request_timeout = 30

# 设置爬取的关键词
[keywords]
physics = ["cond-mat", "hep-", "quant-ph", "physics"]
materials = ["cond-mat.mtrl-sci"]
chemistry = ["physics.chem-ph"]
```

### 4. 运行爬虫
```bash
# 基本运行
./scientific_crawler

# 使用特定配置文件
./scientific_crawler --config /path/to/config.toml

# 只爬取特定学科
./scientific_crawler --categories "physics,chemistry"
```

## 项目结构

```
scientific_crawler/
├── include/                 # 头文件
│   ├── config/             # 配置管理
│   ├── network/            # 网络通信模块
│   ├── parser/             # 论文解析器
│   ├── scheduler/          # 任务调度器
│   ├── storage/            # 数据存储
│   └── model/              # 数据模型
├── src/                    # 源文件
├── config/                 # 配置文件
│   └── config.toml        # 主配置文件
├── third_party/            # 第三方依赖
└── data/                   # 输出数据（自动创建）
```

## 详细配置说明

### 并发模式选择
根据需求选择合适的并发模式：

| 模式 | 适用场景 | 优点 |
|------|----------|------|
| **thread** | I/O密集型任务 | 资源共享方便，调试简单 |
| **coroutine** | 高并发I/O操作 | 轻量级，资源消耗小 |
| **process** | CPU密集型任务 | 稳定性高，进程隔离 |

### 关键词配置
支持按学科领域配置爬取关键词：
```toml
[keywords]
physics = ["cond-mat", "hep-", "quant-ph", "physics"]
materials = ["cond-mat.mtrl-sci"] 
chemistry = ["physics.chem-ph"]
biology = ["q-bio"]
electrical_engineering = ["eess.SP", "eess.SY"]
```

## 输出数据格式

爬取的论文数据以JSON格式存储，包含以下字段：
```json
{
  "id": "arxiv:2101.12345v1",
  "title": "论文标题",
  "abstract": "论文摘要",
  "authors": [{"name": "作者名", "affiliation": "机构"}],
  "doi": "10.1234/example",
  "pdf_url": "https://example.com/paper.pdf",
  "source": "arxiv",
  "categories": ["cond-mat", "physics"],
  "published_date": "2023-01-01T00:00:00Z"
}
```

## 开发指南

### 代码结构说明
项目采用模块化设计，核心模块包括：
- **网络模块**：基于libcurl的HTTP客户端，支持同步/异步请求
- **解析器模块**：各论文源的专用解析器，可扩展支持新数据源
- **调度器模块**：三种并发模式的统一接口实现
- **存储模块**：可插拔的数据存储后端

### 添加新的论文源
1. 在 `include/parser/` 创建新的解析器类
2. 实现 `PaperParser` 接口
3. 在 `PaperParser::create()` 工厂方法中注册新解析器
4. 更新配置文件支持新数据源

### 代码格式化
项目使用.clang-format规范代码风格：
```bash
# 格式化代码
find src include -name '*.cpp' -o -name '*.h' | xargs clang-format -i
```

## 故障排除

### 常见问题
1. **编译错误**：确保编译器支持C++20标准
2. **网络连接失败**：检查防火墙设置和代理配置
3. **依赖缺失**：确认libcurl和CMake已正确安装

### 调试模式
```bash
# 调试构建
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
./scientific_crawler --verbose
```

## 贡献指南

欢迎提交Issue和Pull Request！贡献流程：
1. Fork本项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建Pull Request

## 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 致谢

- 感谢arXiv、bioRxiv、ChemRxiv提供开放的API接口
- 使用了TOML++、pugixml等优秀的开源库
- 项目架构设计参考了现代C++最佳实践

---

**注意**：请遵守各论文源的使用条款和API限制，合理使用爬虫功能。