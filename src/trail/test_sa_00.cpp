#include "evaluator.h"
#include "../include/SASolver.h"
#include <iostream>
#include <fstream>
#include <ctime>

using namespace std;

int main() {
    int trails = 10, maxtimes = 100;
    double cost = 0, curCost, bestCost = 1000, avgCost, avgTime;
    clock_t start, end, total = 0;
    string filename = "..\\resource\\n4_00.dag";
    string logFileName = "..\\resource\\maxtimeTest2500_10t.csv";

    ofstream logFile( logFileName, ios::out | ios::trunc );
    Evaluator evaluator( filename );
    SASolver solver;

    if( logFile.is_open() ) {
        logFile << "times, avgCost, bestCost, avgTime" << endl;
        do {
            cost = 0;
            bestCost = 1000;
            total = 0;
            for( int i = 0; i < trails; i++ ) {
                start = clock();
                curCost = solver.solveProblem( 1, 0.98, 1.005, 1, maxtimes, &evaluator );
                end = clock();
                total += (end-start);
                cost += curCost;
                if( curCost < bestCost )
                    bestCost = curCost;
            }
            avgCost = cost / static_cast<double>(trails);
            avgTime = static_cast<double>(total) / static_cast<double>(trails);
            logFile << maxtimes << "," << avgCost << "," << bestCost << "," << avgTime << endl;
            maxtimes += 50;
        } while( maxtimes <= 2500);
    } else {
        throw runtime_error( "The log file can't open");
    }
    return 0;
}