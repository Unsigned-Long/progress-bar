// #define PROGRESS_COLOR_BAR
#include "progress_bar.hpp"
#include <thread>

int main(int argc, char const *argv[]) {
  try {
    ns_pbar::ProgressBar bar(30, "test for progress bar");
    for (int i = 0; i != 30; ++i) {
      // way 1
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      bar.clear();
      std::cout << "Hello, world" << std::endl;
      bar.show(i);
      // way 2
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      bar.update(i, [&i]() {
        std::cout << "cur Idx is " << i << std::endl;
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
