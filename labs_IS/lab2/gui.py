import tkinter as tk
from tkinter import ttk, messagebox
from dataclasses import dataclass
from collections import deque
from typing import List, Optional


# =========================
# Алгоритм поиска
# =========================

@dataclass
class State:
    board: List[int]          # board[row] = col
    row: int                  # сколько строк уже заполнено
    parent: Optional["State"] # родитель для восстановления пути


@dataclass
class SearchResult:
    boards: List[List[int]]
    n: int
    q: int
    expanded_states: int
    steps_to_first_solution: int


def create_state(n: int, row: int) -> State:
    return State(board=[-1] * n, row=row, parent=None)


def clone_state(src: State) -> State:
    return State(board=src.board[:], row=src.row, parent=src.parent)


def is_safe(board: List[int], row: int, col: int) -> bool:
    for i in range(row):
        if board[i] == col:
            return False
        if abs(board[i] - col) == abs(i - row):
            return False
    return True


def is_goal(state: State, q: int) -> bool:
    return state.row == q


def state_key(state: State) -> tuple:
    return (state.row, tuple(state.board[:state.row]))


def create_children(parent: State, n: int, reverse_order: bool = False) -> List[State]:
    children = []
    columns = range(n - 1, -1, -1) if reverse_order else range(n)

    for col in columns:
        if not is_safe(parent.board, parent.row, col):
            continue

        child = clone_state(parent)
        child.board[parent.row] = col
        child.row = parent.row + 1
        child.parent = parent
        children.append(child)

    return children


def solve_bfs(n: int, q: int) -> SearchResult:
    open_list = deque()
    closed_set = set()
    open_set = set()

    initial = create_state(n, 0)
    open_list.append(initial)
    open_set.add(state_key(initial))

    solutions: List[List[int]] = []
    expanded_states = 0
    steps_to_first_solution = -1

    while open_list:
        x = open_list.popleft()
        x_key = state_key(x)
        open_set.discard(x_key)

        expanded_states += 1
        closed_set.add(x_key)

        children = create_children(x, n, reverse_order=False)

        for child in children:
            if is_goal(child, q):
                if steps_to_first_solution == -1:
                    steps_to_first_solution = expanded_states
                solutions.append(child.board[:])
                continue

            child_key = state_key(child)
            if child_key not in open_set and child_key not in closed_set:
                open_list.append(child)
                open_set.add(child_key)

    return SearchResult(
        boards=solutions,
        n=n,
        q=q,
        expanded_states=expanded_states,
        steps_to_first_solution=steps_to_first_solution
    )


def solve_dfs_limited(n: int, q: int, max_depth: int) -> SearchResult:
    open_list: List[State] = []
    closed_set = set()
    open_set = set()

    initial = create_state(n, 0)
    open_list.insert(0, initial)
    open_set.add(state_key(initial))

    solutions: List[List[int]] = []
    expanded_states = 0
    steps_to_first_solution = -1

    while open_list:
        x = open_list.pop(0)
        x_key = state_key(x)
        open_set.discard(x_key)

        expanded_states += 1
        closed_set.add(x_key)

        if x.row >= max_depth:
            continue

        children = create_children(x, n, reverse_order=True)

        for child in children:
            if child.row > max_depth:
                continue

            if is_goal(child, q):
                if steps_to_first_solution == -1:
                    steps_to_first_solution = expanded_states
                solutions.append(child.board[:])
                continue

            child_key = state_key(child)
            if child_key not in open_set and child_key not in closed_set:
                open_list.insert(0, child)
                open_set.add(child_key)

    return SearchResult(
        boards=solutions,
        n=n,
        q=q,
        expanded_states=expanded_states,
        steps_to_first_solution=steps_to_first_solution
    )


def solve_queens(n: int, q: int, method: str, max_depth: int) -> SearchResult:
    if n < 1 or n > 12:
        raise ValueError("Размерность доски должна быть в диапазоне 1..12.")
    if q < 1 or q > n:
        raise ValueError("Количество ферзей должно быть в диапазоне 1..N.")

    if method == "BFS":
        return solve_bfs(n, q)
    elif method == "DFS":
        if max_depth < 1 or max_depth > q:
            raise ValueError("Для DFS maxDepth должен быть в диапазоне 1..Q.")
        return solve_dfs_limited(n, q, max_depth)
    else:
        raise ValueError("Неизвестный метод поиска.")


# =========================
# GUI
# =========================

class QueensApp:
    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title("Задача о N ферзях")
        self.root.geometry("900x760")
        self.root.minsize(820, 700)

        self.result: Optional[SearchResult] = None
        self.current_index = 0

        self.light_color = "#efc9a0"
        self.dark_color = "#cd8b4a"

        self._build_ui()
        self._draw_board_empty()

    def _build_ui(self):
        top = tk.Frame(self.root, padx=12, pady=10)
        top.pack(side=tk.TOP, fill=tk.X)

        tk.Label(top, text="Размерность доски").grid(row=0, column=0, padx=(0, 8), sticky="w")
        tk.Label(top, text="X").grid(row=0, column=1, sticky="e")

        self.var_x = tk.StringVar(value="8")
        self.entry_x = tk.Entry(top, width=5, textvariable=self.var_x, justify="center")
        self.entry_x.grid(row=0, column=2, padx=(4, 10))

        tk.Label(top, text="Y").grid(row=0, column=3, sticky="e")

        self.var_y = tk.StringVar(value="8")
        self.entry_y = tk.Entry(top, width=5, textvariable=self.var_y, justify="center")
        self.entry_y.grid(row=0, column=4, padx=(4, 18))

        tk.Label(top, text="Количество ферзей").grid(row=0, column=5, padx=(0, 8), sticky="w")

        self.var_q = tk.StringVar(value="8")
        self.entry_q = tk.Entry(top, width=6, textvariable=self.var_q, justify="center")
        self.entry_q.grid(row=0, column=6, padx=(4, 18))

        tk.Label(top, text="Метод").grid(row=1, column=0, pady=(12, 0), sticky="w")

        self.method_box = ttk.Combobox(top, state="readonly", width=18, values=["BFS", "DFS"])
        self.method_box.grid(row=1, column=1, columnspan=3, pady=(12, 0), sticky="w")
        self.method_box.set("BFS")
        self.method_box.bind("<<ComboboxSelected>>", lambda e: self._on_method_change())

        tk.Label(top, text="MaxDepth").grid(row=1, column=4, pady=(12, 0), sticky="e")

        self.var_depth = tk.StringVar(value="8")
        self.entry_depth = tk.Entry(top, width=6, textvariable=self.var_depth, justify="center")
        self.entry_depth.grid(row=1, column=5, pady=(12, 0), sticky="w")

        self.btn_find = tk.Button(top, text="Найти решения", width=16, command=self.find_solutions)
        self.btn_find.grid(row=2, column=0, columnspan=2, pady=(16, 0), sticky="w")

        self.btn_prev = tk.Button(top, text="Предыдущее", width=14, command=self.show_prev, state=tk.DISABLED)
        self.btn_prev.grid(row=2, column=4, pady=(16, 0), sticky="e")

        self.btn_next = tk.Button(top, text="Следующее", width=14, command=self.show_next, state=tk.DISABLED)
        self.btn_next.grid(row=2, column=5, pady=(16, 0), sticky="w")

        self.canvas = tk.Canvas(self.root, bg="#f0f0f0", highlightthickness=0)
        self.canvas.pack(fill=tk.BOTH, expand=True, padx=12, pady=(0, 8))
        self.canvas.bind("<Configure>", lambda e: self.redraw())

        bottom = tk.Frame(self.root, padx=12, pady=8)
        bottom.pack(side=tk.BOTTOM, fill=tk.X)

        self.lbl_solution = tk.Label(bottom, text="Решение 0 из 0", font=("Arial", 11))
        self.lbl_solution.pack(side=tk.LEFT)

        self.lbl_stats = tk.Label(bottom, text="", font=("Arial", 11))
        self.lbl_stats.pack(side=tk.RIGHT)

        self.var_x.trace_add("write", self._sync_y_from_x)
        self._on_method_change()

    def _sync_y_from_x(self, *args):
        x = self.var_x.get().strip()
        self.var_y.set(x)

    def _on_method_change(self):
        method = self.method_box.get()
        if method == "DFS":
            self.entry_depth.config(state=tk.NORMAL)
        else:
            self.entry_depth.config(state=tk.DISABLED)

    def _read_int(self, value: str, name: str) -> int:
        value = value.strip()
        if not value:
            raise ValueError(f"Поле '{name}' не заполнено.")
        try:
            return int(value)
        except ValueError:
            raise ValueError(f"Поле '{name}' должно содержать целое число.")

    def find_solutions(self):
        try:
            x = self._read_int(self.var_x.get(), "X")
            y = self._read_int(self.var_y.get(), "Y")
            q = self._read_int(self.var_q.get(), "Количество ферзей")
            method = self.method_box.get()

            if x != y:
                raise ValueError("Для задачи о ферзях доска должна быть квадратной: X = Y.")

            max_depth = q
            if method == "DFS":
                max_depth = self._read_int(self.var_depth.get(), "MaxDepth")

            self.result = solve_queens(x, q, method, max_depth)
            self.current_index = 0

            self._update_controls()
            self.redraw()

        except Exception as e:
            messagebox.showerror("Ошибка", str(e))

    def _update_controls(self):
        if self.result and self.result.boards:
            total = len(self.result.boards)
            self.lbl_solution.config(text=f"Решение {self.current_index + 1} из {total}")

            first_text = (
                str(self.result.steps_to_first_solution)
                if self.result.steps_to_first_solution != -1 else "не найдено"
            )
            self.lbl_stats.config(
                text=f"Состояний: {self.result.expanded_states}    До первого решения: {first_text}"
            )

            btn_state = tk.NORMAL if total > 1 else tk.DISABLED
            self.btn_prev.config(state=btn_state)
            self.btn_next.config(state=btn_state)
        else:
            self.lbl_solution.config(text="Решение 0 из 0")
            self.lbl_stats.config(text="Решения не найдены")
            self.btn_prev.config(state=tk.DISABLED)
            self.btn_next.config(state=tk.DISABLED)

    def show_prev(self):
        if not self.result or not self.result.boards:
            return
        self.current_index = (self.current_index - 1) % len(self.result.boards)
        self._update_controls()
        self.redraw()

    def show_next(self):
        if not self.result or not self.result.boards:
            return
        self.current_index = (self.current_index + 1) % len(self.result.boards)
        self._update_controls()
        self.redraw()

    def _draw_board_empty(self):
        try:
            n = int(self.var_x.get())
            if n < 1:
                n = 8
        except Exception:
            n = 8
        self._draw_board(n, None)

    def redraw(self):
        if self.result and self.result.boards:
            self._draw_board(self.result.n, self.result.boards[self.current_index])
        else:
            self._draw_board_empty()

    def _draw_board(self, n: int, board: Optional[List[int]]):
        self.canvas.delete("all")

        width = self.canvas.winfo_width()
        height = self.canvas.winfo_height()

        if width < 50 or height < 50:
            return

        left_margin = 70
        right_margin = 70
        top_margin = 50
        bottom_margin = 60

        available_w = max(100, width - left_margin - right_margin)
        available_h = max(100, height - top_margin - bottom_margin)

        cell = min(available_w // n, available_h // n)
        board_w = cell * n
        board_h = cell * n

        x0 = (width - board_w) // 2
        y0 = (height - board_h) // 2
        x1 = x0 + board_w
        y1 = y0 + board_h

        for r in range(n):
            for c in range(n):
                color = self.light_color if (r + c) % 2 == 0 else self.dark_color
                self.canvas.create_rectangle(
                    x0 + c * cell,
                    y0 + r * cell,
                    x0 + (c + 1) * cell,
                    y0 + (r + 1) * cell,
                    fill=color,
                    outline=color
                )

        self.canvas.create_rectangle(x0, y0, x1, y1, outline="black", width=1)

        font_size = max(10, min(16, cell // 3))
        queen_size = max(18, min(36, int(cell * 0.55)))

        for c in range(n):
            letter = chr(ord('a') + c)
            cx = x0 + c * cell + cell / 2
            self.canvas.create_text(cx, y0 - 18, text=letter, font=("Arial", font_size))
            self.canvas.create_text(cx, y1 + 18, text=letter, font=("Arial", font_size))

        for r in range(n):
            num = str(n - r)
            cy = y0 + r * cell + cell / 2
            self.canvas.create_text(x0 - 18, cy, text=num, font=("Arial", font_size))
            self.canvas.create_text(x1 + 18, cy, text=num, font=("Arial", font_size))

        if board:
            for r, c in enumerate(board[:self.result.q if self.result else n]):
                if c < 0:
                    continue
                cx = x0 + c * cell + cell / 2
                cy = y0 + r * cell + cell / 2
                self.canvas.create_text(
                    cx, cy,
                    text="♛",
                    font=("DejaVu Sans", queen_size),
                    fill="black"
                )


if __name__ == "__main__":
    root = tk.Tk()
    app = QueensApp(root)
    root.mainloop()