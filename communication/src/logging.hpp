/*

file: logging.hpp
author: osklu414
created: 2019-11-14

Utility functions for printing to stdout.

*/

#ifndef LOGGING_HPP
#define LOGGING_HPP

#define LOG_TRACE   1
#define LOG_INFO    1
#define LOG_WARN    1
#define LOG_ERROR   1
#define LOG_FATAL   1

#include <iostream>


template<typename ... A>
void log(A && ... args)
{
    (std::cout << ... << args);
}

#define COLOR_RESET "\x1B[0m"
#define COLOR_BLACK "\x1B[30m"
#define COLOR_RED "\x1B[31m"
#define COLOR_GREEN "\x1B[32m"
#define COLOR_YELLOW "\x1B[33m"
#define COLOR_BLUE "\x1B[34m"
#define COLOR_MAGENTA "\x1B[35m"
#define COLOR_CYAN "\x1B[36m"
#define COLOR_WHITE "\x1B[37m"

#if LOG_TRACE
#define TRACE(...) log(COLOR_GREEN "[trace] " COLOR_RESET, __VA_ARGS__, '\n')
#else
#define TRACE(...)
#endif

#if LOG_INFO
#define INFO(...) log(COLOR_CYAN "[info] " COLOR_RESET, __VA_ARGS__, '\n')
#else
#define INFO(...)
#endif

#if LOG_WARN
#define WARN(...) log(COLOR_YELLOW "[warn] " COLOR_RESET, __VA_ARGS__, '\n')
#else
#define WARN(...)
#endif

#if LOG_ERROR
#define ERROR(...) log(COLOR_RED "[error] " COLOR_RESET, __VA_ARGS__, '\n')
#else
#define ERROR(...)
#endif

#if LOG_FATAL
#define FATAL(...) log(COLOR_MAGENTA "[fatal] " COLOR_RESET, __VA_ARGS__, '\n')
#else
#define FATAL(...)
#endif

#endif // LOGGING_HPP
