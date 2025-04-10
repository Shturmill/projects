#include "lab2.h"
#include <iomanip>
#include <sstream>
#include <cmath>
#include <omp.h>

PiCalculator::PiCalculator(int iter) : iterations(iter), pi_value(0.0) {}

void PiCalculator::calculate() {
    double sum = 0.0;

    #pragma omp parallel for reduction(+:sum)
    for (int k = 0; k < iterations; ++k) {
        double term = (1.0 / pow(16, k)) * 
                      (4.0 / (8 * k + 1) - 2.0 / (8 * k + 4) - 
                       1.0 / (8 * k + 5) - 1.0 / (8 * k + 6));
        sum += term;
    }

    pi_value = sum;
}

double PiCalculator::getPi() const {
    return pi_value;
}

std::string PiCalculator::getResultString() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(100) << "value of π: " << pi_value;
    return oss.str();
}