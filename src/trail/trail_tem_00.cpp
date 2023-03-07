#include "../include/TabuSearch.h"
#include "../include/evaluator.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <ctime>
#include <cmath>
using namespace std;

int main() {
    string file[4] = { "n4_00.dag", "n4_02.dag", "n4_04.dag", "n4_06.dag"};
    
    const unsigned int trails = 5;
    clock_t start, end, totalTime;
    double cost, sum, bestCost, worstCost, avgCost, avgTime, result[trails], sd;
    Evaluator evaluator;
    TabuSearch solver;

    cout << setw(10) << "filename" << setw(10) << "Best" << setw(10) << "Worst"
    << setw(10) << "Avg." << setw(10) << "sd" << setw(10) << "Avg.time" << endl;
    for( int i = 0; i < 4; i++ ) {
        sum = 0;
        bestCost = 1000;
        worstCost = 0;
        totalTime = 0;
        for( int j = 0; j < trails; j++ ) {
            start = clock();
            evaluator.readConfig( file[i] );
            cost = solver.searchSolution( 5000, 1.5, 0.05, 0.25, &evaluator, false, 0 );
            end = clock();
            result[j] = cost;
            totalTime += (end-start);
            sum += cost;
            if( cost < bestCost )
                bestCost = cost;

            if( cost > worstCost )
                worstCost = cost;
        }
        avgCost = sum / static_cast<double>(trails);
        avgTime = static_cast<double>(totalTime) / static_cast<double>(trails);

        for( int j = 0; j < trails; j++ ) {
            sd += ( result[j] - avgCost) * ( result[j] - avgCost);
        }

        sd = sqrt( sd/ trails );
        if( sd < 0.00001 )
            sd = 0;
        cout << setw(10) << file[i] << setw(10) << bestCost << setw(10) << worstCost
        << setw(10) << avgCost << setw(10) << sd << setw(10) << avgTime/1000.0 << endl;
    }
    return 0;
}