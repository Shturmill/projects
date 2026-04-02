#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
    int  size;
    int **adj;
} Graph;

typedef struct {
    int *data;
    int  size;
    int  capacity;
} IntList;

typedef struct {
    int *data;
    int  head;
    int  tail;
    int  size;
    int  capacity;
} IntQueue;

typedef struct {
    int *data;
    int  size;
    int  capacity;
} IntStack;

typedef enum {
    SEARCH_ERROR     = -1,
    SEARCH_NOT_FOUND =  0,
    SEARCH_FOUND     =  1
} SearchStatus;

typedef struct {
    SearchStatus status;
    int          steps;
    IntList      path;
} SearchResult;

typedef enum {
    ALG_BFS,
    ALG_DFS_ITER,
    ALG_DFS_REC,
    ALG_DFS_REC_PATH,
    ALG_COMPARE,
    ALG_UNKNOWN
} Algorithm;


static void list_init(IntList *list)
{
    list->data     = NULL;
    list->size     = 0;
    list->capacity = 0;
}


static void list_free(IntList *list)
{
    free(list->data);
    list->data     = NULL;
    list->size     = 0;
    list->capacity = 0;
}

static int list_resize(IntList *list, int new_capacity)
{
    int *tmp = realloc(list->data, (size_t)new_capacity * sizeof(int));
    if (tmp == NULL)
        return 0;

    list->data     = tmp;
    list->capacity = new_capacity;
    return 1;
}

static int list_push_back(IntList *list, int value)
{
    if (list->size == list->capacity) {
        int new_cap = (list->capacity == 0) ? 4 : list->capacity * 2;
        if (!list_resize(list, new_cap))
            return 0;
    }

    list->data[list->size++] = value;
    return 1;
}

static int list_pop_back(IntList *list)
{
    if (list->size == 0)
        return 0;

    list->size--;
    return 1;
}

//Int Queue

static void queue_init(IntQueue *queue)
{
    queue->data     = NULL;
    queue->head     = 0;
    queue->tail     = 0;
    queue->size     = 0;
    queue->capacity = 0;
}

static void queue_free(IntQueue *queue)
{
    free(queue->data);
    queue->data     = NULL;
    queue->head     = 0;
    queue->tail     = 0;
    queue->size     = 0;
    queue->capacity = 0;
}

static int queue_create(IntQueue *queue, int capacity)
{
    queue->data = malloc((size_t)capacity * sizeof(int));
    if (queue->data == NULL)
        return 0;

    queue->head     = 0;
    queue->tail     = 0;
    queue->size     = 0;
    queue->capacity = capacity;
    return 1;
}

static int queue_is_empty(const IntQueue *queue)
{
    return queue->size == 0;
}

static int queue_push(IntQueue *queue, int value)
{
    if (queue->size == queue->capacity)
        return 0;

    queue->data[queue->tail] = value;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->size++;
    return 1;
}

static int queue_pop(IntQueue *queue, int *value)
{
    if (queue->size == 0)
        return 0;

    *value      = queue->data[queue->head];
    queue->head = (queue->head + 1) % queue->capacity;
    queue->size--;
    return 1;
}

//Int Stacks

static void stack_init(IntStack *stack)
{
    stack->data     = NULL;
    stack->size     = 0;
    stack->capacity = 0;
}

static void stack_free(IntStack *stack)
{
    free(stack->data);
    stack->data     = NULL;
    stack->size     = 0;
    stack->capacity = 0;
}

static int stack_resize(IntStack *stack, int new_capacity)
{
    int *tmp = realloc(stack->data, (size_t)new_capacity * sizeof(int));
    if (tmp == NULL)
        return 0;

    stack->data     = tmp;
    stack->capacity = new_capacity;
    return 1;
}

static int stack_push(IntStack *stack, int value)
{
    if (stack->size == stack->capacity) {
        int new_cap = (stack->capacity == 0) ? 4 : stack->capacity * 2;
        if (!stack_resize(stack, new_cap))
            return 0;
    }

    stack->data[stack->size++] = value;
    return 1;
}

static int stack_pop(IntStack *stack, int *value)
{
    if (stack->size == 0)
        return 0;

    *value = stack->data[--stack->size];
    return 1;
}

static int stack_is_empty(const IntStack *stack)
{
    return stack->size == 0;
}

// Graph Methods
static void graph_init_empty(Graph *g)
{
    g->size = 0;
    g->adj  = NULL;
}

static int graph_init(Graph *g, int n)
{
    int i;

    g->size = n;
    g->adj  = malloc((size_t)(n + 1) * sizeof(int *));
    if (g->adj == NULL)
        return 0;

    for (i = 0; i <= n; i++) {
        g->adj[i] = calloc((size_t)(n + 1), sizeof(int));
        if (g->adj[i] == NULL) {
            for (int j = 0; j < i; j++)
                free(g->adj[j]);
            free(g->adj);
            g->adj  = NULL;
            g->size = 0;
            return 0;
        }
    }

    return 1;
}

static void graph_free(Graph *g)
{
    if (g == NULL || g->adj == NULL)
        return;

    for (int i = 0; i <= g->size; i++)
        free(g->adj[i]);
    free(g->adj);

    g->adj  = NULL;
    g->size = 0;
}

static void graph_add_edge(Graph *g, int from, int to)
{
    if (from < 1 || from > g->size || to < 1 || to > g->size)
        return;
    g->adj[from][to] = 1;
}

static int graph_read_file(const char *filename, Graph *out)
{
    FILE *f;
    int   n;

    graph_init_empty(out);

    f = fopen(filename, "r");
    if (f == NULL) {
        fprintf(stderr, "Не удалось открыть файл %s\n", filename);
        return 0;
    }

    if (fscanf(f, "%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "Ошибка чтения числа вершин\n");
        fclose(f);
        return 0;
    }

    if (!graph_init(out, n)) {
        fprintf(stderr, "Ошибка выделения памяти для графа\n");
        fclose(f);
        return 0;
    }

    for (int i = 0; i < n; i++) {
        int v;
        if (fscanf(f, "%d", &v) != 1 || v < 1 || v > n) {
            fprintf(stderr, "Ошибка чтения номера вершины\n");
            fclose(f);
            graph_free(out);
            return 0;
        }

        for (;;) {
            int to;
            if (fscanf(f, "%d", &to) != 1) {
                fprintf(stderr, "Ошибка чтения смежной вершины\n");
                fclose(f);
                graph_free(out);
                return 0;
            }

            if (to == 0)
                break;

            if (to < 1 || to > n) {
                fprintf(stderr, "Некорректная смежная вершина: %d\n", to);
                fclose(f);
                graph_free(out);
                return 0;
            }

            graph_add_edge(out, v, to);
        }
    }

    fclose(f);
    return 1;
}

static void result_init(SearchResult *res)
{
    res->status = SEARCH_NOT_FOUND;
    res->steps  = 0;
    list_init(&res->path);
}

static void result_free(SearchResult *res)
{
    list_free(&res->path);
    res->status = SEARCH_NOT_FOUND;
    res->steps  = 0;
}

// parent[v] == вершина, из которой впервые пришли в v (-1 если нет).
// Возвращает 1 при успехе, 0 при ошибке памяти.

static int build_path(int start, int goal, const int *parent, int n, IntList *path) {
    int current = goal;
    IntList temp;
    list_init(&temp);

    // Если пути до целевой вершины нет в массиве parent
    if (parent[goal] == -1 && start != goal) return 1;

    // Сборка пути в обратном порядке (от Goal до Start)
    while (current != -1) {
        list_push_back(&temp, current);
        if (current == start) break;
        current = parent[current];
    }

    // Перенос пути в выходящий список в правильном порядке
    for (int i = temp.size - 1; i >= 0; i--) {
        if (!list_push_back(path, temp.data[i])) {
            list_free(&temp);
            return 0;
        }
    }

    list_free(&temp);
    return 1;
}


SearchResult bfs(const Graph *g, int start, int goal) {
    SearchResult res;
    IntQueue open;
    int *parent = NULL;
    unsigned char *in_open   = NULL;
    unsigned char *in_closed = NULL;

    result_init(&res);
    queue_init(&open);

    parent    = malloc((size_t)(g->size + 1) * sizeof(int));
    in_open   = calloc((size_t)(g->size + 1), sizeof(unsigned char));
    in_closed = calloc((size_t)(g->size + 1), sizeof(unsigned char));

    if (!parent || !in_open || !in_closed
            || !queue_create(&open, g->size * g->size + 2)) {
        res.status = SEARCH_ERROR;
        goto cleanup;
    }

    for (int i = 0; i <= g->size; i++) parent[i] = -1;

    // Open = [Start]; Closed = []
    queue_push(&open, start);
    in_open[start] = 1;

    // While Open <> [] do
    while (!queue_is_empty(&open)) {
        int x;
        // X = первая вершина из Open; удалить X; добавить в Closed
        queue_pop(&open, &x);
        in_open[x]  = 0;
        in_closed[x] = 1;
        res.steps++;

        // Для каждого потомка X:
        for (int child = 1; child <= g->size; child++) {
            // If X = цель → вернуть True
            if (x == goal) {
                res.status = build_path(start, goal, parent, g->size, &res.path)
                             ? SEARCH_FOUND : SEARCH_ERROR;
                goto cleanup;
            }
            // Else If потомок не в Open и не в Closed -> добавить в конец Open
            if (g->adj[x][child] == 1 && !in_open[child] && !in_closed[child]) {
                queue_push(&open, child);
                in_open[child] = 1;
                parent[child]  = x;
            }
        }
    }

cleanup:
    queue_free(&open); free(parent); free(in_open); free(in_closed);
    return res;
}

SearchResult dfs_iterative(const Graph *g, int start, int goal) {
    SearchResult res;
    IntStack open;
    int *parent    = NULL;
    unsigned char *in_open   = NULL;
    unsigned char *in_closed = NULL;

    result_init(&res);
    stack_init(&open);

    parent    = malloc((size_t)(g->size + 1) * sizeof(int));
    in_open   = calloc((size_t)(g->size + 1), sizeof(unsigned char));
    in_closed = calloc((size_t)(g->size + 1), sizeof(unsigned char));

    if (!parent || !in_open || !in_closed) {
        res.status = SEARCH_ERROR;
        goto cleanup;
    }
    for (int i = 0; i <= g->size; i++) parent[i] = -1;

    // Open = [Start]; Closed = []
    stack_push(&open, start);
    in_open[start] = 1;

    // While Open <> [] do
    while (!stack_is_empty(&open)) {
        int x;
        // X = первая вершина из Open; удалить X; добавить в Closed
        stack_pop(&open, &x);
        in_open[x]   = 0;
        in_closed[x] = 1;
        res.steps++;

        // Для каждого потомка X
        for (int child = g->size; child >= 1; child--) {
            // If X = цель -> вернуть True
            if (x == goal) {
                res.status = build_path(start, goal, parent, g->size, &res.path)
                             ? SEARCH_FOUND : SEARCH_ERROR;
                goto cleanup;
            }
            // Else If потомок не в Open и не в Closed -> добавить в начало Open
            if (g->adj[x][child] == 1 && !in_open[child] && !in_closed[child]) {
                stack_push(&open, child);
                in_open[child] = 1;
                parent[child]  = x;
            }
        }

        if (x == goal) {
            res.status = build_path(start, goal, parent, g->size, &res.path)
                         ? SEARCH_FOUND : SEARCH_ERROR;
            goto cleanup;
        }
    }

cleanup:
    stack_free(&open); free(parent); free(in_open); free(in_closed);
    return res;
}

static int dfs_rec_impl(const Graph *g, int x, int goal,
                        unsigned char *closed, int *parent, int *steps) {
    // Добавить X в Closed
    closed[x] = 1;
    (*steps)++;

    // If X = цель -> вернуть True  (до цикла по потомкам)
    if (x == goal) return 1;

    // Для каждого child X: Else If child не в Closed -> DepthSearch(child)
    for (int child = 1; child <= g->size; child++) {
        if (g->adj[x][child] == 1 && !closed[child]) {
            parent[child] = x;
            if (dfs_rec_impl(g, child, goal, closed, parent, steps))
                return 1;
        }
    }

    return 0;
}

static SearchResult dfs_recursive(const Graph *g, int start, int goal)
{
    SearchResult   res;
    unsigned char *visited = NULL;
    int           *parent  = NULL;

    result_init(&res);


    if (start == goal) {
        list_push_back(&res.path, start);
        res.status = SEARCH_FOUND;
        res.steps  = 1;
        return res;
    }

    visited = calloc((size_t)(g->size + 1), sizeof(unsigned char));
    parent  = malloc((size_t)(g->size + 1) * sizeof(int));

    if (!visited || !parent) {
        fprintf(stderr, "Ошибка выделения памяти (DFS rec)\n");
        res.status = SEARCH_ERROR;
        goto cleanup;
    }

    for (int i = 0; i <= g->size; i++)
        parent[i] = -1;

    if (dfs_rec_impl(g, start, goal, visited, parent, &res.steps)) {
        res.status = build_path(start, goal, parent, g->size, &res.path)
                     ? SEARCH_FOUND : SEARCH_ERROR;
    }

cleanup:
    free(visited);
    free(parent);
    return res;
}

/*
 * Возвращает:
 *   1  — путь найден
 *   0  — путь не найден в этой ветке
 *  -1  — ошибка памяти
 */

static int dfs_rec_path_impl(const Graph *g, int x, int goal, unsigned char *closed, IntList *path, int *steps) {
    // Поместить X в Closed и добавить в текущий путь Path
    closed[x] = 1;
    (*steps)++;
    if (!list_push_back(path, x)) return -1;
 
    // Проверка цели
    if (x == goal) return 1;
 
    // Перебор потомков
    for (int child = 1; child <= g->size; child++) {
        if (g->adj[x][child] == 1 && !closed[child]) {
            // Рекурсивный поиск от потомка с обновленным Path
            int result = dfs_rec_path_impl(g, child, goal, closed, path, steps);
            if (result != 0) return result;
        }
    }
 
    // Откат. Если цель не найдена, удалить X из Path
    list_pop_back(path);
    return 0;
}

static SearchResult dfs_recursive_with_path(const Graph *g, int start, int goal)
{
    SearchResult   res;
    unsigned char *visited = NULL;
    int            r;

    result_init(&res);

    if (start == goal) {
        list_push_back(&res.path, start);
        res.status = SEARCH_FOUND;
        res.steps  = 1;
        return res;
    }

    visited = calloc((size_t)(g->size + 1), sizeof(unsigned char));
    if (!visited) {
        fprintf(stderr, "Ошибка выделения памяти (DFS rec path)\n");
        res.status = SEARCH_ERROR;
        return res;
    }

    r = dfs_rec_path_impl(g, start, goal, visited, &res.path, &res.steps);
    if (r < 0)
        res.status = SEARCH_ERROR;
    else if (r > 0)
        res.status = SEARCH_FOUND;

    free(visited);
    return res;
}

// OUTPUT

static void print_path(const IntList *path)
{
    for (int i = 0; i < path->size; i++) {
        if (i > 0)
            printf(" ");
        printf("%d", path->data[i]);
    }
    printf("\n");
}

static void print_result(const char *name, const SearchResult *res)
{
    printf("ALGORITHM: %s\n", name);

    switch (res->status) {
    case SEARCH_FOUND:
        printf("STATUS: FOUND\n");
        printf("STEPS: %d\n", res->steps);
        printf("PATH: ");
        print_path(&res->path);
        break;

    case SEARCH_NOT_FOUND:
        printf("STATUS: NOT_FOUND\n");
        printf("STEPS: %d\n", res->steps);
        printf("PATH:\n");
        break;

    case SEARCH_ERROR:
        printf("STATUS: ERROR\n");
        printf("STEPS: %d\n", res->steps);
        printf("PATH:\n");
        break;
    }
}

static void print_compare(const SearchResult *bfs_r,
                          const SearchResult *dfs_iter_r,
                          const SearchResult *dfs_rec_r,
                          const SearchResult *dfs_rec_path_r)
{
    const char *names[4]          = { "bfs", "dfs_iter", "dfs_rec", "dfs_rec_path" };
    const SearchResult *results[4] = { bfs_r, dfs_iter_r, dfs_rec_r, dfs_rec_path_r };
    const char *best_name          = NULL;
    int         best_steps         = INT_MAX;

    for (int i = 0; i < 4; i++) {
        print_result(names[i], results[i]);
        if (i < 3)
            printf("---\n");
    }
    printf("===\n");

    for (int i = 0; i < 4; i++) {
        if (results[i]->status == SEARCH_FOUND && results[i]->steps < best_steps) {
            best_steps = results[i]->steps;
            best_name  = names[i];
        }
    }

    if (best_name != NULL) {
        printf("BEST_BY_STEPS: %s\n", best_name);
        printf("BEST_STEPS: %d\n", best_steps);
    } else {
        printf("BEST_BY_STEPS: NONE\n");
        printf("BEST_STEPS: -1\n");
    }
}


static int parse_int(const char *text, int *value)
{
    char *end;
    long  parsed;

    errno  = 0;
    parsed = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0')
        return 0;
    if (parsed < INT_MIN || parsed > INT_MAX)
        return 0;

    *value = (int)parsed;
    return 1;
}

static void print_usage(const char *prog)
{
    fprintf(stderr, "Usage: %s <graph_file> <start> <goal> <algorithm>\n", prog);
    fprintf(stderr, "Algorithms: bfs | dfs_iter | dfs_rec | dfs_rec_path | compare\n");
}

static Algorithm parse_algorithm(const char *s)
{
    if (strcmp(s, "bfs")          == 0) return ALG_BFS;
    if (strcmp(s, "dfs_iter")     == 0) return ALG_DFS_ITER;
    if (strcmp(s, "dfs_rec")      == 0) return ALG_DFS_REC;
    if (strcmp(s, "dfs_rec_path") == 0) return ALG_DFS_REC_PATH;
    if (strcmp(s, "compare")      == 0) return ALG_COMPARE;
    return ALG_UNKNOWN;
}

int main(int argc, char *argv[])
{
    const char *filename;
    int         start, goal;
    Algorithm   alg;
    Graph       g;
    int         exit_code = 0;

    if (argc != 5) {
        print_usage(argv[0]);
        return 1;
    }

    filename = argv[1];

    if (!parse_int(argv[2], &start)) {
        fprintf(stderr, "Некорректная начальная вершина: %s\n", argv[2]);
        return 1;
    }
    if (!parse_int(argv[3], &goal)) {
        fprintf(stderr, "Некорректная целевая вершина: %s\n", argv[3]);
        return 1;
    }

    alg = parse_algorithm(argv[4]);
    if (alg == ALG_UNKNOWN) {
        fprintf(stderr, "Неизвестный алгоритм: %s\n", argv[4]);
        print_usage(argv[0]);
        return 1;
    }

    if (!graph_read_file(filename, &g))
        return 1;

    if (start < 1 || start > g.size || goal < 1 || goal > g.size) {
        fprintf(stderr, "Вершины вне диапазона 1..%d\n", g.size);
        graph_free(&g);
        return 1;
    }

    switch (alg) {
    case ALG_BFS: {
        SearchResult res = bfs(&g, start, goal);
        print_result("bfs", &res);
        exit_code = (res.status == SEARCH_ERROR) ? 1 : 0;
        result_free(&res);
        break;
    }
    case ALG_DFS_ITER: {
        SearchResult res = dfs_iterative(&g, start, goal);
        print_result("dfs_iter", &res);
        exit_code = (res.status == SEARCH_ERROR) ? 1 : 0;
        result_free(&res);
        break;
    }
    case ALG_DFS_REC: {
        SearchResult res = dfs_recursive(&g, start, goal);
        print_result("dfs_rec", &res);
        exit_code = (res.status == SEARCH_ERROR) ? 1 : 0;
        result_free(&res);
        break;
    }
    case ALG_DFS_REC_PATH: {
        SearchResult res = dfs_recursive_with_path(&g, start, goal);
        print_result("dfs_rec_path", &res);
        exit_code = (res.status == SEARCH_ERROR) ? 1 : 0;
        result_free(&res);
        break;
    }
    case ALG_COMPARE: {
        SearchResult bfs_r      = bfs(&g, start, goal);
        SearchResult dfs_iter_r = dfs_iterative(&g, start, goal);
        SearchResult dfs_rec_r  = dfs_recursive(&g, start, goal);
        SearchResult dfs_path_r = dfs_recursive_with_path(&g, start, goal);

        print_compare(&bfs_r, &dfs_iter_r, &dfs_rec_r, &dfs_path_r);

        if (bfs_r.status == SEARCH_ERROR || dfs_iter_r.status == SEARCH_ERROR
                || dfs_rec_r.status == SEARCH_ERROR || dfs_path_r.status == SEARCH_ERROR)
            exit_code = 1;

        result_free(&bfs_r);
        result_free(&dfs_iter_r);
        result_free(&dfs_rec_r);
        result_free(&dfs_path_r);
        break;
    }
    case ALG_UNKNOWN:
        break;
    }

    graph_free(&g);
    return exit_code;
}