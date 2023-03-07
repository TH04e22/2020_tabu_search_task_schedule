#pragma once
#include "evaluator.h"
#include <vector>
#include <random>

class SASolver {
public:
    SASolver();
    ~SASolver();
    double solveProblem( const double& T, const double& a, const double& b, const double& m, const unsigned int& M, Evaluator *e );
    void setEvaluator( Evaluator* evaluator );
    void setAlpha( double alpha );
    double getAlpha() const;
    void setBeta( double beta );
    double getBeta() const;
    void setMega( double mega );
    double getMega() const;
    void setTemper( double temper );
    double getTemper() const;
    void setMaxtime( unsigned int times );
    unsigned int getMaxtime() const;
private:
    void Heating( const std::vector<unsigned int> & curSs, const std::vector<unsigned int> & curMs, double& temper );
    void Metropolis( std::vector<unsigned int> & curSs, std::vector<unsigned int> & curMs, double& curCost, double& bestCost, double temper, unsigned int mega );
    bool getNeighbor( std::vector<unsigned int>& ss, std::vector<unsigned int>& ms, unsigned int& tempPosition, unsigned int& tempProcessor );
    double temper;
    double alpha;
    double beta;
    double mega;
    unsigned int maxtime;
    std::vector<unsigned int> bestMs;
    std::vector<unsigned int> bestSs;
    Evaluator* evaluator;
    std::shuffle_order_engine<std::default_random_engine, 256> random_engine;
    std::uniform_int_distribution<> rand_position;
    std::uniform_int_distribution<> rand_processor;
    std::uniform_real_distribution<> real_dis;
};