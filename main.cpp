#include <iostream>
#include <vector>
#include <queue>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <cmath>

using namespace std;

constexpr int EMPTY = 0;
constexpr int BLACK = -1;
constexpr int FILLED = -2;

int rows, cols;
vector<vector<int>> grid;
vector<char> used;
const int dx[4] = {-1, 1, 0, 0};
const int dy[4] = {0, 0, -1, 1};

// Аліас для «бітової маски» шляху
using Path = std::vector<char>;

// Хеш та рівність для Path
struct PathHash {
    size_t operator()(Path const &p) const noexcept {
        size_t h = 146527;
        for (char c : p) h = h * 31 + static_cast<unsigned char>(c);
        return h;
    }
};
struct PathEq {
    bool operator()(Path const &a, Path const &b) const noexcept {
        return a == b;
    }
};


bool nurikabeSolver();
void collectNumbers(vector<int> &cells);
std::vector<Path> findAllValidPaths(int start, int totalSize);
bool isExpandable(int a, int end, const vector<char> &path);
bool outBounds(int x, int y);
void paintAdjacent(const vector<char> &path, vector<char> &painted);
bool isBlackAreaConnected();
int dfsCountBlack(vector<char> &visited, int p);
bool hasBlack2x2Block();
void fillSpaces(vector<int> &spaces);

int main() {
    auto startTime = chrono::steady_clock::now();

grid = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {3, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 2, 0},
    {0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 1, 0, 0, 0, 8, 0, 5, 0, 0, 0, 1, 0, 0, 2, 0, 0, 4, 0, 0, 0, 2},
    {6, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0},
    {0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 4, 0, 0, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    };

    rows = grid.size();
    cols = grid[0].size();
    used.assign(rows * cols, false);

    if (nurikabeSolver()) {
        cout << "Result:\n";
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                int val = grid[i][j];
                if (val == FILLED) cout << " O ";
                else if (val == BLACK) cout << " X ";
                else cout << setw(2) << val << " ";
            }
            cout << '\n';
        }
    } else {
        cout << "There is no solution\n";
    }

    auto endTime = chrono::steady_clock::now();
    cout << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count() << "\n";
    return 0;
}

bool nurikabeSolver() {
    vector<int> cells;
    collectNumbers(cells);

    if (cells.empty()) {
        vector<int> list;
        fillSpaces(list);
        if (!hasBlack2x2Block() && isBlackAreaConnected()) return true;
        for (int id : list) grid[id / cols][id % cols] = EMPTY;
        return false;
    }

    // Сортуємо за зростанням чисел
    sort(cells.begin(), cells.end(), [](int a, int b) {
        return grid[a / cols][a % cols] < grid[b / cols][b % cols];
    });

    for (int id : cells) {
        int x = id / cols, y = id % cols;
        int totalSize = grid[x][y];
        auto paths = findAllValidPaths(id, totalSize);

        for (const auto& path : paths) {
            for (int p = 0; p < rows * cols; ++p)
                if (path[p] && p != id)
                    grid[p / cols][p % cols] = FILLED;

            vector<char> painted(rows * cols, false);
            paintAdjacent(path, painted);
            used[id] = true;

            if (!hasBlack2x2Block() && isBlackAreaConnected() && nurikabeSolver())
                return true;

            used[id] = false;
            for (int p = 0; p < rows * cols; ++p) {
                if (path[p] && p != id) grid[p / cols][p % cols] = EMPTY;
                if (painted[p]) grid[p / cols][p % cols] = EMPTY;
            }
        }

        return false; // якщо для цього числа не знайшлося варіантів
    }

    return false;
}


void collectNumbers(vector<int> &cells) {
    cells.clear();
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int id = r * cols + c;
            if (grid[r][c] > 0 && !used[id])
                cells.push_back(id);
        }
    }
}

std::vector<Path> findAllValidPaths(int start, int totalSize) {
    unordered_set<Path, PathHash, PathEq> prev, curr;

    Path init(rows * cols, 0);
    init[start] = 1;
    prev.insert(init);

    for (int len = 1; len < totalSize; ++len) {
        for (const auto &mask : prev) {
            for (int idx = 0; idx < rows * cols; ++idx) {
                if (!mask[idx]) continue;
                int r = idx / cols, c = idx % cols;

                for (int d = 0; d < 4; ++d) {
                    int nr = r + dx[d], nc = c + dy[d];
                    if (outBounds(nr, nc)) continue;
                    int nidx = nr * cols + nc;
                    if (grid[nr][nc] == BLACK || mask[nidx]) continue;
                    if (!isExpandable(nidx, start, mask)) continue;

                    Path next = mask;
                    next[nidx] = 1;
                    curr.insert(std::move(next));
                }
            }
        }
        prev.swap(curr);
        curr.clear();
    }

    vector<Path> result;
    result.reserve(prev.size());
    for (const auto &mask : prev) result.push_back(mask);
    return result;
}

bool isExpandable(int a, int end, const vector<char> &path) {
    for (int d = 0; d < 4; ++d) {
        int nx = a / cols + dx[d];
        int ny = a % cols + dy[d];
        int nid = nx * cols + ny;
        if (outBounds(nx, ny) || grid[nx][ny] == BLACK) continue;
        if (path[nid]) continue;
        if (grid[nx][ny] > 0 && nid != end) return false;
    }
    return true;
}

bool outBounds(int x, int y) {
    return x < 0 || y < 0 || x >= rows || y >= cols;
}

void paintAdjacent(const vector<char> &path, vector<char> &painted) {
    for (int p = 0; p < rows * cols; ++p) {
        if (!path[p]) continue;
        int px = p / cols, py = p % cols;
        for (int d = 0; d < 4; ++d) {
            int nx = px + dx[d], ny = py + dy[d];
            if (outBounds(nx, ny) || grid[nx][ny] == BLACK) continue;
            int nid = nx * cols + ny;
            if (path[nid]) continue;
            grid[nx][ny] = BLACK;
            painted[nid] = true;
        }
    }
}

bool isBlackAreaConnected() {
    vector<char> visited(rows * cols, false);
    int start = -1, blackCount = 0;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int id = i * cols + j;
            if (grid[i][j] == BLACK || grid[i][j] == EMPTY) {
                blackCount++;
                if (start == -1) start = id;
            }
        }
    }
    if (start == -1) return true;
    int reachable = dfsCountBlack(visited, start);
    return reachable == blackCount;
}

int dfsCountBlack(vector<char> &visited, int p) {
    visited[p] = true;
    int count = 1;
    int px = p / cols, py = p % cols;
    for (int d = 0; d < 4; ++d) {
        int nx = px + dx[d], ny = py + dy[d];
        int nid = nx * cols + ny;
        if (outBounds(nx, ny) || visited[nid]) continue;
        if (grid[nx][ny] != BLACK && grid[nx][ny] != EMPTY) continue;
        count += dfsCountBlack(visited, nid);
    }
    return count;
}

bool hasBlack2x2Block() {
    for (int i = 0; i < rows - 1; ++i) {
        for (int j = 0; j < cols - 1; ++j) {
            if (grid[i][j] == BLACK && grid[i+1][j] == BLACK &&
                grid[i][j+1] == BLACK && grid[i+1][j+1] == BLACK) {
                return true;
            }
        }
    }
    return false;
}

void fillSpaces(vector<int> &spaces) {
    spaces.clear();
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (grid[i][j] == EMPTY) {
                int id = i * cols + j;
                grid[i][j] = BLACK;
                spaces.push_back(id);
            }
        }
    }
}