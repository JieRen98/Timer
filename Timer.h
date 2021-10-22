//
// Created by Jie Ren (jieren9806@gmail.com) on 2021/10/21.
//

#ifndef TIMER_TIMER_H
#define TIMER_TIMER_H

#if USE_TIMER
#include <chrono>
#include <map>
#include <memory>
#define TIMER_EMPTY_BODY
#else
#include <string>
#define TIMER_EMPTY_BODY {}
#endif

struct Timer {
    template<typename ...Args>
    Timer(Args&& ...args) = delete;

    enum TimeUnit_t { ns, us, ms, s, m, h };

    static void SetDefaultTimeUnit(TimeUnit_t default_time_unit) TIMER_EMPTY_BODY;

    static void StartRecording(const std::string &name, TimeUnit_t=default_time_unit_) TIMER_EMPTY_BODY;

    static void StartRecording(const std::string &name, const std::string &father_name, TimeUnit_t=default_time_unit_) TIMER_EMPTY_BODY;

    static void StopRecording(const std::string &name) TIMER_EMPTY_BODY;

    static void Erase(const std::string &name) TIMER_EMPTY_BODY;

    static void ResetAll() TIMER_EMPTY_BODY;

    static void Reset(const std::string &name) TIMER_EMPTY_BODY;

    static void ReportAll() TIMER_EMPTY_BODY;

    static void Report(const std::string &name) TIMER_EMPTY_BODY;

private:
    // default is "ms"
#if USE_TIMER
    static TimeUnit_t default_time_unit_;
#else
    constexpr static const TimeUnit_t default_time_unit_{ns};
#endif
};

#undef TIMER_EMPTY_BODY
#endif //TIMER_TIMER_H
