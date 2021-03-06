//
// Created by Jie Ren (jieren9806@gmail.com) on 2021/10/21.
//

#ifndef TIMER_TIMER_H
#define TIMER_TIMER_H

#ifndef TIMER_USE_TIMER
#define TIMER_USE_TIMER true
#endif

#include <string>

struct Timer final {
    Timer() = delete;

    enum TimeUnit_t { ns, us, ms, s, m, h };

    static void SetDefaultTimeUnit(TimeUnit_t default_time_unit) { if (TIMER_USE_TIMER) __SetDefaultTimeUnit(default_time_unit); }

    static void StartRecording(const std::string &name, TimeUnit_t time_unit = default_time_unit_) { if (TIMER_USE_TIMER) __StartRecording(name, time_unit); };

    static void StartRecording(const std::string &name, const std::string &father_name, TimeUnit_t time_unit = default_time_unit_) { if (TIMER_USE_TIMER) __StartRecording(name, father_name, time_unit); };

    static std::pair<std::size_t, std::size_t> StopRecording(const std::string &name) { if (TIMER_USE_TIMER) return __StopRecording(name); };

    static void Erase(const std::string &name) { if (TIMER_USE_TIMER) __Erase(name); };

    static void ResetAll() { if (TIMER_USE_TIMER) __ResetAll(); };

    static void Reset(const std::string &name) { if (TIMER_USE_TIMER) __Reset(name); };

    static void ReportAll() { if (TIMER_USE_TIMER) __ReportAll(); };

    static void Report(const std::string &name, bool recursive=true) { if (TIMER_USE_TIMER) __Report(name, recursive); };

private:
    static void __SetDefaultTimeUnit(TimeUnit_t default_time_unit);

    static void __StartRecording(const std::string &name, TimeUnit_t time_unit);

    static void __StartRecording(const std::string &name, const std::string &father_name, TimeUnit_t time_unit);

    static std::pair<std::size_t, std::size_t> __StopRecording(const std::string &name);

    static void __Erase(const std::string &name);

    static void __ResetAll();

    static void __Reset(const std::string &name);

    static void __ReportAll();

    static void __Report(const std::string &name, bool recursive=false);

    // default is "ms"
    static TimeUnit_t default_time_unit_;
};

#endif //TIMER_TIMER_H
