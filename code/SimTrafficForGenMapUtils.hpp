#pragma once

#include "MapSplitMethod.hpp"
#include "SimulationTrafficUtils.hpp"


enum class Iter_Criteria {
    OUTPUT_TRAFFIC,
    INPUT_TRAFFIC,
    OUTPUT_INPUT_MAX,
    OUTPUT_INPUT_SUM
};

namespace dtb {

    using diff_traffic_size_type = double;

    class SimTrafficForGenMapUtils {

    public:
        static bool is_in_changed_gpu_lists(const std::vector<gpu_size_type>  &gpu_idxs);

        void get_criteria_traffic_by_output_input_traffic(std::array<traffic_size_type, GPU_NUM> &criteria_traffic);

        void compute_2_dim_output_input_traffic_for_map_iter_no_recursive(const gpu_size_type &send_idx);


        static diff_traffic_size_type compute_gpu_gpu_or_group_by_out_changed(const gpu_size_type &send_idx,
                                                                       const std::vector<gpu_size_type> &recv_lists);

        static const std::shared_ptr<MapSplitMethod> &getMspPtr();

        explicit SimTrafficForGenMapUtils(Iter_Criteria iterCriFlag = Iter_Criteria::OUTPUT_INPUT_MAX);

        const std::array<diff_traffic_size_type, GPU_NUM> &getOutputTrafficTemp() const;

        const std::array<diff_traffic_size_type, GPU_NUM> &getInputTrafficTemp() const;

        std::array<traffic_size_type, GPU_NUM> &getOutputTraffic();

        std::array<traffic_size_type, GPU_NUM> &getInputTraffic();

    protected:
        Iter_Criteria iter_cri_flag;   //每次根据何种流量进行迭代
        static constexpr unsigned dimensions = 2;

        std::array<diff_traffic_size_type, GPU_NUM> output_traffic_temp{};
        std::array<diff_traffic_size_type, GPU_NUM> input_traffic_temp{};

        std::array<traffic_size_type, GPU_NUM> output_traffic{};       //输出流量
        std::array<traffic_size_type, GPU_NUM> input_traffic{};  //输入流量

        static std::shared_ptr<MapSplitMethod> msp_ptr;

        static SimulationTrafficUtils stutils;

        static std::shared_ptr<LoadData> load_data_ptr;

    };

    std::shared_ptr<MapSplitMethod> SimTrafficForGenMapUtils::msp_ptr = MapSplitMethod::getInstance();

    SimulationTrafficUtils SimTrafficForGenMapUtils::stutils = SimulationTrafficUtils();

    std::shared_ptr<LoadData> SimTrafficForGenMapUtils::load_data_ptr = LoadData::getLoadDataInstance();

    bool SimTrafficForGenMapUtils::is_in_changed_gpu_lists(const std::vector<gpu_size_type> &gpu_idxs) {
        //这里可以替换成any_of
        for (unsigned int gpu_idx: gpu_idxs) {
            if (std::find(msp_ptr->getChangedGpuIdx().begin(), msp_ptr->getChangedGpuIdx().end(), gpu_idx) !=
                msp_ptr->getChangedGpuIdx().end()) {
                return true;
            }
        }
        return false;
    }


    void SimTrafficForGenMapUtils::compute_2_dim_output_input_traffic_for_map_iter_no_recursive(
            const gpu_size_type &send_idx) {
        bool is_changed_send = is_in_changed_gpu_lists({send_idx});

        std::vector<gpu_size_type> recv_lists(dtb::GPU_NUM, 0);
        std::generate(recv_lists.begin(), recv_lists.end(), [i = 0]()mutable { return i++; });
        diff_traffic_size_type temp_traffic{0.0}, temp_traffic_old{0.0};
        auto const &forward_list_send = stutils.get_list_send_by_route_table(send_idx, recv_lists);
        for (auto &in_idx_pair: *forward_list_send) {
            if (in_idx_pair.second.size() == 1) {
                if (!is_in_same_node(send_idx, in_idx_pair.first)) {
                    auto lists_changed_flag = is_in_changed_gpu_lists({in_idx_pair.first});
                    if (is_changed_send && (!lists_changed_flag)) {
                        temp_traffic = compute_gpu_gpu_or_group_by_out_changed(send_idx, {in_idx_pair.first});
                        output_traffic_temp[send_idx] += temp_traffic;
                        input_traffic_temp[in_idx_pair.first] += temp_traffic;
//                        std::cout << 1 << " " << temp_traffic << std::endl;
                    } else if (lists_changed_flag) {
                        temp_traffic = stutils.sim_traffic_between_two_gpu(send_idx, in_idx_pair.first);
                        temp_traffic_old = stutils.sim_traffic_between_two_gpu(send_idx, in_idx_pair.first,
                                                                               msp_ptr->getMapTableBeforeChange());
                        output_traffic_temp[send_idx] += (temp_traffic - temp_traffic_old);

                        input_traffic_temp[in_idx_pair.first] += (temp_traffic - temp_traffic_old);
//                        std::cout << 2 << " " << temp_traffic << " " << temp_traffic_old << std::endl;
                    }
                }
            } else {
                auto recv_changed_flag = is_in_changed_gpu_lists(in_idx_pair.second);
                if (is_changed_send && (!recv_changed_flag)) {
                    temp_traffic = compute_gpu_gpu_or_group_by_out_changed(send_idx, {in_idx_pair.second});
                    output_traffic_temp[send_idx] += temp_traffic;
                    input_traffic_temp[in_idx_pair.first] += temp_traffic;
//                    std::cout << 3 << " " << temp_traffic << std::endl;
                } else if (recv_changed_flag) {

                    temp_traffic = stutils.sim_traffic_between_gpu_group(send_idx, in_idx_pair.second);
                    temp_traffic_old = stutils.sim_traffic_between_gpu_group(send_idx, in_idx_pair.second,
                                                                             msp_ptr->getMapTableBeforeChange());
                    output_traffic_temp[send_idx] += (temp_traffic - temp_traffic_old);
                    input_traffic_temp[in_idx_pair.first] += (temp_traffic - temp_traffic_old);
//                    std::cout << 4 << " " << temp_traffic << " " << temp_traffic_old << std::endl;
                }
                auto forward_sub_idx = stutils.get_list_send_by_route_table(in_idx_pair.first,
                                                                            in_idx_pair.second);
                for (auto &in_idx_pair_1: *forward_sub_idx) {
                    if (!is_in_same_node(send_idx, in_idx_pair_1.first)) {
                        auto recv_2_changed = is_in_changed_gpu_lists({in_idx_pair_1.first});
                        if (is_changed_send && (!recv_2_changed)) {

                            if (!is_in_same_node(send_idx, in_idx_pair_1.first)) {
                                temp_traffic = compute_gpu_gpu_or_group_by_out_changed(send_idx,
                                                                                       {in_idx_pair_1.first});
                                output_traffic_temp[in_idx_pair.first] += temp_traffic;
                                input_traffic_temp[in_idx_pair_1.first] += temp_traffic;
//                                std::cout << 5 << " " << in_idx_pair_1.first << " " << temp_traffic << std::endl;
                            }
                        } else if (recv_changed_flag) {
                            temp_traffic = stutils.sim_traffic_between_two_gpu(send_idx, in_idx_pair_1.first);
                            temp_traffic_old = stutils.sim_traffic_between_two_gpu(send_idx, in_idx_pair_1.first,
                                                                                   msp_ptr->getMapTableBeforeChange());
                            output_traffic_temp[in_idx_pair.first] += (temp_traffic - temp_traffic_old);
                            input_traffic_temp[in_idx_pair_1.first] += (temp_traffic - temp_traffic_old);
//                            std::cout << 6 << " " << temp_traffic << " " << temp_traffic_old << std::endl;
                        }
                    }
                }
            }
        }
    }

    traffic_size_type SimTrafficForGenMapUtils::compute_gpu_gpu_or_group_by_out_changed(const gpu_size_type &send_idx,
                                                                                        const std::vector<gpu_size_type> &recv_lists) {
        std::vector<unsigned> changed_pop_idx_in_new_map{};
        auto &map_table_old = msp_ptr->getMapTableBeforeChange();
        for (auto const &pop_pair: load_data_ptr->getMapTable()[send_idx]) {
            if ((!map_table_old[send_idx].count(pop_pair.first)) ||
                (map_table_old[send_idx][pop_pair.first] != pop_pair.second)) {
                changed_pop_idx_in_new_map.emplace_back(pop_pair.first);
            }
        }
        std::vector<unsigned> changed_pop_idx_in_old_map{};
        for (auto const &pop_pair: map_table_old[send_idx]) {
            if ((!load_data_ptr->getMapTable()[send_idx].count(pop_pair.first)) ||
                (load_data_ptr->getMapTable()[send_idx][pop_pair.first] != pop_pair.second)) {
                changed_pop_idx_in_old_map.emplace_back(pop_pair.first);
            }
        }
        return static_cast<int>( stutils.compute_pop_traffic(send_idx, changed_pop_idx_in_new_map, recv_lists)) -
               static_cast<int>( stutils.compute_pop_traffic(send_idx, changed_pop_idx_in_old_map, recv_lists,
                                                             map_table_old));
    }

    void SimTrafficForGenMapUtils::get_criteria_traffic_by_output_input_traffic(
            std::array<traffic_size_type, GPU_NUM> &criteria_traffic) {
        switch (iter_cri_flag) {
            case Iter_Criteria::INPUT_TRAFFIC:
                criteria_traffic = input_traffic;
                break;
            case Iter_Criteria::OUTPUT_TRAFFIC:
                criteria_traffic = output_traffic;
                break;
            case Iter_Criteria::OUTPUT_INPUT_SUM:
                std::transform(input_traffic.begin(), input_traffic.end(), output_traffic.begin(),
                               criteria_traffic.begin(), [](int a, int b) { return a + b; });
                break;
            case Iter_Criteria::OUTPUT_INPUT_MAX:
                std::transform(input_traffic.begin(), input_traffic.end(), output_traffic.begin(),
                               criteria_traffic.begin(), [](int a, int b) { return std::max(a, b); });
                break;
        }

    }

    const std::shared_ptr<MapSplitMethod> &SimTrafficForGenMapUtils::getMspPtr() {
        return msp_ptr;
    }

    SimTrafficForGenMapUtils::SimTrafficForGenMapUtils(Iter_Criteria iterCriFlag) : iter_cri_flag(iterCriFlag) {}

    const std::array<traffic_size_type, GPU_NUM> &SimTrafficForGenMapUtils::getOutputTrafficTemp() const {
        return output_traffic_temp;
    }

    const std::array<traffic_size_type, GPU_NUM> &SimTrafficForGenMapUtils::getInputTrafficTemp() const {
        return input_traffic_temp;
    }

    std::array<traffic_size_type, GPU_NUM> &SimTrafficForGenMapUtils::getOutputTraffic() {
        return output_traffic;
    }

    std::array<traffic_size_type, GPU_NUM> &SimTrafficForGenMapUtils::getInputTraffic() {
        return input_traffic;
    }


}


