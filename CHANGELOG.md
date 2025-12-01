## 1.0.0 - Nov 23, 2025

- Initial public release

## 1.0.1 - Nov 23, 2025

- Banner messages will use [BNR] instead of [INF]

## 1.0.2 - Nov 28, 2025

- Bugfix(es): 
    - Fixed issue with LOGX_TIMER_AUTO not timing in cases of early return from functions

## 1.1.0 - Nov 29, 2025

- feature(s):
    - Add separate User APIs to enable/disable console logging
    - Add separate User APIs to enable/disable file logging
    - Add separate User APIs to enable/disable colored logging
    - Add separate User APIs to enable/disable tty detection
    - Add new API to enable/disable the print config
    - Add new API to set the max log file size during runtime
    - Added new API to set the number of logfile backups during runtime

- Bugfix(es): 
    - LogX will not log when log level is LOGX_LEVEL_OFF

- Documentation:
    - Add LogX API section

- Installation:
    - Added `deb` and `install` targets to Makefile. Package creation & installation is now easier than before with simple make calls:
    ```bash
    make deb # creates the debian package
    make install # creates & installs the debian package
    ```

## 1.2.1 - Dec 1, 2025

- feature(s)
    - Add User API to print a value as a binary string
    - Add User API to change rotatation interval days
    - Add benchmark
    - Optimization in logx_log - turned off mutex lock - unlock !

- Documentation
    - Update reg. newly added APIs

