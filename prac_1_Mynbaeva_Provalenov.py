from typing import Tuple, Optional
import numpy as np
from scipy.optimize import linprog
import matplotlib.pyplot as plt

#Вычисление равновесия Нэша 
def nash_equilibrium(payoff_matrix: np.ndarray) -> Optional[Tuple[float, np.ndarray, np.ndarray]]:
    matrix = payoff_matrix.copy()
    min_elem = matrix.min()
    shift_value = abs(min_elem) + 1 if min_elem <= 0 else 0
    matrix += shift_value

    # Игрок 1
    res1 = linprog(
        c=np.ones(matrix.shape[0]),
        A_ub=-matrix.T,
        b_ub=-np.ones(matrix.shape[1]),
        method='simplex'
    )
    if not res1.success:
        print(f"Ошибка (игрок 1): {res1.message}")
        return None

    value = res1.fun
    p = res1.x / value

    # Игрок 2
    res2 = linprog(
        c=-np.ones(matrix.shape[1]),
        A_ub=matrix,
        b_ub=np.ones(matrix.shape[0]),
        method='simplex'
    )
    if not res2.success:
        print(f"Ошибка (игрок 2): {res2.message}")
        return None

    q = res2.x / value
    return 1 / value - shift_value, p, q

# Визуализация
def visualize(matrix: np.ndarray, spectrum: Tuple[np.ndarray, np.ndarray]):
    fig = plt.figure(figsize=(14, 7))
    plt.subplot(1, 2, 1)
    plt.ylabel('probability')

    for k in [0, 1]:
        ax = plt.subplot(1, 2, k + 1)
        plt.xlabel('strategy #')
        plt.title(f'Оптимальная стратегия для #{k + 1} игрока')
        for i in range(1, len(spectrum[k]) + 1):
            plt.plot([i, i], [0, spectrum[k][i - 1]],
                     c=plt.cm.plasma(spectrum[k][i - 1] / (max(spectrum[k]) + 1e-9)))
        plt.scatter(
            range(1, len(spectrum[k]) + 1),
            spectrum[k],
            s=100,
            c=spectrum[k],
            cmap='plasma',
            vmax=1
        )
        plt.xticks(range(1, len(spectrum[k]) + 1))
        plt.yticks(np.linspace(0, 1, 21))
        plt.xlim(0.9, len(spectrum[k]) + 0.1)
        plt.ylim(0, max(spectrum[k]) + 0.1)
        plt.grid(axis='y', linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.show()


# Примеры 

examples = {
    "Учебная матрица": np.array([
        [4, 0, 6, 2, 2, 1],
        [3, 8, 4, 10, 4, 4],
        [1, 2, 6, 5, 0, 0],
        [6, 6, 4, 4, 10, 3],
        [10, 4, 6, 4, 0, 9],
        [10, 7, 0, 7, 9, 8]
    ]),
    "Пример 1": np.array([
        [3, 1, 5],
        [1, 3, 3],
        [2, 2, 1]
    ]),
    "Пример 2": np.array([
        [4, 6, 8],
        [2, 5, 7],
        [1, 3, 6]
    ]),
    "Пример 3": np.array([
        [7, 3, 5, 2],
        [6, 4, 2, 3],
        [5, 6, 3, 1],
        [4, 7, 2, 4]
    ])
}


for name, payoff_matrix in examples.items():
    print(f"\n {name}")
    result = nash_equilibrium(payoff_matrix)
    if result:
        value, p, q = result
        print(f"Значение игры: {value:.4f}")
        print(f"Стратегия игрока 1 (p): {np.round(p, 3)}")
        print(f"Стратегия игрока 2 (q): {np.round(q, 3)}")
        visualize(payoff_matrix, (p, q))
    else:
        print(" Равновесие не найдено.")
