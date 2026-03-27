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
    int found;
    int steps;
    IntList path;
} SearchResult;


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
    int *tmp = (int *)realloc(list->data, new_capacity * sizeof(int));
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

int list_push_front(IntList *list, int value) {
    if (list->size == list->capacity) {
        int new_capacity = (list->capacity == 0) ? 4 : list->capacity * 2;
        if (!list_resize(list, new_capacity)) {
            return 0;
        }
    }

    for (int i = list->size; i > 0; i--) {
        list->data[i] = list->data[i - 1];
    }

    list->data[0] = value;
    list->size++;
    return 1;
}

int list_pop_front(IntList *list, int *value) {
    if (list->size == 0) {
        return 0;
    }

    *value = list->data[0];

    for (int i = 1; i < list->size; i++) {
        list->data[i - 1] = list->data[i];
    }

    list->size--;
    return 1;
}

int list_pop_back(IntList *list) {
    if (list->size == 0) {
        return 0;
    }

    list->size--;
    return 1;
}

int list_contains(const IntList *list, int value) {
    for (int i = 0; i < list->size; i++) {
        if (list->data[i] == value) {
            return 1;
        }
    }
    return 0;
}


Graph init_graph(int n) {
    Graph g;
    g.size = n;
    g.adj = NULL;

    g.adj = (int **)malloc((n + 1) * sizeof(int *));
    if (g.adj == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для строк матрицы\n");
        exit(1);
    }

    for (int i = 0; i <= n; i++) {
        g.adj[i] = (int *)calloc(n + 1, sizeof(int));
        if (g.adj[i] == NULL) {
            fprintf(stderr, "Ошибка выделения памяти для строки %d\n", i);
            for (int j = 0; j < i; j++) {
                free(g.adj[j]);
            }
            free(g.adj);
            exit(1);
        }
    }

    return g;
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

Graph read_graph_from_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        fprintf(stderr, "Не удалось открыть файл %s\n", filename);
        exit(1);
    }

    int n;
    if (fscanf(f, "%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "Ошибка чтения числа вершин\n");
        fclose(f);
        exit(1);
    }

    Graph g = init_graph(n);

    for (int i = 0; i < n; i++) {
        int v;
        if (fscanf(f, "%d", &v) != 1) {
            fprintf(stderr, "Ошибка чтения номера вершины\n");
            fclose(f);
            free_graph(&g);
            exit(1);
        }

        if (v < 1 || v > n) {
            fprintf(stderr, "Некорректный номер вершины: %d\n", v);
            fclose(f);
            free_graph(&g);
            exit(1);
        }

        while (1) {
            int to;
            if (fscanf(f, "%d", &to) != 1) {
                fprintf(stderr, "Ошибка чтения смежной вершины\n");
                fclose(f);
                free_graph(&g);
                exit(1);
            }

            if (to == 0) {
                break;
            }

            if (to < 1 || to > n) {
                fprintf(stderr, "Некорректная смежная вершина: %d\n", to);
                fclose(f);
                free_graph(&g);
                exit(1);
            }

            add_edge(&g, v, to);
        }
    }

    fclose(f);
    return g;
}


void result_init(SearchResult *res) {
    res->found = 0;
    res->steps = 0;
    list_init(&res->path);
}

void build_path(int start, int goal, const int *parent, IntList *path) {
    int current = goal;

    if (start == goal) {
        list_push_back(path, start);
        return;
    }

    if (parent[goal] == -1) {
        return;
    }

    while (current != -1) {
        list_push_front(path, current);

        if (current == start) {
            break;
        }

        current = parent[current];
    }
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
    IntList open;
    IntList closed;
    int *parent = NULL;

    result_init(&res);
    list_init(&open);
    list_init(&closed);

    parent = (int *)malloc((g->size + 1) * sizeof(int));
    if (parent == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для parent\n");
        return res;
    }

    for (int i = 0; i <= g->size; i++) {
        parent[i] = -1;
    }

    if (!list_push_back(&open, start)) {
        fprintf(stderr, "Ошибка выделения памяти для OPEN\n");
        free(parent);
        list_free(&open);
        list_free(&closed);
        return res;
    }

    while (open.size > 0) {
        int x;

        if (!list_pop_front(&open, &x)) {
            break;
        }

        if (list_contains(&closed, x)) {
            continue;
        }

        if (!list_push_back(&closed, x)) {
            fprintf(stderr, "Ошибка выделения памяти для CLOSED\n");
            free(parent);
            list_free(&open);
            list_free(&closed);
            return res;
        }

        res.steps++;

        if (x == goal) {
            res.found = 1;
            build_path(start, goal, parent, &res.path);
            free(parent);
            list_free(&open);
            list_free(&closed);
            return res;
        }

        for (int child = 1; child <= g->size; child++) {
            if (g->adj[x][child] == 1) {
                if (!list_contains(&open, child) && !list_contains(&closed, child)) {
                    if (!list_push_back(&open, child)) {
                        fprintf(stderr, "Ошибка выделения памяти для OPEN\n");
                        free(parent);
                        list_free(&open);
                        list_free(&closed);
                        return res;
                    }

                    if (parent[child] == -1) {
                        parent[child] = x;
                    }
                }
            }
        }
    }

    free(parent);
    list_free(&open);
    list_free(&closed);
    return res;
}


SearchResult dfs_iterative(const Graph *g, int start, int goal) {
    SearchResult res;
    IntList open;
    IntList closed;
    int *parent = NULL;

    result_init(&res);
    list_init(&open);
    list_init(&closed);

    parent = (int *)malloc((g->size + 1) * sizeof(int));
    if (parent == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для parent\n");
        return res;
    }

    for (int i = 0; i <= g->size; i++) {
        parent[i] = -1;
    }

    if (!list_push_back(&open, start)) {
        fprintf(stderr, "Ошибка выделения памяти для OPEN\n");
        free(parent);
        list_free(&open);
        list_free(&closed);
        return res;
    }

    while (open.size > 0) {
        int x;

        if (!list_pop_front(&open, &x)) {
            break;
        }

        if (list_contains(&closed, x)) {
            continue;
        }

        if (!list_push_back(&closed, x)) {
            fprintf(stderr, "Ошибка выделения памяти для CLOSED\n");
            free(parent);
            list_free(&open);
            list_free(&closed);
            return res;
        }

        res.steps++;

        if (x == goal) {
            res.found = 1;
            build_path(start, goal, parent, &res.path);
            free(parent);
            list_free(&open);
            list_free(&closed);
            return res;
        }

        for (int child = g->size; child >= 1; child--) {
            if (g->adj[x][child] == 1) {
                if (!list_contains(&open, child) && !list_contains(&closed, child)) {
                    if (!list_push_front(&open, child)) {
                        fprintf(stderr, "Ошибка выделения памяти для OPEN\n");
                        free(parent);
                        list_free(&open);
                        list_free(&closed);
                        return res;
                    }

                    if (parent[child] == -1) {
                        parent[child] = x;
                    }
                }
            }
        }
    }

    free(parent);
    list_free(&open);
    list_free(&closed);
    return res;
}


int dfs_recursive_impl(const Graph *g, int x, int goal,
                       int *visited, int *parent, int *steps) {
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
    int *visited = NULL;
    int *parent = NULL;

    result_init(&res);

    visited = (int *)calloc(g->size + 1, sizeof(int));
    if (visited == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для visited\n");
        return res;
    }

    parent = (int *)malloc((g->size + 1) * sizeof(int));
    if (parent == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для parent\n");
        free(visited);
        return res;
    }

    for (int i = 0; i <= g->size; i++) {
        parent[i] = -1;
    }

    res.found = dfs_recursive_impl(g, start, goal, visited, parent, &res.steps);

    if (res.found) {
        build_path(start, goal, parent, &res.path);
    }

    free(visited);
    free(parent);
    return res;
}


int dfs_recursive_path_impl(const Graph *g, int x, int goal,
                            int *visited, IntList *path, int *steps) {
    visited[x] = 1;
    (*steps)++;

    if (!list_push_back(path, x)) {
        return 0;
    }

    if (x == goal) {
        return 1;
    }

    for (int child = 1; child <= g->size; child++) {
        if (g->adj[x][child] == 1 && !visited[child]) {
            if (dfs_recursive_path_impl(g, child, goal, visited, path, steps)) {
                return 1;
            }
        }
    }

    list_pop_back(path);
    return 0;
}

SearchResult dfs_recursive_with_path(const Graph *g, int start, int goal) {
    SearchResult res;
    int *visited = NULL;

    result_init(&res);

    visited = (int *)calloc(g->size + 1, sizeof(int));
    if (visited == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для visited\n");
        return res;
    }

    res.found = dfs_recursive_path_impl(g, start, goal, visited, &res.path, &res.steps);

    free(visited);
    return res;
}


void print_usage(const char *prog) {
    fprintf(stderr, "Usage: %s <graph_file> <start> <goal> <algorithm>\n", prog);
    fprintf(stderr, "Algorithms:\n");
    fprintf(stderr, "  bfs\n");
    fprintf(stderr, "  dfs_iter\n");
    fprintf(stderr, "  dfs_rec\n");
    fprintf(stderr, "  dfs_rec_path\n");
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        print_usage(argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int start = atoi(argv[2]);
    int goal = atoi(argv[3]);
    const char *algorithm = argv[4];

    Graph g = read_graph_from_file(filename);

    if (start < 1 || start > g.size || goal < 1 || goal > g.size) {
        fprintf(stderr, "Некорректные вершины. Допустимый диапазон: 1..%d\n", g.size);
        free_graph(&g);
        return 1;
    }

    SearchResult res;
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

    if (res.found) {
        printf("FOUND\n");
        printf("STEPS: %d\n", res.steps);
        printf("PATH: ");
        print_path_machine(&res.path);
    } else {
        printf("NOT_FOUND\n");
        printf("STEPS: %d\n", res.steps);
        printf("PATH:\n");
    }

    list_free(&res.path);
    free_graph(&g);
    return 0;
}