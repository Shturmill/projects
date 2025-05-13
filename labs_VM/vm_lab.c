#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    double *x;
    double *y;
    int n;
} DataPoints;

typedef struct {
    int n;
    double Sx;
    double Sy;
    double Sxx;
    double Sxy;
    double Sxxx;
    double Sxxxx;
    double Sxxy;
} Sums;

double **create_matrix(int rows, int cols) {
    double **matrix = (double **)malloc(rows * sizeof(double *));
    if (matrix == NULL) {
        perror("Не удалось выделить память для строк матрицы");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < rows; i++) {
        matrix[i] = (double *)malloc(cols * sizeof(double));
        if (matrix[i] == NULL) {
            perror("Не удалось выделить память для столбцов матрицы");
            for (int k = 0; k < i; k++) {
                free(matrix[k]);
            }
            free(matrix);
            exit(EXIT_FAILURE);
        }
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = 0.0;
        }
    }
    return matrix;
}

void free_matrix(double **matrix, int rows) {
    if (matrix == NULL) return;
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void print_vector(const double *vec, int n, const char *name) {
    printf("%s:\n", name);
    for (int i = 0; i < n; i++) {
        printf("%.6f ", vec[i]);
    }
    printf("\n");
}

void print_matrix(double **matrix, int rows, int cols, const char *name) {
    printf("%s:\n", name);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%.6f\t", matrix[i][j]);
        }
        printf("\n");
    }
}

double **copy_matrix(double **src, int rows, int cols) {
    double **dest = create_matrix(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            dest[i][j] = src[i][j];
        }
    }
    return dest;
}

double *copy_vector(const double *src, int n) {
    double *dest = (double *)malloc(n * sizeof(double));
    if (dest == NULL) {
        perror("Не удалось выделить память для копии вектора");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; i++) {
        dest[i] = src[i];
    }
    return dest;
}


DataPoints read_data(const char *filename) {
    FILE *data_file;
    char line[256];
    int count = 0;

    data_file = fopen(filename, "r");
    if (data_file == NULL) {
        perror("Ошибка открытия файла для подсчета строк");
        exit(EXIT_FAILURE);
    }
    while (fgets(line, sizeof(line), data_file)) {
        if (strchr(line, ',') != NULL) {
            count++;
        }
    }
    fclose(data_file);

    if (count == 0) {
        fprintf(stderr, "Точки данных не найдены в %s\n", filename);
        DataPoints dp_empty = {NULL, NULL, 0};
        return dp_empty;
    }

    DataPoints dp;
    dp.n = count;
    dp.x = (double *)malloc(count * sizeof(double));
    dp.y = (double *)malloc(count * sizeof(double));

    if (dp.x == NULL || dp.y == NULL) {
        perror("Ошибка выделения памяти для точек данных");
        exit(EXIT_FAILURE);
    }

    data_file = fopen(filename, "r");
    if (data_file == NULL) {
        perror("Ошибка открытия файла для чтения");
        free(dp.x);
        free(dp.y);
        exit(EXIT_FAILURE);
    }

    int i = 0;
    while (fgets(line, sizeof(line), data_file) && i < count) {
        char *token_x, *token_y;
        token_x = strtok(line, ",");
        if (token_x != NULL) {
            token_y = strtok(NULL, ",\n\r");
            if (token_y != NULL) {
                dp.x[i] = atof(token_x);
                dp.y[i] = atof(token_y);
                i++;
            }
        }
    }
    fclose(data_file);

    return dp;
}

Sums compute_sums(const DataPoints *dp) {
    Sums s = {0};
    s.n = dp->n;
    for (int i = 0; i < dp->n; i++) {
        double x_val = dp->x[i];
        double y_val = dp->y[i];
        double x_sq = x_val * x_val;
        double x_cub = x_sq * x_val;
        double x_quad = x_cub * x_val;

        s.Sx += x_val;
        s.Sy += y_val;
        s.Sxx += x_sq;
        s.Sxy += x_val * y_val;
        s.Sxxx += x_cub;
        s.Sxxxx += x_quad;
        s.Sxxy += x_sq * y_val;
    }
    return s;
}

// Функция для метода Гаусса
// A_orig и b_orig не изменяются
void gauss_elimination(double **A_orig, const double *b_orig, double *x_sol, int n, double ***A_ext_out) {
    // Создаем копии, чтобы не изменять оригинальные A и b
    double **A = copy_matrix(A_orig, n, n);
    double *b = copy_vector(b_orig, n);
    double **A_ext = create_matrix(n, n + 1);

    // Создание расширенной матрицы [A|b]
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            A_ext[i][j] = A[i][j];
        }
        A_ext[i][n] = b[i];
    }

    // Прямой ход
    for (int i = 0; i < n; i++) {
        int max_row = i;
        for (int k = i + 1; k < n; k++) {
            if (fabs(A_ext[k][i]) > fabs(A_ext[max_row][i])) {
                max_row = k;
            }
        }
        if (max_row != i) {
            double *temp_row = A_ext[i];
            A_ext[i] = A_ext[max_row];
            A_ext[max_row] = temp_row;
        }

        for (int j = i + 1; j < n; j++) {
            double factor = A_ext[j][i] / A_ext[i][i];
            for (int k = i; k < n + 1; k++) {
                A_ext[j][k] = A_ext[j][k] - factor * A_ext[i][k];
            }
        }
    }

    // Обратный ход (обратная подстановка)
    for (int i = n - 1; i >= 0; i--) {
        x_sol[i] = A_ext[i][n];
        for (int j = i + 1; j < n; j++) {
            x_sol[i] = x_sol[i] - A_ext[i][j] * x_sol[j];
        }
        x_sol[i] = x_sol[i] / A_ext[i][i];
    }

    if (A_ext_out) {
        *A_ext_out = A_ext;
    } else {
        free_matrix(A_ext, n);
    }
    free_matrix(A, n);
    free(b);
}


// Функция для LU-разложения и решения Ax = b
// A_orig и b_orig не изменяются.
// L_out и U_out будут хранить матрицы L и U.
void lu_decomposition_solve(double **A_orig, const double *b_orig, double *x_sol, int n, double ***L_out, double ***U_out) {
    double **A = copy_matrix(A_orig, n, n);
    double *b = copy_vector(b_orig, n);
    
    double **L = create_matrix(n, n);
    double **U = create_matrix(n, n);
    double *y = (double *)malloc(n * sizeof(double));

    if (y == NULL) {
        perror("Не удалось выделить память для вектора y в LU-разложении");
        free_matrix(A, n); free(b); free_matrix(L, n); free_matrix(U, n);
        exit(EXIT_FAILURE);
    }

    // LU-разложение
    for (int i = 0; i < n; i++) {
        L[i][i] = 1.0;

        // Верхний треугольник U
        for (int j = i; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < i; k++) {
                sum += L[i][k] * U[k][j];
            }
            U[i][j] = A[i][j] - sum;
        }

        // Нижний треугольник L
        for (int j = i + 1; j < n; j++) {
            if (fabs(U[i][i]) < 1e-12) {
                fprintf(stderr, "Ошибка: Деление на ноль в LU-разложении (U[%d][%d] равно нулю).\n", i, i);
                for(int k=0; k<n; ++k) x_sol[k] = NAN;
                // Освобождаем память
                if (L_out) *L_out = L; else free_matrix(L,n); // Передаем владение или освобождаем
                if (U_out) *U_out = U; else free_matrix(U,n);
                free_matrix(A, n); free(b); free(y);
                return;
            }
            double sum = 0.0;
            for (int k = 0; k < i; k++) {
                sum += L[j][k] * U[k][i];
            }
            L[j][i] = (A[j][i] - sum) / U[i][i];
        }
    }

    // Решаем Ly = b (Прямая подстановка)
    for (int i = 0; i < n; i++) {
        double sum = 0.0;
        for (int j = 0; j < i; j++) {
            sum += L[i][j] * y[j];
        }
        y[i] = b[i] - sum;
    }

    // Решаем Ux = y (Обратная подстановка)
    for (int i = n - 1; i >= 0; i--) {
        double sum = 0.0;
        for (int j = i + 1; j < n; j++) {
            sum += U[i][j] * x_sol[j];
        }
        x_sol[i] = (y[i] - sum) / U[i][i];
    }

    if (L_out) *L_out = L; else free_matrix(L, n);
    if (U_out) *U_out = U; else free_matrix(U, n);

    free_matrix(A, n);
    free(b);
    free(y);
}

// Функция для вычисления погрешностей
void calc_errors(const double *y_true, const double *y_approx, int n, double *max_error_out, double *avg_rel_error_out) {
    double max_err = 0.0;
    double sum_rel_err = 0.0;
    int valid_rel_err_count = 0;

    for (int i = 0; i < n; i++) {
        double abs_err = fabs(y_true[i] - y_approx[i]);
        if (abs_err > max_err) {
            max_err = abs_err;
        }
        if (fabs(y_true[i]) > 1e-9) {
            sum_rel_err += abs_err / fabs(y_true[i]);
            valid_rel_err_count++;
        }
    }
    *max_error_out = max_err;
    if (valid_rel_err_count > 0) {
        *avg_rel_error_out = (sum_rel_err / valid_rel_err_count) * 100.0;
    } else {
        *avg_rel_error_out = NAN;
    }
}

// Функция для линейной аппроксимации
// Возвращает коэффициенты [a0, a1]
// Вывод матрицы L и U из LU-разложения
void linear_fit(const Sums *s, double *coeffs_gauss, double *coeffs_lu,
                double ***lin_L_out, double ***lin_U_out, double ***A_ext_gauss_out) {
    double **A = create_matrix(2, 2);
    double b[2];

    A[0][0] = (double)s->n; A[0][1] = s->Sx;
    A[1][0] = s->Sx;        A[1][1] = s->Sxx;

    b[0] = s->Sy;
    b[1] = s->Sxy;

    gauss_elimination(A, b, coeffs_gauss, 2, A_ext_gauss_out);
    lu_decomposition_solve(A, b, coeffs_lu, 2, lin_L_out, lin_U_out);

    free_matrix(A, 2);
}

// Функция для квадратичной аппроксимации
// Возвращает коэффициенты [a0, a1, a2]
// Выводит матрицы L и U из LU-разложения
void quadratic_fit(const Sums *s, double *coeffs_gauss, double *coeffs_lu,
                   double ***quad_L_out, double ***quad_U_out, double ***A_ext_gauss_out) {
    double **A = create_matrix(3, 3);
    double b[3];

    A[0][0] = (double)s->n; A[0][1] = s->Sx;   A[0][2] = s->Sxx;
    A[1][0] = s->Sx;        A[1][1] = s->Sxx;  A[1][2] = s->Sxxx;
    A[2][0] = s->Sxx;       A[2][1] = s->Sxxx; A[2][2] = s->Sxxxx;

    b[0] = s->Sy;
    b[1] = s->Sxy;
    b[2] = s->Sxxy;

    gauss_elimination(A, b, coeffs_gauss, 3, A_ext_gauss_out);
    lu_decomposition_solve(A, b, coeffs_lu, 3, quad_L_out, quad_U_out);

    free_matrix(A, 3);
}


int main() {
    DataPoints dp = read_data("sample_11.csv");
    if (dp.n == 0) {
        fprintf(stderr, "Данные не загружены или ошибка при чтении данных. Выход.\n");
        if(dp.x) free(dp.x);
        if(dp.y) free(dp.y);
        return 1;
    }
    Sums sums = compute_sums(&dp);

    printf("\n  Промежуточные значения и их суммы  \n");
    printf("| %-12s | %-12s | %-12s | %-12s | %-12s | %-12s | %-12s |\n",
           "x", "x*x", "x*x*x", "x*x*x*x", "y", "x*y", "x*x*y");

    for (int i = 0; i < dp.n; i++) {
        double x_val = dp.x[i];
        double y_val = dp.y[i];
        double x_sq = x_val * x_val;
        double x_cub = x_sq * x_val;
        double x_quad = x_cub * x_val;
        double xy_val = x_val * y_val;
        double xxy_val = x_sq * y_val;

        printf("| %-12.4f | %-12.4f | %-12.4f | %-12.4f | %-12.4f | %-12.4f | %-12.4f |\n",
               x_val, x_sq, x_cub, x_quad, y_val, xy_val, xxy_val);
    }
    printf("| %-12s | %-12s | %-12s | %-12s | %-12s | %-12s | %-12s |\n",
           "Sum(x)", "Sum(x*x)", "Sum(x*x*x)", "Sum(x*x*x*x)", "Sum(y)", "Sum(x*y)", "Sum(x*x*y)");
    printf("| %-12.4f | %-12.4f | %-12.4f | %-12.4f | %-12.4f | %-12.4f | %-12.4f |\n",
           sums.Sx, sums.Sxx, sums.Sxxx, sums.Sxxxx, sums.Sy, sums.Sxy, sums.Sxxy);

    double lin_coeffs_gauss[2];
    double lin_coeffs_lu[2];
    double **lin_L = NULL, **lin_U = NULL;
    double **lin_A_ext_gauss = NULL;

    printf("\nЛинейная аппроксимация (y = a0 + a1*x)\n");
    linear_fit(&sums, lin_coeffs_gauss, lin_coeffs_lu, &lin_L, &lin_U, &lin_A_ext_gauss);

    print_vector(lin_coeffs_gauss, 2, "Решение (Гаусс)");
    if (lin_A_ext_gauss) print_matrix(lin_A_ext_gauss, 2, 3, "Расширенная матрица A_ext (Гаусс)");
    if (lin_L) print_matrix(lin_L, 2, 2, "L матрица (LU)");
    if (lin_U) print_matrix(lin_U, 2, 2, "U матрица (LU)");
    print_vector(lin_coeffs_lu, 2, "Решение (LU)");

    double *y_lin_approx = (double *)malloc(dp.n * sizeof(double));
    if (!y_lin_approx) { perror("Ошибка выделения памяти для y_lin_approx"); exit(1); }
    for (int i = 0; i < dp.n; i++) {
        y_lin_approx[i] = lin_coeffs_gauss[0] + lin_coeffs_gauss[1] * dp.x[i];
    }

    // Квадратичная аппроксимация  
    double quad_coeffs_gauss[3];
    double quad_coeffs_lu[3];
    double **quad_L = NULL, **quad_U = NULL;
    double **quad_A_ext_gauss = NULL;

    printf("\nКвадратичная аппроксимация (y = a0 + a1*x + a2*x^2)\n");
    quadratic_fit(&sums, quad_coeffs_gauss, quad_coeffs_lu, &quad_L, &quad_U, &quad_A_ext_gauss);

    print_vector(quad_coeffs_gauss, 3, "Решение (Гаусс)");
    if (quad_A_ext_gauss) print_matrix(quad_A_ext_gauss, 3, 4, "Расширенная матрица A_ext (Гаусс)");
    if (quad_L) print_matrix(quad_L, 3, 3, "L матрица (LU)");
    if (quad_U) print_matrix(quad_U, 3, 3, "U матрица (LU)");
    print_vector(quad_coeffs_lu, 3, "Решение (LU)");

    double *y_quad_approx = (double *)malloc(dp.n * sizeof(double));
    if (!y_quad_approx) { perror("Ошибка выделения памяти для y_quad_approx"); exit(1); }
    for (int i = 0; i < dp.n; i++) {
        y_quad_approx[i] = quad_coeffs_gauss[0] + quad_coeffs_gauss[1] * dp.x[i] + quad_coeffs_gauss[2] * dp.x[i] * dp.x[i];
    }

    // Вычисление и вывод погрешностей
    double lin_max_err, lin_avg_rel_err;
    double quad_max_err, quad_avg_rel_err;

    calc_errors(dp.y, y_lin_approx, dp.n, &lin_max_err, &lin_avg_rel_err);
    calc_errors(dp.y, y_quad_approx, dp.n, &quad_max_err, &quad_avg_rel_err);

    printf("\nПогрешности:\n");
    printf("Линейная аппроксимация:\n");
    printf("  max погрешность ε: %.4f\n", lin_max_err);
    printf("  средняя относительная ошибка εr: %.2f%%\n", lin_avg_rel_err);
    printf("Квадратичная аппроксимация:\n");
    printf("  max погрешность ε: %.4f\n", quad_max_err);
    printf("  средняя относительная ошибка εr: %.2f%%\n", quad_avg_rel_err);

    free(dp.x);
    free(dp.y);
    free(y_lin_approx);
    free(y_quad_approx);

    if (lin_L) free_matrix(lin_L, 2);
    if (lin_U) free_matrix(lin_U, 2);
    if (lin_A_ext_gauss) free_matrix(lin_A_ext_gauss, 2);

    if (quad_L) free_matrix(quad_L, 3);
    if (quad_U) free_matrix(quad_U, 3);
    if (quad_A_ext_gauss) free_matrix(quad_A_ext_gauss, 3);

    return 0;
}