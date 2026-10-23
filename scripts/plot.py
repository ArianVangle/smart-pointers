import csv
import os
import sys
from collections import defaultdict

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RESULTS = os.path.join(ROOT, "results")
CSV_PATH = os.path.join(RESULTS, "results.csv")

METHODS = ["raw", "UnqPtr", "ShrdPtr", "std_shared", "std_unique"]
LABELS = {
    "raw": "обычный указатель",
    "UnqPtr": "UnqPtr",
    "ShrdPtr": "ShrdPtr",
    "std_shared": "std::shared_ptr",
    "std_unique": "std::unique_ptr",
}
COLORS = {
    "raw": "#2a78d6",
    "UnqPtr": "#eb6834",
    "ShrdPtr": "#1baf7a",
    "std_shared": "#eda100",
    "std_unique": "#e87ba4",
}
MARKERS = {"raw": "o", "UnqPtr": "s", "ShrdPtr": "^", "std_shared": "D", "std_unique": "v"}
LINE_STYLES = {"raw": "-", "UnqPtr": "-", "ShrdPtr": "-", "std_shared": "--", "std_unique": "--"}
PHASES = [("create_ms", "создание"), ("access_ms", "доступ"), ("destroy_ms", "удаление")]
X_LABEL = "число объектов"

TEXT = "#3a3a38"
GRID = "#e4e3df"

plt.rcParams.update({
    "font.family": "DejaVu Sans",
    "axes.edgecolor": GRID,
    "axes.labelcolor": TEXT,
    "axes.titlecolor": TEXT,
    "xtick.color": TEXT,
    "ytick.color": TEXT,
    "legend.frameon": False,
    "font.size": 10,
})


def load():
    data = defaultdict(dict)
    try:
        with open(CSV_PATH, encoding="utf-8") as file:
            for row in csv.DictReader(file):
                values = {key: float(row[key]) for key in row if key not in ("method", "n")}
                data[row["method"]][int(row["n"])] = values
    except FileNotFoundError:
        print("results/results.csv не найден, сначала запустите нагрузочный тест")
        sys.exit(1)
    return data


def style(axis, ylabel, title):
    axis.set_xlabel(X_LABEL)
    axis.set_ylabel(ylabel)
    axis.set_title(title, loc="left")
    axis.grid(True, which="major", color=GRID, linewidth=0.8)
    axis.set_axisbelow(True)
    for side in ("top", "right"):
        axis.spines[side].set_visible(False)


def plot_line(axis, xs, ys, method):
    axis.plot(xs, ys, color=COLORS[method], marker=MARKERS[method],
              linestyle=LINE_STYLES[method], markersize=6, linewidth=2,
              label=LABELS[method])


def save(figure, name):
    figure.tight_layout()
    figure.savefig(os.path.join(RESULTS, name), dpi=120)
    plt.close(figure)
    print("сохранен results/" + name)


def draw_log_chart(data, key, ylabel, title, name):
    figure, axis = plt.subplots(figsize=(8, 5))
    for method in METHODS:
        ns = sorted(data[method])
        plot_line(axis, ns, [data[method][n][key] for n in ns], method)
    axis.set_xscale("log")
    axis.set_yscale("log")
    style(axis, ylabel, title)
    axis.legend()
    save(figure, name)


def draw_overhead(data):
    figure, axis = plt.subplots(figsize=(10, 5))
    for method in METHODS[1:]:
        ns = sorted(data[method])
        ratios = [data[method][n]["time_ms"] / data["raw"][n]["time_ms"] for n in ns]
        plot_line(axis, ns, ratios, method)
    axis.axhline(1.0, color=COLORS["raw"], linewidth=1.5, linestyle=":",
                 label="обычный указатель = 1")
    axis.set_xscale("log")
    axis.set_ylim(bottom=0)
    style(axis, "во сколько раз медленнее", "Время относительно обычных указателей")
    axis.legend()
    save(figure, "overhead.png")


def draw_phases(data):
    n = max(data["raw"])
    figure, axis = plt.subplots(figsize=(10, 5))
    width = 0.16
    middle = (len(METHODS) - 1) / 2
    for index, method in enumerate(METHODS):
        offsets = [p + (index - middle) * width for p in range(len(PHASES))]
        values = [data[method][n][key] for key, _ in PHASES]
        axis.bar(offsets, values, width=width * 0.9, color=COLORS[method],
                 label=LABELS[method])
    axis.set_xticks(range(len(PHASES)))
    axis.set_xticklabels([label for _, label in PHASES])
    count = f"{n:,}".replace(",", " ")
    style(axis, "время, мс", "Время по этапам, " + count + " объектов")
    axis.set_xlabel("этап")
    axis.grid(False, axis="x")
    axis.legend()
    save(figure, "phases.png")


data = load()
draw_log_chart(data, "time_ms", "время, мс", "Общее время: создание + доступ + удаление", "time.png")
draw_log_chart(data, "memory_kb", "память, КБ", "Занятая память в куче", "memory.png")
draw_overhead(data)
draw_phases(data)
