# ======================================
# LogX - Production-grade Logger Library
# ======================================

# ---- Project setup ----
PROJECT     := LogX
BUILD_DIR   := build
SRC_DIR     := src
INC_DIR     := include

EXAMPLE_DIR := examples
EXAMPLE_SRC := $(wildcard $(EXAMPLE_DIR)/*.c)
EXAMPLE_BIN_DIR := examples/binaries
EXAMPLE_LOG_DIR := examples/logs
EXAMPLE_BINS := $(patsubst examples/%.c,$(EXAMPLE_BIN_DIR)/%,$(EXAMPLE_SRC))

TEST_DIR    := tests
TEST_SRC := $(wildcard $(TEST_DIR)/*.c)
TEST_BIN_DIR := tests/binaries
TEST_LOG_DIR := tests/logs
TEST_BINS := $(patsubst tests/%.c,$(TEST_BIN_DIR)/%,$(TEST_SRC))

# Extract version numbers from version.h
MAJOR := $(shell grep -oP '(?<=#define LOGX_MAJOR_VERSION )\d+' $(INC_DIR)/logx/version.h)
MINOR := $(shell grep -oP '(?<=#define LOGX_MINOR_VERSION )\d+' $(INC_DIR)/logx/version.h)
PATCH := $(shell grep -oP '(?<=#define LOGX_PATCH_VERSION )\d+' $(INC_DIR)/logx/version.h)
VERSION := $(MAJOR).$(MINOR).$(PATCH)

TARGET			:= logx
TARGET_STATIC := $(BUILD_DIR)/lib$(TARGET).a
TARGET_SHARED := $(BUILD_DIR)/lib$(TARGET).so
SONAME      := lib$(TARGET).so.$(VERSION)

LIBS = -llogx -lyaml -lcjson

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
all: dirs $(TARGET_STATIC) $(TARGET_SHARED) example test

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
	@$(CC) $(CFLAGS) $(LDFLAGS) -Wl,-soname,$(SONAME) -o $(BUILD_DIR)/lib$(TARGET).so.$(VERSION) $^
	@cd $(BUILD_DIR) && ln -sf lib$(TARGET).so.$(VERSION) lib$(TARGET).so.$(MAJOR)
	@cd $(BUILD_DIR) && ln -sf lib$(TARGET).so.$(MAJOR) lib$(TARGET).so
	@echo "✅ Built: lib$(TARGET).so.$(VERSION)"
	
# ---- Formatting & Linting ----
format:
	@echo "🎨 Running clang-format..."
	@find $(SRC_DIR) $(INC_DIR) -name "*.[ch]" | xargs $(FORMATTER) -i
	@echo "✅ Formatting done."

tidy:
	@echo "🔍 Running clang-tidy..."
	@$(TIDY) $(SRC_FILES) -- -I$(INC_DIR)
	@echo "✅ Clang-tidy check complete."

# ---- Testing ----
test: $(TEST_BIN_DIR) $(TEST_BINS)
	@echo "🧪 Building test..."
	@echo "✅ Built tests → $(TEST_BIN_DIR)"

$(TEST_BIN_DIR):
	mkdir -p $(TEST_BIN_DIR)

$(TEST_BIN_DIR)/%: tests/%.c $(STATIC_LIB) $(SHARED_LIB)
	$(CC) $(CFLAGS) $< -L$(BUILD_DIR) $(LIBS) -o $@

# ---- Examples ----
example: $(EXAMPLE_BIN_DIR) $(EXAMPLE_BINS)
	@echo "🗺️  Building examples..."
	@echo "✅ Built examples → $(EXAMPLE_BIN_DIR)"

$(EXAMPLE_BIN_DIR):
	mkdir -p $(EXAMPLE_BIN_DIR)

# Pattern rule for building example binaries
$(EXAMPLE_BIN_DIR)/%: examples/%.c $(STATIC_LIB) $(SHARED_LIB)
	$(CC) $(CFLAGS) $< -L$(BUILD_DIR) $(LIBS) -o $@


# ---- Cleanup ----
clean:
	@echo "🧹 Cleaning build..."
	@rm -rf $(BUILD_DIR)
	@rm -rf $(EXAMPLE_BIN_DIR)
	@rm -rf $(EXAMPLE_LOG_DIR)
	@rm -rf $(TEST_BIN_DIR)
	@rm -rf $(TEST_LOG_DIR)
	@echo "✅ Clean complete."

# ---- Fresh Build ----
fresh:
	@echo "🧼 Clean build started..."
	@$(MAKE) --no-print-directory clean
	@$(MAKE) --no-print-directory all



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
	@echo "  make example									 - Build examples"
	@echo "  make test                     - Build tests"
	@echo "  make help                     - Show this help message"
	@echo ""
	@echo "------------------------------------------------------------"
	@echo "Example:"
	@echo "  make BUILD_TYPE=Release clean all"
	@echo ""

.PHONY: all dirs format tidy check example test install clean fresh
