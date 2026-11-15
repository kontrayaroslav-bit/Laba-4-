#pragma once
#include <mutex>
#include <shared_mutex>
#include <string>

const int M = 3;

class Data {
    struct alignas(64) Field {
        mutable std::shared_mutex mtx;
        int value = 0;
    };

    Field f[M]; 

public:
    int get(int i) const {
        std::shared_lock<std::shared_mutex> lk(f[i].mtx);
        return f[i].value;
    }

    void set(int i, int v) {
        std::unique_lock<std::shared_mutex> lk(f[i].mtx);
        f[i].value = v;
    }

    operator std::string() const {
        std::shared_lock<std::shared_mutex> l0(f[0].mtx);
        std::shared_lock<std::shared_mutex> l1(f[1].mtx);
        std::shared_lock<std::shared_mutex> l2(f[2].mtx);
        return std::to_string(f[0].value) + " " +
            std::to_string(f[1].value) + " " +
            std::to_string(f[2].value);
    }
};
