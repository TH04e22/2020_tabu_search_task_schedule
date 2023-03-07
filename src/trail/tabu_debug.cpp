#include "../include/TabuSearch.h"
#include "../include/evaluator.h"
#include <iostream>
using namespace std;

int main() {
    double cost;
    string file[4] = { "n4_00.dag", "n4_02.dag", "n4_04.dag", "n4_06.dag"};
    for( int i = 0; i < 4; i++ ) {
        Evaluator evaluator( file[i] );
        TabuSearch solver;
        cost = solver.searchSolution( 5000, 1.5, 0.05, 0.25, &evaluator, true, 1 );
        cout << cost << endl;
    }
    
    return 0;
}