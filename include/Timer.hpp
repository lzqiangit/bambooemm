#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <mutex>

class Timer {
public:
    // 获取单例实例
    static Timer& getInstance() {
        static Timer instance;
        return instance;
    }

    // 开始计时
    void start() {
        std::lock_guard<std::mutex> lock(mutex_);
        start_time_ = std::chrono::high_resolution_clock::now();
        running_ = true;
    }

    // 结束计时
    void stop() {
        std::lock_guard<std::mutex> lock(mutex_);
        end_time_ = std::chrono::high_resolution_clock::now();
        running_ = false;
    }

    // 获取运行时间（毫秒）
    double getDuration() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (running_) {
            auto current_time = std::chrono::high_resolution_clock::now();
            return std::chrono::duration<double, std::milli>(current_time - start_time_).count();
        } else {
            return std::chrono::duration<double, std::milli>(end_time_ - start_time_).count();
        }
    }

private:
    // 私有构造函数，防止外部实例化
    Timer() : running_(false) {}

    // 禁用拷贝构造函数和赋值运算符
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
    std::chrono::time_point<std::chrono::high_resolution_clock> end_time_;
    bool running_;
    mutable std::mutex mutex_;
};

#endif // TIMER_H