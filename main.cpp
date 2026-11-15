// Кондратенко Ярослав К-26
// Програмний проєкт 4, варіант 7
// Багатопотокова структура даних з 3 полями

#include <chrono>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "Data.h"

using namespace std;
using namespace chrono;

const int N = 3000000;
const int RUNS = 5;      

const int WEIGHTS_A[7] = { 5, 5, 30, 5, 24, 1, 30 };   
const int WEIGHTS_B[7] = { 1, 1, 1, 1, 1, 1, 1 };     
const int WEIGHTS_C[7] = { 1, 2, 2, 30, 1, 60, 4 };    

struct Op {
    char type;   
    int field;
    int value;
};

void generate(const string& name, const int w[7], unsigned seed) {
    mt19937 gen(seed);
    discrete_distribution<int> dist(w, w + 7);
    ofstream out(name);
    for (int i = 0; i < N; i++) {
        int k = dist(gen);
        if (k == 6) {
            out << "string\n";
        }
        else if (k % 2 == 0) {
            out << "read " << k / 2 << "\n";
        }
        else {
            out << "write " << k / 2 << " 1\n";
        }
    }
}

vector<Op> load(const string& name, double percent[7]) {
    vector<Op> ops;
    ops.reserve(N);
    long long cnt[7] = { 0 };
    ifstream in(name);
    string cmd;
    while (in >> cmd) {
        Op o = { 's', 0, 0 };
        if (cmd == "read") {
            o.type = 'r';
            in >> o.field;
            cnt[o.field * 2]++;
        }
        else if (cmd == "write") {
            o.type = 'w';
            in >> o.field >> o.value;
            cnt[o.field * 2 + 1]++;
        }
        else {
            cnt[6]++;
        }
        ops.push_back(o);
    }
    for (int i = 0; i < 7; i++)
        percent[i] = 100.0 * cnt[i] / ops.size();
    return ops;
}

void worker(Data& d, const vector<Op>& ops, long long& res) {
    long long sum = 0;
    for (const Op& o : ops) {
        if (o.type == 'r') {
            sum += d.get(o.field);
        }
        else if (o.type == 'w') {
            d.set(o.field, o.value);
        }
        else {
            string s = d;
            sum += s.size();
        }
    }
    res = sum;  
}

double run(const vector<Op>& ops, int nt) {
    vector<vector<Op>> parts(nt);
    size_t part = ops.size() / nt;
    for (int i = 0; i < nt; i++)
        parts[i].assign(ops.begin() + i * part, ops.begin() + (i + 1) * part);

    Data d;
    vector<long long> res(nt, 0);
    vector<thread> th;

    auto start = steady_clock::now();
    for (int i = 0; i < nt; i++)
        th.emplace_back(worker, ref(d), cref(parts[i]), ref(res[i]));
    for (auto& t : th)
        t.join();
    auto end = steady_clock::now();

    return duration<double, milli>(end - start).count();
}

int main() {
    const char* files[3] = { "test_a.txt", "test_b.txt", "test_c.txt" };
    const char* names[3] = { "a (as in task)", "b (all equal)", "c (very different)" };
    const int* weights[3] = { WEIGHTS_A, WEIGHTS_B, WEIGHTS_C };

    cout << "Generating files, " << N << " operations each...\n";
    for (int i = 0; i < 3; i++)
        generate(files[i], weights[i], 100 + i);

    double table[3][3];

    for (int f = 0; f < 3; f++) {
        double p[7];
        vector<Op> ops = load(files[f], p);

        cout << "\n" << files[f] << " (" << names[f] << "), " << ops.size() << " operations\n";
        cout << fixed << setprecision(1);
        cout << "  actual %: r0=" << p[0] << " w0=" << p[1]
            << " r1=" << p[2] << " w1=" << p[3]
            << " r2=" << p[4] << " w2=" << p[5]
            << " string=" << p[6] << "\n";

        for (int nt = 1; nt <= 3; nt++) {
            double total = 0;
            for (int r = 0; r < RUNS; r++)
                total += run(ops, nt);
            table[f][nt - 1] = total / RUNS;
            cout << "  " << nt << " thread(s): " << table[f][nt - 1]
                << " ms (average of " << RUNS << " runs)\n";
        }
    }

    cout << "\nAverage time, ms\n";
    cout << left << setw(22) << "file" << right << setw(12) << "1 thread"
        << setw(12) << "2 threads" << setw(12) << "3 threads" << "\n";
    for (int f = 0; f < 3; f++) {
        cout << left << setw(22) << names[f] << right;
        for (int t = 0; t < 3; t++)
            cout << setw(12) << table[f][t];
        cout << "\n";
    }
    return 0;
}