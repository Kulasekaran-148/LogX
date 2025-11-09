# ======================================
# LogX - Production-grade Logger Library
# ======================================

# ---- Project setup ----
PROJECT     := LogX
BUILD_DIR   := build
SRC_DIR     := src
INC_DIR     := include

# Extract version numbers from version.h
MAJOR := $(shell grep -oP '(?<=#define LOGX_MAJOR_VERSION )\d+' $(INC_DIR)/logx/version.h)
MINOR := $(shell grep -oP '(?<=#define LOGX_MINOR_VERSION )\d+' $(INC_DIR)/logx/version.h)
PATCH := $(shell grep -oP '(?<=#define LOGX_PATCH_VERSION )\d+' $(INC_DIR)/logx/version.h)
VERSION := $(MAJOR).$(MINOR).$(PATCH)

TARGET			:= logx
TARGET_STATIC := $(BUILD_DIR)/lib$(TARGET).a
TARGET_SHARED := $(BUILD_DIR)/lib$(TARGET).so
SONAME      := lib$(TARGET).so.$(VERSION)

# ---- Compiler settings ----
CC          := gcc
AR          := ar
CFLAGS      := -Wall -Wextra -std=c11 -fPIC -I$(INC_DIR)
ARFLAGS     := rcs
LDFLAGS     := -shared

# ---- Build type configuration ----
DEBUG_FLAGS   := -g -O0 -DDEBUG
RELEASE_FLAGS := -O3 -DNDEBUG
BUILD_TYPE   ?= Debug

ifeq ($(BUILD_TYPE),Release)
  CFLAGS += $(RELEASE_FLAGS)
else
  CFLAGS += $(DEBUG_FLAGS)
endif

# ---- Tools ----
FORMATTER := clang-format
TIDY      := clang-tidy

# ---- Sources and objects ----
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

# ---- Default rule ----
all: dirs $(TARGET_STATIC) $(TARGET_SHARED)

dirs:
	@mkdir -p $(BUILD_DIR)

# ---- Compilation ----
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "🧩 Compiling $< ..."
	@$(CC) $(CFLAGS) -c $< -o $@

# ---- Static library ----
$(TARGET_STATIC): $(OBJ_FILES)
	@echo "📦 Creating static library: $@ ..."
	@$(AR) $(ARFLAGS) $@ $^
	@echo "✅ Static library built: $@"

# ---- Shared library with versioning ----
$(TARGET_SHARED): $(OBJ_FILES)
	@echo "⚙️  Creating versioned shared library..."
	@$(CC) $(CFLAGS) $(LDFLAGS) -Wl,-soname,$(SONAME) -o $(BUILD_DIR)/liblogx.so.$(VERSION) $^
	@ln -sf liblogx.so.$(VERSION) $(SONAME)
	@ln -sf $(SONAME) $(TARGET_SHARED)
	@echo "✅ Built: liblogx.so.$(VERSION)"
	
# ---- Formatting & Linting ----
format:
	@echo "🎨 Running clang-format..."
	@find $(SRC_DIR) $(INC_DIR) -name "*.[ch]" | xargs $(FORMATTER) -i
	@echo "✅ Formatting done."

tidy:
	@echo "🔍 Running clang-tidy..."
	@$(TIDY) $(SRC_FILES) -- -I$(INC_DIR)
	@echo "✅ Clang-tidy check complete."

check: format tidy

# ---- Testing ----
test: all
	@echo "🧪 Building test..."
	@$(CC) $(CFLAGS) tests/test_logx.c -L. -llogx -o $(BUILD_DIR)/test_logx
	@echo "▶️  Running test..."
	@LD_LIBRARY_PATH=. ./$(BUILD_DIR)/test_logx

# ---- Installation ----
install: all
	@echo "📦 Installing LogX v$(VERSION)..."
	@sudo mkdir -p /usr/local/include/logx /usr/local/lib/pkgconfig
	@sudo cp $(INC_DIR)/*.h /usr/local/include/logx/

	# Install libraries
	@sudo cp liblogx.so.$(VERSION) /usr/local/lib/
	@cd /usr/local/lib && sudo ln -sf liblogx.so.$(VERSION) $(SONAME)
	@cd /usr/local/lib && sudo ln -sf $(SONAME) liblogx.so
	@sudo cp $(TARGET_STATIC) /usr/local/lib/

	# Install pkg-config file
	@echo "prefix=/usr/local" | sudo tee /usr/local/lib/pkgconfig/logx.pc > /dev/null
	@echo "exec_prefix=\$${prefix}" | sudo tee -a /usr/local/lib/pkgconfig/logx.pc > /dev/null
	@echo "includedir=\$${prefix}/include" | sudo tee -a /usr/local/lib/pkgconfig/logx.pc > /dev/null
	@echo "libdir=\$${prefix}/lib" | sudo tee -a /usr/local/lib/pkgconfig/logx.pc > /dev/null
	@echo "" | sudo tee -a /usr/local/lib/pkgconfig/logx.pc > /dev/null
	@echo "Name: logx" | sudo tee -a /usr/local/lib/pkgconfig/logx.pc > /dev/null
	@echo "Description: LogX - Production-grade logging library" | sudo tee -a /usr/local/lib/pkgconfig/logx.pc > /dev/null
	@echo "Version: $(VERSION)" | sudo tee -a /usr/local/lib/pkgconfig/logx.pc > /dev/null
	@echo "Cflags: -I\$${includedir}/logx" | sudo tee -a /usr/local/lib/pkgconfig/logx.pc > /dev/null
	@echo "Libs: -L\$${libdir} -llogx" | sudo tee -a /usr/local/lib/pkgconfig/logx.pc > /dev/null

	@sudo ldconfig
	@echo "✅ Installed LogX $(VERSION) to /usr/local"


# ---- Cleanup ----
clean:
	@echo "🧹 Cleaning build..."
	@rm -rf $(BUILD_DIR) $(TARGET_STATIC) $(TARGET_SHARED)
	@echo "✅ Clean complete."

# ---- Fresh Build ----
fresh: clean all
	@echo "🧼 Clean build started..."

# ---- Help ----
help:
	@echo ""
	@echo "🧰  $(PROJECT) - v$(VERSION)"
	@echo "------------------------------------------------------------"
	@echo "Available targets:"
	@echo ""
	@echo "  make                          - Build both static and shared libraries"
	@echo "  make BUILD_TYPE=Release       - Build in Release mode (optimized)"
	@echo "  make clean                    - Clean all build artifacts"
	@echo "  make fresh                    - Clean and build"
	@echo "  make format                   - Run clang-format on all source and headers"
	@echo "  make tidy                     - Run clang-tidy static analysis"
	@echo "  make check                    - Run both formatting and lint checks"
	@echo "  make test                     - Build and run example test"
	@echo "  sudo make install              - Install library system-wide (headers, libs, pkg-config)"
	@echo "  make help                     - Show this help message"
	@echo ""
	@echo "Directories:"
	@echo "  Source:     $(SRC_DIR)"
	@echo "  Includes:   $(INC_DIR)"
	@echo "  Build out:  $(BUILD_DIR)"
	@echo ""
	@echo "------------------------------------------------------------"
	@echo "Example:"
	@echo "  make BUILD_TYPE=Release clean all"
	@echo ""

.PHONY: all dirs format tidy check test install clean fresh
