#define PAR_USE_COLOR
#include "progressbar.hpp"
#include <thread>

int main(int argc, char const *argv[]) {
  try {
    ns_pbar::ProgressBar bar(15, ns_pbar::RED, ns_pbar::LIGHT_YELLOW);
    for (int i = 0; i != bar.taskCount(); ++i) {
      bar.setCurTask(i, "doing task " + std::to_string(i + 1));

      // way 1 [this way makes program faster]
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      bar.lock();
      std::cout << "Hello, world" << std::endl;
      bar.unlock();

      // way 2 [this way makes code cleaner]
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      bar.lockAndUnlock([&]() {
        std::cout << "cur Idx is " << i + 1 << std::endl;
      });
    }
    // use 'release' to stop the progress bar
    bar.release();
    std::cout << "Over" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }
  return 0;
}