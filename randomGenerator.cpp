#include "randomGenerator.h"

RandomGenerator::RandomGenerator(int rangeMin, int rangeMax, int seed)
{
    generator = new std::mt19937(seed);
    dist = new std::uniform_int_distribution<int>(rangeMin, rangeMax);
}

RandomGenerator::~RandomGenerator()
{
    delete dist;
    delete generator;
}

int RandomGenerator::generateNext()
{
    int val = (*dist)(*generator);
    return val;
}

