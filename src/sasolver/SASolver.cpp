#include "../header/solver.h"
#include <stdexcept>
#include <iostream>
#include <ctime>
#include <random>
#include <cmath>

using namespace std;

SASolver::SASolver() {
    random_engine.seed(time(NULL));
    real_dis.param( uniform_real_distribution<double>::param_type(0.0, 1.0));
}

SASolver::~SASolver() {

}

void SASolver::setEvaluator( Evaluator* evaluator ) {
    if( evaluator == nullptr ) {
        throw invalid_argument( "Evaluator is necessary!\n" ); 
    }
    this->evaluator = evaluator;
}

void SASolver::setAlpha( double alpha ) {
    if( !( alpha >= 0 && alpha < 1 ) ) {
        throw invalid_argument( "Alpha should between 0 to 1!\n" );
    }
    this->alpha = alpha;
}

double SASolver::getAlpha() const {
    return alpha;
}

void SASolver::setBeta( double beta ) {
    if( !( beta > 1 ) ) {
        throw invalid_argument( "Beta should bigger than 1!\n" );
    }
    this->beta = beta;
}

double SASolver::getBeta() const {
    return beta;
}

void SASolver::setMega( double mega ) {
    if( !( mega > 0 ) ) {
        throw invalid_argument( "Mega should bigger than 0!\n" );
    }
    this->mega = mega;
}

double SASolver::getMega() const {
    return mega;
}

void SASolver::setTemper( double temper ) {
    if( !( temper > 0 ) ) {
        throw invalid_argument( "Initial Temper should bigger than 0!\n" );
    }
    this->temper = temper;
}

double SASolver::getTemper() const {
    return temper;
}

void SASolver::setMaxtime( unsigned int times ) {
    maxtime = times;
}

unsigned int SASolver::getMaxtime() const {
    return maxtime;
}

double SASolver::solveProblem( const double& T, const double& a, const double& b, const double& m, const unsigned int& M, Evaluator *e ) {
    setTemper( T );
    setAlpha( a );
    setBeta( b );
    setMega( m );
    setMaxtime( M );
    setEvaluator( e );

    rand_position.param( uniform_int_distribution<int>::param_type( 2, evaluator->getTCount() - 2));
    rand_processor.param( uniform_int_distribution<int>::param_type( 1, evaluator->getPCount() - 1));

    vector<unsigned int> curSs;
    vector<unsigned int> curMs;
    evaluator->getInitStr( curSs, curMs );

    bestSs = curSs;
    bestMs = curMs;

    double curCost, bestCost;
    curCost = bestCost = evaluator->getCost( curSs, curMs );

    unsigned int time = 0;
    // cout << "------Procedure Start------" << endl;
    Heating( curSs, curMs, temper );
    do {
        Metropolis( curSs, curMs, curCost, bestCost, temper, static_cast<unsigned int>(round(mega)) );
        time += static_cast<unsigned int>(round(mega));
        temper = alpha * temper;
        mega = beta * mega;
        // cout << "#" <<time << " Temper:" << temper << " Cost:" << curCost << " Best Cost:" << bestCost << endl;
    } while( time < maxtime );
    return bestCost;
}

void SASolver::Heating( const std::vector<unsigned int>& curSs, const std::vector<unsigned int>& curMs, double& temper ) {
    double deltaCost, curCost, acceptRate;
    unsigned int accept;
    unsigned int tempProcessor;
    vector<unsigned int> newSs = curSs;
    vector<unsigned int> newMs = curMs;

    curCost = evaluator->getCost( newSs, newMs );

    while(true) {
        accept = 0;

        tempProcessor = newMs[1];
        newMs[1] = rand_processor(random_engine);

        deltaCost = evaluator->getCost( newSs, newMs ) - curCost;
        newMs[1] = tempProcessor;

        if( deltaCost < 0 || (real_dis(random_engine) < exp(-deltaCost/temper)) )
            accept++;

        for( int pos = 2; pos < newSs.size() - 1; pos++ ) {
            if( random_engine() % 2 || evaluator->isSuccessor( newSs[pos], newSs[pos-1]) ) {
                tempProcessor = newMs[newSs[pos]];
                newMs[newSs[pos]] = (newMs[newSs[pos]] + rand_processor(random_engine)) % evaluator->getPCount();
                deltaCost = evaluator->getCost( newSs, newMs ) - curCost;
                newMs[newSs[pos]] = tempProcessor;
            } else {
                swap( newSs[pos], newSs[pos-1] );
                deltaCost = evaluator->getCost( newSs, newMs ) - curCost;
                swap( newSs[pos], newSs[pos-1] );
            }

            if( deltaCost < 0 || real_dis(random_engine) < exp(-deltaCost/temper) )
                accept++;
        }
        
        acceptRate = static_cast<double>(accept) / static_cast<double>(newSs.size() - 2);
        // cout << "Heating acceptRate:" << acceptRate << " temper:" << temper << endl;
        if( accept == newSs.size() - 2 ) {
            break;
        } else {
            temper *= 1/(alpha*acceptRate);
        }
    };
}

void SASolver::Metropolis( std::vector<unsigned int> & curSs, std::vector<unsigned int> & curMs, double& curCost, double& bestCost, double temper, unsigned int mega ) {
    // vector<unsigned int> newSs;
    // vector<unsigned int> newMs;
    unsigned int tempPosition, tempProcessor;
    double newCost, deltaCost;
    bool flag;

    do {
        // newSs = curSs;
        // newMs = curMs;
        // getNeighbor( newSs, newMs );
        // newCost = evaluator->getCost( newSs, newMs );

        flag = getNeighbor( curSs, curMs, tempPosition, tempProcessor );
        newCost = evaluator->getCost( curSs, curMs );
        deltaCost = newCost - curCost;

        if( deltaCost < 0.0 ) {
            // curSs = newSs;
            // curMs = newMs;
            curCost = newCost;
            if( newCost < bestCost ) {
                // bestSs = newSs;
                // bestMs = newMs;

                bestSs = curSs;
                bestMs = curMs;
                bestCost = newCost;
            } else {
                if( flag ) {
                    curMs[curSs[tempPosition]] = tempProcessor;
                } else {
                    swap( curSs[tempPosition], curSs[tempPosition-1] );
                }
            }
        } else {
            if( real_dis(random_engine) < exp(-deltaCost/temper) ) {
                // curSs = newSs;
                // curMs = newMs;
                curCost = newCost;
            } else {
                if( flag ) {
                    curMs[curSs[tempPosition]] = tempProcessor;
                } else {
                    swap( curSs[tempPosition], curSs[tempPosition-1] );
                }
            }
        }

        mega--;
    } while( mega > 0 );
}

bool SASolver::getNeighbor( std::vector<unsigned int>& ss, std::vector<unsigned int>& ms, unsigned int& tempPosition, unsigned int& tempProcessor  ) {
    unsigned int pos = rand_position(random_engine);
    tempPosition = pos;
    if( random_engine() % 2 || evaluator->isSuccessor( ss[pos], ss[pos-1]) ) {
        tempProcessor = ms[ss[pos]];
        ms[ss[pos]] = (ms[ss[pos]] + rand_processor(random_engine)) % evaluator->getPCount();
        return true;
    } else {
        swap( ss[pos], ss[pos-1] );
        return false;
    }
}