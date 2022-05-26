#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H

#include <algorithm>
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#ifdef __linux__
#include <sys/ioctl.h>
#else
#include <windows.h>
#endif

namespace ns_pbar {

  std::mutex pbMtx;

#define PBAR_LOCK \
  {               \
    std::lock_guard<std::mutex> pb_lock_stdos(ns_pbar::pbMtx);

#define PBAR_UNLOCK \
  }

  /**
   * @brief colors for the progress bar
   */
  enum BarColor : int {
    /**
     * @brief options
     */
    NONE = 0,
    BLACK = 40,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    PURPLE,
    PEACOCK_BLUE,
    WHITE,
    LIGHT_BLACK = 100,
    LIGHT_RED,
    LIGHT_GREEN,
    LIGHT_YELLOW,
    LIGHT_BLUE,
    LIGHT_PURPLE,
    LIGHT_PEACOCK_BLUE,
    LIGHT_WHITE
  };

#define THROW_EXCEPTION(where, what) \
  throw std::runtime_error(std::string("[ error from 'libprogress-bar':'") + #where + "'] " + what);

  class ProgressBar {
  private:
    // Total number of tasks
    const unsigned short _taskCount;
    // the description for these tasks
    const std::string _desc;

    // the progress bar color [filled]
    const BarColor _fillColor;
    // the progress bar color [empty]
    const BarColor _emptyColor;
    // the output stream
    std::ostream *_os;

    // inner parameters
    // if thw progress bar is released
    bool _isReleased;
    // used to clear the last progress bar
    unsigned short _lastProgressBarWidth;
    // the start time point
    const double _startTimePoint;
    // the task index to display
    unsigned short _taskIdxToShow;
    // the thread to display progress bar
    std::thread *_showBarThread;

  public:
    /**
     * @brief Construct a new Progress Bar object
     */
    explicit ProgressBar(unsigned short taskCount, std::string desc = "New Task",
                         BarColor fillColor = BarColor::WHITE, BarColor emptyColor = BarColor::NONE,
                         std::ostream &os = std::clog)
        : _taskCount(taskCount), _desc(std::move(desc)), _fillColor(fillColor), _emptyColor(emptyColor),
          _os(&os), _isReleased(false), _startTimePoint(ProgressBar::curTime()), _taskIdxToShow(0) {
      this->_showBarThread = new std::thread(&ProgressBar::run, this);
    }

    /**
     * @brief Destroy the Progress Bar object
     */
    ~ProgressBar() {
      this->release();
      this->_showBarThread->join();
      delete this->_showBarThread;
    }

    /**
     * @brief release the progress bar, which means subsequent messages will be printed below the progress bar
     */
    ProgressBar &release() {
      if (!this->_isReleased) {
        this->_isReleased = true;
        // for last time
        std::lock_guard<std::mutex> lock(pbMtx);
        this->show();
        *this->_os << std::endl;
      }
      return *this;
    }

    /**
     * @brief Set the Task Idx which would been displayed
     */
    ProgressBar &setCurTask(unsigned short idx) {
      this->checkIdx(idx);
      this->_taskIdxToShow = idx;
      return *this;
    }

    /**
     * @brief get the task count for the progress bar
     */
    unsigned short taskCount() const {
      return this->_taskCount;
    }

  protected:
    /**
     * @brief print next progress bar, it's called when a task is done usually
     */
    ProgressBar &show() {
      unsigned short idx = this->_taskIdxToShow;
      idx += 1;
      auto progressBarWidth = static_cast<unsigned short>(ProgressBar::winWidth() * 0.7);

      // task count
      auto taskCountStrSize = std::to_string(this->_taskCount).size();
      auto curCountStrSize = std::to_string(idx).size();
      std::string taskCountStr = std::string(taskCountStrSize - curCountStrSize, ' ') +
                                 std::to_string(idx) + "/" +
                                 std::to_string(this->_taskCount);

      // percent
      double curPercent = static_cast<double>(idx) / this->_taskCount;
      std::stringstream stream;
      stream << std::fixed << std::setprecision(1) << curPercent * 100.0;
      std::string curPercentStr;
      stream >> curPercentStr;
      std::string percentStr = std::string(5 - curPercentStr.size(), ' ') + curPercentStr;

      // time
      stream = std::stringstream();
      stream << std::fixed << std::setprecision(3) << ProgressBar::curTime() - _startTimePoint;
      std::string timeCostStr;
      stream >> timeCostStr;

#if (defined __linux__) && (defined PAR_USE_COLOR)
      // bar
      unsigned short barWidth = progressBarWidth - taskCountStr.size() - percentStr.size() - 9;
      auto fillWidth = static_cast<unsigned short>(barWidth * curPercent);
      unsigned short emptyWidth = barWidth - fillWidth;

      std::string fillStr;
      std::string emptyStr;

      std::string progressBarStr = std::string(barWidth, ' ');

      if (_desc.size() <= barWidth) {
        // fill description string
        unsigned short descStartPos = (barWidth - _desc.size()) / 2;
        unsigned short descEndPos = descStartPos + _desc.size();
        progressBarStr.replace(progressBarStr.begin() + descStartPos, progressBarStr.begin() + descEndPos, _desc);
      }

      fillStr = ProgressBar::colorFlag(this->_fillColor) + "\033[3m" + progressBarStr.substr(0, fillWidth) + ProgressBar::colorFlag(BarColor::NONE);
      emptyStr = ProgressBar::colorFlag(this->_emptyColor) + "\033[3m" + progressBarStr.substr(fillWidth) + ProgressBar::colorFlag(BarColor::NONE);

      progressBarStr = fillStr + emptyStr;

      this->printBar('[' + taskCountStr + "] |" + progressBarStr + "| [" + percentStr + "%]-[" + timeCostStr + "(S)]");
#else
      // bar
      int barWidth = progressBarWidth - taskCountStr.size() - percentStr.size() - _desc.size() - 12;
      // if left char size is small, than add more.
      if (barWidth < 5) {
        progressBarWidth += 5 - barWidth;
        barWidth = 5;
      }
      unsigned short fillWidth = barWidth * curPercent;
      unsigned short emptyWidth = barWidth - fillWidth;

      std::string fillStr = std::string(fillWidth, '@');
      std::string emptyStr = std::string(emptyWidth, '-');
      std::string progressBarStr = fillStr + emptyStr;

      this->printBar('[' + taskCountStr + "]-[" + _desc + "] |" + progressBarStr + "| [" + percentStr + "%]-[" + timeCostStr + "(S)]");
#endif

      _lastProgressBarWidth = progressBarWidth + timeCostStr.size() + 6;
      return *this;
    }

    /**
     * @brief clear the progress bar
     */
    ProgressBar &clear() {
      *this->_os << ("\r" + std::string(_lastProgressBarWidth, ' ') + "\r") << std::flush;
      return *this;
    }

    /**
     * @brief the function to draw progress bar in the new thread
     */
    void run() {
      while (!this->_isReleased) {
        // make standard ostream accessable
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // lock standard ostream
        std::lock_guard<std::mutex> lock(pbMtx);
        this->show();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        this->clear();
      }
    }

    /**
     * @brief check the current task index
     */
    void checkIdx(unsigned short idx) const {
      if (idx >= this->_taskCount) {
        THROW_EXCEPTION(checkIdx, "the index is out of range. ['curTaskIdx' > 'taskCount'].")
      }
    }

    /**
     * @brief print the progress bar to the console
     */
    void printBar(const std::string &barStr) {
      *(this->_os) << barStr << std::flush;
    }

    /**
     * @brief get the console columns
     */
    static unsigned short winWidth() {
#ifdef __linux__
      struct winsize win {};
      ioctl(0, TIOCGWINSZ, &win);
      unsigned short width = win.ws_col;
#else
      CONSOLE_SCREEN_BUFFER_INFO csbi;
      GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
      unsigned short width = csbi.srWindow.Right - csbi.srWindow.Left;
#endif
      return width;
    }

    /**
     * @brief map BarColor to color string
     */
    static std::string colorFlag(int color) {
      return "\033[" + std::to_string(color) + "m";
    }

    static double curTime() {
      auto time = std::chrono::system_clock::now().time_since_epoch();
      return std::chrono::duration_cast<std::chrono::duration<double>>(time).count();
    }
  };
#undef THROW_EXCEPTION

} // namespace ns_pbar

#endif