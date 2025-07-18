// nurikabe_pairs.cpp
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdexcept>
#include <chrono>
#include "grid_reader.hpp"
#include "nurikabe_solver.hpp"
#include "user_solution_checker.hpp"

using grid = std::vector<std::vector<int>>;

// Вивід довільної сітки у консолі
void display(const grid &grid) {
    int rows = grid.size();
    int cols = rows ? grid[0].size() : 0;

    const std::string horizontal = "───";
    const std::string corner_tl = "┌", corner_tr = "┐";
    const std::string corner_bl = "└", corner_br = "┘";
    const std::string vertical = "│";

    // Верхня межа
    std::cout << corner_tl;
    for (int c = 0; c < cols; ++c)
        std::cout << horizontal;
    std::cout << corner_tr << "\n";

    // Вміст рядків
    for (int r = 0; r < rows; ++r) {
        std::cout << vertical;
        for (int c = 0; c < cols; ++c) {
            int v = grid[r][c];
            if (v == nurikabe_solver::FILLED)
                std::cout << " #";
            else if (v == nurikabe_solver::BLACK)
                std::cout << " X";
            else
                std::cout << std::setw(2) << v;

            std::cout << " ";
        }
        std::cout << vertical << "\n";
    }

    // Нижня межа
    std::cout << corner_bl;
    for (int c = 0; c < cols; ++c)
        std::cout << horizontal;
    std::cout << corner_br << "\n";
}

int main() {
    nurikabe_solver solver;

    while (true) {
        std::cout << "\n=== Головне меню ===\n"
                << "1) Ввести вхідні дані вручну\n"
                << "2) Виконати всі тести\n"
                << "3) Вийти\n"
                << "\nВаш вибір: ";
        int choice1;
        if (!(std::cin >> choice1) || choice1 == 3) {
            std::cout << "Вихід!\n";
            break;
        }

        if (choice1 == 1) {
            grid_data data;
            try {
                data = grid_reader::from_console(std::cin, std::cout);
            } catch (const std::runtime_error &e) {
                std::cerr << "Помилка зчитування: " << e.what() << "\n";
                continue;
            }

            std::cout << "\n--- Початкова сітка ---\n";
            display(data.grid);

            while (true) {
                std::cout << "\n--- Режим розв'язання ---\n"
                        << "1) Ввести своє рішення для перевірки\n"
                        << "2) Покроково вирішити програмно\n"
                        << "3) Повернутися до головного меню\n"
                        << "\nВаш вибір: ";
                int choice2;
                if (!(std::cin >> choice2) || choice2 == 3) break;

                if (choice2 == 1) {
                    user_solution_checker checker(data.grid);
                    checker.input_solution_interactive(std::cin, std::cout);
                } else if (choice2 == 2) {
                    try {
                        auto start = std::chrono::high_resolution_clock::now();
                        auto steps = solver.solve(data.grid);
                        auto end = std::chrono::high_resolution_clock::now();
                        std::chrono::duration<double> duration = end - start;

                        std::cout << "\n--- Покрокове рішення ---\n";
                        for (size_t i = 0; i < steps.size(); ++i) {
                            std::cout << "Крок " << (i+1) << " з " << steps.size() << ":\n";
                            display(steps[i]);
                            std::cout << "---\n";
                        }
                        std::cout << "Час розв'язання: " << std::fixed << std::setprecision(2) << duration.count() << " секунд\n";
                    } catch (const std::runtime_error &e) {
                        std::cerr << e.what() << "\n";
                    }
                } else {
                    std::cerr << "Невірний пункт меню\n";
                }
            }

        } else if (choice1 == 2) {
            std::ifstream fin("input.txt");
            if (!fin) {
                std::cerr << "Не вдалося відкрити input.txt\n";
                continue;
            }

            int test_number = 0;
            while (true) {
                grid_data data;
                if (!(fin >> data.rows >> data.cols)) break;

                data.grid.assign(data.rows, std::vector<int>(data.cols));
                for (int r = 0; r < data.rows; ++r) {
                    for (int c = 0; c < data.cols; ++c) {
                        if (!(fin >> data.grid[r][c])) {
                            std::cerr << "Недостатньо даних у input.txt для тесту " << (test_number + 1) << "\n";
                            goto end_tests;
                        }
                    }
                }

                test_number++;
                std::cout << "\n=== Тест " << test_number << " ===\n";
                std::cout << "\nВхідна сітка:\n";
                display(data.grid);

                try {
                    auto start = std::chrono::high_resolution_clock::now();
                    auto steps = solver.solve(data.grid);
                    auto end = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> duration = end - start;

                    std::cout << "\nРозв'язана сітка:\n";
                    display(steps.back());

                    std::cout << "Час розв'язання: " << std::fixed << std::setprecision(2) << duration.count() << " секунд\n";
                } catch (const std::runtime_error &e) {
                    std::cerr << "Помилка при розв'язанні тесту " << test_number << ": " << e.what() << "\n";
                }
            }

            end_tests:
            std::cout << "\n=== Виконано " << test_number << " тестів ===\n";

        } else {
            std::cerr << "Невірний пункт меню\n";
        }
    }

    return 0;
}

