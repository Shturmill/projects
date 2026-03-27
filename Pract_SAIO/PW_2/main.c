#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

// Целевая функция f(x,y) = 5x^2 + 2y^2 - 2x
double func(double x, double y) {
    return 5.0 * x * x + 2.0 * y * y - 2.0 * x;
}

double grad_x(double x) {
    return 10.0 * x - 2.0;
}

double grad_y(double y) {
    return 4.0 * y;
}

typedef struct {
    double x_min;
    double y_min;
    double f_min;
    int iterations;
} Result;

static void make_dir_if_needed(const char *dirname) {
    if (mkdir(dirname, 0777) == -1 && errno != EEXIST) {
        perror(dirname);
        exit(EXIT_FAILURE);
    }
}

// ГДШ
static void gdsh_iterations(double x, double y, int n_iterations, const char *traj_file) {
    double lambda_0 = 0.5; // Начальный шаг
    double delta = 0.5;    // Коэффициент дробления

    FILE *fp = fopen(traj_file, "w");
    if (!fp) {
        perror("Ошибка открытия файла ГДШ");
        exit(EXIT_FAILURE);
    }

    printf("\n--- ГДШ: %d итерации (Ручной расчет) ---\n", n_iterations);
    printf("iter\t x\t\t y\t\t f(x,y)\t\t gx\t\t gy\t\t lambda\n");

    fprintf(fp, "%.10f %.10f\n", x, y);
    printf("0\t %.6f\t %.6f\t %.6f\t %.6f\t %.6f\t -\n",
           x, y, func(x, y), grad_x(x), grad_y(y));

    for (int k = 1; k <= n_iterations; k++) {
        double gx = grad_x(x);
        double gy = grad_y(y);
        double lambda = lambda_0;

        while (func(x - lambda * gx, y - lambda * gy) >= func(x, y)) {
            lambda *= delta;
        }

        x = x - lambda * gx;
        y = y - lambda * gy;

        printf("%d\t %.6f\t %.6f\t %.6f\t %.6f\t %.6f\t %.6f\n",
               k, x, y, func(x, y), gx, gy, lambda);
        fprintf(fp, "%.10f %.10f\n", x, y);
    }

    fclose(fp);
}

// НСА
static Result nsa_optimize(double x, double y, double eps, const char *traj_file, FILE *protocol) {
    Result r = {0};
    int iter = 0;

    FILE *fp = fopen(traj_file, "w");
    if (!fp) {
        perror("Ошибка открытия файла НСА");
        exit(EXIT_FAILURE);
    }

    if (protocol) {
        fprintf(protocol, "iter,x,y,f_xy,grad_norm,lambda\n");
    }

    while (1) {
        double gx = grad_x(x);
        double gy = grad_y(y);
        double g_norm = sqrt(gx * gx + gy * gy);
        double lambda = 0.0;

        fprintf(fp, "%.10f %.10f\n", x, y);

        if (g_norm < eps) {
            if (protocol) {
                fprintf(protocol, "%d,%.10f,%.10f,%.10f,%.10f,%.10f\n",
                        iter, x, y, func(x, y), g_norm, lambda);
            }
            break;
        }

        // Аналитический шаг для f(x,y) = 5x^2 + 2y^2 - 2x
        // lambda = (gx^2 + gy^2) / (10*gx^2 + 4*gy^2)
        lambda = (pow(gx, 2) + pow(gy, 2)) / (10.0 * pow(gx, 2) + 4.0 * pow(gy, 2));

        if (protocol) {
            fprintf(protocol, "%d,%.10f,%.10f,%.10f,%.10f,%.10f\n",
                    iter, x, y, func(x, y), g_norm, lambda);
        }

        x = x - lambda * gx;
        y = y - lambda * gy;
        iter++;
    }

    fclose(fp);

    r.x_min = x;
    r.y_min = y;
    r.f_min = func(x, y);
    r.iterations = iter;

    return r;
}

// Запись точки минимума
static void write_min_point(const char *filename, double x, double y) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Ошибка открытия файла min_point");
        exit(EXIT_FAILURE);
    }

    fprintf(fp, "%.10f %.10f\n", x, y);
    fclose(fp);
}


static void write_gnuplot_script(const char *filename,
                                 const char *nsa_dat,
                                 const char *gdsh_dat,
                                 const char *min_dat) {
    FILE *gp = fopen(filename, "w");
    if (!gp) {
        perror("Ошибка открытия файла plot.gp");
        exit(EXIT_FAILURE);
    }

    fprintf(gp,
        "set grid\n"
        "set title 'Вариант 11: Траектории НСА и ГДШ'\n"
        "set xlabel 'x'\n"
        "set ylabel 'y'\n"
        "plot '%s' with linespoints title 'НСА (программа)', \\\n"
        "     '%s' with linespoints title 'ГДШ (ручной)', \\\n"
        "     '%s' with points pt 7 ps 2 title 'min'\n",
        nsa_dat, gdsh_dat, min_dat);

    fclose(gp);
}

int main(void) {
    double x0 = 1.0, y0 = 1.0;
    double eps = 1e-4;

    make_dir_if_needed("data");
    make_dir_if_needed("plot");

    // 1) Ручной расчет: 3 итерации ГДШ
    gdsh_iterations(x0, y0, 3, "data/gdsh.dat");

    // 2) Программный расчет: НСА до точности
    FILE *protocol = fopen("protocol.csv", "w");
    if (!protocol) {
        perror("Ошибка открытия protocol.csv");
        return EXIT_FAILURE;
    }

    Result res = nsa_optimize(x0, y0, eps, "data/nsa.dat", protocol);
    fclose(protocol);

    printf("\n--- НСА до точности 1e-4 ---\n");
    printf("Итераций: %d\n", res.iterations);
    printf("x* ≈ %.10f, y* ≈ %.10f\n", res.x_min, res.y_min);
    printf("f(x*, y*) ≈ %.10f\n", res.f_min);

    // 3) Генерация файлов для графика
    write_min_point("data/min_point.dat", 0.2, 0.0); // аналитический минимум
    write_gnuplot_script("plot/plot.gp",
                         "data/nsa.dat",
                         "data/gdsh.dat",
                         "data/min_point.dat");

    printf("\nСозданы файлы:\n");
    printf("data/gdsh.dat\n");
    printf("data/nsa.dat\n");
    printf("data/min_point.dat\n");
    printf("plot/plot.gp\n");
    printf("protocol.csv\n");

    printf("\nДля графика:\n");
    printf("gnuplot -persist plot/plot.gp\n");

    return 0;
}