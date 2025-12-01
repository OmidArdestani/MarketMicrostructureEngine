#pragma once

#include <chrono>
#include <print>    // C++23: std::print / std::println

// Optional compile-time switch to disable all timing with zero runtime cost
#ifndef SCOPE_TIMER_DISABLED
#define SCOPE_TIMER_DISABLED 0
#endif

// Max number of concurrent timers per thread.
// You can tweak this if needed.
#ifndef SCOPE_TIMER_MAX_SLOTS
#define SCOPE_TIMER_MAX_SLOTS 32
#endif

template<class T>
class ScopeTimer final {
public:
    using Clock     = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration  = Clock::duration;

    bool InUse{false};

public:
    ScopeTimer(bool raii = false)
    {
#if !SCOPE_TIMER_DISABLED
        std::locale::global(std::locale("en_US.utf8")); // set locale (platform dependent)

        if(raii)
        {
            start();
        }
#endif
    }

    ~ScopeTimer()
    {
#if !SCOPE_TIMER_DISABLED
        if(InUse)
        {
            endAndLog();
        }
#endif
    }

    // Start a timer associated with this label.
    // Overwrites any existing timer with the same label in this thread.
    void start() noexcept
    {
#if !SCOPE_TIMER_DISABLED
        InUse = true;
        beginTime_ = Clock::now();
#endif
    }

    // End timer, return elapsed time in nanoseconds.
    // Returns 0 if label not found.
    [[nodiscard]] Duration end() noexcept
    {
#if !SCOPE_TIMER_DISABLED
        InUse = false;
        auto current = Clock::now();
        auto duration = (current - beginTime_);

        return duration;
#endif
    }

    // End timer, log, and return elapsed time in nanoseconds.
    void endAndLog() noexcept
    {
#if !SCOPE_TIMER_DISABLED
        log_(end());
#endif
    }

private:
    void log_(Duration duration)
    {
        using namespace std::chrono;

        auto units = std::chrono::duration_cast<T>(duration);

        std::println("[ScopeTimer] took {:L}", units);
    }

    TimePoint beginTime_;
};

template<class T>
class ScopeTimerManagement{
public:
    using Timer    = ScopeTimer<T>;
    using Duration = typename Timer::Duration;
    using ScopeTimerPtr = std::unique_ptr<ScopeTimer<T>>;

public:
    static void start(std::string_view label)
    {
#if !SCOPE_TIMER_DISABLED
        auto& slots = slots_;
        for(auto& item : slots)
        {
            if(!item.used)
            {
                item.label = label;
                item.used  = true;
                item.scopeTimer.start();

                return;
            }
        }
#endif
    }

    [[nodiscard]] static Duration end(std::string_view label)
    {
#if !SCOPE_TIMER_DISABLED
        auto& slots = slots_;
        for(auto& item : slots)
        {
            if(item.used && item.label == label)
            {
                item.used = false;
                return item.scopeTimer.end();
            }
        }
#else
        return Duration::zero();
#endif
    }

    static void endAndLog(std::string_view label)
    {
#if !SCOPE_TIMER_DISABLED
        auto& slots = slots_;
        for(auto& item : slots)
        {
            if(item.label == label)
            {
                // item.used = false;
                auto d = item.scopeTimer.end();
                log_(d, item.label);

                return;
            }
        }
#endif
    }

private:
    static void log_(Duration duration, std::string_view label)
    {
        using namespace std::chrono;

        auto units = std::chrono::duration_cast<T>(duration);

        std::println("[ScopeTimer] {} took {:L}", label, units);
    }

private:
    struct Slot {
        std::string_view label{};
        ScopeTimer<T>    scopeTimer{};
        bool             used{false};
    };
    // One small array of slots per thread, no mutex, no heap.
    static thread_local inline Slot slots_[SCOPE_TIMER_MAX_SLOTS];
};


using NScopeTimers = ScopeTimerManagement<std::chrono::nanoseconds>;
