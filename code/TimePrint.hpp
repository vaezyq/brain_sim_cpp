
#include <chrono>
#include <iostream>

namespace dtb {
    class TimePrint {
    public:
        TimePrint() {
            start = std::chrono::system_clock::now();     //构造时构造当前时间
        }

        void print_time() {
            end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";
        }

    private:
        std::chrono::time_point<std::chrono::system_clock> start;
        std::chrono::time_point<std::chrono::system_clock> end;

    };
};
