/****************************************************************************/
/*                                                                          */
/* HexBed -- Hex editor                                                     */
/* Copyright (c) 2021-2022 Sampo Hippeläinen (hisahi)                       */
/*                                                                          */
/* This program is free software: you can redistribute it and/or modify     */
/* it under the terms of the GNU General Public License as published by     */
/* the Free Software Foundation, either version 3 of the License, or        */
/* (at your option) any later version.                                      */
/*                                                                          */
/* This program is distributed in the hope that it will be useful,          */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/* GNU General Public License for more details.                             */
/*                                                                          */
/* You should have received a copy of the GNU General Public License        */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>.   */
/*                                                                          */
/****************************************************************************/
// common/logger.hh -- used to include the logger

#ifndef HEXBED_COMMON_LOGGER_HH
#define HEXBED_COMMON_LOGGER_HH

#include <ctime>
#include <exception>
#include <memory>
#include <vector>

#include "common/format.hh"

namespace hexbed {

std::string exceptionAsString(const std::exception_ptr& eptr);

inline std::string currentExceptionAsString() {
    return exceptionAsString(std::current_exception());
};

enum class LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, FAIL };

class LogHandler {
  public:
    LogHandler(const LogHandler& copy) = default;
    LogHandler& operator=(const LogHandler& copy) = default;
    LogHandler(LogHandler&& move) = default;
    LogHandler& operator=(LogHandler&& move) = default;
    virtual void handle(LogLevel level, const std::tm& tm, const char* file,
                        size_t line, const std::string& msg) = 0;
    virtual ~LogHandler() {}

  protected:
    LogHandler() {}
};

class Logger {
  public:
    using LogHandlerPtr = std::unique_ptr<LogHandler>;
    template <typename T>
    using LogHandlerContainer = std::vector<T>;

    template <typename... Ts>
    void log(const char* file, size_t line, LogLevel level,
             const std::string& fmt, Ts&&... args) {
        if constexpr (sizeof...(Ts) > 0)
            log_(level, file, line,
                 stringFormat(fmt, std::forward<Ts>(args)...));
        else
            log_(level, file, line, fmt);
    }
    template <typename... Ts>
    void trace(const char* file, size_t line, const std::string& fmt,
               Ts&&... args) {
        log(file, line, LogLevel::TRACE, fmt, std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    void debug(const char* file, size_t line, const std::string& fmt,
               Ts&&... args) {
        log(file, line, LogLevel::DEBUG, fmt, std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    void info(const char* file, size_t line, const std::string& fmt,
              Ts&&... args) {
        log(file, line, LogLevel::INFO, fmt, std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    void warn(const char* file, size_t line, const std::string& fmt,
              Ts&&... args) {
        log(file, line, LogLevel::WARN, fmt, std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    void error(const char* file, size_t line, const std::string& fmt,
               Ts&&... args) {
        log(file, line, LogLevel::ERROR, fmt, std::forward<Ts>(args)...);
    }
    template <typename... Ts>
    void fail(const char* file, size_t line, const std::string& fmt,
              Ts&&... args) {
        log(file, line, LogLevel::FAIL, fmt, std::forward<Ts>(args)...);
    }

    template <typename T, typename... Ts>
    void addHandler(Ts&&... args) {
        handlers_.emplace_back(std::make_unique<T>(std::forward<Ts>(args)...));
        debug(__FILE__, __LINE__,
              std::string("Added new logger ") + typeid(T).name());
    }

  private:
    void log_(LogLevel level, const char* file, size_t line,
              const std::string& s);
    LogHandlerContainer<LogHandlerPtr> handlers_;
};

extern Logger logger;
extern bool logger_ok;

#define HEXBED_NOOP (static_cast<void>(0))

#define LOG_ADD_HANDLER(T, ...) logger.addHandler<T>(__VA_ARGS__)
#define LOG_TRACE(...) \
    (logger_ok ? logger.trace(__FILE__, __LINE__, __VA_ARGS__) : HEXBED_NOOP)
#define LOG_DEBUG(...) \
    (logger_ok ? logger.debug(__FILE__, __LINE__, __VA_ARGS__) : HEXBED_NOOP)
#define LOG_INFO(...) \
    (logger_ok ? logger.info(__FILE__, __LINE__, __VA_ARGS__) : HEXBED_NOOP)
#define LOG_WARN(...) \
    (logger_ok ? logger.warn(__FILE__, __LINE__, __VA_ARGS__) : HEXBED_NOOP)
#define LOG_ERROR(...) \
    (logger_ok ? logger.error(__FILE__, __LINE__, __VA_ARGS__) : HEXBED_NOOP)
#define LOG_FAIL(...) \
    (logger_ok ? logger.fail(__FILE__, __LINE__, __VA_ARGS__) : HEXBED_NOOP)

#if NDEBUG
#define HEXBED_BREAKPOINT()
#define HEXBED_ASSERT(...) HEXBED_NOOP
#else
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#define HEXBED_BREAKPOINT() __asm__ volatile("int $0x03")
#elif defined(_MSC_VER)
#include <intrin.h>
#define HEXBED_BREAKPOINT() __debugbreak()
#elif defined(_POSIX_VERSION)
#include <signal.h>
#define HEXBED_BREAKPOINT() raise(SIGTRAP)
#else
#define HEXBED_BREAKPOINT() std::terminate()
#endif
inline void hexbedAssert(const char* file, size_t line, bool cond,
                         const char* msg, const char* scond) {
    if (!cond) {
        if (logger_ok)
            logger.fail(file, line, "%s (assertion '%s' failed)", msg, scond);
        HEXBED_BREAKPOINT();
    }
}

#define HEXBED_ASSERT1(cond)                                                \
    hexbed::hexbedAssert(__FILE__, __LINE__, !!(cond), "assertion failure", \
                         #cond)
#define HEXBED_ASSERT2(cond, msg) \
    hexbed::hexbedAssert(__FILE__, __LINE__, !!(cond), msg, #cond)

#define HEXBED_ASSERT_PICK(_, cond, msg, F, ...) F
#define HEXBED_ASSERT(...)                                           \
    HEXBED_ASSERT_PICK(, ##__VA_ARGS__, HEXBED_ASSERT2(__VA_ARGS__), \
                       HEXBED_ASSERT1(__VA_ARGS__))
#endif

};  // namespace hexbed

#endif /* HEXBED_COMMON_LOGGER_HH */
