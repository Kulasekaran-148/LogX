<p align="center">
  <img src="https://svg-banners.vercel.app/api?type=glitch&text1=LogX&height=200" width="100%" />
</p>

[![Build Status](https://img.shields.io/github/actions/workflow/status/Kulasekaran-148/LogX/build.yml?branch=main)](https://github.com/Kulasekaran-148/LogX/actions)  
[![License](https://img.shields.io/github/license/Kulasekaran-148/LogX)](LICENSE)  
[![Documentation](https://img.shields.io/badge/docs-Doxygen-blue)](https://<USERNAME>.github.io/<REPO>/)

---

## Table of Contents
- [Introduction](#introduction)
- [Features](#features)
- [Installation](#installation)
- [Changelog](#changelog)
- [Quick Start](#quick-start)
- [Benchmarks](#benchmarks)
- [Contributing](#contributing)
- [License](#license)
- [Contact](#contact)

---

## Introduction

**LogX** is an easy, sophisticated, flexible, user-friendly and robust logging library in C, designed for Linux based systems with high customizability.

---

## Features

- Log Levels: `TRACE`, `DEBUG`, `INFO`, `BANNER`, `WARN`, `ERROR`, `FATAL`
- ANSI Colored logs
  - Auto TTY detection to enable / disable colored logs
- Load logger configuration from .YAML or .JSON file
- Console and file logging
  - Enable/Disable Console logging
  - Enable/Disable File logging
  - Set Console Log Level
  - Set File Log Level
  - Log rotation
    - Based on Size / Date
    - Set number of backups
- Stopwatch timing
- Thread-safe implementation
- Lightweight and minimal dependencies

## Features (coming soon ... )
- Configurable log format
- Asynchronous logging support
- Log tags
- Privacy filtering
- Logging mode switch
- Logfile compression
- Multiple file logging
- Syslog support

---

## Dependencies

- CMake
- libyaml-dev, libcjson-dev (Used by parsing configuration from file)

```bash
sudo apt install cmake libyaml-dev libcjson-dev -y
```

- *NOTE:* 
  - Dependencies need to be installed manually before building from source
  - Dependencies are auto installed when installing from Releases

---

## Installation

### From Releases

- Click [here](https://github.com/Kulasekaran-148/LogX/releases) to download the latest release of LogX
- In the Releases page, download the ` liblogx-x.x.x.deb` file to your PC
- Navigate to the directory where you downloaded the file
- Run the following command to install LogX to your PC:

```bash
sudo apt update && sudo apt install liblogx-x.x.x.deb -y
```

- Once it is installed, refere to [Quick Start](#quick-start) section on how to get started with using LogX

### From Source

```bash
git clone https://github.com/Kulasekaran-148/LogX.git
cd LogX
make install # This will automaitcally trigger a build and install the package
```

- After installation, you can refer to the [Quick Start](#quick-start) example on how to integrate LogX with your project.
- Once that's done, you can use the following command to compile:

```bash
gcc main.c -o main -llogx
```

---

## Generating Doxygen Documentation

This project uses [Doxygen](http://www.doxygen.nl/) to generate API documentation. You can generate the docs locally by following these steps:

1. **Install Doxygen** (and Graphviz for call graphs):

```bash
# Ubuntu / WSL
sudo apt-get update
sudo apt-get install doxygen graphviz

# Optional for Latex documentation
sudo apt-get install texlive-latex-base texlive-latex-extra
```

2. **Ensure the Doxyfile exists** in the project root

The default Doxyfile is configured to generate HTML & LATEX documentation inside docs folder

3. **Run Doxygen**

```bash
doxygen Doxyfile
```

4. **Open HTML documentation**

Once the doxygen generation is complete, you can open `./docs/html/index.html` using any browser.

5. **(Optional) PDF Generation**

Go to `./docs/latex/` and run make to generate the pdf documentation. It will be generated as `refman.pdf`

---

## Changelog
- View [changelog](./CHANGELOG.md)

---

## Quick Start

```C
#include <stdio.h>
#include <logx/logx.h>

int main() {
    // Initialize logger
    logx_t *logger = logx_create(NULL); // passing NULL to use default configuration
    if(!logger)
    {
      fprintf(stderr, "Failed to create logx logger instance\n");
      return -1;
    }

    LOGX_BANNER(logger, "LogX is working");
    LOGX_TRACE(logger, "LogX Trace message");
    LOGX_DEBUG(logger, "LogX Debug message");
    LOGX_INFO(logger, "LogX Info message");
    LOGX_WARN(logger, "LogX warn message");
    LOGX_ERROR(logger, "LogX error message");
    LOGX_FATAL(logger, "LogX fatal message");
    
    // Destroy logger to clean up resources
    logx_destroy(logger);
    return 0;
}
```

- Click [here](./GUIDE.md) to view the full guide on LogX's features, how-to-use with neat little examples.

---

## Benchmarks

| Component            | Details                                                             |
| -------------------- | ------------------------------------------------------------------- |
| **OS**               | Ubuntu 22.04.5 LTS (Jammy)                                          |
| **Kernel**           | 6.8.0-87-generic                                                    |
| **CPU**              | AMD Ryzen 5 3550H with Radeon Vega Mobile Gfx (4 cores / 8 threads) |
| **CPU Architecture** | x86_64 (Supports 32-bit & 64-bit mode)                              |
| **CPU Frequency**    | 1.40 GHz (min) — 2.10 GHz (max), Boost enabled                      |
| **Cache**            | L1: 128 KiB (data), 256 KiB (instruction) • L2: 2 MiB • L3: 4 MiB   |
| **Virtualization**   | AMD-V                                                               |
| **Memory (RAM)**     | 15 GiB total (≈4.0 GiB used, 8.1 GiB free, 10 GiB available)        |
| **Swap**             | 6.6 GiB                                                             |
| **GPU**              | AMD/ATI Picasso/Raven 2 — Radeon Vega Mobile Series                 |
| **NUMA Nodes**       | 1 (CPUs 0–7)                                                        |


The following table shows approximate benchmark results for printing different numbers of log lines in various configurations.  
(All values are indicative and depend on system performance.)

| Sl. No | Console Log | File Log | Num. of Log Lines | LogX Time | Other Logger Time |
|--------|-------------|----------|--------------------|-----------|--------------------|
| 1      | Enabled     | Disabled | 100,000            | ~12s      | ~4s                |


---

## Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository.
2. Create a new branch:
   ```bash
   git checkout -b feature/my-feature
3. Make your changes and commit:
   ```bash
   git commit -m "Add feature"
4. Push to your branch
   ```bash
   git push origin feature/my-feature
5. Open a Pull Request

---

## License

This project is licensed under the **MIT License**. See [LICENSE](https://github.com/Kulasekaran-148/LogX/blob/main/LICENSE) for details.

---

## Contact

<p align="center">
  Developed with ❤️ by <strong>Kulasekaran Sadagopan</strong><br><br>
  <a href="https://github.com/Kulasekaran-148">
    <img src="https://img.shields.io/badge/GitHub-000?logo=github&logoColor=white">
  </a>
  &nbsp;
  <a href="mailto:kulasekaranslrk@gmail.com">
    <img src="https://img.shields.io/badge/Email-666?logo=gmail&logoColor=white">
  </a>
  &nbsp;
  <a href="https://www.linkedin.com/in/kulasekaran148/">
    <img src="https://img.shields.io/badge/LinkedIn-0A66C2?logo=linkedin&logoColor=white">
  </a>
</p>