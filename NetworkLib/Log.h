#pragma once
#include <iostream>
#include <utility>


namespace Log {


// Empty function prototypes to be replaced with your implementations
inline void Debug(void){
    std::cout << std::endl;
}

template <typename T> 
inline void Debug(const T& t) {
    std::cout << t << std::endl;
}

template<typename H, typename... T> 
inline void Debug(H h, T... args) {
    std::cout << h << ", ";
    Debug(args...);
};

 inline void Info(void) {
    std::cout << std::endl;
}


 template <typename T>
 inline void Info(const T& t) {
     std::cout << t << std::endl;
 }


template<typename H, typename... T>
inline void Info(H h, T... args) {
    std::cout << h << std::endl;
    Info(args...);
};

 inline void Warning(void) {
    std::cout << std::endl;
}


 template <typename T>
 inline void Warning(const T& t) {
     std::cout << t << std::endl;
 }


template<typename H, typename... T>
inline void Warning(H h, T... args) {
    std::cout << h << std::endl;
    Warning(args...);
};

 inline void Error(void) {
    std::cout << std::endl;
}

 template <typename T>
 inline void Error(const T& t) {
     std::cout << t << std::endl;
 }

template<typename H, typename... T>
inline void Error(H h, T... args) {
    std::cout << h << std::endl;
    Error(args...);
};
}