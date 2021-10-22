//
// Created by Jie Ren (jieren9806@gmail.com) on 2021/10/21.
//

#ifndef TIMER_TIMER_H
#define TIMER_TIMER_H

#define TIMER_RECORD_TIME_TYPE std::chrono::nanoseconds
#define TIMER_REPORT_TIME_TYPE std::chrono::milliseconds
#define TIMER_REPORT_TIME_NAME "ms"

#include <chrono>
#include <map>
#include <unordered_map>
#include <memory>
#include <set>

namespace Timer {
    struct Timer {
        template<typename ...Args>
        Timer(Args&& ...args) = delete;

        enum TimeUnit_t { ns, us, ms, s, m, h };

        static void SetTimeUnit(TimeUnit_t time_unit) { time_unit_ = time_unit; };

        static void StartRecording(const std::string &name, TimeUnit_t=time_unit_);

        static void StartRecording(const std::string &name, const std::string &father_name, TimeUnit_t=time_unit_);

        static void StopRecording(const std::string &name);

        static void Erase(const std::string &name);

        static void ResetAll();

        static void Reset(const std::string &name);

        static void ReportAll();

        static void Report(const std::string &name);

    private:
        // default is "ms"
        static TimeUnit_t time_unit_;
    };
}

#endif //TIMER_TIMER_H
