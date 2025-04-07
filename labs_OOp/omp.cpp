#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <omp.h>

using namespace std;

void matrixMultiply(const vector<double>& A, const vector<double>& B, vector<double>& C, int N) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            double sum = 0.0;
            for (int k = 0; k < N; k++) {
                sum += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
        }
    }
}

void matrixAdd(vector<double>& A, const vector<double>& B, int N) {
    #pragma omp parallel for
    for (int i = 0; i < N * N; i++) {
        A[i] += B[i];
    }
}

void scaleMatrix(vector<double>& A, double factor, int N) {
    #pragma omp parallel for
    for (int i = 0; i < N * N; i++) {
        A[i] *= factor;
    }
}

double matrixNorm(const vector<double>& A, int N) {
    double norm = 0.0;
    #pragma omp parallel for reduction(+:norm)
    for (int i = 0; i < N * N; i++) {
        norm += fabs(A[i]);
    }
    return norm;
}

int main() {
    int N;
    cout << "Enter the size of the square matrix: ";
    cin >> N;

    srand(static_cast<unsigned int>(time(NULL)));
    vector<double> A(N * N);

    for (int i = 0; i < N * N; i++) {
        A[i] = (rand() % 10) + 1;
        cout << A[i] << " ";
        if ((i + 1) % N == 0) {
            cout << "\n";
        }
    }

    vector<double> expA(N * N, 0.0), term(N * N, 0.0);

    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        expA[i * N + i] = 1.0;
        term[i * N + i] = 1.0;
    }

    const int max_iter = 50;
    const double tol = 1e-6;

    
    for (int n = 1; n <= max_iter; n++) {
        vector<double> temp(N * N, 0.0);
        matrixMultiply(A, term, temp, N);
        scaleMatrix(temp, 1.0 / n, N);
        term = temp;          
        matrixAdd(expA, term, N);
        if (matrixNorm(term, N) < tol) {
            cout << "Converged at iteration " << n << endl;
            break;
        }
    }

    cout << "Matrix exponential (exp(A)):" << endl;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            cout << expA[i * N + j] << " ";
        }
        cout << endl;
    }
    return 0;
}