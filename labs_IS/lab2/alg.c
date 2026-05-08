#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct State {
    int *board;              // board[row] = col, если в строке row стоит ферзь; иначе -1
    int row;                 // глубина состояния: сколько ферзей уже поставлено
    struct State *parent;    // родитель для восстановления пути
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
    int solutionCount;        // сколько целевых состояний найдено
    int expandedStates;       // счетчик шагов: сколько состояний извлечено/рассмотрено
    int stepsToFirstSolution; // номер шага, на котором впервые найдено решение; -1, если нет
} SearchStats;

typedef enum {
    METHOD_BFS = 1,
    METHOD_DFS_ITER = 2,
    METHOD_DFS_REC = 3,
    METHOD_DFS_REC_PATH = 4,
    METHOD_ALL = 5
} SearchMethod;

// Работа со списками OPEN, CLOSED и PATH.

static void initList(List *lst)
{
    lst->front = NULL;
    lst->back = NULL;
    lst->size = 0;
}

static bool isListEmpty(const List *lst)
{
    return lst->front == NULL;
}

// Удаление первого элемента списка
static State *removeFirst(List *lst)
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
static void addFirst(List *lst, State *state)
{
    Node *nd = (Node *)malloc(sizeof(Node));
    if (!nd) {
        fprintf(stderr, "Ошибка выделения памяти для элемента списка.\n");
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
static void addLast(List *lst, State *state)
{
    Node *nd = (Node *)malloc(sizeof(Node));
    if (!nd) {
        fprintf(stderr, "Ошибка выделения памяти для элемента списка.\n");
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

// Удаление последнего узла без освобождения состояния: нужно для Path при откате рекурсии
static State *removeLastNodeOnly(List *lst)
{
    if (lst->front == NULL) {
        return NULL;
    }

    Node *prev = NULL;
    Node *cur = lst->front;

    while (cur->next != NULL) {
        prev = cur;
        cur = cur->next;
    }

    State *state = cur->state;

    if (prev == NULL) {
        lst->front = NULL;
        lst->back = NULL;
    } else {
        prev->next = NULL;
        lst->back = prev;
    }

    lst->size--;
    free(cur);
    return state;
}

// Определение наличия состояния в списке
static bool contains(const List *lst, const State *s)
{
    for (Node *cur = lst->front; cur != NULL; cur = cur->next) {
        const State *t = cur->state;

        if (t->row != s->row) {
            continue;
        }

        bool equal = true;
        for (int i = 0; i < s->row; i++) {
            if (t->board[i] != s->board[i]) {
                equal = false;
                break;
            }
        }

        if (equal) {
            return true;
        }
    }

    return false;
}

// Работа с состояниями задачи о ферзях.

static State *createState(int N, int row)
{
    State *s = (State *)malloc(sizeof(State));
    if (!s) {
        fprintf(stderr, "Ошибка выделения памяти для состояния.\n");
        exit(EXIT_FAILURE);
    }

    s->board = (int *)malloc((size_t)N * sizeof(int));
    if (!s->board) {
        free(s);
        fprintf(stderr, "Ошибка выделения памяти для доски.\n");
        exit(EXIT_FAILURE);
    }

    s->row = row;
    s->parent = NULL;

    for (int i = 0; i < N; i++) {
        s->board[i] = -1;
    }

    return s;
}

static State *cloneState(const State *src, int N)
{
    State *dst = createState(N, src->row);

    for (int i = 0; i < N; i++) {
        dst->board[i] = src->board[i];
    }

    dst->parent = src->parent;
    return dst;
}

static void freeState(State *s)
{
    if (!s) {
        return;
    }

    free(s->board);
    free(s);
}

// Полная очистка списка вместе с состояниями
static void freeListWithStates(List *lst)
{
    while (!isListEmpty(lst)) {
        freeState(removeFirst(lst));
    }
}

// Очистка только узлов списка: состояния принадлежат CLOSED
static void freeListNodesOnly(List *lst)
{
    while (!isListEmpty(lst)) {
        (void)removeFirst(lst);
    }
}

// Проверка допустимости применения оператора: можно ли поставить ферзя в (row, col)
static bool isSafe(const int *board, int row, int col)
{
    for (int i = 0; i < row; i++) {
        if (board[i] == col || abs(board[i] - col) == abs(i - row)) {
            return false;
        }
    }

    return true;
}

// Проверка достижения целевого состояния: поставлено Q ферзей
static bool isGoal(const State *state, int Q)
{
    return state->row == Q;
}

// Порождение дочерних вершин путем применения допустимых операторов
static void createChildren(const State *parent, int N, bool reverseOrder, List *children)
{
    initList(children);

    if (parent->row >= N) {
        return;
    }

    if (reverseOrder) {
        for (int col = N - 1; col >= 0; col--) {
            if (!isSafe(parent->board, parent->row, col)) {
                continue;
            }

            State *child = cloneState(parent, N);
            child->board[parent->row] = col;
            child->row = parent->row + 1;
            child->parent = (State *)parent;
            addLast(children, child);
        }
    } else {
        for (int col = 0; col < N; col++) {
            if (!isSafe(parent->board, parent->row, col)) {
                continue;
            }

            State *child = cloneState(parent, N);
            child->board[parent->row] = col;
            child->row = parent->row + 1;
            child->parent = (State *)parent;
            addLast(children, child);
        }
    }
}

// Вывод решений и статистики.

static void printBoard(const int *board, int N)
{
    printf("   ");
    for (int c = 0; c < N; c++) {
        printf("%2d ", c + 1);
    }
    printf("\n");

    for (int r = 0; r < N; r++) {
        printf("%2d ", r + 1);
        for (int c = 0; c < N; c++) {
            printf(board[r] == c ? " Q " : " . ");
        }
        printf("\n");
    }
}

// Восстановление пути через parent: используется BFS, итерационным DFS и обычным рекурсивным DFS
static void printSolutionByParent(const State *goal, int N, int *solutionCount)
{
    (*solutionCount)++;
    printf("=== Решение %d ===\n", *solutionCount);

    int depth = 0;
    for (const State *cur = goal; cur != NULL; cur = cur->parent) {
        depth++;
    }

    const State **path = (const State **)malloc((size_t)depth * sizeof(State *));
    if (!path) {
        fprintf(stderr, "Ошибка выделения памяти для восстановления пути.\n");
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
        printf("  Шаг %d: строка %d -> столбец %d\n", depth - i, rowPlaced + 1, colPlaced + 1);
    }

    printf("Доска:\n");
    printBoard(goal->board, N);
    printf("\n");

    free(path);
}

// Непосредственный вывод пути Path
static void printSolutionByPath(const List *path, int N, int *solutionCount)
{
    (*solutionCount)++;
    printf("=== Решение %d ===\n", *solutionCount);
    printf("Последовательность операторов Path (строка -> столбец):\n");

    Node *prev = path->front;
    Node *cur = (prev != NULL) ? prev->next : NULL;
    int step = 1;

    while (prev != NULL && cur != NULL) {
        int rowPlaced = prev->state->row;
        int colPlaced = cur->state->board[rowPlaced];
        printf("  Шаг %d: строка %d -> столбец %d\n", step, rowPlaced + 1, colPlaced + 1);

        prev = cur;
        cur = cur->next;
        step++;
    }

    printf("Доска:\n");
    if (path->back != NULL) {
        printBoard(path->back->state->board, N);
    }
    printf("\n");
}

static void initStats(SearchStats *stats)
{
    stats->solutionCount = 0;
    stats->expandedStates = 0;
    stats->stepsToFirstSolution = -1;
}

static void registerSolutionStep(SearchStats *stats)
{
    if (stats->stepsToFirstSolution == -1) {
        stats->stepsToFirstSolution = stats->expandedStates;
    }
}

static void printStats(const char *methodName, const SearchStats *stats)
{
    printf("%s: всего решений = %d | раскрыто состояний = %d | шагов до первого решения = ", methodName, stats->solutionCount, stats->expandedStates);

    if (stats->stepsToFirstSolution == -1) {
        printf("не найдено\n\n");
    } else {
        printf("%d\n\n", stats->stepsToFirstSolution);
    }
}

static SearchStats solveBFS(int N, int Q)
{
    List openList;
    List closedList;
    SearchStats stats;

    initList(&openList);
    initList(&closedList);
    initStats(&stats);

    State *start = createState(N, 0);

    // Open = [Start]
    addLast(&openList, start);

    // Closed = []

    // While Open <> []
    while (!isListEmpty(&openList)) {
        // X = первая вершина из Open; удалить X из Open
        State *X = removeFirst(&openList);

        // Добавить X в Closed
        addLast(&closedList, X);
        stats.expandedStates++;

        // If X = цель then решение найдено
        if (isGoal(X, Q)) {
            registerSolutionStep(&stats);
            printSolutionByParent(X, N, &stats.solutionCount);
            continue;
        }

        List children;
        createChildren(X, N, false, &children);

        // Для каждого потомка X
        while (!isListEmpty(&children)) {
            State *child = removeFirst(&children);

            // If он не в списке Open или Closed then добавить в конец списка Open
            if (!contains(&openList, child) && !contains(&closedList, child)) {
                addLast(&openList, child);
            } else {
                freeState(child);
            }
        }
    }

    freeListWithStates(&openList);
    freeListWithStates(&closedList);
    return stats;
}

static SearchStats solveDFSIterative(int N, int Q, int maxDepth)
{
    List openList;
    List closedList;
    SearchStats stats;

    initList(&openList);
    initList(&closedList);
    initStats(&stats);

    State *start = createState(N, 0);

    // Open = [Start]
    addFirst(&openList, start);

    // Closed = []

    // While Open <> []
    while (!isListEmpty(&openList)) {
        // X = первая вершина из Open; удалить X из Open
        State *X = removeFirst(&openList);

        // Добавить X в Closed
        addLast(&closedList, X);
        stats.expandedStates++;

        // If X = цель then решение найдено
        if (isGoal(X, Q)) {
            registerSolutionStep(&stats);
            printSolutionByParent(X, N, &stats.solutionCount);
            continue;
        }

        // Ограниченный перебор в глубину: ниже maxDepth не идем
        if (X->row >= maxDepth) {
            continue;
        }

        List children;
        // reverseOrder нужен, чтобы при добавлении в начало OPEN столбцы просматривались слева направо
        createChildren(X, N, true, &children);

        // Для каждого потомка X
        while (!isListEmpty(&children)) {
            State *child = removeFirst(&children);

            if (child->row > maxDepth) {
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

    freeListWithStates(&openList);
    freeListWithStates(&closedList);
    return stats;
}

static void dfsRecursiveImpl(State *X, int N, int Q, int maxDepth, List *closedList, SearchStats *stats)
{
    // Добавить вершину X в список Closed
    addLast(closedList, X);
    stats->expandedStates++;

    // If X = цель then распечатать путь
    if (isGoal(X, Q)) {
        registerSolutionStep(stats);
        printSolutionByParent(X, N, &stats->solutionCount);
        return;
    }

    // Ограничение глубины просмотра
    if (X->row >= maxDepth) {
        return;
    }

    List children;
    createChildren(X, N, false, &children);

    // Для каждого child
    while (!isListEmpty(&children)) {
        State *child = removeFirst(&children);

        if (child->row > maxDepth) {
            freeState(child);
            continue;
        }

        // Else If child не в списке Closed then DepthSearch(child)
        if (!contains(closedList, child)) {
            dfsRecursiveImpl(child, N, Q, maxDepth, closedList, stats);
        } else {
            freeState(child);
        }
    }
}

static SearchStats solveDFSRecursive(int N, int Q, int maxDepth)
{
    List closedList;
    SearchStats stats;

    initList(&closedList);
    initStats(&stats);

    State *start = createState(N, 0);
    dfsRecursiveImpl(start, N, Q, maxDepth, &closedList, &stats);

    freeListWithStates(&closedList);
    return stats;
}

static void dfsRecursivePathImpl(State *X, int N, int Q, int maxDepth, List *closedList, List *path, SearchStats *stats)
{
    // Добавить вершину X в список Closed
    addLast(closedList, X);
    stats->expandedStates++;

    // Path = Path + X
    addLast(path, X);

    // If X = цель then распечатать Path
    if (isGoal(X, Q)) {
        registerSolutionStep(stats);
        printSolutionByPath(path, N, &stats->solutionCount);
        removeLastNodeOnly(path);
        return;
    }

    // Ограничение глубины просмотра
    if (X->row < maxDepth) {
        List children;
        createChildren(X, N, false, &children);

        // Для каждого child
        while (!isListEmpty(&children)) {
            State *child = removeFirst(&children);

            if (child->row > maxDepth) {
                freeState(child);
                continue;
            }

            // Else If child не в списке Closed then DepthSearch(child, Path + child)
            if (!contains(closedList, child)) {
                dfsRecursivePathImpl(child, N, Q, maxDepth, closedList, path, stats);
            } else {
                freeState(child);
            }
        }
    }

    // Откат Path при возврате из рекурсии
    removeLastNodeOnly(path);
}

static SearchStats solveDFSRecursiveWithPath(int N, int Q, int maxDepth)
{
    List closedList;
    List path;
    SearchStats stats;

    initList(&closedList);
    initList(&path);
    initStats(&stats);

    State *start = createState(N, 0);
    dfsRecursivePathImpl(start, N, Q, maxDepth, &closedList, &path, &stats);

    freeListNodesOnly(&path);
    freeListWithStates(&closedList);
    return stats;
}

// Пользовательский ввод и сравнение.

static int readInt(const char *prompt, int lo, int hi)
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

static const char *bestBySteps(const SearchStats *bfsStats, const SearchStats *dfsIterStats, const SearchStats *dfsRecStats, const SearchStats *dfsPathStats)
{
    const char *bestName = NULL;
    int bestSteps = 0;

    const char *names[4] = {"BFS", "DFS итерационный", "DFS рекурсивный", "DFS рекурсивный с Path"};
    const SearchStats *stats[4] = {bfsStats, dfsIterStats, dfsRecStats, dfsPathStats};

    for (int i = 0; i < 4; i++) {
        if (stats[i] == NULL || stats[i]->stepsToFirstSolution == -1) {
            continue;
        }

        if (bestName == NULL || stats[i]->stepsToFirstSolution < bestSteps) {
            bestName = names[i];
            bestSteps = stats[i]->stepsToFirstSolution;
        }
    }

    return bestName;
}

static void printBestSummary(const SearchStats *bfsStats, const SearchStats *dfsIterStats, const SearchStats *dfsRecStats, const SearchStats *dfsPathStats)
{
    const char *bestName = bestBySteps(bfsStats, dfsIterStats, dfsRecStats, dfsPathStats);

    printf("=== Сравнение методов по числу шагов до первого решения ===\n");
    if (bestName != NULL) {
        printf("Лучший метод: %s\n\n", bestName);
    } else {
        printf("Лучший метод: не определен, решений не найдено.\n\n");
    }
}

static void printNoSolutionIfNeeded(const char *methodName, int N, int Q, const SearchStats *stats)
{
    if (stats->solutionCount == 0) {
        printf("%s: для N=%d, Q=%d решений не найдено.\n\n", methodName, N, Q);
    }
}

int main(void)
{
    printf("=== Задача о N ферзях ===\n");
    printf("Поиск решения в неявном графе пространства состояний\n\n");

    int N = readInt("Введите размерность доски N (1..12): ", 1, 12);
    int Q = readInt("Введите число ферзей Q (1..N):       ", 1, N);

    printf("\nВыберите метод поиска:\n");
    printf("1 - Поиск в ширину (BFS)\n");
    printf("2 - Поиск в глубину, итерационный (DFS)\n");
    printf("3 - Поиск в глубину, рекурсивный\n");
    printf("4 - Поиск в глубину, рекурсивный с Path\n");
    printf("5 - Выполнить все методы и сравнить\n\n");

    int method = readInt("Ваш выбор (1..5): ", 1, 5);

    int maxDepth = Q;
    if (method == METHOD_DFS_ITER || method == METHOD_DFS_REC || method == METHOD_DFS_REC_PATH || method == METHOD_ALL) {
        printf("\nДля DFS используется ограниченный перебор в глубину.\n");
        maxDepth = readInt("Введите максимальную глубину просмотра (1..Q): ", 1, Q);
    }

    SearchStats bfsStats;
    SearchStats dfsIterStats;
    SearchStats dfsRecStats;
    SearchStats dfsPathStats;

    bool hasBFS = false;
    bool hasDFSIter = false;
    bool hasDFSRec = false;
    bool hasDFSPath = false;

    if (method == METHOD_BFS || method == METHOD_ALL) {
        printf("\n--- Поиск в ШИРИНУ (BFS) ---\n");
        bfsStats = solveBFS(N, Q);
        hasBFS = true;
        printNoSolutionIfNeeded("BFS", N, Q, &bfsStats);
        printStats("BFS", &bfsStats);
    }

    if (method == METHOD_DFS_ITER || method == METHOD_ALL) {
        printf("\n--- Поиск в ГЛУБИНУ (DFS, итерационный, maxDepth=%d) ---\n", maxDepth);
        dfsIterStats = solveDFSIterative(N, Q, maxDepth);
        hasDFSIter = true;
        printNoSolutionIfNeeded("DFS итерационный", N, Q, &dfsIterStats);
        printStats("DFS итерационный", &dfsIterStats);
    }

    if (method == METHOD_DFS_REC || method == METHOD_ALL) {
        printf("\n--- Поиск в ГЛУБИНУ (DFS, рекурсивный, maxDepth=%d) ---\n", maxDepth);
        dfsRecStats = solveDFSRecursive(N, Q, maxDepth);
        hasDFSRec = true;
        printNoSolutionIfNeeded("DFS рекурсивный", N, Q, &dfsRecStats);
        printStats("DFS рекурсивный", &dfsRecStats);
    }

    if (method == METHOD_DFS_REC_PATH || method == METHOD_ALL) {
        printf("\n--- Поиск в ГЛУБИНУ (DFS, рекурсивный с Path, maxDepth=%d) ---\n", maxDepth);
        dfsPathStats = solveDFSRecursiveWithPath(N, Q, maxDepth);
        hasDFSPath = true;
        printNoSolutionIfNeeded("DFS рекурсивный с Path", N, Q, &dfsPathStats);
        printStats("DFS рекурсивный с Path", &dfsPathStats);
    }

    if (method == METHOD_ALL) {
        printBestSummary(hasBFS ? &bfsStats : NULL, hasDFSIter ? &dfsIterStats : NULL, hasDFSRec ? &dfsRecStats : NULL, hasDFSPath ? &dfsPathStats : NULL);
    }

    return 0;
}
