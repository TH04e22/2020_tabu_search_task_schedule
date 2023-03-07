#pragma once
#include "evaluator.h"
#include <vector>
#include <random>

/*
排程字串擾動方式為task i往前插入task j，如果操作不允許則執行匹配字串擾動。
匹配字串擾動方式為隨機抽取處理器但不跟現在匹配的處理器一樣。
Tabu List:
* 排程字串的tabu list紀錄被插入的task j，task j不可再往前插入n個回合。
* 匹配字串的tabu list則是task j不可以在使用被替換掉的處理器持續n個回合。

Tabu List 更新規則: 
當擾動排程字串時，更新排程字串的tabu list;
當擾動匹配字串時，更新匹配字串的tabu list。

Aspiration Level:
COST(S) < COST(S*)

*/
class TabuSearch {
public:
    TabuSearch();
    ~TabuSearch();
    double searchSolution( unsigned int maxIter, float insertFactor, float changeFactor, float neiborFactor, Evaluator* evaluator, bool keepRecord, unsigned int recordSteps );
private:
    std::vector<unsigned int> bestMs;
    std::vector<unsigned int> bestSs;
    std::shuffle_order_engine<std::default_random_engine, 256> random_engine;
    std::uniform_int_distribution<> rand_processor;
};