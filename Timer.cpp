//
// Created by Jie Ren (jieren9806@gmail.com) on 2021/10/21.
//

#include <iostream>
#include <iomanip>
#include <vector>
#include "Timer.h"

namespace Timer {
    namespace {
        template<typename Iterator>
        inline void ExistChecker(const Iterator &find, const Iterator &end, const std::string &name) {
            if (find == end)
                throw std::runtime_error("name {" + name + "} not exist");
        }

        template<typename Duration_t, typename Node_t>
        void PrintOneNode(const Duration_t &duration, const Node_t root, const std::string &name, int level) {
            if (level >= 0) {
                auto find = duration.find(root.get());
                ExistChecker(find, duration.end(), name);
                if (level > 0)
                    std::cout
                            << std::setw(5 * (level - 1) + 1)
                            << std::setfill(' ')
                            << '|'
                            << std::setw(5)
                            << std::setfill('-');
                std::cout
                        << name
                        << ": "
                        << std::chrono::duration_cast<TIMER_REPORT_TIME_TYPE>(find->second).count()
                        << TIMER_REPORT_TIME_NAME
                        << std::endl;
            }
            for (const auto &node: root->descendants_) {
                PrintOneNode(duration, node.second, node.first, level + 1);
            }
        }

        template<typename RelationTree_t>
       inline auto getNodePtr(const std::string &name, const RelationTree_t &rt) {
            auto find = rt.plain_nodes_.find(name);
            ExistChecker(find, rt.plain_nodes_.end(), name);
            return find->second.get();
        }

        struct RelationNode {
            std::unordered_map<std::string, std::shared_ptr<RelationNode>> descendants_{};
            std::weak_ptr<RelationNode> parent;
            explicit RelationNode(const std::shared_ptr<RelationNode>& father_ptr);
            ~RelationNode();
        };

        struct RelationTree {
            std::shared_ptr<RelationNode> root_{new RelationNode{nullptr}};
            std::map<std::string, std::shared_ptr<RelationNode>> plain_nodes_{};
        } rt{};

        RelationNode::RelationNode(const std::shared_ptr<RelationNode>& father_ptr) : parent(father_ptr) {};

        RelationNode::~RelationNode() { for (const auto &node : descendants_) rt.plain_nodes_.erase(node.first); }

        std::map<void *, TIMER_RECORD_TIME_TYPE> duration_{};
        std::map<void *, std::chrono::time_point<std::chrono::system_clock>> start_time_{};
    }

    void Timer::StartRecording(const std::string &name) {
        if (rt.plain_nodes_.find(name) == rt.plain_nodes_.end()) {
            std::shared_ptr<RelationNode> node_ptr{new RelationNode{rt.root_}};
            rt.root_->descendants_.emplace(name, node_ptr);
            rt.plain_nodes_.emplace(name, std::move(node_ptr));
        } else if (rt.root_->descendants_.find(name) == rt.root_->descendants_.end()) {
            throw std::runtime_error("name {" + name + "} duplicated");
        }
        start_time_[getNodePtr(name, rt)] = std::chrono::system_clock::now();
    }

    void Timer::StartRecording(const std::string &name, const std::string &father_name) {
        auto father_find = rt.plain_nodes_.find(father_name);
        auto find = rt.plain_nodes_.find(name);
        ExistChecker(father_find, rt.plain_nodes_.end(), father_name);
        if (rt.plain_nodes_.find(name) == rt.plain_nodes_.end()) {
            std::shared_ptr<RelationNode> node_ptr{new RelationNode{father_find->second}};
            father_find->second->descendants_.emplace(name, node_ptr);
            rt.plain_nodes_.emplace(name, std::move(node_ptr));
        } else if (father_find->second->descendants_.find(name) == father_find->second->descendants_.end()) {
            throw std::runtime_error("name {" + name + "} duplicated");
        }
        start_time_[getNodePtr(name, rt)] = std::chrono::system_clock::now();
    }

    void Timer::StopRecording(const std::string &name) {
        RelationNode *node_ptr = getNodePtr(name, rt);
        auto end_time_point = std::chrono::system_clock::now();
        ExistChecker(start_time_.find(node_ptr), start_time_.end(), name);
        const auto duration_find = duration_.find(node_ptr);
        if (duration_find == duration_.end())
            duration_.emplace(node_ptr, end_time_point - start_time_.find(node_ptr)->second);
        else
            duration_find->second += end_time_point - start_time_.find(node_ptr)->second;
    }

    void Timer::Erase(const std::string &name) {
        auto find = rt.plain_nodes_.find(name);
        ExistChecker(find, rt.plain_nodes_.end(), name);
        std::vector<std::weak_ptr<RelationNode>> vs;
        find->second->parent.lock()->descendants_.erase(name);
        rt.plain_nodes_.erase(name);
    }

    void Timer::ResetAll() {
        for (auto &duration: duration_)
            duration.second = TIMER_RECORD_TIME_TYPE::zero();
    }

    void Timer::Reset(const std::string &name) {
        auto find = rt.plain_nodes_.find(name);
        ExistChecker(find, rt.plain_nodes_.end(), name);
        auto ptr_find = duration_.find(find->second.get());
        ExistChecker(ptr_find, duration_.end(), name);
        ptr_find->second = TIMER_RECORD_TIME_TYPE::zero();
    }

    void Timer::ReportAll() {
        std::cout << "Report all the results in the recorder:" << std::endl;
        PrintOneNode(duration_, rt.root_, "root", -1);
    }

    void Timer::Report(const std::string &name) {
        auto find = rt.plain_nodes_.find(name);
        ExistChecker(find, rt.plain_nodes_.end(), name);
        std::cout << "Report {" + name + "} in the recorder:" << std::endl;
        PrintOneNode(duration_, find->second, name, 0);
    }
}
