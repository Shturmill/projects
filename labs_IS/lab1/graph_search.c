#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
    int size;
    int **adj;
} Graph;


typedef struct {
    int *data;
    int size;
    int capacity;
} IntList;


typedef struct {
    int *data;
    int head;
    int tail;
    int size;
    int capacity;
} IntQueue;


typedef struct {
    int *data;
    int size;
    int capacity;
} IntStack;

typedef enum {
    SEARCH_ERROR = -1,
    SEARCH_NOT_FOUND = 0,
    SEARCH_FOUND = 1
} SearchStatus;

typedef struct {
    SearchStatus status;
    int steps; 
    IntList path;
} SearchResult;

//Int list methods

void list_init(IntList *list) {
    list->data = NULL;
    list->size = 0;
    list->capacity = 0;
}

void list_free(IntList *list) {
    free(list->data);
    list->data = NULL;
    list->size = 0;
    list->capacity = 0;
}

int list_resize(IntList *list, int new_capacity) {
    int *tmp = (int *)realloc(list->data, (size_t)new_capacity * sizeof(int));
    if (tmp == NULL) {
        return 0;
    }

    list->data = tmp;
    list->capacity = new_capacity;
    return 1;
}

int list_push_back(IntList *list, int value) {
    if (list->size == list->capacity) {
        int new_capacity = (list->capacity == 0) ? 4 : list->capacity * 2;
        if (!list_resize(list, new_capacity)) {
            return 0;
        }
    }

    list->data[list->size] = value;
    list->size++;
    return 1;
}

int list_pop_back(IntList *list) {
    if (list->size == 0) {
        return 0;
    }

    list->size--;
    return 1;
}

// methods fot bfs

void queue_init(IntQueue *queue) {
    queue->data = NULL;
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
    queue->capacity = 0;
}

void queue_free(IntQueue *queue) {
    free(queue->data);
    queue->data = NULL;
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
    queue->capacity = 0;
}

int queue_create(IntQueue *queue, int capacity) {
    queue->data = (int *)malloc((size_t)capacity * sizeof(int));
    if (queue->data == NULL) {
        return 0;
    }

    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
    queue->capacity = capacity;
    return 1;
}

int queue_is_empty(const IntQueue *queue) {
    return queue->size == 0;
}

int queue_push(IntQueue *queue, int value) {
    if (queue->size == queue->capacity) {
        return 0;
    }

    queue->data[queue->tail] = value;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->size++;
    return 1;
}

int queue_pop(IntQueue *queue, int *value) {
    if (queue->size == 0) {
        return 0;
    }

    *value = queue->data[queue->head];
    queue->head = (queue->head + 1) % queue->capacity;
    queue->size--;
    return 1;
}

// for iterative dfs

void stack_init(IntStack *stack) {
    stack->data = NULL;
    stack->size = 0;
    stack->capacity = 0;
}

void stack_free(IntStack *stack) {
    free(stack->data);
    stack->data = NULL;
    stack->size = 0;
    stack->capacity = 0;
}

int stack_resize(IntStack *stack, int new_capacity) {
    int *tmp = (int *)realloc(stack->data, (size_t)new_capacity * sizeof(int));
    if (tmp == NULL) {
        return 0;
    }

    stack->data = tmp;
    stack->capacity = new_capacity;
    return 1;
}

int stack_push(IntStack *stack, int value) {
    if (stack->size == stack->capacity) {
        int new_capacity = (stack->capacity == 0) ? 4 : stack->capacity * 2;
        if (!stack_resize(stack, new_capacity)) {
            return 0;
        }
    }

    stack->data[stack->size] = value;
    stack->size++;
    return 1;
}

int stack_pop(IntStack *stack, int *value) {
    if (stack->size == 0) {
        return 0;
    }

    stack->size--;
    *value = stack->data[stack->size];
    return 1;
}

int stack_is_empty(const IntStack *stack) {
    return stack->size == 0;
}

//GRAPH

void graph_init_empty(Graph *g) {
    g->size = 0;
    g->adj = NULL;
}

int graph_init(Graph *g, int n) {
    g->size = n;
    g->adj = (int **)malloc((size_t)(n + 1) * sizeof(int *));
    if (g->adj == NULL) {
        return 0;
    }

    for (int i = 0; i <= n; i++) {
        g->adj[i] = (int *)calloc((size_t)(n + 1), sizeof(int));
        if (g->adj[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(g->adj[j]);
            }
            free(g->adj);
            g->adj = NULL;
            g->size = 0;
            return 0;
        }
    }

    return 1;
}

void free_graph(Graph *g) {
    if (g == NULL || g->adj == NULL) {
        return;
    }

    for (int i = 0; i <= g->size; i++) {
        free(g->adj[i]);
    }
    free(g->adj);

    g->adj = NULL;
    g->size = 0;
}

void add_edge(Graph *g, int from, int to) {
    if (from < 1 || from > g->size || to < 1 || to > g->size) {
        return;
    }
    g->adj[from][to] = 1;
}

int read_graph_from_file(const char *filename, Graph *out_graph) {
    FILE *f = fopen(filename, "r");
    int n;

    graph_init_empty(out_graph);

    if (f == NULL) {
        fprintf(stderr, "Не удалось открыть файл %s\n", filename);
        return 0;
    }

    if (fscanf(f, "%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "Ошибка чтения числа вершин\n");
        fclose(f);
        return 0;
    }

    if (!graph_init(out_graph, n)) {
        fprintf(stderr, "Ошибка выделения памяти для графа\n");
        fclose(f);
        return 0;
    }

    for (int i = 0; i < n; i++) {
        int v;
        if (fscanf(f, "%d", &v) != 1) {
            fprintf(stderr, "Ошибка чтения номера вершины\n");
            fclose(f);
            free_graph(out_graph);
            return 0;
        }

        if (v < 1 || v > n) {
            fprintf(stderr, "Некорректный номер вершины: %d\n", v);
            fclose(f);
            free_graph(out_graph);
            return 0;
        }

        while (1) {
            int to;
            if (fscanf(f, "%d", &to) != 1) {
                fprintf(stderr, "Ошибка чтения смежной вершины\n");
                fclose(f);
                free_graph(out_graph);
                return 0;
            }

            if (to == 0) {
                break;
            }

            if (to < 1 || to > n) {
                fprintf(stderr, "Некорректная смежная вершина: %d\n", to);
                fclose(f);
                free_graph(out_graph);
                return 0;
            }

            add_edge(out_graph, v, to);
        }
    }

    fclose(f);
    return 1;
}


void result_init(SearchResult *res) {
    res->status = SEARCH_NOT_FOUND;
    res->steps = 0;
    list_init(&res->path);
}

void result_free(SearchResult *res) {
    list_free(&res->path);
    res->status = SEARCH_NOT_FOUND;
    res->steps = 0;
}



 //Восстанавливаем путь по массиву parent.
 //parent[v] = вершина, из которой впервые пришли в v.
 
int build_path(int start, int goal, const int *parent, int n, IntList *path) {
    int *reverse_path = NULL;
    int count = 0;
    int current = goal;

    if (start == goal) {
        return list_push_back(path, start);
    }

    if (parent[goal] == -1) {
        return 1;
    }

    reverse_path = (int *)malloc((size_t)(n + 1) * sizeof(int));
    if (reverse_path == NULL) {
        return 0;
    }

    while (current != -1) {
        if (count > n) {
            free(reverse_path);
            return 0;
        }

        reverse_path[count] = current;
        count++;

        if (current == start) {
            break;
        }
        current = parent[current];
    }

    for (int i = count - 1; i >= 0; i--) {
        if (!list_push_back(path, reverse_path[i])) {
            free(reverse_path);
            return 0;
        }
    }

    free(reverse_path);
    return 1;
}

void print_path_machine(const IntList *path) {
    for (int i = 0; i < path->size; i++) {
        printf("%d", path->data[i]);
        if (i + 1 < path->size) {
            printf(" ");
        }
    }
    printf("\n");
}


SearchResult bfs(const Graph *g, int start, int goal) {
    SearchResult res;
    IntQueue queue;
    int *parent = NULL;
    unsigned char *state = NULL; /* 0 = unseen, 1 = in_open, 2 = processed */

    result_init(&res);
    queue_init(&queue);

    parent = (int *)malloc((size_t)(g->size + 1) * sizeof(int));
    state = (unsigned char *)calloc((size_t)(g->size + 1), sizeof(unsigned char));
    if (parent == NULL || state == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для BFS\n");
        res.status = SEARCH_ERROR;
        free(parent);
        free(state);
        return res;
    }

    if (!queue_create(&queue, g->size + 1)) {
        fprintf(stderr, "Ошибка выделения памяти для очереди BFS\n");
        res.status = SEARCH_ERROR;
        free(parent);
        free(state);
        return res;
    }

    for (int i = 0; i <= g->size; i++) {
        parent[i] = -1;
    }

    if (!queue_push(&queue, start)) {
        fprintf(stderr, "Ошибка помещения вершины в очередь BFS\n");
        res.status = SEARCH_ERROR;
        queue_free(&queue);
        free(parent);
        free(state);
        return res;
    }
    state[start] = 1;

    while (!queue_is_empty(&queue)) {
        int x;
        if (!queue_pop(&queue, &x)) {
            break;
        }

        state[x] = 2;
        res.steps++;

        if (x == goal) {
            if (!build_path(start, goal, parent, g->size, &res.path)) {
                fprintf(stderr, "Ошибка выделения памяти при восстановлении пути BFS\n");
                res.status = SEARCH_ERROR;
            } else {
                res.status = SEARCH_FOUND;
            }
            queue_free(&queue);
            free(parent);
            free(state);
            return res;
        }

        for (int child = 1; child <= g->size; child++) {
            if (g->adj[x][child] == 1 && state[child] == 0) {
                if (!queue_push(&queue, child)) {
                    fprintf(stderr, "Ошибка помещения вершины в очередь BFS\n");
                    res.status = SEARCH_ERROR;
                    queue_free(&queue);
                    free(parent);
                    free(state);
                    return res;
                }
                state[child] = 1;
                parent[child] = x;
            }
        }
    }

    queue_free(&queue);
    free(parent);
    free(state);
    return res;
}


SearchResult dfs_iterative(const Graph *g, int start, int goal) {
    SearchResult res;
    IntStack stack;
    int *parent = NULL;
    unsigned char *state = NULL; /* 0 = unseen, 1 = in_open, 2 = processed */

    result_init(&res);
    stack_init(&stack);

    parent = (int *)malloc((size_t)(g->size + 1) * sizeof(int));
    state = (unsigned char *)calloc((size_t)(g->size + 1), sizeof(unsigned char));
    if (parent == NULL || state == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для DFS\n");
        res.status = SEARCH_ERROR;
        free(parent);
        free(state);
        return res;
    }

    for (int i = 0; i <= g->size; i++) {
        parent[i] = -1;
    }

    if (!stack_push(&stack, start)) {
        fprintf(stderr, "Ошибка помещения вершины в стек DFS\n");
        res.status = SEARCH_ERROR;
        stack_free(&stack);
        free(parent);
        free(state);
        return res;
    }
    state[start] = 1;

    while (!stack_is_empty(&stack)) {
        int x;
        if (!stack_pop(&stack, &x)) {
            break;
        }

        state[x] = 2;
        res.steps++;

        if (x == goal) {
            if (!build_path(start, goal, parent, g->size, &res.path)) {
                fprintf(stderr, "Ошибка выделения памяти при восстановлении пути DFS\n");
                res.status = SEARCH_ERROR;
            } else {
                res.status = SEARCH_FOUND;
            }
            stack_free(&stack);
            free(parent);
            free(state);
            return res;
        }

        /*
         * Идём с конца, чтобы стек дал привычный порядок обхода:
         * меньшие номера вершин будут обрабатываться раньше.
         */
        for (int child = g->size; child >= 1; child--) {
            if (g->adj[x][child] == 1 && state[child] == 0) {
                if (!stack_push(&stack, child)) {
                    fprintf(stderr, "Ошибка помещения вершины в стек DFS\n");
                    res.status = SEARCH_ERROR;
                    stack_free(&stack);
                    free(parent);
                    free(state);
                    return res;
                }
                state[child] = 1;
                parent[child] = x;
            }
        }
    }

    stack_free(&stack);
    free(parent);
    free(state);
    return res;
}


int dfs_recursive_impl(const Graph *g, int x, int goal,
                       unsigned char *visited, int *parent, int *steps) {
    visited[x] = 1;
    (*steps)++;

    if (x == goal) {
        return 1;
    }

    for (int child = 1; child <= g->size; child++) {
        if (g->adj[x][child] == 1 && !visited[child]) {
            parent[child] = x;
            if (dfs_recursive_impl(g, child, goal, visited, parent, steps)) {
                return 1;
            }
        }
    }

    return 0;
}

SearchResult dfs_recursive(const Graph *g, int start, int goal) {
    SearchResult res;
    unsigned char *visited = NULL;
    int *parent = NULL;

    result_init(&res);

    visited = (unsigned char *)calloc((size_t)(g->size + 1), sizeof(unsigned char));
    parent = (int *)malloc((size_t)(g->size + 1) * sizeof(int));
    if (visited == NULL || parent == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для рекурсивного DFS\n");
        res.status = SEARCH_ERROR;
        free(visited);
        free(parent);
        return res;
    }

    for (int i = 0; i <= g->size; i++) {
        parent[i] = -1;
    }

    if (dfs_recursive_impl(g, start, goal, visited, parent, &res.steps)) {
        if (!build_path(start, goal, parent, g->size, &res.path)) {
            fprintf(stderr, "Ошибка выделения памяти при восстановлении пути dfs_rec\n");
            res.status = SEARCH_ERROR;
        } else {
            res.status = SEARCH_FOUND;
        }
    }

    free(visited);
    free(parent);
    return res;
}

/* =========================
   DFS recursive with direct path building
   ========================= */

/*
 * Возвращает:
 *   1  - путь найден
 *   0  - путь не найден
 *  -1  - ошибка памяти
 */
int dfs_recursive_path_impl(const Graph *g, int x, int goal,
                            unsigned char *visited, IntList *path, int *steps) {
    visited[x] = 1;
    (*steps)++;

    if (!list_push_back(path, x)) {
        return -1;
    }

    if (x == goal) {
        return 1;
    }

    for (int child = 1; child <= g->size; child++) {
        if (g->adj[x][child] == 1 && !visited[child]) {
            int child_result = dfs_recursive_path_impl(g, child, goal, visited, path, steps);
            if (child_result != 0) {
                return child_result;
            }
        }
    }

    if (!list_pop_back(path)) {
        return -1;
    }
    return 0;
}

SearchResult dfs_recursive_with_path(const Graph *g, int start, int goal) {
    SearchResult res;
    unsigned char *visited = NULL;
    int dfs_result;

    result_init(&res);

    visited = (unsigned char *)calloc((size_t)(g->size + 1), sizeof(unsigned char));
    if (visited == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для dfs_rec_path\n");
        res.status = SEARCH_ERROR;
        return res;
    }

    dfs_result = dfs_recursive_path_impl(g, start, goal, visited, &res.path, &res.steps);
    if (dfs_result < 0) {
        fprintf(stderr, "Ошибка выделения памяти при построении пути dfs_rec_path\n");
        res.status = SEARCH_ERROR;
    } else if (dfs_result > 0) {
        res.status = SEARCH_FOUND;
    }

    free(visited);
    return res;
}

//OUTPUT

void print_single_result(const char *name, const SearchResult *res) {
    printf("ALGORITHM: %s\n", name);

    if (res->status == SEARCH_ERROR) {
        printf("STATUS: ERROR\n");
        printf("STEPS: %d\n", res->steps);
        printf("PATH:\n");
        return;
    }

    if (res->status == SEARCH_FOUND) {
        printf("STATUS: FOUND\n");
    } else {
        printf("STATUS: NOT_FOUND\n");
    }

    printf("STEPS: %d\n", res->steps);
    printf("PATH: ");
    print_path_machine(&res->path);
}

void print_legacy_result(const SearchResult *res) {
    if (res->status == SEARCH_ERROR) {
        printf("ERROR\n");
        printf("STEPS: %d\n", res->steps);
        printf("PATH:\n");
        return;
    }

    if (res->status == SEARCH_FOUND) {
        printf("FOUND\n");
    } else {
        printf("NOT_FOUND\n");
    }
    printf("STEPS: %d\n", res->steps);
    printf("PATH: ");
    print_path_machine(&res->path);
}

void print_compare_summary(const SearchResult *bfs_res,
                           const SearchResult *dfs_iter_res,
                           const SearchResult *dfs_rec_res,
                           const SearchResult *dfs_rec_path_res) {
    const char *best_name = NULL;
    int best_steps = INT_MAX;

    print_single_result("bfs", bfs_res);
    printf("---\n");
    print_single_result("dfs_iter", dfs_iter_res);
    printf("---\n");
    print_single_result("dfs_rec", dfs_rec_res);
    printf("---\n");
    print_single_result("dfs_rec_path", dfs_rec_path_res);
    printf("===\n");

    if (bfs_res->status == SEARCH_FOUND && bfs_res->steps < best_steps) {
        best_steps = bfs_res->steps;
        best_name = "bfs";
    }
    if (dfs_iter_res->status == SEARCH_FOUND && dfs_iter_res->steps < best_steps) {
        best_steps = dfs_iter_res->steps;
        best_name = "dfs_iter";
    }
    if (dfs_rec_res->status == SEARCH_FOUND && dfs_rec_res->steps < best_steps) {
        best_steps = dfs_rec_res->steps;
        best_name = "dfs_rec";
    }
    if (dfs_rec_path_res->status == SEARCH_FOUND && dfs_rec_path_res->steps < best_steps) {
        best_steps = dfs_rec_path_res->steps;
        best_name = "dfs_rec_path";
    }

    if (best_name != NULL) {
        printf("BEST_BY_STEPS: %s\n", best_name);
        printf("BEST_STEPS: %d\n", best_steps);
    } else {
        printf("BEST_BY_STEPS: NONE\n");
        printf("BEST_STEPS: -1\n");
    }
}


//CLI parsing
int parse_int_arg(const char *text, int *value) {
    char *end = NULL;
    long parsed;

    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0') {
        return 0;
    }
    if (parsed < INT_MIN || parsed > INT_MAX) {
        return 0;
    }

    *value = (int)parsed;
    return 1;
}

void print_usage(const char *prog) {
    fprintf(stderr, "Usage: %s <graph_file> <start> <goal> <algorithm>\n", prog);
    fprintf(stderr, "Algorithms:\n");
    fprintf(stderr, "  bfs\n");
    fprintf(stderr, "  dfs_iter\n");
    fprintf(stderr, "  dfs_rec\n");
    fprintf(stderr, "  dfs_rec_path\n");
    fprintf(stderr, "  compare\n");
}

int main(int argc, char *argv[]) {
    const char *filename;
    const char *algorithm;
    int start;
    int goal;
    Graph g;

    graph_init_empty(&g);

    if (argc != 5) {
        print_usage(argv[0]);
        return 1;
    }

    filename = argv[1];
    if (!parse_int_arg(argv[2], &start)) {
        fprintf(stderr, "Некорректная начальная вершина: %s\n", argv[2]);
        return 1;
    }
    if (!parse_int_arg(argv[3], &goal)) {
        fprintf(stderr, "Некорректная целевая вершина: %s\n", argv[3]);
        return 1;
    }
    algorithm = argv[4];

    if (!read_graph_from_file(filename, &g)) {
        return 1;
    }

    if (start < 1 || start > g.size || goal < 1 || goal > g.size) {
        fprintf(stderr, "Некорректные вершины. Допустимый диапазон: 1..%d\n", g.size);
        free_graph(&g);
        return 1;
    }

    if (strcmp(algorithm, "compare") == 0) {
        SearchResult bfs_res = bfs(&g, start, goal);
        SearchResult dfs_iter_res = dfs_iterative(&g, start, goal);
        SearchResult dfs_rec_res = dfs_recursive(&g, start, goal);
        SearchResult dfs_rec_path_res = dfs_recursive_with_path(&g, start, goal);
        int exit_code = 0;

        print_compare_summary(&bfs_res, &dfs_iter_res, &dfs_rec_res, &dfs_rec_path_res);

        if (bfs_res.status == SEARCH_ERROR ||
            dfs_iter_res.status == SEARCH_ERROR ||
            dfs_rec_res.status == SEARCH_ERROR ||
            dfs_rec_path_res.status == SEARCH_ERROR) {
            exit_code = 1;
        }

        result_free(&bfs_res);
        result_free(&dfs_iter_res);
        result_free(&dfs_rec_res);
        result_free(&dfs_rec_path_res);
        free_graph(&g);
        return exit_code;
    } else {
        SearchResult res;
        int exit_code = 0;

        result_init(&res);

        if (strcmp(algorithm, "bfs") == 0) {
            res = bfs(&g, start, goal);
        } else if (strcmp(algorithm, "dfs_iter") == 0) {
            res = dfs_iterative(&g, start, goal);
        } else if (strcmp(algorithm, "dfs_rec") == 0) {
            res = dfs_recursive(&g, start, goal);
        } else if (strcmp(algorithm, "dfs_rec_path") == 0) {
            res = dfs_recursive_with_path(&g, start, goal);
        } else {
            fprintf(stderr, "Неизвестный алгоритм: %s\n", algorithm);
            print_usage(argv[0]);
            free_graph(&g);
            return 1;
        }

        print_legacy_result(&res);
        if (res.status == SEARCH_ERROR) {
            exit_code = 1;
        }

        result_free(&res);
        free_graph(&g);
        return exit_code;
    }
}