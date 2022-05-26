#define PAR_USE_COLOR
#include "artwork/timer/timer.h"
#include "progressbar.hpp"
#include <thread>

int main(int argc, char const *argv[]) {
  try {
    ns_pbar::ProgressBar bar(30, "test for progress bar", ns_pbar::RED, ns_pbar::LIGHT_YELLOW);
    for (int i = 0; i != 30; ++i) {
      // way 1
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      bar.lock();
      std::cout << "Hello, world" << std::endl;
      bar.unlock();

      // way 2 [this way makes code clean]
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      bar.lockAndUnlock([&]() {
        std::cout << "cur Idx is " << i << std::endl;
      });
      bar.setCurTask(i);
    }
    // use 'release' to stop the progress bar
    bar.release();
    std::cout << "Over" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
  }
  return 0;
}