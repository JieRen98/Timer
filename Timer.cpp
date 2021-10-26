//
// Created by Jie Ren (jieren9806@gmail.com) on 2021/10/21.
//

#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cmath>
#include "Timer.h"

namespace {
    struct RelationTree {
        RelationTree() = delete;

        struct RelationNode {
            RelationNode() = delete;

            std::unordered_map<std::string, std::unique_ptr<RelationNode>> descendants_{};
            const RelationNode *parent;

            explicit RelationNode(const RelationNode *father_ptr);

            ~RelationNode();
        };

        static std::unordered_map<std::string, const RelationNode *> plain_nodes_;
        const static std::unique_ptr<RelationNode> root_;

        static const RelationTree::RelationNode *getNodePtr(const std::string &name);
    };

    std::unordered_map<const RelationTree::RelationNode *, std::chrono::time_point<std::chrono::system_clock>> start_time_{};
    std::unordered_map<const RelationTree::RelationNode *, Timer::TimeUnit_t> time_unit_map_{};
    std::unordered_map<std::string, const RelationTree::RelationNode *> RelationTree::plain_nodes_{};
    const std::unique_ptr<RelationTree::RelationNode> RelationTree::root_{new RelationNode{nullptr}};

    template<typename Iterator>
    inline void ExistChecker(const Iterator &find, const Iterator &end, const std::string &name) {
        if (find == end)
            throw std::runtime_error("name {" + name + "} not exist");
    }

    template<typename Iterator>
    inline void ExistChecker(const Iterator &find, const Iterator &end) {
        if (find == end)
            throw std::runtime_error("not exist");
    }

    template<typename Iterator>
    inline bool ExistCheckerNoThrow(const Iterator &find, const Iterator &end) {
        return find != end;
    }

    template<typename Map_t, typename Key_t>
    inline auto TryErase(Map_t &map, const Key_t &key) {
        auto find = map.find(key);
        if (find != map.end())
            map.erase(find);
    }

    RelationTree::RelationNode::RelationNode(const RelationTree::RelationNode *father_ptr) : parent(father_ptr) {}

    RelationTree::RelationNode::~RelationNode() {
        for (const auto &node: descendants_) {
            TryErase(start_time_, node.second.get());
            TryErase(time_unit_map_, node.second.get());
            TryErase(RelationTree::plain_nodes_, node.first);
        }
    }

    const RelationTree::RelationNode *RelationTree::getNodePtr(const std::string &name) {
        auto find = plain_nodes_.find(name);
        ExistChecker(find, plain_nodes_.end(), name);
        return find->second;
    }

    template<Timer::TimeUnit_t>
    struct TimeUnitWrapper;

    template<>
    struct TimeUnitWrapper<Timer::ns> {
        typedef std::pair<int64_t, std::chrono::nanoseconds> duration_t;
        static const std::string name_;
        static std::unordered_map<const RelationTree::RelationNode *, duration_t> duration_;
    };
    const std::string TimeUnitWrapper<Timer::ns>::name_{"ns"};
    std::unordered_map<const RelationTree::RelationNode *, TimeUnitWrapper<Timer::ns>::duration_t> TimeUnitWrapper<Timer::ns>::duration_{};

    template<>
    struct TimeUnitWrapper<Timer::us> {
        typedef std::pair<int64_t, std::chrono::microseconds> duration_t;
        static const std::string name_;
        static std::unordered_map<const RelationTree::RelationNode *, duration_t> duration_;
    };
    const std::string TimeUnitWrapper<Timer::us>::name_{"us"};
    std::unordered_map<const RelationTree::RelationNode *, TimeUnitWrapper<Timer::us>::duration_t> TimeUnitWrapper<Timer::us>::duration_{};

    template<>
    struct TimeUnitWrapper<Timer::ms> {
        typedef std::pair<int64_t, std::chrono::milliseconds> duration_t;
        static const std::string name_;
        static std::unordered_map<const RelationTree::RelationNode *, duration_t> duration_;
    };
    const std::string TimeUnitWrapper<Timer::ms>::name_{"ms"};
    std::unordered_map<const RelationTree::RelationNode *, TimeUnitWrapper<Timer::ms>::duration_t> TimeUnitWrapper<Timer::ms>::duration_{};

    template<>
    struct TimeUnitWrapper<Timer::s> {
        typedef std::pair<int64_t, std::chrono::seconds> duration_t;
        static const std::string name_;
        static std::unordered_map<const RelationTree::RelationNode *, duration_t> duration_;
    };
    const std::string TimeUnitWrapper<Timer::s>::name_{"s"};
    std::unordered_map<const RelationTree::RelationNode *, TimeUnitWrapper<Timer::s>::duration_t> TimeUnitWrapper<Timer::s>::duration_{};

    template<>
    struct TimeUnitWrapper<Timer::m> {
        typedef std::pair<int64_t, std::chrono::minutes> duration_t;
        static const std::string name_;
        static std::unordered_map<const RelationTree::RelationNode *, duration_t> duration_;
    };
    const std::string TimeUnitWrapper<Timer::m>::name_{"m"};
    std::unordered_map<const RelationTree::RelationNode *, TimeUnitWrapper<Timer::m>::duration_t> TimeUnitWrapper<Timer::m>::duration_{};

    template<>
    struct TimeUnitWrapper<Timer::h> {
        typedef std::pair<int64_t, std::chrono::hours> duration_t;
        static const std::string name_;
        static std::unordered_map<const RelationTree::RelationNode *, duration_t> duration_;
    };
    const std::string TimeUnitWrapper<Timer::h>::name_{"h"};
    std::unordered_map<const RelationTree::RelationNode *, TimeUnitWrapper<Timer::h>::duration_t> TimeUnitWrapper<Timer::h>::duration_{};

    class DurationManager {
        template<typename Duration_t, typename Node_t>
        static auto
        getCountWrapper(const std::unordered_map<const Node_t *, Duration_t> &duration, const Node_t *node_ptr) {
            typedef std::pair<int64_t, int64_t> Ret_t;
            auto find = duration.find(node_ptr);
            ExistCheckerNoThrow(find, duration.end());
            return ExistCheckerNoThrow(find, duration.end()) ? Ret_t{find->second.first, find->second.second.count()}
                                                             : Ret_t{1, -1};
        }

        template<typename Duration_t, typename TimePoint_t, typename Node_t>
        static void
        insertRecordWrapper(std::unordered_map<const Node_t *, std::pair<int64_t, Duration_t>> &duration,
                            TimePoint_t &&start, TimePoint_t &&end,
                            const Node_t *node_ptr) {
            const auto duration_find = duration.find(node_ptr);
            if (duration_find == duration.end())
                duration.emplace(node_ptr, std::pair<int64_t, Duration_t>{1, std::chrono::duration_cast<Duration_t>(
                        end - start)});
            else {
                duration_find->second.first++;
                duration_find->second.second += std::chrono::duration_cast<Duration_t>(end - start);
            }
        }

        template<typename Duration_t, typename Node_t>
        static void ResetWrapper(std::unordered_map<const Node_t *, std::pair<int64_t, Duration_t>> &duration,
                                 const Node_t *node_ptr) {
            duration.find(node_ptr)->second.second = Duration_t::zero();
        }

    public:
        static const std::string &getName(Timer::TimeUnit_t time_unit) {
            switch (time_unit) {
                case Timer::ns:
                    return TimeUnitWrapper<Timer::ns>::name_;
                case Timer::us:
                    return TimeUnitWrapper<Timer::us>::name_;
                case Timer::ms:
                    return TimeUnitWrapper<Timer::ms>::name_;
                case Timer::s:
                    return TimeUnitWrapper<Timer::s>::name_;
                case Timer::m:
                    return TimeUnitWrapper<Timer::m>::name_;
                case Timer::h:
                    return TimeUnitWrapper<Timer::h>::name_;
            }
        }

        static auto getCount(Timer::TimeUnit_t time_unit, const RelationTree::RelationNode *node_ptr) {
            switch (time_unit) {
                case Timer::ns:
                    return getCountWrapper(TimeUnitWrapper<Timer::ns>::duration_, node_ptr);
                case Timer::us:
                    return getCountWrapper(TimeUnitWrapper<Timer::us>::duration_, node_ptr);
                case Timer::ms:
                    return getCountWrapper(TimeUnitWrapper<Timer::ms>::duration_, node_ptr);
                case Timer::s:
                    return getCountWrapper(TimeUnitWrapper<Timer::s>::duration_, node_ptr);
                case Timer::m:
                    return getCountWrapper(TimeUnitWrapper<Timer::m>::duration_, node_ptr);
                case Timer::h:
                    return getCountWrapper(TimeUnitWrapper<Timer::h>::duration_, node_ptr);
            }
        }

        template<typename TimePoint_t>
        static void
        insertRecord(TimePoint_t &&start, TimePoint_t &&end, const RelationTree::RelationNode *node_ptr) {
            switch (time_unit_map_.find(node_ptr)->second) {
                case Timer::ns:
                    insertRecordWrapper(TimeUnitWrapper<Timer::ns>::duration_, std::forward<TimePoint_t>(start),
                                        std::forward<TimePoint_t>(end), node_ptr);
                    return;
                case Timer::us:
                    insertRecordWrapper(TimeUnitWrapper<Timer::us>::duration_, std::forward<TimePoint_t>(start),
                                        std::forward<TimePoint_t>(end), node_ptr);
                    return;
                case Timer::ms:
                    insertRecordWrapper(TimeUnitWrapper<Timer::ms>::duration_, std::forward<TimePoint_t>(start),
                                        std::forward<TimePoint_t>(end), node_ptr);
                    return;
                case Timer::s:
                    insertRecordWrapper(TimeUnitWrapper<Timer::s>::duration_, std::forward<TimePoint_t>(start),
                                        std::forward<TimePoint_t>(end), node_ptr);
                    return;
                case Timer::m:
                    insertRecordWrapper(TimeUnitWrapper<Timer::m>::duration_, std::forward<TimePoint_t>(start),
                                        std::forward<TimePoint_t>(end), node_ptr);
                    return;
                case Timer::h:
                    insertRecordWrapper(TimeUnitWrapper<Timer::h>::duration_, std::forward<TimePoint_t>(start),
                                        std::forward<TimePoint_t>(end), node_ptr);
                    return;
            }
        }

        static void Reset(Timer::TimeUnit_t time_unit, const RelationTree::RelationNode *node_ptr) {
            switch (time_unit) {
                case Timer::ns:
                    ResetWrapper(TimeUnitWrapper<Timer::ns>::duration_, node_ptr);
                    return;
                case Timer::us:
                    ResetWrapper(TimeUnitWrapper<Timer::us>::duration_, node_ptr);
                    return;
                case Timer::ms:
                    ResetWrapper(TimeUnitWrapper<Timer::ms>::duration_, node_ptr);
                    return;
                case Timer::s:
                    ResetWrapper(TimeUnitWrapper<Timer::s>::duration_, node_ptr);
                    return;
                case Timer::m:
                    ResetWrapper(TimeUnitWrapper<Timer::m>::duration_, node_ptr);
                    return;
                case Timer::h:
                    ResetWrapper(TimeUnitWrapper<Timer::h>::duration_, node_ptr);
                    return;
            }
        }

        static void Reset(const RelationTree::RelationNode *node_ptr) {
            Timer::TimeUnit_t time_unit = time_unit_map_.find(node_ptr)->second;
            switch (time_unit) {
                case Timer::ns:
                    ResetWrapper(TimeUnitWrapper<Timer::ns>::duration_, node_ptr);
                    return;
                case Timer::us:
                    ResetWrapper(TimeUnitWrapper<Timer::us>::duration_, node_ptr);
                    return;
                case Timer::ms:
                    ResetWrapper(TimeUnitWrapper<Timer::ms>::duration_, node_ptr);
                    return;
                case Timer::s:
                    ResetWrapper(TimeUnitWrapper<Timer::s>::duration_, node_ptr);
                    return;
                case Timer::m:
                    ResetWrapper(TimeUnitWrapper<Timer::m>::duration_, node_ptr);
                    return;
                case Timer::h:
                    ResetWrapper(TimeUnitWrapper<Timer::h>::duration_, node_ptr);
                    return;
            }
        }

        static void ResetAll() {
            for (const auto time_unit_pair: time_unit_map_) {
                Reset(time_unit_pair.second, time_unit_pair.first);
            }
        }
    };

    inline long double CastUnit(int64_t count, Timer::TimeUnit_t from, Timer::TimeUnit_t to) {
        long double ratio = std::pow(10., 3 * (from - to));
        return ratio * count;
    }

    template<typename Node_t>
    void PrintOneNodeHelper(int64_t father_count,
                            Timer::TimeUnit_t time_unit_father,
                            const Node_t *root,
                            const std::string &name,
                            int level) {
        Timer::TimeUnit_t time_unit;
        int64_t count;
        if (level >= 0) {
            time_unit = time_unit_map_.find(root)->second;
            auto count_pair = DurationManager::getCount(time_unit, root);
            count = count_pair.second;
            std::cout
                    << std::setw(5 * level + 1)
                    << std::setfill(' ')
                    << '|'
                    << std::setw(5)
                    << std::setfill('-')
                    << name
                    << ": "
                    << count_pair.first
                    << " call(s), average "
                    << count / count_pair.first
                    << DurationManager::getName(time_unit);
            if (level > 0) {
                if (father_count == -1)
                    std::cout << ", ratio " << "N/A";
                else
                    std::cout
                            << ", ratio "
                            << std::setprecision(4)
                            << count / CastUnit(father_count, time_unit_father, time_unit);
            }
            std::cout << std::endl;
        }

        for (const auto &node: root->descendants_)
            PrintOneNodeHelper(count, time_unit, node.second.get(), node.first, level + 1);
    }

    template<typename ...Args>
    void PrintOneNode(Args&& ...args) {
        PrintOneNodeHelper(0, Timer::s, std::forward<Args>(args)...);
    }

}

void Timer::__SetDefaultTimeUnit(TimeUnit_t default_time_unit) { default_time_unit_ = default_time_unit; }

void Timer::__StartRecording(const std::string &name, TimeUnit_t time_unit) {
    if (RelationTree::plain_nodes_.find(name) == RelationTree::plain_nodes_.end()) {
        std::unique_ptr<RelationTree::RelationNode> node_ptr{
                new RelationTree::RelationNode{RelationTree::root_.get()}};
        RelationTree::plain_nodes_.emplace(name, node_ptr.get());
        time_unit_map_.emplace(node_ptr.get(), time_unit);
        RelationTree::root_->descendants_.emplace(name, std::move(node_ptr));
    } else if (RelationTree::root_->descendants_.find(name) == RelationTree::root_->descendants_.end()) {
        throw std::runtime_error("name {" + name + "} duplicated");
    }
    start_time_[RelationTree::getNodePtr(name)] = std::chrono::system_clock::now();
}

void Timer::__StartRecording(const std::string &name, const std::string &father_name, TimeUnit_t time_unit) {
    auto father_find = RelationTree::plain_nodes_.find(father_name);
    auto find = RelationTree::plain_nodes_.find(name);
    ExistChecker(father_find, RelationTree::plain_nodes_.end(), father_name);
    if (RelationTree::plain_nodes_.find(name) == RelationTree::plain_nodes_.end()) {
        std::unique_ptr<RelationTree::RelationNode> node_ptr{new RelationTree::RelationNode{father_find->second}};
        RelationTree::plain_nodes_.emplace(name, node_ptr.get());
        time_unit_map_.emplace(node_ptr.get(), time_unit);
        const_cast<RelationTree::RelationNode *>(father_find->second)->descendants_.emplace(name,
                                                                                            std::move(node_ptr));
    } else if (father_find->second->descendants_.find(name) == father_find->second->descendants_.end()) {
        throw std::runtime_error("name {" + name + "} duplicated");
    }
    start_time_[RelationTree::getNodePtr(name)] = std::chrono::system_clock::now();
}

void Timer::__StopRecording(const std::string &name) {
    const RelationTree::RelationNode *node_ptr = RelationTree::getNodePtr(name);
    auto end_time_point = std::chrono::system_clock::now();
    ExistChecker(start_time_.find(node_ptr), start_time_.end(), name);
    DurationManager::insertRecord(start_time_.find(node_ptr)->second, end_time_point, node_ptr);
}

void Timer::__Erase(const std::string &name) {
    auto find = RelationTree::plain_nodes_.find(name);
    ExistChecker(find, RelationTree::plain_nodes_.end(), name);
    const_cast<RelationTree::RelationNode *>(find->second->parent)->descendants_.erase(name);
    RelationTree::plain_nodes_.erase(name);
}

void Timer::__ResetAll() {
    DurationManager::ResetAll();
}

void Timer::__Reset(const std::string &name) {
    auto find = RelationTree::plain_nodes_.find(name);
    ExistChecker(find, RelationTree::plain_nodes_.end(), name);
    DurationManager::Reset(find->second);
}

void Timer::__ReportAll() {
    std::cout << "Report {all} in the recorder (-1 means recorder not stopped):" << std::endl;
    PrintOneNode(RelationTree::root_.get(), "root", -1);
}

void Timer::__Report(const std::string &name) {
    auto find = RelationTree::plain_nodes_.find(name);
    ExistChecker(find, RelationTree::plain_nodes_.end(), name);
    std::cout << "Report {" + name + "} in the recorder (-1 means recorder not stopped):" << std::endl;
    PrintOneNode(find->second, name, 0);
}

Timer::TimeUnit_t Timer::default_time_unit_{ms};
