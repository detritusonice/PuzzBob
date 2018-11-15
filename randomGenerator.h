#ifndef MY_RANDOM_H

#include <random>

class RandomGenerator
{
    std::mt19937* generator;
    std::uniform_int_distribution<int>* dist;

public:
    RandomGenerator(int min, int max, int seed);
    ~RandomGenerator();

    int generateNext();
};

#endif // MY_RANDOM_H
