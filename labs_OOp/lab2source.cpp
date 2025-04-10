#include <iostream>
#include <iomanip>
#include <cmath>
#include <omp.h>

using namespace std;

class PiCalculator {
private:
    int iterations;
    double pi_value;

public:
    PiCalculator(int iter) : iterations(iter), pi_value(0.0) {}

    void calculate() {
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

    double getPi() const {
        return pi_value;
    }

    void printResult() const {
        cout << fixed << setprecision(100) << "value of π: " << pi_value << endl;
    }
};

int main() {
    int iterations;
    cout << "Enter the count of iterations: ";
    cin >> iterations;

    PiCalculator calculator(iterations);
    calculator.calculate();
    calculator.printResult();

    return 0;
}
