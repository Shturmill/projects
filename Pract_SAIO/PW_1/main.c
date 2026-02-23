#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

// -8 * sin(-x^3) + e^(-x) => 8 * sin(x^3) + e^(-x)
// t = 1 -- dichotomy
// p = 2 -- golden ratio

double func (double x){
     return 8 * sin(pow(x,3) + exp(-x));
}

typedef struct{
    double x_min;
    double f_min;
    int iterations;
    double a_final;
    double b_final;
}Result;

// Золотое сечение
static void golden_section_iterations (double a, double b, int n_iterations){
    const double phi = (1.0 + sqrt(5.0))/2.0;
    const double k = 1.0 / phi;
    const double one_minus_k = 1.0 - k;

    double x1 = a + one_minus_k * (b - a);
    double x2 = a + k * (b - a);
    double f1 = func(x1);
    double f2 = func(x2);

    printf ("\n--- Золотое сечение: %d итерации (p=2) ---\n", n_iterations);
    printf ("iter\t a\t\t b\t\t x1\t\t x2\t\t f(x1)\t\t f(x2)\t\t (b-a)\n");

    for (int i = 1; i <= n_iterations; ++i) {
        printf("%d\t %.10f\t %.10f\t %.10f\t %.10f\t %.10f\t %.10f\t %.10f\n",
               i, a, b, x1, x2, f1, f2, (b - a));

        if (f1 <= f2) {
            // минимум в [a; x2]
            b = x2;
            x2 = x1;
            f2 = f1;
            x1 = a + one_minus_k * (b - a);
            f1 = func(x1);
        } else {
            // минимум в [x1; b]
            a = x1;
            x1 = x2;
            f1 = f2;
            x2 = a + k * (b - a);
            f2 = func(x2);
        }
    }

    printf("После %d итераций (ЗС): отрезок [a;b] = [%.10f; %.10f], длина = %.10f\n",
           n_iterations, a, b, (b - a));
}



// Метод дихотомии
static Result dichotomy(double a, double b, double eps, double delta, FILE *protocol_csv) {
    Result r = {0};

    if (!(delta > 0.0 && delta < eps / 2.0)) {
        fprintf(stderr, "Ошибка: delta должна удовлетворять 0 < delta < eps/2\n");
        exit(EXIT_FAILURE);
    }

    if (protocol_csv) {
        fprintf(protocol_csv, "iter,a,b,x1,x2,f_x1,f_x2,b_minus_a\n");
    }

    int iter = 0;
    while ((b - a) > eps) {
        ++iter;

        const double mid = (a + b) / 2.0;
        const double x1 = mid - delta;
        const double x2 = mid + delta;

        const double fx1 = func(x1);
        const double fx2 = func(x2);

        if (protocol_csv) {
            fprintf(protocol_csv, "%d,%.15f,%.15f,%.15f,%.15f,%.15f,%.15f,%.15f\n",
                    iter, a, b, x1, x2, fx1, fx2, (b - a));
        }

        if (fx1 <= fx2) {
            b = x2;
        } else {
            a = x1;
        }
    }

    r.a_final = a;
    r.b_final = b;
    r.iterations = iter;

    r.x_min = (a + b) / 2.0;
    r.f_min = func(r.x_min);
    return r;
}



// Запись данных для графика
static void write_function_data(const char *filename, double x_from, double x_to, int n_points) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Не удалось открыть func.dat");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i <= n_points; ++i) {
        double x = x_from + (x_to - x_from) * ((double)i / (double)n_points);
        fprintf(fp, "%.15f %.15f\n", x, func(x));
    }
    fclose(fp);
}

// Минимальная точка
static void write_min_point(const char *filename, double x, double y) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Не удалось открыть min_point.dat");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "%.15f %.15f\n", x, y);
    fclose(fp);
}

// Файл для построения графика
static void write_gnuplot_script(const char *filename,
                                double x_from, double x_to,
                                const char *func_dat,
                                const char *min_dat) {
    FILE *gp = fopen(filename, "w");
    if (!gp) {
        perror("Не удалось открыть plot.gp");
        exit(EXIT_FAILURE);
    }

    fprintf(gp,
        "set grid\n"
        "set key left top\n"
        "set xlabel 'x'\n"
        "set ylabel 'f(x)'\n"
        "set title 'Вариант 11: f(x)=8*sin(x^3)+exp(-x)'\n"
        "set xrange [%.10f:%.10f]\n"
        "plot '%s' with lines title 'f(x)', \\\n"
        "     '%s' with points pointtype 7 pointsize 1.5 title 'xmin'\n",
        x_from, x_to, func_dat, min_dat
    );

    fclose(gp);
}

int main(void) {
    double a = 2.85;
    double b = 2.88;

    // Точность по заданию
    const double eps = 1e-4;
    // Параметр дихотомии delta
    const double delta = eps / 5.0;

    // 1) "Ручной метод": 3 итерации золотого сечения (p=2)
    golden_section_iterations(a, b, 3);

    // 2) Расчет : метод дихотомии (t=1)
    FILE *protocol = fopen("protocol.csv", "w");
    if (!protocol) {
        perror("Не удалось открыть protocol.csv");
        return EXIT_FAILURE;
    }

    Result res = dichotomy(a, b, eps, delta, protocol);
    fclose(protocol);

    printf("\n--- Дихотомия до eps=1e-4 (t=1) ---\n");
    printf("Итераций: %d\n", res.iterations);
    printf("Финальный отрезок: [%.10f; %.10f], длина = %.10f\n", res.a_final, res.b_final, (res.b_final - res.a_final));
    printf("x* ≈ %.10f\n", res.x_min);
    printf("f(x*) ≈ %.10f\n", res.f_min);

    // 3) Данные для графика
    const double x_from = 2.80;
    const double x_to   = 2.93;
    write_function_data("data/func.dat", x_from, x_to, 2000);
    write_min_point("data/min_point.dat", res.x_min, res.f_min);
    write_gnuplot_script("plot/plot.gp", x_from, x_to, "data/func.dat", "data/min_point.dat");

    printf ("\n Файлы успешно созданы:\n");
    printf ("-- data/func.dat\n");
    printf ("-- data/min_point\n");
    printf ("-- protocol.csv\n");
    printf ("\n запуск gnuplot\n");
    printf ("gnuplot -persist plot/plot.gp\n");

    return EXIT_SUCCESS;
}