// nurikabe_solver.cpp
#include "nurikabe_solver.hpp"
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <stdexcept>

// Рекурсивний алгоритм пошуку
std::vector<nurikabe_solver::grid> nurikabe_solver::solve(const grid initial) {
    // Зберігаємо незмінну копію та ініціалізуємо робочу сітку
    initial_grid_ = initial;
    rows_ = static_cast<int>(initial.size());
    cols_ = rows_ ? static_cast<int>(initial[0].size()) : 0;
    grid_ = initial;
    used_.assign(rows_ * cols_, 0);

    // Рекурсивний пошук
    if (!solve_recurse()) {
        // Повертаємо початковий стан глобальних полів
        grid_.clear();
        used_.clear();
        rows_ = 0;
        cols_ = 0;
        initial_grid_.clear();
        throw std::runtime_error("Рішення не існує");
    }

    grid_.clear();
    used_.clear();

    // Відновлюємо покроково рішення
    std::vector<nurikabe_solver::grid> res = recover_step_by_step();
    // Повертаємо початковий стан глобальних полів
    rows_ = 0;
    cols_ = 0;
    initial_grid_.clear();

    // Повертаємо покрокове відновлення
    return res;
}

bool nurikabe_solver::solve_recurse() {
    // Список для зберігання вільних клітинок з числами
    std::vector<cell> nums;
    collect_numbers(nums);

    if (nums.empty()) {
        /*
            Якщо вільних клітинок не залишилося - заповнюємо можливі пропуски в сітці і перевіряємо чи є чорні блоки 2x2,
            якщо ні - рішення знайдено і повертаємо true, якщо так - відновлюєм пропуски і повертаємо false
        */
        std::vector<cell> spaces;
        fill_spaces(spaces);
        if (!has_black_2x2_block()) return true;
        for (auto &cc : spaces) grid_[cc.row][cc.col] = EMPTY;
        return false;
    }

    /*
        Сортуємо числа за зростанням, щоб обробляти області з меншою кількістю клітинок в першу чергу —
        це дозволяє раніше виключати помилкові гілки
    */
    std::sort(nums.begin(), nums.end(), [&](cell const& a, cell const& b) {
        return grid_[a.row][a.col] < grid_[b.row][b.col];
    });

    // Перебираємо кожну числову клітинку і намагаємося збудувати область потрібного розміру
    for (cell center : nums) {
        int total = grid_[center.row][center.col];
        // Знаходимо всі допустимі способи розширити область з однієї клітинки
        auto paths = find_all_valid_paths(center, total);
        for (auto &path : paths) {
            // Позначаємо поточний шлях як "FILLED", тобто уже сформовану область
            for (int idx = 0; idx < rows_*cols_; ++idx) {
                if (path[idx] && idx != center.row*cols_ + center.col)
                    grid_[idx / cols_][idx % cols_] = FILLED;
            }
            /* 
                Позначаємо всі сусідні клітинки області в чорний, попередньо записуючи їх у список для майбутнього можливого відновлення,
                якщо шлях некоректний
            */
            std::vector<char> painted(rows_*cols_, 0);
            paint_adjacent(path, painted);

            // Позначаємо клітинку як "використану"
            used_[center.row * cols_ + center.col] = 1;

            /*
                Перевірка чи має поточна область чорні блоки 2 на 2 та чи чорна область зв'язна,
                якщо - так викликаємо рекурсивно метод solve, якщо він повертає true - передаємо цей результат далі по стеку
            */
            if (!has_black_2x2_block() && is_black_area_connected() && solve_recurse()) {
                path_stack_.push(path);
                return true;
            }

            // Шлях був неправильний, повертаємо клітинку назад
            used_[center.row * cols_ + center.col] = 0;
            // Шлях був неправильний, прибираємо позначення області як заповненої і перефарбовуємо сусідів в пусті клітинки
            for (int idx = 0; idx < rows_*cols_; ++idx) {
                if (path[idx] && idx != center.row*cols_ + center.col)
                    grid_[idx / cols_][idx % cols_] = EMPTY;
                if (painted[idx])
                    grid_[idx / cols_][idx % cols_] = EMPTY;
            }
        }

        // Якщо для поточного числа не знайдено жодного допустимого варіанту області — гілка неправильна
        return false;
    }

    // Якщо не вдалося заповнити хоча б одну область — повертаємо false
    return false;
}

std::vector<nurikabe_solver::grid> nurikabe_solver::recover_step_by_step() {
    std::vector<grid> steps;
    grid current = initial_grid_;

    // Для кожного шляху послідовно маркуємо клітинки й додаємо новий стан
    while (!path_stack_.empty()) {
        // Заповнюємо FILLED
        Path path = path_stack_.top();
        path_stack_.pop();
        for (int idx = 0; idx < rows_*cols_; ++idx) {
            if (path[idx] && current[idx/cols_][idx%cols_] == EMPTY)
                current[idx/cols_][idx%cols_] = FILLED;
        }
        // Фарбуємо чорним
        for (int idx = 0; idx < rows_*cols_; ++idx) {
            if (!path[idx]) continue;
            int r = idx/cols_, c = idx%cols_;
            for (int d = 0; d < 4; ++d) {
                int nr = r + DX_[d], nc = c + DY_[d];
                int nid = nr*cols_ + nc;
                if (out_of_bounds(nr,nc) || current[nr][nc]==BLACK || path[nid]) continue;
                current[nr][nc] = BLACK;
            }
        }
        steps.push_back(current);
    }

    // Фінальне заповнення решти порожніх
    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            if (current[r][c] == EMPTY) {
                current[r][c] = BLACK;
            }
        }
    }
    steps.push_back(current);
    return steps;
}

// Метод для зібрання клітинок з числами в список
void nurikabe_solver::collect_numbers(std::vector<cell> &cells) {
    cells.clear();
    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            int id = r*cols_ + c;
            if (grid_[r][c] > 0 && !used_[id]) {
                cells.emplace_back(r,c);
            }
        }
    }
}

// Метод для знаходження всіх можливих шляхів (областей) з однієї числової клітини
std::vector<nurikabe_solver::Path>
nurikabe_solver::find_all_valid_paths(cell start, int totalSize) {
    using Path = std::vector<char>;
    /* 
        Створюємо множину унікальних шляхів, prev для шляхів довжини k і curr для шляхів довжини k+1
        Кожен шлях - це бітова маска, яка вказує які клітинки входять в поточну білу область
    */
    std::unordered_set<Path, PathHash, PathEq> prev, curr;

    // Ініціалізуємо prev шляхом з однієї стартової клітини
    Path init(rows_ * cols_, 0);
    init[start.row * cols_ + start.col] = 1;
    prev.insert(init);

    // Шукаємо всі шляхи довжини від 1 до totalSize
    for (int len = 1; len < totalSize; ++len) {
        for (const auto &mask : prev) {
            // Пробуємо розширитися з усіх клітинок поточного шляху
            for (int idx = 0; idx < rows_ * cols_; ++idx) {
                if (!mask[idx]) continue;
                int r = idx / cols_, c = idx % cols_;
                for (int d = 0; d < 4; ++d) {
                    int nr = r + DX_[d], nc = c + DY_[d];
                    int nid = nr * cols_ + nc;
                    // Якщо клітинка за межами поля, або це чорна клітинка, або вона вже належить до шляху — пропускаємо
                    if (out_of_bounds(nr, nc) || grid_[nr][nc] == BLACK || mask[nid]) continue;
                    cell nb(nr, nc);
                    // Перевірка: чи не має клітинка сусідів з числами (щоб не захопити інше число)
                    if (!is_expandable(nb, start, mask)) continue;
                    // Додаємо нову клітинку до поточного шляху
                    Path np = mask;
                    np[nid] = 1;
                    curr.insert(std::move(np));
                }
            }
        }
        // Переходимо до наступного кроку розширення
        prev.swap(curr);
        curr.clear();
    }

    // Зберігаємо результат в вектор
    std::vector<Path> result;
    for (const auto &p : prev) result.push_back(p);
    return result;
}

// Метод для перевірки наявності інших клітинок з числами крім end для заданої клітинки
bool nurikabe_solver::is_expandable(cell a, cell end, const Path &path) const {
    for (int d = 0; d < 4; ++d) {
        int nr = a.row + DX_[d], nc = a.col + DY_[d];
        int nid = nr*cols_ + nc;
        if (out_of_bounds(nr,nc) || grid_[nr][nc]==BLACK || path[nid]) continue;
        if (grid_[nr][nc]>0 && !(cell(nr,nc)==end)) return false;
    }
    return true;
}

// Метод для перевірки меж для заданих координат
bool nurikabe_solver::out_of_bounds(int r, int c) const {
    return r<0 || c<0 || r>=rows_ || c>=cols_;
}

// Метод для фарбування сусідніх клітинок області в чорний колір і попереднє збереження їх в список "painted" для майбутнього можливого відновлення
void nurikabe_solver::paint_adjacent(const Path &path, std::vector<char> &painted) {
    for (int idx = 0; idx < rows_*cols_; ++idx) {
        if (!path[idx]) continue;
        int r = idx/cols_, c = idx%cols_;
        for (int d = 0; d < 4; ++d) {
            int nr = r + DX_[d], nc = c + DY_[d];
            int nid = nr*cols_ + nc;
            if (out_of_bounds(nr,nc) || grid_[nr][nc]==BLACK || path[nid]) continue;
            grid_[nr][nc] = BLACK;
            painted[nid] = 1;
        }
    }
}

// Метод для перевірки зв'язності чорної області
bool nurikabe_solver::is_black_area_connected() const {
    // Список visited для зберігання пройдених клітинок
    std::vector<char> vis(rows_*cols_, 0);
    // st - стартова клітинка, found - чи була знайдена стартова клітинка, cnt - кількість чорних клітинок
    int start = -1, cnt = 0;
    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            int id = r*cols_ + c;
            /*
                Тут дуже цікавий момент, так як пусті клітинки в майбутньому можуть стати чорними, ми їх також рахуємо за чорні,
                не рахуємо тільки заповнені і клітинки з цифрами
            */
            if (grid_[r][c]==BLACK || grid_[r][c]==EMPTY) {
                if (start<0) start = id;
                ++cnt;
            }
        }
    }
    // Якщо не було знайдено ні одної чорної клітинки, повератаємо true, так як технічно нульова область також зв'язна
    if (start<0) return true;
    /*
        Перевіряємо чи збігається кількість усіх чорних клітинок з кількістю досяжних клітинок починаючи з start
        (якщо область зв'язна, вони повинні співпадати)
    */
    return dfs_count_black(vis, {start/cols_, start%cols_}) == cnt;
}

// Метод для підрахунку всіх досяжних чорних клітинок починаючи з start
int nurikabe_solver::dfs_count_black(std::vector<char> &vis, cell c) const {
    int id = c.row*cols_ + c.col;
    vis[id] = 1;
    int sum = 1;
    for (int d = 0; d < 4; ++d) {
        int nr = c.row + DX_[d], nc = c.col + DY_[d];
        int nid = nr*cols_ + nc;
        if (out_of_bounds(nr,nc) || vis[nid]) continue;
        if (grid_[nr][nc]!=BLACK && grid_[nr][nc]!=EMPTY) continue;
        sum += dfs_count_black(vis, {nr,nc});
    }
    return sum;
}

// Метод для перевірки існування чорних блоків 2x2
bool nurikabe_solver::has_black_2x2_block() const {
    for (int r = 0; r+1 < rows_; ++r) {
        for (int c = 0; c+1 < cols_; ++c) {
            if (grid_[r][c]==BLACK && grid_[r+1][c]==BLACK &&
                grid_[r][c+1]==BLACK && grid_[r+1][c+1]==BLACK) {
                return true;
            }
        }
    }
    return false;
}

// Метод для заповнення можливих пропусків в кінці і попередній запис їх в список spaces для майбутнього можливого відновлення
void nurikabe_solver::fill_spaces(std::vector<cell> &spaces) {
    spaces.clear();
    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            if (grid_[r][c]==EMPTY) {
                spaces.emplace_back(r,c);
                grid_[r][c] = BLACK;
            }
        }
    }
}
