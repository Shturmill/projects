#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 11

double f(double x) {
    return (12.0 / 13.0) * cos((11.0 / 7.0) * x);
}

void thomas_algorithm(int n, double *a, double *b, double *c, double *d, double *result) {
    double *c_prime = (double *)malloc(n * sizeof(double));
    double *d_prime = (double *)malloc(n * sizeof(double));

    c_prime[0] = c[0] / b[0];
    d_prime[0] = d[0] / b[0];

    for (int i = 1; i < n; ++i) {
        double m = 1.0 / (b[i] - a[i - 1] * c_prime[i - 1]);
        c_prime[i] = (i < n - 1) ? c[i] * m : 0.0;
        d_prime[i] = (d[i] - a[i - 1] * d_prime[i - 1]) * m;
    }

    result[n - 1] = d_prime[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        result[i] = d_prime[i] - c_prime[i] * result[i + 1];
    }

    free(c_prime);
    free(d_prime);
}

typedef struct {
    double a, b, c, d, x0;
} Spline;

Spline *build_spline(int n, double *x, double *y) {
    int i;
    double *h = (double *)malloc(n * sizeof(double));
    for (i = 0; i < n; ++i)
        h[i] = x[i + 1] - x[i];

    double *A = (double *)calloc(n + 1, sizeof(double));
    double *B = (double *)calloc(n + 1, sizeof(double));
    double *C = (double *)calloc(n + 1, sizeof(double));
    double *D = (double *)calloc(n + 1, sizeof(double));

    B[0] = 1.0;
    B[n] = 1.0;

    for (i = 1; i < n; ++i) {
        A[i] = h[i - 1];
        B[i] = 2.0 * (h[i - 1] + h[i]);
        C[i] = h[i];
        D[i] = 3.0 * ((y[i + 1] - y[i]) / h[i] - (y[i] - y[i - 1]) / h[i - 1]);
    }

    double *c_vals = (double *)malloc((n + 1) * sizeof(double));
    thomas_algorithm(n + 1, A, B, C, D, c_vals);

    Spline *splines = (Spline *)malloc(n * sizeof(Spline));
    for (i = 0; i < n; ++i) {
        splines[i].a = y[i];
        splines[i].b = (y[i + 1] - y[i]) / h[i] - h[i] * (c_vals[i + 1] + 2.0 * c_vals[i]) / 3.0;
        splines[i].c = c_vals[i];
        splines[i].d = (c_vals[i + 1] - c_vals[i]) / (3.0 * h[i]);
        splines[i].x0 = x[i];
    }

    free(h); free(A); free(B); free(C); free(D); free(c_vals);
    return splines;
}

double eval_spline(Spline s, double x) {
    double dx = x - s.x0;
    return s.a + dx * (s.b + dx * (s.c + dx * s.d));
}

int main() {
    double a = 1.0, b = 2.5, h_val = 0.3;
    int num_segments = (int)((b - a) / h_val);
    int i, j, points = 100000;
    double step;

    double *nodes = (double *)malloc((num_segments + 1) * sizeof(double));
    double *values = (double *)malloc((num_segments + 1) * sizeof(double));

    for (i = 0; i <= num_segments; ++i) {
        nodes[i] = a + i * h_val;
        values[i] = f(nodes[i]);
    }

    printf("Function values:\n");
    for (i = 0; i <= num_segments; ++i) {
        printf("x = %.5f,\tf(x) = %.5f\n", nodes[i], values[i]);
    }

    Spline *splines = build_spline(num_segments, nodes, values);

    printf("\nCubic splines:\n");
    for (i = 0; i < num_segments; ++i) {
        printf("Interval [%.5f, %.5f]:\n", nodes[i], nodes[i + 1]);
        printf("S%d(x) = %.5f + %.5f(x - %.5f) + %.5f(x - %.5f)^2 + %.5f(x - %.5f)^3\n",
               i, splines[i].a, splines[i].b, splines[i].x0,
               splines[i].c, splines[i].x0, splines[i].d, splines[i].x0);
    }

    printf("\nMaximum interpolation errors:\n");
    for (i = 0; i < num_segments; ++i) {
        double max_err = 0.0;
        step = (nodes[i + 1] - nodes[i]) / points;
        for (j = 0; j <= points; ++j) {
            double x_val = nodes[i] + j * step;
            double spline_val = eval_spline(splines[i], x_val);
            double exact_val = f(x_val);
            double err = fabs(spline_val - exact_val);
            if (err > max_err)
                max_err = err;
        }
        printf("On [%.5f, %.5f] max error: %e\n", nodes[i], nodes[i + 1], max_err);
    }

    free(nodes); free(values); free(splines);
    return 0;
}
