#include <iostream>

#pragma once
namespace Log {
// Empty function prototypes to be replaced with your implementations

inline void Debug() {std::cout << std::endl;}

template<typename T, typename... Ts> 
inline void Debug(const T& t, const Ts&... args) {
    std::cout << t << ", ";
    Debug(args...);
};

inline void Info() { std::cout << std::endl; }

template<typename T, typename... Ts>
inline void Info(const T& t, const Ts&... args) {
    std::cout << t << ", ";
    Info(args...);
};

inline void Warning() { std::cout << std::endl; }

template<typename T, typename... Ts>
inline void Warning(const T& t, const Ts&... args) {
    std::cout << t << ", ";
    Warning(args...);
};

inline void Error() { std::cout << std::endl; }

template<typename T, typename... Ts> 
inline void Error(const T& t, const Ts&... args) {
    std::cout << t << ", ";
    Error(args...);
};
}