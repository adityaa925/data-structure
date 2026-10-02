#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include <chrono>
#include <iomanip>

namespace cds::test {

struct TestResult {
    std::string name;
    bool passed;
    std::string message;
    double duration_ms;
};

class TestRunner {
    std::vector<TestResult> results_;
    int passed_ = 0;
    int failed_ = 0;

public:
    template <typename F>
    void run(const std::string& name, F&& test_func) {
        auto start = std::chrono::high_resolution_clock::now();
        bool passed = false;
        std::string message;
        try {
            test_func(*this);
            passed = true;
        } catch (const std::exception& e) {
            message = e.what();
        } catch (...) {
            message = "Unknown exception";
        }
        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double, std::milli>(end - start).count();

        results_.push_back({name, passed, message, duration});
        if (passed) ++passed_; else ++failed_;

        std::cout << (passed ? "[PASS] " : "[FAIL] ") << name 
                  << " (" << std::fixed << std::setprecision(2) << duration << " ms)" << std::endl;
        if (!passed) std::cout << "  -> " << message << std::endl;
    }

    void assert_true(bool condition, const std::string& msg = "") {
        if (!condition) throw std::runtime_error(msg.empty() ? "Assertion failed" : msg);
    }

    void assert_false(bool condition, const std::string& msg = "") {
        if (condition) throw std::runtime_error(msg.empty() ? "Assertion failed" : msg);
    }

    template <typename T>
    void assert_eq(const T& expected, const T& actual, const std::string& msg = "") {
        if (expected != actual) {
            throw std::runtime_error(msg.empty() ? 
                "Expected: " + std::to_string(expected) + ", Actual: " + std::to_string(actual) : msg);
        }
    }

    template <typename T>
    void assert_ne(const T& expected, const T& actual, const std::string& msg = "") {
        if (expected == actual) {
            throw std::runtime_error(msg.empty() ? "Values should not be equal" : msg);
        }
    }

    void assert_throws(std::function<void()> func, const std::string& msg = "") {
        try {
            func();
            throw std::runtime_error(msg.empty() ? "Expected exception was not thrown" : msg);
        } catch (const std::runtime_error&) {
            throw;
        } catch (...) {
        }
    }

    int summary() {
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Passed: " << passed_ << ", Failed: " << failed_ << ", Total: " << (passed_ + failed_) << std::endl;
        return failed_ == 0 ? 0 : 1;
    }
};

} // namespace cds::test