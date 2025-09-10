# LambdaPlotter

[![Build Status](https://github.com/ender878/LambdaPlotter/actions/workflows/build_and_test.yml/badge.svg)](https://github.com/ender878/LambdaPlotter/actions/workflows/build_and_test.yml)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Latest Release](https://img.shields.io/github/v/release/ender878/LambdaPlotter)](https://github.com/ender878/LambdaPlotter/releases)

![LambdaPlotter Screenshot](./.github/imgs/lp.gif)

**LambdaPlotter** is a fast, modern, and lightweight serial plotter for visualizing any type of streaming data in real time.

## Table of Contents

- [Features](#features)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Building from Source](#building-from-source)
- [Contributing](#contributing)
- [Roadmap](#roadmap)

## Features

- **Lightweight and Fast:** Built with modern C++ for high performance and low resource usage.
- **Cross-Platform:** Runs natively on Linux, macOS, and Windows.
- **Advanced Serial Configuration:** Configure low-level serial port settings, like *parity*, *stop bits*, and more.
- **Custom Data Formatting:** A powerful formatting tool lets you parse virtually any data stream by defining frame endings and value separators.
- **Channel-Based Plotting:** Plot multiple variables simultaneously. Each channel can be customized with its own name, color, scale, and offset.
- **Interactive Plots:** Powered by [ImPlot](https://github.com/epezent/implot), plots can be panned, zoomed, and inspected in real-time.
- **Data Export:** Save the captured plot data to a **.csv** file for analysis in other tools.

## Getting Started

The easiest way to use **LambdaPlotter** is to download the latest pre-built release for your operating system:

<p align="center">
  <a href="https://github.com/ender878/LambdaPlotter/releases">
    <img src="https://img.shields.io/badge/Download_Latest_Release-4078c0?style=for-the-badge&logo=github" alt="Download Latest Release"/>
  </a>
</p>

## Usage

1. **Connect your device** (e.g., Arduino, ESP32) that is sending serial data.
2. **Launch LambdaPlotter.**
3. **Configure the Serial Port:** Select the correct **Port** and set the **Baud Rate** to match your device.
4. **Define the Data Format:** This is the most important step. You must tell LambdaPlotter how your data is structured.
    - **Frame End:** The character(s) that mark the end of a complete data packet (e.g., `\n` for a newline).
    - **Channel Separator:** The character that separates different data values (e.g., `,` or ` `).
    - **Named Channels:** Check this if your data includes names for each value.
      - **Name Separator:** The character(s) that separates names from values (e.g., `:`)
5. **Click the 'Play' button** to begin plotting!

### Example Data Formats

#### **1. Simple (Unnamed) Values**

If your device sends comma-separated values ending with a newline: `12.5,50.2,1012\n`

- **Frame End:** `\n`
- **Channel Separator:** `,`
- **Named Channels:** Unchecked

#### **2. Named Values**

If your device sends named values: `temp:25.5,humidity:45.8,pressure:1012.5\n`

- **Frame End:** `\n`
- **Channel Separator:** `,`
- **Named Channels:** Checked
- **Name/Value Separator:** `:`

## Building from source

<details>
<summary>Click to expand instructions for building from source</summary>

### Prerequisites

- A **C++20** compatible compiler (Clang, MSVC, or GCC)
- **CMake** (3.16 or newer)
- **Python** (for installing **Conan**)

### 1. Install conan

This project uses **conan** as the package manager for managing all the major dependencies. You can install it using **pip**:

```bash
pip install conan
```

### 2. Clone the repository

```bash
git clone https://github.com/ender878/lambda_plotter.git

cd lambda_plotter
```

### 3. Install conan dependencies

Once inside the project's folder, install all the dependencies using conan:

```bash
# On Linux/macOS, it's best to specify the compiler
CC=clang CXX=clang++ conan install . --build=missing -s build_type=Release

# On Windows with Visual Studio
conan install . --build=missing -s build_type=Release
```

### 4. Build with CMake

Once all the dependencies are installed, we can finally build the project:

```bash
# For Release builds
cmake --preset conan-release
cmake --build --preset conan-release

# For Debug builds
cmake --preset conan-debug
cmake --build --preset conan-debug
```

> [!NOTE]
>
> On Windows MSVC builds, you may need to specify `conan-default` as the preset for release builds. On Linux/macOS it is recommended to specify the compilers: `-DCMAKE_C_COMPILER=<CC> -DCMAKE_CXX_COMPILER=<CXX>`

The final executable will be located in the `build/Release` or `build/Debug` directory.

</details>

## Contributing

Contributions are welcome! Please feel free to submit a pull request or open an issue for bugs, feature requests, or suggestions.

## Roadmap

Here are some of the planned features:

- [ ] Binary data formatting
- [ ] Integrated serial monitor
- [ ] UDP support
- [ ] Websocket support
- [ ] MQTT support

---

Licensed under the [MIT License](LICENSE).
