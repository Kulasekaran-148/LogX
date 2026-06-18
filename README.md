<p align="center">
  <img src="https://svg-banners.vercel.app/api?type=glitch&text1=LogX&height=200" width="100%" />
</p>

[![Build Status](https://img.shields.io/github/actions/workflow/status/Kulasekaran-148/LogX/build.yml?branch=main)](https://github.com/Kulasekaran-148/LogX/actions)  
[![License](https://img.shields.io/github/license/Kulasekaran-148/LogX)](LICENSE)  
[![Documentation](https://img.shields.io/badge/docs-Doxygen-blue)](https://kulasekaran-148.github.io/LogX/)

---

## Table of Contents
- [Introduction](#introduction)
- [Features](#features)
- [Installation](#installation)
- [Generating Docs](#generating-doxygen-documentation)
- [Changelog](#changelog)
- [Quick Start](#quick-start)
- [Contributing](#contributing)
- [License](#license)
- [Contact](#contact)

---

## Introduction

**LogX** is a highly-customizable, thread-safe logging library in C, built for Linux systems.

---

## Features

- Log Levels: `TRACE`, `DEBUG`, `INFO`, `BANNER`, `WARN`, `ERROR`, `FATAL`
- ANSI Colored logs
  - Auto TTY detection to enable / disable colored logs
- Load logger configuration from `.YAML` or `.JSON` file
- Console and file logging
  - Enable/Disable Console logging
  - Enable/Disable File logging
  - Set Console Log Level
  - Set File Log Level
  - Log rotation
    - Based on Size / Date
    - Set number of backups
  - Automatic log directory creation — intermediate directories for the log path are created if they do not exist
- Customizable timestamp format — `LOCAL`, `UTC`, `EPOCH_S`, `EPOCH_MS`, `EPOCH_US`, `ISO8601`, `RFC2822`
- Log rate limiting — throttle noisy log sites to at most once every N seconds
- Stopwatch timing
- Thread-safe implementation
- Consistent error code returns on all public APIs
- Lightweight and minimal dependencies

<details>
<summary>Planned Features</summary>

- Configurable log format
- Asynchronous logging support
- Log tags
- Privacy filtering
- Logging mode switch (dev, prod)
- Logfile compression
- Multiple file logging
- Syslog support
- Logger configuration from INI
- Persisting logger configuration

</details>

---

## Installation

### Prerequisites
Ensure the following are installed before proceeding:
- `gcc` (or any C compiler)
- `make`
- `cmake`

```bash
sudo apt install build-essential cmake -y
```

### From Releases
- Click [here](https://github.com/Kulasekaran-148/LogX/releases) to download the latest release of LogX
- In the Releases page, download the `liblogx-<VERSION>.deb` file to your PC
- Navigate to the directory where you downloaded the file
- Run the following command to install LogX:

```bash
sudo apt install ./liblogx-<VERSION>.deb -y
```

- Verify the installation:

```bash
dpkg -l | grep logx
```

- Once installed, refer to the [Quick Start](#quick-start) section to get started.

### From Source

```bash
git clone --recurse-submodules https://github.com/Kulasekaran-148/LogX.git # why --recurse-submodules ? As repo contains git submodules

# If you already cloned without --recurse-submodules, run:
git submodule update --init --recursive

# Navigate to repository root directory
cd LogX

# Build the project
make

# Install LogX (uses apt to automatically resolve and install dependencies)
make install
```

- Verify the installation:

```bash
dpkg -l | grep logx
```

### Compiling Your Project with LogX

```bash
gcc main.c -o main -llogx
```

### Uninstalling

```bash
sudo apt remove liblogx
```

Next, check out the [Quick Start](#quick-start) section to get started.

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

The following examples shows how to use logx in your code

main.c:

```C
#include <stdio.h>
#include <logx.h> // <-- Include logx header

int main() {
    logx_t *logger = NULL;

    // Initialize logger — pass NULL to use default/auto-detected configuration
    if (logx_create(NULL, &logger) != LOGX_ERR_SUCCESS)
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

Compile with:
```bash
gcc main.c -o main -llogx
```

Click [here](./GUIDE.md) to view the full guide on LogX's features, how-to-use with perfectly curated examples.

---

## Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository.
2. Create a new branch:
```bash
   git checkout -b feature/my-feature
```
3. Make your changes and commit:
```bash
   git commit -m "Add feature"
```
4. Push to your branch:
```bash
   git push origin feature/my-feature
```
5. Open a Pull Request.

### Guidelines
- Follow the existing code style and formatting.
- Use clear, descriptive commit messages (e.g. `Fix log rotation bug` not `fix stuff`).
- Keep PRs focused — one feature or fix per PR.
- If adding a new feature, update the documentation accordingly.
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