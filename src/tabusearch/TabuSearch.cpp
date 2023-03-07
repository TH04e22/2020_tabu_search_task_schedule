#include "../include/TabuSearch.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <ctime>
#include <random>
#include <algorithm>
#include <array>
#include <forward_list>
#include <cmath>

using namespace std;

enum action {
    insert, change
};

struct opInfo {
    double cost; // 花費
    unsigned int position, task, proccessor; // 於字串中的位置(回復用)，工作，處理器
    action op; // 操作
    bool isTabu;
};

bool compare( const opInfo& a, const opInfo& b ) {
    if( a.cost < b.cost )
        return true;
    return false;
}

bool condition( const opInfo& a ) {
    return a.isTabu;
}

TabuSearch::TabuSearch() {
    random_engine.seed(time(NULL));
}

TabuSearch::~TabuSearch(){

}

double TabuSearch::searchSolution( unsigned int maxIter, float insertFactor, float changeFactor, float neiborFactor, Evaluator* evaluator, bool keepRecord, unsigned int recordSteps ) {
    if( 0 > insertFactor ) {
        throw invalid_argument( "insertFactor should be positive" );
    }

    if( 0 > changeFactor ) {
        throw invalid_argument( "changeFactor should be positive" );
    }

    if( 0 > neiborFactor || neiborFactor > 1 ) {
        throw invalid_argument( "neiborFactor should between 0 to 1" );
    }

    if( evaluator == nullptr ) {
        throw invalid_argument( "Must give a evaluator" );
    }

    // 記錄檔設定
    string logFile;
    fstream outFstream;
    if( keepRecord ) {
        const string fileName = evaluator->getFileName();
        logFile += "log/tabu_";
        logFile += fileName.substr(0, fileName.find('.'));
        logFile += "_";
        time_t t = time(0);
        tm* now = localtime(&t);
        logFile += to_string( now->tm_year + 1900 );
        logFile += to_string( now->tm_mon + 1 );
        logFile += to_string( now->tm_mday );
        logFile += to_string( now->tm_hour );
        logFile += to_string( now->tm_min );
        logFile += to_string( now->tm_sec );
        logFile += ".csv";
        outFstream.open( logFile, ios::out );
        if( !outFstream.is_open() ) {
            throw runtime_error("Fatal error: Can't open the file!" );
        } else {
            outFstream << "it, curCost, bestCost\n";
        }
    }

    unsigned int Iter = 0;
    double curCost, bestCost;
    const size_t spaceSize = 2*evaluator->getTCount() - 4; // 可以進行擾動的位置大小( insert: t - 2 + change: t - 2 )
    const size_t neiborSize = static_cast<size_t>(neiborFactor * spaceSize ); // 鄰居大小
    const unsigned int insertMaxRounds = static_cast<unsigned int>(insertFactor * (evaluator->getTCount()-2)); // insert tabu list的長度
    const unsigned int changeMaxRounds = static_cast<unsigned int>(changeFactor * (evaluator->getTCount()-2) * evaluator->getPCount()); // change tabu list的長度

    bool existAdmissSol; // 是否存在可接受的解
    vector<unsigned int> sampleArray( spaceSize ); // 隨機決定那些鄰居要進鄰居池
    vector<opInfo> neibors( neiborSize ); // 鄰居池

    vector<unsigned int> curSs; // 排序字串
    vector<unsigned int> curMs; // 匹配字串
    vector<unsigned int> insertTabu( evaluator->getTCount(), 0 ); // 插入的tabu list
    forward_list<unsigned int> insert_track; // 用於更新tabu list陣列的list

    rand_processor.param( uniform_int_distribution<int>::param_type( 1, evaluator->getPCount() - 1)); // 隨機處理器，但不與之前的相同
    array<unsigned int, 2> change_tuple; // 紀錄(task, processor)的數值
    vector<vector<unsigned int>> changeTabu; // 改變處理器的tabu list
    forward_list<array<unsigned int,2>> change_track; // 用於更新tabu list陣列的list

    /* 初始設定 */
    evaluator->getInitStr( curSs, curMs );
    bestSs = curSs;
    bestMs = curMs;
    opInfo admissibleMove;

    curCost = bestCost = evaluator->getCost( curSs, curMs );
    
    for( int i = 0; i < sampleArray.size(); i++ )
        sampleArray[i] = i+1;

    changeTabu.resize( evaluator->getTCount() );
    for( int t = 0; t < evaluator->getTCount(); t++ )
        changeTabu[t].resize( evaluator->getPCount(), 0 );

    while( Iter++ < maxIter ) {
        existAdmissSol = false;

        /* create a candidate list */
        shuffle( sampleArray.begin(), sampleArray.end(), random_engine );
        
        /* evaluate each candidate move */
        size_t i, j;
        for( i = 0, j = 0; !existAdmissSol && ( j + (sampleArray.size() - i + 1) >= neiborSize ); i++ ) { // shift window to extends solution neibors
            if( sampleArray[i] < evaluator->getTCount() - 1 ) {
                if( !evaluator->isSuccessor(curSs[sampleArray[i]],curSs[sampleArray[i]-1])) {
                    neibors[j].op = insert;
                    neibors[j].position = sampleArray[i]; // 用於之後回復用
                    neibors[j].task = curSs[sampleArray[i]-1]; // 被插入的工作
                    neibors[j].proccessor = -1;
                    neibors[j].isTabu = false;
                    if( insertTabu[curSs[sampleArray[i]]] != 0 )
                        neibors[j].isTabu = true;
                    swap( curSs[sampleArray[i]], curSs[sampleArray[i]-1] );
                    neibors[j].cost = evaluator->getCost( curSs, curMs );
                    swap( curSs[sampleArray[i]], curSs[sampleArray[i]-1] ); // recover
                    j++;
                }
            } else {
                const unsigned int msPos = sampleArray[i]- evaluator->getTCount() + 2;
                neibors[j].op = change;
                neibors[j].task = msPos;
                neibors[j].proccessor = curMs[msPos]; // used as a temp
                curMs[msPos] = ( curMs[msPos] + rand_processor(random_engine)) % evaluator->getPCount();
                neibors[j].cost = evaluator->getCost( curSs, curMs );
                swap( neibors[j].proccessor, curMs[msPos] ); // recover
                neibors[j].isTabu = false;
                if( changeTabu[msPos][neibors[j].proccessor] != 0 )
                    neibors[j].isTabu = true;
                j++;
            }
            
            if( j == neiborSize ) {
                vector<opInfo>::iterator it = partition( neibors.begin(), neibors.end(), condition );
                if( distance( it, neibors.end() ) != 0 ) { // 並不是所有都是在tabu裡
                    opInfo tabuMove = *min_element( neibors.begin(), it, compare );
                    opInfo normalMove = *min_element( it, neibors.end(), compare );
                    if( tabuMove.cost < normalMove.cost && tabuMove.cost < bestCost ) { // tabu list 失效
                        admissibleMove = tabuMove;
                    } else {
                        admissibleMove = normalMove;
                    }
                    existAdmissSol = true;
                } else { // 全部都是tabu
                    opInfo move = *min_element( neibors.begin(), neibors.end(), compare );
                    if( move.cost < bestCost ) { // tabu list 失效
                        admissibleMove = move;
                        existAdmissSol = true;
                    } else // 擴充鄰居池
                        j = 0;
                }
            }
        }
        
        /* update tabu list */
        auto pre_insert_it = insert_track.before_begin(); 
        for( auto insert_tabu_it = insert_track.begin(); insert_tabu_it != insert_track.end(); ) {
            if( --insertTabu[*insert_tabu_it] == 0 ) {
                insert_tabu_it = next(insert_tabu_it);
                insert_track.erase_after(pre_insert_it);
            } else {
                pre_insert_it = insert_tabu_it;
                insert_tabu_it = next(insert_tabu_it);
            }
        }

        auto pre_change_it = change_track.before_begin(); 
        for( auto change_tabu_it = change_track.begin(); change_tabu_it  != change_track.end(); ) {
            unsigned int t = (*change_tabu_it)[0];
            unsigned int p = (*change_tabu_it)[1];
            if( --changeTabu[t][p] == 0 ) {
                change_tabu_it = next(change_tabu_it);
                change_track.erase_after(pre_change_it);
            } else {
                pre_change_it = change_tabu_it;
                change_tabu_it = next(change_tabu_it);
            }
        }
        

        /* update admissibility conditions */
        if( existAdmissSol ) {
            curCost = admissibleMove.cost;

            if( admissibleMove.op == insert ) {
                swap( curSs[admissibleMove.position], curSs[admissibleMove.position-1]);
                if( insertTabu[curSs[admissibleMove.position-1]] == 0 ) {
                    insert_track.push_front( curSs[admissibleMove.position-1] );
                    insertTabu[curSs[admissibleMove.position-1]] = insertMaxRounds;
                }
            } else {
                array<unsigned int, 2> temp{ admissibleMove.task, curMs[admissibleMove.task] };
                curMs[admissibleMove.task] = admissibleMove.proccessor;
                if( changeTabu[admissibleMove.task][temp[1]] == 0 ) {
                    change_track.push_front( temp );
                }
                changeTabu[admissibleMove.task][temp[1]] = changeMaxRounds;
            }
        
            if( admissibleMove.cost < bestCost ) {
                bestCost = curCost;
                bestSs = curSs;
                bestMs = curMs;
            }
        }

        if( keepRecord && Iter % recordSteps == 0 )
            outFstream << Iter << "," << curCost << "," << bestCost << endl;
    };

    if( keepRecord )
        outFstream.close();

    return bestCost;
}