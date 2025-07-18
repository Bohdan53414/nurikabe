// user_solution_checker.cpp
#include "user_solution_checker.hpp"
#include <limits>
#include <queue>
#include <iomanip>

user_solution_checker::user_solution_checker(const std::vector<std::vector<int>>& initialGrid) : initial_(initialGrid),
    rows_(static_cast<int>(initialGrid.size())),
    cols_(rows_ ? static_cast<int>(initialGrid[0].size()) : 0),
    user_grid_(rows_, std::vector<int>(cols_, 0))
{}

// Метод для перевірки правильності вводу і коректності користувацького рішеня
bool user_solution_checker::input_solution_interactive(std::istream& in, std::ostream& out) {
    out << "\nПримітка для рішення:\n"
        "n = -1 — чорна клітинка\n"
        "n = -2 — заповнена клітинка\n"
        "n >  0 — підказка (тільки у клітинках з початковими підказками)\n\n";

    // Звичитуємо користувацьку сітку, якщо вона має неправильний формат - повертаєм false
    if (!input_phase(in, out))
        return false;

    // Повертаємо результат перевірки користувацього рішення на коректність
    return validation_phase(out);
}

// Метод для перевірки правильності вводу користувацього рішення
bool user_solution_checker::input_phase(std::istream& in, std::ostream& out) {
    // Порядково зчитуєм сітку рішення від користувача
    for (int r = 0; r < rows_; ++r) {
        while (true) {
            out << "Введіть " << cols_ << " значень для рядка " << (r+1)
    << " через пробіл: " << std::flush;
            std::vector<int> row_vals(cols_);
            bool format_error = false;
            // Зчитуємо весь рядок користувача в список для подальшої перевірки
            for (int c = 0; c < cols_; ++c) {
                if (!(in >> row_vals[c])) {
                    out << "Помилка: введіть ціле число.\n";
                    format_error = true;
                    break;
                }

                int v = row_vals[c];
                // Допускаються тільки заповнені, чорні, і клітинки з підказками
                if (v < -2 || v == 0) {
                    out << "Помилка: недопустиме значення "<<v
                        <<" ("<<r+1<<","<<c+1<<").\n";
                    format_error = true;
                    break;
                }
                /*
                    Якщо користувач ввів число більше нуля (клітинка з підказкою), перевіряєм чи є вона на цьому місці
                    в початковій сітці і чи значення збігаються
                */
                if (v > 0 && initial_[r][c] == 0) {
                    out << "Помилка: у клітинці ("<< r+1 << "," << c+1
                        <<") не було підказки, а ви ввели " << v << ".\n";
                    format_error = true;
                    break;
                }
                // Якщо користувач не ввів підказку, яка була на цьому місці - виводимо відповідну помилку
                if (initial_[r][c] > 0 && initial_[r][c] != v) {
                    out << "Помилка: у клітинці ("<< r+1 << "," << c+1
                        <<") очікувалась підказка " << initial_[r][c]
                        << ", а ви ввели " << v << ".\n";
                    format_error = true;
                    break;
                }
            }
            if (format_error) {
                in.clear();
                in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            } else {
                // Якщо рядок коректний - записуєм його користвацьку сітку для майбутньої перевірки
                user_grid_[r] = std::move(row_vals);
                break;
            }
        }
    }
    return true;
}

bool user_solution_checker::validation_phase(std::ostream& out) {
    // Координати верхнього лівого кута чорного блока 2x2, якщо він буде
    cell lt;
    if (check_black_2x2_block(lt)) {
        // Якщо чорний блок було знайдено, виводим помилку і координати лівого верхнього кута
        out << "\nПомилка: знайдено 2×2 чорний блок у клітині ("
            << lt.row+1 << "," << lt.col+1 << ")\n";
        return false;
    }
    // Перевірка чорної області на зв'язність
    if (!check_black_connectivity()) {
        out << "\nПомилка: чорна область незв'язна\n";
        return false;
    }
    // Причина помилки
    std::string reason;
    // Клітинка де знайдена помилка
    cell loc;
    if (!check_islands(reason, loc)) {
        out << "\nПомилка: " << reason;
        if (loc.row >= 0)
            out << " (рядок " << loc.row+1 << ", стовпець " << loc.col+1 << ")";
        out << "\n";
        return false;
    }

    out << "\nРішення правильне!\n";
    return true;
}

// Метод для перевірки наявності чорних блоків 2x2 і повернення лівої верхньої клітинки проблемного блоку
bool user_solution_checker::check_black_2x2_block(cell &lt) const {
    for (int r = 0; r+1 < rows_; ++r) {
        for (int c = 0; c+1 < cols_; ++c) {
            if (user_grid_[r][c]==-1 &&
                user_grid_[r+1][c]==-1 &&
                user_grid_[r][c+1]==-1 &&
                user_grid_[r+1][c+1]==-1)
            {
                lt.row = r; lt.col = c;
                return true;
            }
        }
    }
    return false;
}

// Метод для перевірки зв'язності чорної області
bool user_solution_checker::check_black_connectivity() const {
    std::vector<char> vis(rows_ * cols_, 0);
    cell start(-1, -1);
    int total_black = 0;
    for (int r = 0; r < rows_; ++r)
        for (int c = 0; c < cols_; ++c)
            if (user_grid_[r][c] == -1) {
                if (start.row < 0) start = cell(r, c);
                ++total_black;
            }
    if (start.row < 0) return true;
    std::queue<cell> q;
    q.push(start);
    vis[start.row * cols_ + start.col] = 1;
    int count = 0;
    while (!q.empty()) {
        cell u = q.front(); q.pop();
        ++count;
        for (int d = 0; d < 4; ++d) {
            int nr = u.row + DR_[d], nc = u.col + DC_[d];
            int nid = nr * cols_ + nc;
            if (out_of_bounds(nr, nc) || vis[nid] || user_grid_[nr][nc] != -1)
                continue;
            vis[nid] = 1;
            q.push(cell(nr, nc));
        }
    }
    return count == total_black;
}

// Метод для перевірки площі островів
bool user_solution_checker::check_islands(std::string &reason, cell &loc) const {
    // Список для зберігання пройдених клітинок
    std::vector<char> vis(rows_ * cols_, 0);

    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            // Якщо клітинка не пройдена і належить білій області - перевіряємо її
            if (!vis[r * cols_ + c] && (user_grid_[r][c] > 0 || user_grid_[r][c] == -2)) {
                // Черга для проходу в ширину по білій області
                std::queue<cell> q;
                cell start(-1, -1);
                // Змінна для підрахунку розміру білої області
                int size = 0;
                // Додаємо в чергу стартову клітинку і помічаємо її як пройдену
                q.push(cell(r, c));
                vis[r * cols_ + c] = 1;

                // Проходимося по всім клітинкам і рахуємо розмір області, також паралельно записуємо клітинку з числом в start
                while (!q.empty()) {
                    cell u = q.front(); q.pop();
                    size++;
                    if (user_grid_[u.row][u.col] > 0) {
                        if (start.row == -1) {
                            start = u;
                        } else {
                            reason = "острів має більше 1 підказки";
                            loc = u;
                            return false;
                        }
                    }
                    for (int d = 0; d < 4; ++d) {
                        int nr = u.row + DR_[d], nc = u.col + DC_[d];
                        int nid = nr * cols_ + nc;
                        if (out_of_bounds(nr, nc) || vis[nid]) continue;
                        if (user_grid_[nr][nc] > 0 || user_grid_[nr][nc] == -2) {
                            vis[nid] = 1;
                            q.push(cell(nr, nc));
                        }
                    }
                }

                // Якщо в області немає клітинки з підказкою - повертаємо false
                if (start.row == -1) {
                    reason = "острів не має підказки";
                    loc = cell(r, c);
                    return false;
                }

                // Якщо площа області не збігається з сумою клітинок з підказками - повертаємо false
                int area = user_grid_[start.row][start.col];
                if (area != size) {
                    reason = "площа острова (" + std::to_string(size) + ") не дорівнює значенню підказки: " + std::to_string(area);
                    loc = start;
                    return false;
                }
            }
        }
    }
    return true;
}
