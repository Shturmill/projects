#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct State {
    int *board;
    int row;
    struct State *parent;
} State;

typedef struct Node {
    State *state;
    struct Node *next;
} Node;

typedef struct {
    Node *front;
    Node *back;
    int size;
} List;

typedef struct {
    int solutionCount;
    int expandedStates;
    int stepsToFirstSolution;
} SearchStats;


// Инициализация списка
void initList(List *lst)
{
    lst->front = NULL;
    lst->back = NULL;
    lst->size = 0;
}


// Проверка пустоты списка
bool isListEmpty(const List *lst)
{
    return lst->front == NULL;
}


// Удаление первого элемента списка
State *removeFirst(List *lst)
{
    if (lst->front == NULL) {
        return NULL;
    }

    Node *tmp = lst->front;
    State *state = tmp->state;

    lst->front = tmp->next;
    if (lst->front == NULL) {
        lst->back = NULL;
    }

    lst->size--;
    free(tmp);
    return state;
}


// Добавление нового элемента в начало списка
void addFirst(List *lst, State *state)
{
    Node *nd = (Node *)malloc(sizeof(Node));
    if (!nd) {
        fprintf(stderr, "malloc node\n");
        exit(EXIT_FAILURE);
    }

    nd->state = state;
    nd->next = lst->front;
    lst->front = nd;

    if (lst->back == NULL) {
        lst->back = nd;
    }

    lst->size++;
}


// Добавление нового элемента в конец списка
void addLast(List *lst, State *state)
{
    Node *nd = (Node *)malloc(sizeof(Node));
    if (!nd) {
        fprintf(stderr, "malloc node\n");
        exit(EXIT_FAILURE);
    }

    nd->state = state;
    nd->next = NULL;

    if (lst->back != NULL) {
        lst->back->next = nd;
    } else {
        lst->front = nd;
    }

    lst->back = nd;
    lst->size++;
}


// Определение наличия элемента в списке
bool contains(const List *lst, const State *s)
{
    for (Node *cur = lst->front; cur != NULL; cur = cur->next) {
        const State *t = cur->state;

        if (t->row != s->row) {
            continue;
        }

        bool eq = true;
        for (int i = 0; i < s->row && eq; i++) {
            if (t->board[i] != s->board[i]) {
                eq = false;
            }
        }

        if (eq) {
            return true;
        }
    }

    return false;
}


State *createState(int N, int row)
{
    State *s = (State *)malloc(sizeof(State));
    if (!s) {
        fprintf(stderr, "malloc State\n");
        exit(EXIT_FAILURE);
    }

    s->board = (int *)malloc(N * sizeof(int));
    if (!s->board) {
        free(s);
        fprintf(stderr, "malloc board\n");
        exit(EXIT_FAILURE);
    }

    s->row = row;
    s->parent = NULL;

    for (int i = 0; i < N; i++) {
        s->board[i] = -1;
    }

    return s;
}


State *cloneState(const State *src, int N)
{
    State *dst = createState(N, src->row);

    for (int i = 0; i < src->row; i++) {
        dst->board[i] = src->board[i];
    }

    dst->parent = src->parent;
    return dst;
}


void freeState(State *s)
{
    if (!s) {
        return;
    }

    free(s->board);
    free(s);
}


// Полная очистка списка
void freeList(List *lst)
{
    while (!isListEmpty(lst)) {
        freeState(removeFirst(lst));
    }
}


// Проверка допустимости постановки ферзя
bool isSafe(const int *board, int row, int col)
{
    for (int i = 0; i < row; i++) {
        if (board[i] == col || abs(board[i] - col) == abs(i - row)) {
            return false;
        }
    }
    return true;
}


// Проверка достижения целевого состояния
bool isGoal(const State *state, int Q)
{
    return state->row == Q;
}


// Процедура порождения дочерних вершин путем применения допустимых операторов
void createChildren(const State *parent, int N, bool reverseOrder, List *children)
{
    initList(children);

    // Для каждого оператора // Цикл по всем операторам
    // В задаче о ферзях оператором является постановка ферзя
    // в очередную строку в один из допустимых столбцов

    if (reverseOrder) {
        for (int col = N - 1; col >= 0; col--) {

            // Проверить ее допустимость
            if (!isSafe(parent->board, parent->row, col)) {
                continue;
            }

            // Создать дочернюю вершину
            State *child = cloneState(parent, N);
            child->board[parent->row] = col;
            child->row = parent->row + 1;
            child->parent = (State *)parent;

            addLast(children, child);
        }
    } else {
        for (int col = 0; col < N; col++) {

            // Проверить ее допустимость
            if (!isSafe(parent->board, parent->row, col)) {
                continue;
            }

            // Создать дочернюю вершину
            State *child = cloneState(parent, N);
            child->board[parent->row] = col;
            child->row = parent->row + 1;
            child->parent = (State *)parent;

            addLast(children, child);
        }
    }
}


void printBoard(const int *board, int N)
{
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            printf(board[r] == c ? " Q " : " . ");
        }
        printf("\n");
    }
}


void printSolution(const State *goal, int N, int *solutionCount)
{
    (*solutionCount)++;
    printf("=== Решение %d ===\n", *solutionCount);

    int depth = 0;
    for (const State *cur = goal; cur != NULL; cur = cur->parent) {
        depth++;
    }

    const State **path = (const State **)malloc(depth * sizeof(State *));
    if (!path) {
        fprintf(stderr, "malloc path\n");
        exit(EXIT_FAILURE);
    }

    int index = 0;
    for (const State *cur = goal; cur != NULL; cur = cur->parent) {
        path[index++] = cur;
    }

    printf("Последовательность операторов (строка -> столбец):\n");
    for (int i = depth - 1; i > 0; i--) {
        int rowPlaced = path[i]->row;
        int colPlaced = path[i - 1]->board[rowPlaced];

        printf("  Шаг %d: строка %d -> столбец %d\n",
               depth - i,
               rowPlaced + 1,
               colPlaced + 1);
    }

    printf("Доска:\n");
    printBoard(goal->board, N);
    printf("\n");

    free(path);
}


void printStats(const char *methodName, const SearchStats *stats)
{
    printf("%s: всего решений = %d | раскрыто состояний = %d | шагов до первого решения = ",
           methodName,
           stats->solutionCount,
           stats->expandedStates);

    if (stats->stepsToFirstSolution == -1) {
        printf("не найдено\n\n");
    } else {
        printf("%d\n\n", stats->stepsToFirstSolution);
    }
}


// Поиск в ширину
SearchStats solveBFS(int N, int Q)
{
    List openList;
    List closedList;

    initList(&openList);
    initList(&closedList);

    SearchStats stats;
    stats.solutionCount = 0;
    stats.expandedStates = 0;
    stats.stepsToFirstSolution = -1;

    State *initial = createState(N, 0);

    // Open = [Start]; // инициализация
    addLast(&openList, initial);

    // Closed = []

    // While Open <> [] do // еще есть вершины
    while (!isListEmpty(&openList)) {

        // X = первая вершина из Open // выбрать первую вершину из Open
        State *X = removeFirst(&openList);
        stats.expandedStates++;

        // Добавить вершину X в список Closed
        addLast(&closedList, X);

        List children;
        createChildren(X, N, false, &children);

        // Для каждого потомка X // цикл по всем потомкам X
        while (!isListEmpty(&children)) {
            State *child = removeFirst(&children);

            // If X = цель then вернуть True // цель найдена
            if (isGoal(child, Q)) {
                if (stats.stepsToFirstSolution == -1) {
                    stats.stepsToFirstSolution = stats.expandedStates;
                }

                printSolution(child, N, &stats.solutionCount);
                freeState(child);
                continue;
            }

            // If он не в списке Open или Closed then добавить в конец списка Open
            if (!contains(&openList, child) && !contains(&closedList, child)) {
                addLast(&openList, child);
            } else {
                freeState(child);
            }
        }
    }

    freeList(&openList);
    freeList(&closedList);

    return stats;
}


// Поиск в глубину с ограничением глубины просмотра дерева
SearchStats solveDFS(int N, int Q, int maxDepth)
{
    List openList;
    List closedList;

    initList(&openList);
    initList(&closedList);

    SearchStats stats;
    stats.solutionCount = 0;
    stats.expandedStates = 0;
    stats.stepsToFirstSolution = -1;

    State *initial = createState(N, 0);

    // Open = [Start]
    addFirst(&openList, initial);

    // Closed = []

    // While Open <> [] do
    while (!isListEmpty(&openList)) {

        // X = первая вершина из Open
        State *X = removeFirst(&openList);
        stats.expandedStates++;

        // Добавить вершину X в список Closed
        addLast(&closedList, X);

        if (X->row >= maxDepth) {
            continue;
        }

        List children;
        createChildren(X, N, true, &children);

        // Для каждого потомка X
        while (!isListEmpty(&children)) {
            State *child = removeFirst(&children);

            if (child->row > maxDepth) {
                freeState(child);
                continue;
            }

            // If X = цель then вернуть True
            if (isGoal(child, Q)) {
                if (stats.stepsToFirstSolution == -1) {
                    stats.stepsToFirstSolution = stats.expandedStates;
                }

                printSolution(child, N, &stats.solutionCount);
                freeState(child);
                continue;
            }

            // If он не в списке Open или Closed then добавить в начало списка Open
            if (!contains(&openList, child) && !contains(&closedList, child)) {
                addFirst(&openList, child);
            } else {
                freeState(child);
            }
        }
    }

    freeList(&openList);
    freeList(&closedList);

    return stats;
}


int readInt(const char *prompt, int lo, int hi)
{
    int v;
    int rc;

    while (1) {
        printf("%s", prompt);
        rc = scanf("%d", &v);

        if (rc == 1 && v >= lo && v <= hi) {
            return v;
        }

        printf("Ошибка: введите число от %d до %d.\n", lo, hi);

        while (getchar() != '\n') {
        }
    }
}


int main(void)
{
    printf("=== Задача о N ферзях ===\n\n");

    int N = readInt("Введите размерность доски N (1..12): ", 1, 12);
    int Q = readInt("Введите число ферзей Q (1..N):       ", 1, N);

    printf("\nВыберите метод поиска:\n");
    printf("1 - BFS\n");
    printf("2 - DFS\n");
    printf("3 - BFS и DFS\n\n");

    int method = readInt("Ваш выбор (1..3): ", 1, 3);

    if (method == 1 || method == 3) {
        printf("\n--- Поиск в ШИРИНУ (BFS) ---\n");
        SearchStats bfsStats = solveBFS(N, Q);

        if (bfsStats.solutionCount == 0) {
            printf("BFS: для N=%d, Q=%d решений не найдено.\n\n", N, Q);
        }

        printStats("BFS", &bfsStats);
    }

    if (method == 2 || method == 3) {
        int maxDepth = readInt("Введите максимальную глубину для DFS (1..Q): ", 1, Q);

        printf("\n--- Поиск в ГЛУБИНУ (DFS, ограниченный, maxDepth=%d) ---\n", maxDepth);
        SearchStats dfsStats = solveDFS(N, Q, maxDepth);

        if (dfsStats.solutionCount == 0) {
            printf("DFS: для N=%d, Q=%d решений не найдено.\n\n", N, Q);
        }

        printStats("DFS", &dfsStats);
    }

    return 0;
}