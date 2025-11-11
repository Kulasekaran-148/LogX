# LogX

[![Build Status](https://img.shields.io/github/actions/workflow/status/Kulasekaran-148/LogX/build.yml?branch=main)](https://github.com/Kulasekaran-148/LogX/actions)  
[![License](https://img.shields.io/github/license/Kulasekaran-148/LogX)](LICENSE)  
[![Documentation](https://img.shields.io/badge/docs-Doxygen-blue)](https://<USERNAME>.github.io/<REPO>/)

---

## Table of Contents
- [Introduction](#introduction)
- [Features](#features)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Contributing](#contributing)
- [License](#license)
- [Contact](#contact)

---

## Introduction

**LogX** is a Sophisticated, flexible, user-friendly and robust C logging library designed for Linux based systems with high customizability.

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
- Thread-safe implementation
- Configurable log format
- Lightweight and minimal dependencies
- Asynchronous logging support (coming soon...)
- Log tags (coming soon...)
- Stopwatch timing (cominng soon...)
- Privacy filtering (coming soon...)
- Logging mode switch (coming soon...)

---

## Dependencies

- CMake >= 3.15
- libyaml-dev, libcjson-dev (Used by parsing configuration from file)

## Installation

### From Source

```bash
git clone https://github.com/Kulasekaran-148/LogX.git
cd LogX
chmod +x make_package
./make_package
sudo apt install ./build/liblogx-1.0.0.deb
```

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

## License

This project is licensed under the **MIT License**. See [LICENSE](https://github.com/Kulasekaran-148/LogX/blob/main/LICENSE) for details.

## Contact

Developed and maintained by **Kulasekaran S**

Github: https://github.com/Kulasekaran-148

Email: kulasekaranslrk@gmail.com

Linkedin: https://www.linkedin.com/in/kulasekaran148/