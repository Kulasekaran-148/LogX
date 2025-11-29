# ======================================
# LogX - Production-grade Logger Library
# ======================================

# ---- Project setup ----
PROJECT     := LogX
LIB_NAME	:= liblogx
BUILD_DIR   := build
SRC_DIR     := src
INC_DIR     := include

# ---- Version ----
MAJOR 	:= $(shell grep -oP '(?<=#define LOGX_MAJOR_VERSION )\d+' $(INC_DIR)/logx/version.h)
MINOR 	:= $(shell grep -oP '(?<=#define LOGX_MINOR_VERSION )\d+' $(INC_DIR)/logx/version.h)
PATCH 	:= $(shell grep -oP '(?<=#define LOGX_PATCH_VERSION )\d+' $(INC_DIR)/logx/version.h)
VERSION := $(MAJOR).$(MINOR).$(PATCH)

# ---- Examples ----
EXAMPLE_DIR 		:= examples
EXAMPLE_SRC_DIR 	:= $(EXAMPLE_DIR)/src
EXAMPLE_BIN_ROOT 	:= $(EXAMPLE_DIR)/binaries
EXAMPLE_LOG_DIR 	:= $(EXAMPLE_DIR)/logs

# Find all example source files recursively
EXAMPLE_SRC := $(shell [ -d $(EXAMPLE_SRC_DIR) ] && find $(EXAMPLE_SRC_DIR) -name "*.c")

# Compute corresponding binary paths:
#   examples/src/basic/foo.c  →  examples/binaries/basic/foo
EXAMPLE_BINS := $(patsubst $(EXAMPLE_SRC_DIR)/%.c,$(EXAMPLE_BIN_ROOT)/%,$(EXAMPLE_SRC))

# ---- Tests ----
TEST_DIR    	:= tests
TEST_SRC_DIR 	:= $(TEST_DIR)/src
TEST_BIN_ROOT 	:= $(TEST_DIR)/binaries
TEST_LOG_DIR 	:= $(TEST_DIR)/logs

# Find all example source files recursively
TEST_SRC := $(shell [ -d $(TEST_SRC_DIR) ] && find $(TEST_SRC_DIR) -name "*.c")

# Compute corresponding binary paths:
#   tests/src/basic/foo.c  →  tests/binaries/basic/foo
TEST_BINS := $(patsubst $(TEST_SRC_DIR)/%.c,$(TEST_BIN_ROOT)/%,$(TEST_SRC))

# ---- Benchmarks ----
BENCHMARKS_DIR    	:= benchmarks
BENCHMARKS_SRC_DIR 	:= $(BENCHMARKS_DIR)/src
BENCHMARKS_BIN_ROOT 	:= $(BENCHMARKS_DIR)/binaries
BENCHMARKS_LOG_DIR 	:= $(BENCHMARKS_DIR)/logs

# Find all example source files recursively
BENCHMARKS_SRC := $(shell [ -d $(BENCHMARKS_SRC_DIR) ] && find $(BENCHMARKS_SRC_DIR) -name "*.c")

# Compute corresponding binary paths:
#   benchmarks/src/basic/foo.c  →  benchmarks/binaries/basic/foo
BENCHMARKS_BINS := $(patsubst $(BENCHMARKS_SRC_DIR)/%.c,$(BENCHMARKS_BIN_ROOT)/%,$(BENCHMARKS_SRC))

# ---- Target ----
TARGET			:= logx
TARGET_STATIC 	:= $(BUILD_DIR)/lib$(TARGET).a
TARGET_SHARED 	:= $(BUILD_DIR)/lib$(TARGET).so
SONAME      	:= lib$(TARGET).so.$(VERSION)

# ---- Documentattion ----
DOXYFILE 	:= Doxyfile
DOCS_DIR 	:= docs
LATEX_DIR 	:= $(DOCS_DIR)/latex

LIBS = -llogx -lyaml -lcjson

# ---- Compiler settings ----
CC          := gcc
AR          := ar
CFLAGS      := -Wall -Wextra -std=c11 -fPIC -pthread -I$(INC_DIR)
ARFLAGS     := rcs
LDFLAGS     := -shared -pthread

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
all: dirs format $(TARGET_STATIC) $(TARGET_SHARED) example test

dirs:
	@mkdir -p $(BUILD_DIR) $(EXAMPLE_LOG_DIR) $(TEST_LOG_DIR)

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

# ---- Examples ----
example: dirs $(TARGET_STATIC) $(TARGET_SHARED) $(EXAMPLE_BINS)
	@echo "✅ Built examples → $(EXAMPLE_BIN_ROOT)"

# Ensure binary directories exist (auto-mkdir)
$(EXAMPLE_BIN_ROOT)/%: $(EXAMPLE_SRC_DIR)/%.c $(TARGET_STATIC) $(TARGET_SHARED)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -L$(BUILD_DIR) $(LIBS) -o $@

# ---- Tests ----
test: dirs $(TARGET_STATIC) $(TARGET_SHARED) $(TEST_BINS)
	@echo "✅ Built tests → $(TEST_BIN_ROOT)"

# Ensure binary directories exist (auto-mkdir)
$(TEST_BIN_ROOT)/%: $(TEST_SRC_DIR)/%.c $(TARGET_STATIC) $(TARGET_SHARED)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -L$(BUILD_DIR) $(LIBS) -o $@

# ---- Benchmarks ----
benchmark: dirs $(TARGET_STATIC) $(TARGET_SHARED) $(BENCHMARKS_BINS)
	@echo "✅ Built benchmarks → $(BENCHMARKS_BIN_ROOT)"

# Ensure binary directories exist (auto-mkdir)
$(BENCHMARKS_BIN_ROOT)/%: $(BENCHMARKS_SRC_DIR)/%.c $(TARGET_STATIC) $(TARGET_SHARED)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -L$(BUILD_DIR) $(LIBS) -o $@

# ---- Cleanup ----
clean:
	@echo "🧹 Cleaning build..."
	@rm -rf $(BUILD_DIR)
	@echo "🧹 Cleaning example binaries..."
	@rm -rf $(EXAMPLE_BIN_ROOT)
	@echo "🧹 Cleaning example log files..."
	@rm -rf $(EXAMPLE_LOG_DIR)
	@echo "🧹 Cleaning test binaries..."
	@rm -rf $(TEST_BIN_ROOT)
	@echo "🧹 Cleaning test log files..."
	@rm -rf $(TEST_LOG_DIR)
	@echo "✅ Clean complete."

# ---- Fresh Build ----
fresh:
	@echo "🧼 Clean build started..."
	@$(MAKE) --no-print-directory clean
	@$(MAKE) --no-print-directory all

# Clean the docs folder
clean_docs:
	@echo "🧹 Cleaning existing docs..."
	@rm -rf $(DOCS_DIR)

# Generate fresh docs
fresh_docs: clean_docs
	@echo "📄 Generating HTML and LaTeX docs..."
	@doxygen $(DOXYFILE)
	@echo "📚 Building refman.pdf..."
	@$(MAKE) -C $(LATEX_DIR)  # Run make in the LaTeX folder
	@echo "✅ Docs generation complete."

# Run Debian package making script
deb: clean
	@./scripts/make_package.sh

# Install the deb package
install: deb
	@echo "📦 Installing Debian package..."
	@sudo apt install --reinstall ./build/$(LIB_NAME)-$(VERSION).deb
	@echo "✅ Installation complete."

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
	@echo "  make example				   - Build examples"
	@echo "  make test                     - Build tests"
	@echo "  make benchmark                - Build benchmarks"
	@echo "  make clean_docs               - Clean the doxygen docs folder"
	@echo "  make fresh_docs               - Clean and build the doxygen docs folder"
	@echo "  make help                     - Show this help message"
	@echo "  make deb                      - Create Debian package"
	@echo "  make install                  - Install the Debian package"
	@echo ""
	@echo "------------------------------------------------------------"
	@echo "Example:"
	@echo "  make BUILD_TYPE=Release clean all"
	@echo ""

.PHONY: all dirs format tidy check example test benchmark clean fresh fresh_docs clean_docs deb install help
