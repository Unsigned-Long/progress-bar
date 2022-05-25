#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H

#include <algorithm>
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#ifdef __linux__
#include <sys/ioctl.h>
#else
#include <windows.h>
#endif

namespace ns_pbar {

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

    const double _startTimePoint;

  public:
    /**
     * @brief Construct a new Progress Bar object
     */
    ProgressBar(unsigned short taskCount, const std::string &desc = "New Task",
                BarColor fillColor = BarColor::WHITE, BarColor emptyColor = BarColor::NONE,
                std::ostream &os = std::clog)
        : _taskCount(taskCount), _desc(desc), _fillColor(fillColor), _emptyColor(emptyColor),
          _os(&os), _isReleased(false), _startTimePoint(ProgressBar::curTime()) {}

    /**
     * @brief Destroy the Progress Bar object
     */
    ~ProgressBar() {
      this->release();
    }

    /**
     * @brief release the progress bar, which means subsequent messages will be printed below the progress bar
     */
    ProgressBar &release() {
      if (!this->_isReleased) {
        *this->_os << std::endl;
        this->_isReleased = true;
      }
      return *this;
    }

    /**
     * @brief update the progress bar
     *
     * @param idx the index of the task
     * @param fun the function to
     * @return ProgressBar&
     */
    ProgressBar &update(unsigned short idx, const std::function<void()> &fun) {
      this->clear();
      fun();
      this->show(idx);
      return *this;
    }

    /**
     * @brief get the task count for the progress bar
     */
    unsigned short taskCount() const {
      return this->_taskCount;
    }

    /**
     * @brief print next progress bar, it's called when a task is done usually
     */
    ProgressBar &show(unsigned short idx) {
      if (this->_isReleased) {
        return *this;
      }
      idx += 1;
      this->checkIdx(idx);
      unsigned short progressBarWidth = ProgressBar::winWidth() * 0.7;

      // task count
      auto taskCountStrSize = std::to_string(this->_taskCount).size();
      auto curCountStrSize = std::to_string(idx).size();
      std::string taskCountStr = '[' + std::string(taskCountStrSize - curCountStrSize, ' ') +
                                 std::to_string(idx) + "/" +
                                 std::to_string(this->_taskCount) + ']';

      // percent
      double curPercent = static_cast<double>(idx) / this->_taskCount;
      std::stringstream stream;
      stream << std::fixed << std::setprecision(1) << curPercent * 100.0;
      std::string curPercentStr;
      stream >> curPercentStr;
      std::string percentStr = '[' + std::string(5 - curPercentStr.size(), ' ') + curPercentStr + "%]";

      // time
      stream = std::stringstream();
      stream << std::fixed << std::setprecision(3) << ProgressBar::curTime() - _startTimePoint;
      std::string timeCostStr;
      stream >> timeCostStr;
      timeCostStr = "-[" + timeCostStr + "(S)]";

#if (defined __linux__) && (defined PROGRESS_COLOR_BAR)
      // bar
      unsigned short barWidth = progressBarWidth - percentStr.size() - 4 - taskCountStrSize * 2 - 3;
      unsigned short fillWidth = barWidth * curPercent;
      unsigned short emptyWidth = barWidth - fillWidth;

      std::string fillStr = std::string(fillWidth, ' ');
      std::string emptyStr = std::string(emptyWidth, ' ');

      if (_desc.size() <= barWidth) {

        unsigned short descStartPos = (barWidth - _desc.size()) / 2;
        unsigned short descEndPos = descStartPos + _desc.size();

        // replace for the '_desc'
        if (descStartPos > fillStr.size()) {
          // '_desc' is all in 'emptyStr'
          emptyStr.replace(emptyStr.begin() + descStartPos - fillStr.size(), emptyStr.begin() + descEndPos - fillStr.size(), _desc);
        } else if (descEndPos < fillStr.size()) {
          // '_desc' is all in 'fillStr'
          fillStr.replace(fillStr.begin() + descStartPos, fillStr.begin() + descEndPos, _desc);
        } else {
          // '_desc' is in 'fillStr' and 'emptyStr'
          fillStr.replace(fillStr.begin() + descStartPos, fillStr.end(),
                          std::string(_desc.begin(), _desc.begin() + fillStr.size() - descStartPos));
          emptyStr.replace(emptyStr.begin(), emptyStr.begin() + descEndPos - fillStr.size(),
                           std::string(_desc.begin() + fillStr.size() - descStartPos, _desc.end()));
        }
      }

      std::string progressBarStr = ProgressBar::colorFlag(this->_fillColor) + "\033[3m" + fillStr + ProgressBar::colorFlag(BarColor::NONE) +
                                   ProgressBar::colorFlag(this->_emptyColor) + "\033[3m" + emptyStr + ProgressBar::colorFlag(BarColor::NONE);
      this->printBar(taskCountStr + " |" + progressBarStr + "| " + percentStr + timeCostStr);
#else
      // bar
      int barWidth = progressBarWidth - percentStr.size() - 4 - taskCountStrSize * 2 - 3 - _desc.size() - 3;
      // if left char size is small, than add more.
      if (barWidth < 5)
        barWidth = 5;
      unsigned short fillWidth = barWidth * curPercent;
      unsigned short emptyWidth = barWidth - fillWidth;

      std::string fillStr = std::string(fillWidth, '@');
      std::string emptyStr = std::string(emptyWidth, '-');
      std::string progressBarStr = fillStr + emptyStr;
      std::string descStr = "-[" + _desc + ']';

      this->printBar(taskCountStr + descStr + " |" + progressBarStr + "| " + percentStr + timeCostStr);
#endif

      _lastProgressBarWidth = progressBarWidth + timeCostStr.size();
      return *this;
    }

    /**
     * @brief clear the progress bar
     */
    ProgressBar &clear() {
      if (this->_isReleased) {
        return *this;
      }
      *this->_os << (std::string(_lastProgressBarWidth, ' ') + "\r") << std::flush;
      return *this;
    }

  protected:
    /**
     * @brief check the current task index
     */
    void checkIdx(unsigned short idx) const {
      if (idx > this->_taskCount) {
        THROW_EXCEPTION(checkIdx, "the index is out of range. ['curTaskIdx' > 'taskCount'].");
      }
      return;
    }

    /**
     * @brief print the progress bar to the console
     *
     * @param barStr
     */
    void printBar(const std::string &barStr) {
      *(this->_os) << barStr << '\r' << std::flush;
      return;
    }

    /**
     * @brief get the console columns
     */
    static unsigned short winWidth() {
#ifdef __linux__
      struct winsize win;
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