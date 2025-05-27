#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h> // Для perror

// Функция для интерполяции (выбирается согласно варианту, стр. 31)
double f(double x) {
    return (12.0 / 13.0) * cos((11.0 / 7.0) * x);
}

// Вспомогательная функция для проверки выделения памяти
void* checked_malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void* checked_calloc(size_t num, size_t size) {
    void *ptr = calloc(num, size);
    if (ptr == NULL) {
        perror("Failed to allocate memory with calloc");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

// Алгоритм Томаса (метод прогонки) для решения СЛАУ (см. раздел 2.1.3, стр. 27)
void thomas_algorithm(int n, double *a, double *b, double *c, double *d, double *result) {
    double *c_prime = (double *)checked_malloc(n * sizeof(double));
    double *d_prime = (double *)checked_malloc(n * sizeof(double));

    // Прямой ход
    c_prime[0] = c[0] / b[0];
    d_prime[0] = d[0] / b[0];

    for (int i = 1; i < n; ++i) {
        double m = 1.0 / (b[i] - a[i - 1] * c_prime[i - 1]);
        c_prime[i] = (i < n - 1) ? c[i] * m : 0.0;
        d_prime[i] = (d[i] - a[i - 1] * d_prime[i - 1]) * m;
    }

    // Обратный ход
    result[n - 1] = d_prime[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        result[i] = d_prime[i] - c_prime[i] * result[i + 1];
    }

    free(c_prime);
    free(d_prime);
}

// Структура для хранения коэффициентов сплайна (см. формулу 2.1, стр. 24)
typedef struct {
    double a, b, c, d, x0;
} Spline;

// Функция построения сплайна (см. раздел 2.1.2, стр. 23-25)
Spline *build_spline(int n_segments, double *x, double *y) {
    int n = n_segments;
    int i;
    double *h = (double *)checked_malloc(n * sizeof(double));
    for (i = 0; i < n; ++i)
        h[i] = x[i + 1] - x[i];

    // n+1 узлов -> n+1 уравнений для c_i
    double *A = (double *)checked_calloc(n + 1, sizeof(double));
    double *B = (double *)checked_calloc(n + 1, sizeof(double));
    double *C = (double *)checked_calloc(n + 1, sizeof(double));
    double *D = (double *)checked_calloc(n + 1, sizeof(double));

    // Натуральные граничные условия c0 = 0, cn = 0 (см. 2.5, стр. 24)
    B[0] = 1.0;
    B[n] = 1.0;

    // Формирование системы (2.17), стр. 25
    for (i = 1; i < n; ++i) {
        A[i] = h[i - 1]; // Соответствует h_i в (2.17)
        B[i] = 2.0 * (h[i - 1] + h[i]); // Соответствует 2(h_i + h_{i+1}) в (2.17)
        C[i] = h[i]; // Соответствует h_{i+1} в (2.17)
        D[i] = 3.0 * ((y[i + 1] - y[i]) / h[i] - (y[i] - y[i - 1]) / h[i - 1]); // Правая часть (2.17)
    }

    double *c_vals = (double *)checked_malloc((n + 1) * sizeof(double));
    thomas_algorithm(n + 1, A, B, C, D, c_vals); // Решаем систему для c_i

    Spline *splines = (Spline *)checked_malloc(n * sizeof(Spline));
    for (i = 0; i < n; ++i) {
        splines[i].a = y[i]; // ai = y_i (или y_{i-1} в методичке, зависит от индексации)
        // Формула (2.15), стр. 25
        splines[i].b = (y[i + 1] - y[i]) / h[i] - h[i] * (c_vals[i + 1] + 2.0 * c_vals[i]) / 3.0;
        splines[i].c = c_vals[i]; // c_i
        // Формула (2.14), стр. 25
        splines[i].d = (c_vals[i + 1] - c_vals[i]) / (3.0 * h[i]);
        splines[i].x0 = x[i]; // Узел начала сегмента
    }

    free(h); free(A); free(B); free(C); free(D); free(c_vals);
    return splines;
}

// Функция вычисления значения сплайна в точке x
double eval_spline(Spline s, double x) {
    double dx = x - s.x0;
    return s.a + dx * (s.b + dx * (s.c + dx * s.d));
}

int main() {
    // Параметры из задания (2.2, стр. 30)
    double a = 1.0, b = 2.5; // Отрезок [a, b]
    int num_segments = 5; // 5 отрезков -> 6 узлов (согласно п. 2.2.2)
    double h_val = (b - a) / num_segments; // Шаг h = 0.3 (согласно п. 2.2.2, h <= 0.3)

    int num_nodes = num_segments + 1;
    int i, j;

    double *nodes = (double *)checked_malloc(num_nodes * sizeof(double));
    double *values = (double *)checked_malloc(num_nodes * sizeof(double));

    // Табулирование функции (пункт 2.2.2) [cite: 224]
    printf("Tabulating function on [%.2f, %.2f] with h = %.2f:\n", a, b, h_val);
    for (i = 0; i < num_nodes; ++i) {
        nodes[i] = a + i * h_val;
        values[i] = f(nodes[i]);
        printf("x = %.5f,\tf(x) = %.5f\n", nodes[i], values[i]);
    }

    // Построение сплайнов (пункт 2.2.3, 2.2.4) [cite: 225, 226]
    Spline *splines = build_spline(num_segments, nodes, values);

    printf("\nCubic splines (пункт 2.2.4):\n");
    for (i = 0; i < num_segments; ++i) {
        printf("Interval [%.5f, %.5f]:\n", nodes[i], nodes[i + 1]);
        printf("S%d(x) = %.5f + %.5f(x - %.5f) + %.5f(x - %.5f)^2 + %.5f(x - %.5f)^3\n",
               i, splines[i].a, splines[i].b, splines[i].x0,
               splines[i].c, splines[i].x0, splines[i].d, splines[i].x0);
    }

    // Оценка погрешности (пункт 2.2.6, стр. 31) [cite: 228]
    printf("\nMaximum interpolation errors (Формула 2.28, стр. 30 [cite: 217]):\n");
    int error_points_per_segment = 10000; // 10000 точек - хороший компромисс

    for (i = 0; i < num_segments; ++i) {
        double max_err = 0.0;
        double step = (nodes[i + 1] - nodes[i]) / error_points_per_segment;
        for (j = 0; j <= error_points_per_segment; ++j) {
            double x_val = nodes[i] + j * step;
            double spline_val = eval_spline(splines[i], x_val);
            double exact_val = f(x_val);
            double err = fabs(spline_val - exact_val);
            if (err > max_err)
                max_err = err;
        }
        printf("On [%.5f, %.5f] max error: %e\n", nodes[i], nodes[i + 1], max_err);
    }

    // --- Генерация данных для графика (пункт 2.2.5, стр. 30) [cite: 227] ---
    FILE *plot_file = fopen("plot_data.dat", "w");
    FILE *nodes_file = fopen("nodes.dat", "w");

    if (plot_file == NULL || nodes_file == NULL) {
        perror("Error opening output files");
        free(nodes); free(values); free(splines);
        if(plot_file) fclose(plot_file);
        if(nodes_file) fclose(nodes_file);
        return 1;
    }

    fprintf(plot_file, "# x\tAnalytical\tSpline\n");
    int plot_points_total = 1000;
    double plot_step = (b - a) / plot_points_total;
    int seg_idx = 0;

    for (j = 0; j <= plot_points_total; ++j) {
        double x_val = a + j * plot_step;
        if (x_val > b) x_val = b;

        while (seg_idx < num_segments - 1 && x_val > nodes[seg_idx + 1]) {
            seg_idx++;
        }

        double spline_val = eval_spline(splines[seg_idx], x_val);
        double exact_val = f(x_val);
        fprintf(plot_file, "%.6f\t%.6f\t%.6f\n", x_val, exact_val, spline_val);
    }

    fprintf(nodes_file, "# x\tf(x)\n");
    for (i = 0; i < num_nodes; ++i) {
        fprintf(nodes_file, "%.6f\t%.6f\n", nodes[i], values[i]);
    }

    fclose(plot_file);
    fclose(nodes_file);
    printf("\nPlot data saved to 'plot_data.dat' and 'nodes.dat'.\n");
    printf("Use gnuplot or other software to plot these files.\n");
    // --- Конец генерации данных ---

    free(nodes); free(values); free(splines);
    return 0;
}