// #define PAR_USE_COLOR
#include "artwork/timer/timer.h"
#include "progressbar-v2.hpp"

int main(int argc, char const *argv[]) {
  try {
    ns_pbar::ProgressBar bar(30, "test for progress bar", ns_pbar::RED, ns_pbar::LIGHT_YELLOW);

    for (int i = 0; i != 30; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      PBAR_LOCK
      std::cout << "Hello, world!" << std::endl;
      PBAR_UNLOCK
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
