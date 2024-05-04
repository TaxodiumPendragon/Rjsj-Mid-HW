#pragma once
#ifndef CITATION_H
#define CITATION_H

#include <string>

class Citation
{
    std::string id;

public:
    Citation(std::string x) : id(x) {}
    virtual void print(std::ostream &os) = 0;
    virtual void ask() = 0;
    std::string getid()
    {
        return id;
    }
};

class CitBook : public Citation
{
    std::string isbn;

public:
    std::string author{};
    std::string title{};
    std::string p{};
    std::string year{};
    CitBook(std::string x, std::string i) : isbn(i), Citation(x) {}
    void print(std::ostream &os) override
    {
        os << "[" << getid() << "] book: " << author << ", " << title << ", " << p << ", " << year << std::endl;
    }
    void ask() override;
};

class CitWeb : public Citation
{
    std::string url;

public:
    std::string title{"sword"}; //! test
    CitWeb(std::string x, std::string i) : url(i), Citation(x) {}
    void print(std::ostream &os) override
    {
        os << "[" << getid() << "] webpage: " << title << ". Available at " << url << std::endl;
    }
    void ask() override;
};

class CitArt : public Citation
{
    std::string title;
    std::string author;
    std::string journal;
    int year;
    int volume;
    int issue;

public:
    CitArt(std::string x, std::string t, std::string a, std::string j, int y, int v, int i) : title(t), author(a), journal(j), year(y), volume(v), issue(i), Citation(x) {}
    void print(std::ostream& os) override
    {
        os << "[" << getid() << "] article: " << author << ", " << title << ", " << journal << ", " << year << ", " << volume << ", " << issue << std::endl;
    }
    void ask() override{};
};

#endif