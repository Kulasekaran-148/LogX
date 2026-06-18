## 2.0.0 - Jun 18, 2026

- feature(s):
    - Customizable timestamp format — choose from `LOCAL`, `UTC`, `EPOCH_S`, `EPOCH_MS`, `EPOCH_US`, `ISO8601`, or `RFC2822` via config or runtime APIs
    - Log rate limiting — new `LOGX_*_FREQ` macros throttle repeated log calls to at most once every N seconds
    - Automatic log directory creation — `logx_create` now creates intermediate directories for the log file path automatically
    - All public APIs now return `logx_errorcodes_t` for consistent error handling, including `logx_create` and `logx_destroy`
    - `logx_create` signature changed to output-parameter style: `logx_errorcodes_t logx_create(const logx_cfg_t *cfg, logx_t **out_logger)`

- Breaking changes:
    - `logx_create` no longer returns `logx_t *`; callers must pass `&logger` as the second argument
    - `logx_destroy` now returns `logx_errorcodes_t` instead of `void`

- Documentation:
    - Doxygen comments added to all public and internal functions
    - GUIDE.md updated with new examples for timestamp format and rate limiting

## 1.2.1 - Dec 1, 2025

- feature(s)
    - Add User API to print a value as a binary string
    - Add User API to change rotatation interval days
    - Add benchmark
    - Optimization in logx_log - turned off mutex lock - unlock !

- Documentation
    - Update reg. newly added APIs

## 1.0.2 - Nov 28, 2025

- Bugfix(es): 
    - Fixed issue with LOGX_TIMER_AUTO not timing in cases of early return from functions

## 1.0.1 - Nov 23, 2025

- Banner messages will use [BNR] instead of [INF]

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

## 1.0.0 - Nov 23, 2025

- Initial public release



