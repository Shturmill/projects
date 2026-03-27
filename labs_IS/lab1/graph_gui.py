import math
import os
import subprocess
import tkinter as tk
from collections import deque
from tkinter import ttk, filedialog, messagebox


# -------------------------
# Работа с файлом графа
# -------------------------

def read_graph_from_file(filename: str):
    with open(filename, 'r', encoding='utf-8') as f:
        tokens = f.read().split()

    if not tokens:
        raise ValueError('Файл пустой')

    pos = 0
    n = int(tokens[pos])
    pos += 1

    if n <= 0:
        raise ValueError('Число вершин должно быть положительным')

    adj = [[0] * (n + 1) for _ in range(n + 1)]

    for _ in range(n):
        if pos >= len(tokens):
            raise ValueError('Неожиданный конец файла при чтении вершины')

        v = int(tokens[pos])
        pos += 1

        if not (1 <= v <= n):
            raise ValueError(f'Некорректный номер вершины: {v}')

        while True:
            if pos >= len(tokens):
                raise ValueError(f'Не найден 0 в строке вершины {v}')

            to = int(tokens[pos])
            pos += 1

            if to == 0:
                break

            if not (1 <= to <= n):
                raise ValueError(f'Некорректная смежная вершина: {to}')

            adj[v][to] = 1

    return n, adj


def format_matrix(n: int, adj) -> str:
    header = '    ' + ' '.join(f'{j:>3}' for j in range(1, n + 1))
    lines = [header]
    for i in range(1, n + 1):
        row = ' '.join(f'{adj[i][j]:>3}' for j in range(1, n + 1))
        lines.append(f'{i:>3} {row}')
    return '\n'.join(lines)


# -------------------------
# Работа с C-программой
# -------------------------

def run_c_search(program_path, graph_file, start, goal, algorithm):
    result = subprocess.run(
        [program_path, graph_file, str(start), str(goal), algorithm],
        capture_output=True,
        text=True
    )

    if result.returncode != 0:
        error_text = result.stderr.strip() or 'Ошибка запуска C-программы'
        raise RuntimeError(error_text)

    return result.stdout


def parse_c_output(output: str):
    lines = [line.strip() for line in output.splitlines()]

    found = False
    steps = 0
    path = []

    for line in lines:
        if line == 'FOUND':
            found = True
        elif line == 'NOT_FOUND':
            found = False
        elif line.startswith('STEPS:'):
            steps = int(line.split(':', 1)[1].strip())
        elif line.startswith('PATH:'):
            rest = line.split(':', 1)[1].strip()
            if rest:
                path = list(map(int, rest.split()))

    return found, steps, path


# -------------------------
# GUI
# -------------------------

class GraphApp:
    def __init__(self, root):
        self.root = root
        self.root.title('Графы: Python GUI + C search')
        self.root.geometry('1450x820')
        self.root.minsize(1200, 700)

        self.program_var = tk.StringVar(value='./build/graph_search')
        self.file_var = tk.StringVar(value='./data/graph.txt')
        self.start_var = tk.StringVar(value='1')
        self.goal_var = tk.StringVar(value='1')

        self.n = 0
        self.adj = None
        self.node_positions = {}

        self.build_ui()

    def build_ui(self):
        main = ttk.Frame(self.root, padding=10)
        main.pack(fill='both', expand=True)

        top = ttk.LabelFrame(main, text='Настройки', padding=10)
        top.pack(fill='x')

        ttk.Label(top, text='Путь к C-программе:').grid(row=0, column=0, sticky='w')
        ttk.Entry(top, textvariable=self.program_var, width=55).grid(row=0, column=1, padx=5, sticky='ew')

        ttk.Label(top, text='Файл графа:').grid(row=1, column=0, sticky='w')
        ttk.Entry(top, textvariable=self.file_var, width=55).grid(row=1, column=1, padx=5, sticky='ew')
        ttk.Button(top, text='Обзор...', command=self.choose_file).grid(row=1, column=2, padx=5)
        ttk.Button(top, text='Загрузить граф', command=self.load_graph).grid(row=1, column=3, padx=5)

        ttk.Label(top, text='Начальная вершина:').grid(row=2, column=0, sticky='w')
        ttk.Entry(top, textvariable=self.start_var, width=10).grid(row=2, column=1, sticky='w', padx=5)

        ttk.Label(top, text='Целевая вершина:').grid(row=2, column=2, sticky='w')
        ttk.Entry(top, textvariable=self.goal_var, width=10).grid(row=2, column=3, sticky='w', padx=5)

        ttk.Button(top, text='Запустить BFS и DFS', command=self.run_searches).grid(row=2, column=4, padx=10)
        ttk.Button(top, text='Очистить результат', command=self.clear_result).grid(row=2, column=5, padx=5)

        top.columnconfigure(1, weight=1)

        body = ttk.Frame(main)
        body.pack(fill='both', expand=True, pady=(10, 0))

        left = ttk.Frame(body)
        left.pack(side='left', fill='both', expand=False)

        right = ttk.Frame(body)
        right.pack(side='left', fill='both', expand=True, padx=(10, 0))

        matrix_frame = ttk.LabelFrame(left, text='Матрица смежности', padding=10)
        matrix_frame.pack(fill='both', expand=True)

        self.matrix_text = tk.Text(matrix_frame, width=45, height=18, wrap='none')
        self.matrix_text.pack(side='left', fill='both', expand=True)

        matrix_scroll_y = ttk.Scrollbar(matrix_frame, orient='vertical', command=self.matrix_text.yview)
        matrix_scroll_y.pack(side='right', fill='y')
        self.matrix_text.configure(yscrollcommand=matrix_scroll_y.set)

        result_frame = ttk.LabelFrame(left, text='Результаты поиска', padding=10)
        result_frame.pack(fill='both', expand=True, pady=(10, 0))

        self.result_text = tk.Text(result_frame, width=45, height=16, wrap='word')
        self.result_text.pack(fill='both', expand=True)

        bfs_frame = ttk.LabelFrame(right, text='Визуализация BFS', padding=10)
        bfs_frame.pack(fill='both', expand=True)

        self.bfs_canvas = tk.Canvas(
            bfs_frame,
            bg='white',
            highlightthickness=1,
            highlightbackground='#bbbbbb'
        )
        self.bfs_canvas.pack(fill='both', expand=True)
        self.bfs_canvas.bind('<Configure>', self.on_canvas_resize)

        dfs_frame = ttk.LabelFrame(right, text='Визуализация DFS', padding=10)
        dfs_frame.pack(fill='both', expand=True, pady=(10, 0))

        self.dfs_canvas = tk.Canvas(
            dfs_frame,
            bg='white',
            highlightthickness=1,
            highlightbackground='#bbbbbb'
        )
        self.dfs_canvas.pack(fill='both', expand=True)
        self.dfs_canvas.bind('<Configure>', self.on_canvas_resize)

        legend = ttk.Label(
            right,
            text='Тёмные рёбра — дерево обхода. Серые пунктирные — дополнительные связи. '
                 'Красные рёбра и жёлтые вершины — найденный путь.'
        )
        legend.pack(anchor='w', pady=(8, 0))

    def choose_file(self):
        filename = filedialog.askopenfilename(
            title='Выбери файл графа',
            filetypes=[('Text files', '*.txt'), ('All files', '*.*')],
        )
        if filename:
            self.file_var.set(filename)

    def load_graph(self):
        filename = self.file_var.get().strip()
        if not filename:
            messagebox.showwarning('Нет файла', 'Сначала выбери файл графа.')
            return

        try:
            self.n, self.adj = read_graph_from_file(filename)
        except Exception as e:
            messagebox.showerror('Ошибка', str(e))
            return

        self.matrix_text.delete('1.0', tk.END)
        self.matrix_text.insert(tk.END, format_matrix(self.n, self.adj))

        self.result_text.delete('1.0', tk.END)
        self.result_text.insert(tk.END, f'Граф загружен: {os.path.basename(filename)}\n')
        self.result_text.insert(tk.END, f'Число вершин: {self.n}\n')
        self.result_text.insert(tk.END, 'Теперь можно запускать BFS и DFS.\n')

        self.draw_graph(self.bfs_canvas, path=None)
        self.draw_graph(self.dfs_canvas, path=None)

    def clear_result(self):
        self.result_text.delete('1.0', tk.END)
        if self.adj is not None:
            self.draw_graph(self.bfs_canvas, path=None)
            self.draw_graph(self.dfs_canvas, path=None)

    def on_canvas_resize(self, event):
        if self.adj is not None:
            self.draw_graph(self.bfs_canvas, path=None)
            self.draw_graph(self.dfs_canvas, path=None)

    def choose_root(self):
        try:
            start = int(self.start_var.get())
            if 1 <= start <= self.n:
                return start
        except ValueError:
            pass

        indeg = [0] * (self.n + 1)
        for i in range(1, self.n + 1):
            for j in range(1, self.n + 1):
                if self.adj[i][j] == 1:
                    indeg[j] += 1

        for v in range(1, self.n + 1):
            if indeg[v] == 0:
                return v

        return 1

    def compute_tree_positions(self, canvas):
        self.node_positions = {}
        if not self.n:
            return {}, set()

        canvas.update_idletasks()
        width = max(canvas.winfo_width(), 500)
        height = max(canvas.winfo_height(), 300)

        root = self.choose_root()

        visited = [False] * (self.n + 1)
        level = {}
        parent = {}
        tree_edges = set()

        q = deque([root])
        visited[root] = True
        level[root] = 0
        parent[root] = None

        while q:
            v = q.popleft()
            for child in range(1, self.n + 1):
                if self.adj[v][child] == 1 and not visited[child]:
                    visited[child] = True
                    level[child] = level[v] + 1
                    parent[child] = v
                    tree_edges.add((v, child))
                    q.append(child)

        current_max_level = max(level.values()) if level else 0

        for start in range(1, self.n + 1):
            if not visited[start]:
                current_max_level += 1
                q = deque([start])
                visited[start] = True
                level[start] = current_max_level
                parent[start] = None

                while q:
                    v = q.popleft()
                    for child in range(1, self.n + 1):
                        if self.adj[v][child] == 1 and not visited[child]:
                            visited[child] = True
                            level[child] = level[v] + 1
                            parent[child] = v
                            tree_edges.add((v, child))
                            q.append(child)

                current_max_level = max(level.values())

        levels = {}
        for v, lv in level.items():
            levels.setdefault(lv, []).append(v)

        total_levels = len(levels)
        if total_levels == 0:
            return parent, tree_edges

        top_margin = 45
        bottom_margin = 45
        left_margin = 60
        right_margin = 60

        usable_h = max(height - top_margin - bottom_margin, 100)
        usable_w = max(width - left_margin - right_margin, 100)

        sorted_levels = sorted(levels.keys())

        for row_index, lv in enumerate(sorted_levels):
            vertices = levels[lv]
            vertices.sort()

            if total_levels == 1:
                y = top_margin + usable_h / 2
            else:
                y = top_margin + row_index * (usable_h / (total_levels - 1))

            count = len(vertices)
            for col_index, v in enumerate(vertices):
                if count == 1:
                    x = left_margin + usable_w / 2
                else:
                    x = left_margin + col_index * (usable_w / (count - 1))
                self.node_positions[v] = (x, y)

        return parent, tree_edges

    def draw_arrow_line(self, canvas, x1, y1, x2, y2, color='black', width=2, dash=None):
        dx = x2 - x1
        dy = y2 - y1
        dist = math.hypot(dx, dy)
        if dist == 0:
            return

        node_r = 22
        start_x = x1 + dx / dist * node_r
        start_y = y1 + dy / dist * node_r
        end_x = x2 - dx / dist * node_r
        end_y = y2 - dy / dist * node_r

        canvas.create_line(
            start_x, start_y, end_x, end_y,
            arrow=tk.LAST,
            width=width,
            fill=color,
            dash=dash,
        )

    def draw_loop(self, canvas, x, y, color='black', width=2):
        r = 24
        canvas.create_arc(
            x - r, y - 2 * r,
            x + r, y,
            start=30,
            extent=300,
            style='arc',
            outline=color,
            width=width,
        )
        canvas.create_line(
            x + r * 0.75, y - r * 0.9,
            x + r * 0.55, y - r * 0.55,
            arrow=tk.LAST,
            fill=color,
            width=width
        )

    def draw_graph(self, canvas, path=None):
        canvas.delete('all')

        if self.adj is None:
            canvas.create_text(
                250, 100,
                text='Сначала загрузи граф из файла',
                font=('Arial', 14),
            )
            return

        _, tree_edges = self.compute_tree_positions(canvas)

        path_edges = set()
        path_nodes = set(path or [])
        if path and len(path) >= 2:
            for i in range(len(path) - 1):
                path_edges.add((path[i], path[i + 1]))

        for i in range(1, self.n + 1):
            for j in range(1, self.n + 1):
                if self.adj[i][j] != 1:
                    continue

                if i not in self.node_positions or j not in self.node_positions:
                    continue

                x1, y1 = self.node_positions[i]
                x2, y2 = self.node_positions[j]

                if (i, j) in path_edges:
                    color = 'red'
                    width = 3
                    dash = None
                elif (i, j) in tree_edges:
                    color = '#444444'
                    width = 2
                    dash = None
                else:
                    color = '#aaaaaa'
                    width = 1
                    dash = (4, 2)

                if i == j:
                    self.draw_loop(canvas, x1, y1, color=color, width=width)
                else:
                    self.draw_arrow_line(canvas, x1, y1, x2, y2, color=color, width=width, dash=dash)

        for v in range(1, self.n + 1):
            if v not in self.node_positions:
                continue

            x, y = self.node_positions[v]
            r = 22

            fill = '#ffe680' if v in path_nodes else '#cfe8ff'
            outline = '#cc0000' if v in path_nodes else '#336699'
            line_w = 3 if v in path_nodes else 2

            canvas.create_oval(
                x - r, y - r, x + r, y + r,
                fill=fill,
                outline=outline,
                width=line_w
            )
            canvas.create_text(x, y, text=str(v), font=('Arial', 12, 'bold'))

    def run_searches(self):
        if self.adj is None:
            messagebox.showwarning('Граф не загружен', 'Сначала загрузи граф из файла.')
            return

        program = self.program_var.get().strip()
        graph_file = self.file_var.get().strip()

        if not program:
            messagebox.showerror('Ошибка', 'Укажи путь к C-программе.')
            return

        if not graph_file:
            messagebox.showerror('Ошибка', 'Укажи путь к файлу графа.')
            return

        try:
            start = int(self.start_var.get())
            goal = int(self.goal_var.get())
        except ValueError:
            messagebox.showerror('Ошибка', 'Начальная и целевая вершины должны быть целыми числами.')
            return

        if not (1 <= start <= self.n) or not (1 <= goal <= self.n):
            messagebox.showerror('Ошибка', f'Вершины должны быть в диапазоне от 1 до {self.n}.')
            return

        try:
            bfs_output = run_c_search(program, graph_file, start, goal, 'bfs')
            bfs_found, bfs_steps, bfs_path = parse_c_output(bfs_output)

            dfs_output = run_c_search(program, graph_file, start, goal, 'dfs_iter')
            dfs_found, dfs_steps, dfs_path = parse_c_output(dfs_output)
        except Exception as e:
            messagebox.showerror('Ошибка', str(e))
            return

        self.result_text.delete('1.0', tk.END)
        self.result_text.insert(tk.END, f'Старт: {start}\n')
        self.result_text.insert(tk.END, f'Цель: {goal}\n\n')

        self.result_text.insert(tk.END, 'BFS\n')
        self.result_text.insert(tk.END, '-' * 30 + '\n')
        if bfs_found:
            self.result_text.insert(tk.END, 'Путь найден\n')
            self.result_text.insert(tk.END, f'Шагов: {bfs_steps}\n')
            self.result_text.insert(tk.END, 'Путь: ' + ' -> '.join(map(str, bfs_path)) + '\n\n')
        else:
            self.result_text.insert(tk.END, 'Путь не найден\n')
            self.result_text.insert(tk.END, f'Шагов: {bfs_steps}\n\n')

        self.result_text.insert(tk.END, 'DFS\n')
        self.result_text.insert(tk.END, '-' * 30 + '\n')
        if dfs_found:
            self.result_text.insert(tk.END, 'Путь найден\n')
            self.result_text.insert(tk.END, f'Шагов: {dfs_steps}\n')
            self.result_text.insert(tk.END, 'Путь: ' + ' -> '.join(map(str, dfs_path)) + '\n')
        else:
            self.result_text.insert(tk.END, 'Путь не найден\n')
            self.result_text.insert(tk.END, f'Шагов: {dfs_steps}\n')

        self.draw_graph(self.bfs_canvas, path=bfs_path if bfs_found else None)
        self.draw_graph(self.dfs_canvas, path=dfs_path if dfs_found else None)


def main():
    root = tk.Tk()
    app = GraphApp(root)
    root.mainloop()


if __name__ == '__main__':
    main()