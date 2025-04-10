#ifndef LAB2_H
#define LAB2_H

#include <string>

class PiCalculator {
private:
    int iterations;
    double pi_value;

public:
    PiCalculator(int iter);
    void calculate();
    double getPi() const;
    std::string getResultString() const;
};

#endif // LAB2_H