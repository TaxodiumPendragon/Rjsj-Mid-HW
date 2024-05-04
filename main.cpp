#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>

#include "json.hpp"
#include "httplib.h"
#include "utils.hpp"
#include "citation.h"

std::string readFromFile(const std::string &inputPath)
{
    std::ifstream file(inputPath);
    if (!file)
    {
        std::cerr << "can not open input file: " << inputPath << std::endl;
        std::exit(1);
    }

    std::string text;
    std::string line;
    while (std::getline(file, line))
    {
        text += line + "\n";
    }
    return text;
}

std::vector<Citation *> loadCitations(const std::string &filename)
{
    // load citations from file
    std::vector<Citation *> citations;
    std::ifstream file{filename};
    if (!file.is_open())
    {
        printf("can not open cite file\n");
        std::exit(1);
    }
    nlohmann::json data;
    try
    {
        data = nlohmann::json::parse(file);
    }
    catch (nlohmann::json::parse_error &)
    {
        std::exit(1);
    }
    for (auto &cite : data["citations"])
    {
        Citation *citation = nullptr;
        if (!cite["type"].is_string() || !cite["id"].is_string())
        {
            printf("ID or type Error");
            std::exit(1);
        }
        std::string t = cite["type"].get<std::string>();
        std::string id = cite["id"].get<std::string>();

        if (t == "book")
        {
            if (!cite["isbn"].is_string())
            {
                printf("ISBN Error!");
                std::exit(1);
            }
            citation = new CitBook(id, cite["isbn"].get<std::string>());
        }

        else if (t == "webpage")
        {
            if (!cite["url"].is_string())
            {
                printf("url error!\n");
                std::exit(1);
            }
            citation = new CitWeb(id, cite["url"].get<std::string>());
        }
        else if (t == "article")
        {
            if (!cite["year"].is_number_integer() || !cite["volume"].is_number_integer() || !cite["issue"].is_number_integer() ||
                !cite["title"].is_string() || !cite["author"].is_string() || !cite["journal"].is_string())
            {
                printf("Article Error!\n");
                std::exit(1);
            }
            int year = cite["year"].get<int>();
            int volume = cite["volume"].get<int>();
            int issue = cite["issue"].get<int>();
            citation = new CitArt(id, cite["title"].get<std::string>(), cite["author"].get<std::string>(), cite["journal"].get<std::string>(), year, volume, issue);
        }
        if (citation != nullptr)
        {
            citations.push_back(citation);
        }
        else
            std::exit(1);
    }
    return citations;
}

httplib::Client cli{API_ENDPOINT};

void CitWeb::ask()
{
    auto result = cli.Get("/title/" + encodeUriComponent(url));
    if (result && result->status == httplib::OK_200)
    {
        nlohmann::json t = nlohmann::json::parse(result->body);
        if (!t["title"].is_string())
        {
            printf("Web ask Error\n");
            std::exit(1);
        }
        title = t["title"].get<std::string>();
    }
    else
    {
        auto err = result.error();
        std::cerr << "HTTP error: " << httplib::to_string(err) << std::endl;
    }
}

void CitBook::ask()
{
    auto result = cli.Get("/isbn/" + encodeUriComponent(isbn));
    if (result && result->status == httplib::OK_200)
    {
        nlohmann::json t = nlohmann::json::parse(result->body);
        if (!t["author"].is_string() || !t["title"].is_string() || !t["publisher"].is_string() || !t["year"].is_string())
        {
            printf("Book Ask error\n");
            std::exit(1);
        }
        author = t["author"].get<std::string>();
        title = t["title"].get<std::string>();
        p = t["publisher"].get<std::string>();
        year = t["year"].get<std::string>();
    }
    else
    {
        auto err = result.error();
        std::cerr << "HTTP error: " << httplib::to_string(err) << std::endl;
    }
}

int main(int argc, char **argv)
{
    bool stdo{true};
    bool stdi{true};
    // 处理一些命令行参数错误输入情况，处理了命令行本身参数有误的可能情况
    if (argc != 4 && argc != 6)
    {
        printf("Command Error!\n");
        std::exit(1);
    }
    std::string citationPath;
    std::string inputPath;
    std::string outputPath;

    for (int i = 1; i < argc - 1; i++)
    {
        std::string arg = argv[i];
        if (arg == "-c" && i + 1 < argc)
        {
            citationPath = argv[++i];
        }
        else if (arg == "-o" && i + 1 < argc)
        {
            stdo = false;
            outputPath = argv[++i];
        }
        else
        {
            // 错误的参数
            printf("Command Error!\n");
            std::exit(1);
        }
    }
    std::string arg = argv[argc - 1]; // 应该使用std::string来接受为好，要不然需要很麻烦的比较
    if (arg != "-")
    {
        stdi = false;
        inputPath = argv[argc - 1];
    }

    // "docman", "-c", "citations.json", "input.txt"
    // 处理错误：文章的中括号有误
    auto citations = loadCitations(citationPath);

    // read all input to the string
    std::string input;
    if (stdi)
    {
        std::string line;
        while (std::getline(std::cin, line))
        {
            if (line.empty())
            {
                break;
            }
            input += line + "\n";
        }
    }
    else
    {
        input = readFromFile(inputPath);
    }

    std::vector<Citation *> printedCitations{};
    // 处理输入中的引用
    std::vector<std::string> ids;
    bool idNum = true;
    std::string::size_type pos = 0;
    std::string::size_type endPos = 0;
    while ((pos = input.find('[', pos)) != std::string::npos)
    {
        endPos = input.find(']', pos);
        std::string::size_type nextPos = input.find('[', pos + 1);
        std::string::size_type prevEndPos = input.rfind(']', pos);
        if (endPos == std::string::npos || (nextPos != std::string::npos && nextPos < endPos) || (prevEndPos != std::string::npos && prevEndPos > pos))
        {
            printf("What you input\n");
            std::exit(1);
        }
        std::string idStr = input.substr(pos + 1, endPos - pos - 1);
        ids.push_back(idStr);
        pos = endPos;
    }
    std::sort(ids.begin(), ids.end());
    for (const auto &id : ids)
    {
        bool find = false;
        for (auto c : citations)
        {

            if (c->getid() == id)
            {
                find = true;
                printedCitations.push_back(c);
                c->ask();
                break;
            }
        }
        if (!find)
        {
            printf("Id not found\n");
            std::exit(1);
        }
    }
    std::ostream &output = std::cout;
    if (!stdo)
    {
        std::ofstream file(outputPath);
        if (!file)
        {
            std::cerr << "can not open: " << outputPath << std::endl;
            std::exit(1);
        }
        std::ostream &output = file;
    }
    // output part
    output << input; // print the paragraph first
    output << "\nReferences:\n";
    for (auto c : printedCitations)
    {
        // 打印引用格式输出
        c->print(output);
    }

    for (auto c : citations)
    {
        delete c;
    }
    return 0;
}
