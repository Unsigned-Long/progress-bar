# Progress Bar

**Author: shlChen**

**E-Mail: 3079625093@qq.com**

[TOC]

## 1. Overview

Console is more robust than GUI. How to output effective information on the console is a knowledge. This library provides the function of outputting progress bar information on the console under Linux, which is simple and easy to use.

## 2. Usage

code:

<img src="./output/code.png">

output:

<img src="./output/charset.png">

<img src="./output/default.png">

<img src="./output/green_light_blue.png">

<img src="./output/red_light_yellow.png">

## 3. Apis

the macro below controls the color, if this macro defined, the progress bar will be printed with color.

```cpp
#define PAR_USE_COLOR
```

```cpp
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
```

```cpp
/**
 * @brief Construct a new Progress Bar object
 */
explicit ProgressBar(unsigned short taskCount, BarColor fillColor = BarColor::WHITE, BarColor emptyColor = BarColor::NONE, std::ostream &os = std::clog);
```

```cpp
/**
 * @brief get the task count for the progress bar
 */
std::size_t taskCount() const;
```

```cpp
/**
 * @brief Set the Task Idx which would been displayed
 */
ProgressBar &setCurTask(unsigned short idx, const std::string &desc);
```

```cpp
/**
 * @brief print next progress bar, it's called when a task is done usually
 */
ProgressBar &unlock();
```

```cpp
/**
 * @brief clear the progress bar
 */
ProgressBar &lock();
```

```cpp
/**
 * @brief update the progress bar
 *
 * @param idx the index of the task
 * @param fun the function to
 * @return ProgressBar&
 */
ProgressBar &lockAndUnlock(const std::function<void()> &fun);
```

```cpp
/**
 * @brief release the progress bar, which means subsequent messages will be printed below the progress bar
 */
ProgressBar &release();
```

