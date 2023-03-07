#pragma once
#include <string>
#include <vector>
#include <bitset>
#include <forward_list>

struct TransData {
    unsigned int toTask;
    double commCost;

    TransData( unsigned int Task, double Cost) {
        toTask = Task;
        commCost = Cost;
    };

    ~TransData() {

    };
};

class Evaluator {
public:
    Evaluator();
    Evaluator( std::string const& filename );
    ~Evaluator();
    unsigned int getPCount() const { return thePCount; }; // 獲取處理器個數
    unsigned int getTCount() const { return theTCount; }; // 獲取工作個數
    std::string getFileName() const { return filename; }; // 獲取開啟的檔案名稱
    void getInitStr( std::vector<unsigned int> &ss, std::vector<unsigned int> &ms ) const; // 獲得初始字串
    bool isSuccessor( unsigned int i, unsigned int j ) const; // 工作i是否是工作j的後繼者
    double getCost( const std::vector<unsigned int> &ss, const std::vector<unsigned int> &ms ); // ms : 匹配字串 ss : 排程字串
    void readConfig( std::string const& filename );
    void printConfig() const;
    void printTaskExecTime() const;
private:
    unsigned int thePCount;
    unsigned int theTCount;
    unsigned int theECount;
    std::string filename;
    std::vector<std::forward_list<TransData>> theTransDataVol;
    std::vector<std::vector<unsigned char>> taskPredecessor;
    std::vector<std::vector<double>> theCompCost;
    std::vector<std::vector<double>> theCommRate;
    std::vector<double> st;
    std::vector<double> ft;
    std::vector<double> p_ft;
    std::vector<unsigned int> initStr;
};
