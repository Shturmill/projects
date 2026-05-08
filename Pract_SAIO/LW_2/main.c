#include <stdio.h>
#include <math.h>

#define EPS         1.0e-9
#define MAX_M2      3
#define MAX_LINES   5
#define MAX_G3      2
#define ACTIVE_CNT  3

typedef struct
{
    double c[2];
    double a[MAX_M2][2];
    double b[MAX_M2];
    int m;
} LP2Max;

typedef struct
{
    double c[3];
    double g[MAX_G3][3];
    double h[MAX_G3];
    int m;
} LP3MinGe;

typedef struct
{
    double x1;
    double x2;
    double value;
    int found;
} Result2;

typedef struct
{
    double x[3];
    double value;
    int found;
} Result3;

static double abs_d(double x)
{
    if (x < 0.0)
    {
        return -x;
    }

    return x;
}

static int is_ge(double left, double right)
{
    if (left + EPS >= right)
    {
        return 1;
    }

    return 0;
}

static int is_le(double left, double right)
{
    if (left <= right + EPS)
    {
        return 1;
    }

    return 0;
}

static int is_non_negative(double x)
{
    if (x >= -EPS)
    {
        return 1;
    }

    return 0;
}

static double objective_2(const LP2Max *lp, double x1, double x2)
{
    return lp->c[0] * x1 + lp->c[1] * x2;
}

static double objective_3(const LP3MinGe *lp, const double x[3])
{
    return lp->c[0] * x[0] + lp->c[1] * x[1] + lp->c[2] * x[2];
}

static int feasible_2max(const LP2Max *lp, double x1, double x2)
{
    int i;
    double left;

    if (!is_non_negative(x1) || !is_non_negative(x2))
    {
        return 0;
    }

    for (i = 0; i < lp->m; ++i)
    {
        left = lp->a[i][0] * x1 + lp->a[i][1] * x2;

        if (!is_le(left, lp->b[i]))
        {
            return 0;
        }
    }

    return 1;
}

static int feasible_3minge(const LP3MinGe *lp, const double x[3])
{
    int i;
    double left;

    if (!is_non_negative(x[0]) ||
        !is_non_negative(x[1]) ||
        !is_non_negative(x[2]))
    {
        return 0;
    }

    for (i = 0; i < lp->m; ++i)
    {
        left = lp->g[i][0] * x[0] +
               lp->g[i][1] * x[1] +
               lp->g[i][2] * x[2];

        if (!is_ge(left, lp->h[i]))
        {
            return 0;
        }
    }

    return 1;
}

static int solve_2x2(double a11, double a12, double b1, double a21, double a22, double b2, double *x1, double *x2)
{
    double det;
    double nx1;
    double nx2;

    det = a11 * a22 - a12 * a21;

    if (abs_d(det) < EPS)
    {
        return 0;
    }

    nx1 = (b1 * a22 - a12 * b2) / det;
    nx2 = (a11 * b2 - b1 * a21) / det;

    *x1 = nx1;
    *x2 = nx2;

    return 1;
}

static int solve_3x3(double a[3][3], double b[3], double x[3])
{
    int i;
    int j;
    int k;
    int pivot_row;
    double tmp;
    double factor;
    double max_val;

    for (i = 0; i < 3; ++i)
    {
        pivot_row = i;
        max_val = abs_d(a[i][i]);

        for (j = i + 1; j < 3; ++j)
        {
            if (abs_d(a[j][i]) > max_val)
            {
                max_val = abs_d(a[j][i]);
                pivot_row = j;
            }
        }

        if (max_val < EPS)
        {
            return 0;
        }

        if (pivot_row != i)
        {
            for (k = 0; k < 3; ++k)
            {
                tmp = a[i][k];
                a[i][k] = a[pivot_row][k];
                a[pivot_row][k] = tmp;
            }

            tmp = b[i];
            b[i] = b[pivot_row];
            b[pivot_row] = tmp;
        }

        for (j = i + 1; j < 3; ++j)
        {
            factor = a[j][i] / a[i][i];

            for (k = i; k < 3; ++k)
            {
                a[j][k] = a[j][k] - factor * a[i][k];
            }

            b[j] = b[j] - factor * b[i];
        }
    }

    for (i = 2; i >= 0; --i)
    {
        x[i] = b[i];

        for (j = i + 1; j < 3; ++j)
        {
            x[i] = x[i] - a[i][j] * x[j];
        }

        if (abs_d(a[i][i]) < EPS)
        {
            return 0;
        }

        x[i] = x[i] / a[i][i];
    }

    return 1;
}

static Result2 solve_lp2_max_vertices(const LP2Max *lp)
{
    Result2 res;
    double lines_a[MAX_LINES][2];
    double lines_b[MAX_LINES];
    int line_count;
    int i;
    int j;
    double x1;
    double x2;
    double val;

    res.x1 = 0.0;
    res.x2 = 0.0;
    res.value = 0.0;
    res.found = 0;

    line_count = 0;

    for (i = 0; i < lp->m; ++i)
    {
        lines_a[line_count][0] = lp->a[i][0];
        lines_a[line_count][1] = lp->a[i][1];
        lines_b[line_count] = lp->b[i];
        line_count = line_count + 1;
    }

    lines_a[line_count][0] = 1.0;
    lines_a[line_count][1] = 0.0;
    lines_b[line_count] = 0.0;
    line_count = line_count + 1;

    lines_a[line_count][0] = 0.0;
    lines_a[line_count][1] = 1.0;
    lines_b[line_count] = 0.0;
    line_count = line_count + 1;

    for (i = 0; i < line_count; ++i)
    {
        for (j = i + 1; j < line_count; ++j)
        {
            if (!solve_2x2(
                    lines_a[i][0], lines_a[i][1], lines_b[i],
                    lines_a[j][0], lines_a[j][1], lines_b[j],
                    &x1, &x2))
            {
                continue;
            }

            if (!feasible_2max(lp, x1, x2))
            {
                continue;
            }

            val = objective_2(lp, x1, x2);

            if (!res.found || (val > res.value + EPS))
            {
                res.x1 = x1;
                res.x2 = x2;
                res.value = val;
                res.found = 1;
            }
        }
    }

    return res;
}

static void fill_active_constraint(const LP3MinGe *lp, int active_id, double row[3], double *rhs)
{
    if (active_id == 0)
    {
        row[0] = lp->g[0][0];
        row[1] = lp->g[0][1];
        row[2] = lp->g[0][2];
        *rhs = lp->h[0];
        return;
    }

    if (active_id == 1)
    {
        row[0] = lp->g[1][0];
        row[1] = lp->g[1][1];
        row[2] = lp->g[1][2];
        *rhs = lp->h[1];
        return;
    }

    if (active_id == 2)
    {
        row[0] = 1.0;
        row[1] = 0.0;
        row[2] = 0.0;
        *rhs = 0.0;
        return;
    }

    if (active_id == 3)
    {
        row[0] = 0.0;
        row[1] = 1.0;
        row[2] = 0.0;
        *rhs = 0.0;
        return;
    }

    row[0] = 0.0;
    row[1] = 0.0;
    row[2] = 1.0;
    *rhs = 0.0;
}

static Result3 solve_lp3_min_ge_vertices(const LP3MinGe *lp)
{
    Result3 res;
    int a_id;
    int b_id;
    int c_id;
    double mat[3][3];
    double rhs[3];
    double cand[3];
    double val;
    int r;
    int c;

    res.x[0] = 0.0;
    res.x[1] = 0.0;
    res.x[2] = 0.0;
    res.value = 0.0;
    res.found = 0;

    for (a_id = 0; a_id < 5; ++a_id)
    {
        for (b_id = a_id + 1; b_id < 5; ++b_id)
        {
            for (c_id = b_id + 1; c_id < 5; ++c_id)
            {
                fill_active_constraint(lp, a_id, mat[0], &rhs[0]);
                fill_active_constraint(lp, b_id, mat[1], &rhs[1]);
                fill_active_constraint(lp, c_id, mat[2], &rhs[2]);

                if (!solve_3x3(mat, rhs, cand))
                {
                    continue;
                }

                if (!feasible_3minge(lp, cand))
                {
                    continue;
                }

                val = objective_3(lp, cand);

                if (!res.found || (val < res.value - EPS))
                {
                    for (r = 0; r < 3; ++r)
                    {
                        res.x[r] = cand[r];
                    }

                    res.value = val;
                    res.found = 1;
                }

                for (r = 0; r < 3; ++r)
                {
                    for (c = 0; c < 3; ++c)
                    {
                        mat[r][c] = 0.0;
                    }
                }
            }
        }
    }

    return res;
}

static void print_task1(void)
{
    LP2Max lp;
    Result2 res;

    lp.c[0] = 4.0;
    lp.c[1] = 3.0;

    lp.a[0][0] = -3.0;
    lp.a[0][1] =  4.0;
    lp.b[0] = 32.0;

    lp.a[1][0] =  2.0;
    lp.a[1][1] =  1.0;
    lp.b[1] = 19.0;

    lp.a[2][0] =  3.0;
    lp.a[2][1] = -1.0;
    lp.b[2] = 21.0;

    lp.m = 3;

    res = solve_lp2_max_vertices(&lp);

    printf("\nЗадание 1, вариант 11\n");
    printf("z = 4x1 + 3x2 -> max\n");

    if (res.found)
    {
        printf("x1 = %.10f\n", res.x1);
        printf("x2 = %.10f\n", res.x2);
        printf("z  = %.10f\n", res.value);
    }
    else
    {
        printf("Решение не найдено.\n");
    }
}

static void print_task2_primal(void)
{
    LP2Max lp;
    Result2 res;

    lp.c[0] =  1.0;
    lp.c[1] = -1.0;

    lp.a[0][0] =  1.0;
    lp.a[0][1] =  1.0;
    lp.b[0] = 2.0;

    lp.a[1][0] = -2.0;
    lp.a[1][1] =  1.0;
    lp.b[1] = 1.0;

    lp.a[2][0] =  3.0;
    lp.a[2][1] = -1.0;
    lp.b[2] = 3.0;

    lp.m = 3;

    res = solve_lp2_max_vertices(&lp);


    printf("\nЗадание 2, вариант 11, прямая задача\n");
    printf("z = x1 - x2 -> max\n");

    if (res.found)
    {
        printf("x1 = %.10f\n", res.x1);
        printf("x2 = %.10f\n", res.x2);
        printf("z  = %.10f\n", res.value);
    }
    else
    {
        printf("Решение не найдено.\n");
    }
}

static void print_task2_dual(void)
{
    LP3MinGe lp;
    Result3 res;

    lp.c[0] = 2.0;
    lp.c[1] = 1.0;
    lp.c[2] = 3.0;

    lp.g[0][0] =  1.0;
    lp.g[0][1] = -2.0;
    lp.g[0][2] =  3.0;
    lp.h[0] = 1.0;

    lp.g[1][0] =  1.0;
    lp.g[1][1] =  1.0;
    lp.g[1][2] = -1.0;
    lp.h[1] = -1.0;

    lp.m = 2;

    res = solve_lp3_min_ge_vertices(&lp);

    printf("\nЗадание 2, вариант 11, двойственная задача\n");
    printf("W = 2y1 + y2 + 3y3 -> min\n");

    if (res.found)
    {
        printf("y1 = %.10f\n", res.x[0]);
        printf("y2 = %.10f\n", res.x[1]);
        printf("y3 = %.10f\n", res.x[2]);
        printf("W  = %.10f\n", res.value);
    }
    else
    {
        printf("Решение не найдено.\n");
    }
}

int main(void)
{
    print_task1();
    print_task2_primal();
    print_task2_dual();

    printf("\nОжидаемые ответы:\n");
    printf("Задание 1: x1 = 4, x2 = 11, z = 49\n");
    printf("Задание 2 (прямая): x1 = 1, x2 = 0, z = 1\n");
    printf("Задание 2 (двойственная): y1 = 0, y2 = 0, y3 = 1/3, W = 1\n");

    return 0;
}