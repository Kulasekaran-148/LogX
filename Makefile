# ======================================
# LogX - Production-grade Logger Library
# ======================================

# ---- Project setup ----
BUILD_DIR   := build
CLANG_FORMAT := clang-format-14
LIB_NAME := liblogx
RELEASES_DIR := releases
SRCS 	:= logx examples benchmarks tests
FORMAT_FILES := $(shell find $(SRCS) -type f \( -name "*.c" -o -name "*.h" \))


# ---- Parse version from version.h ----
MAJOR 	:= $(shell grep -oP '(?<=#define LOGX_MAJOR_VERSION )\d+' ./logx/version.h)
MINOR 	:= $(shell grep -oP '(?<=#define LOGX_MINOR_VERSION )\d+' ./logx/version.h)
PATCH 	:= $(shell grep -oP '(?<=#define LOGX_PATCH_VERSION )\d+' ./logx/version.h)
VERSION := $(MAJOR).$(MINOR).$(PATCH)

RELEASES_EXAMPLES_DIR := $(RELEASES_DIR)/$(VERSION)/examples
RELEASES_BENCHMARKS_DIR := $(RELEASES_DIR)/$(VERSION)/benchmarks

# ---- Documentattion ----
DOXYFILE 	:= Doxyfile
DOCS_DIR 	:= docs
LATEX_DIR 	:= $(DOCS_DIR)/latex

# ─────────────────────────────────────────────────────────────
# Helper: copy all executables found under a build subtree
# into a destination directory, printing each one installed.
#   Usage: $(call install_exes_from,build_subdir,dest_dir)
# ─────────────────────────────────────────────────────────────
define install_exes_from
	@find $(1) -type f -perm /111 2>/dev/null | sort | while read exe; do \
		name=$$(basename "$$exe"); \
		install -m 755 "$$exe" "$(2)/$$name" && echo "  ✔ $$name"; \
	done
endef

# ==============================
#default
# ==============================
.PHONY: all
all: clean build deb ## Build project

# ==============================
# Build step
# ==============================
.PHONY: build
build: ## Build all targets
	echo "📦 Configuring project with CMake..."
	cmake -S . -B $(BUILD_DIR)
	echo "📦 Building project..."
	cmake --build $(BUILD_DIR)

# ==============================
# Verbose build
# ==============================
.PHONY: verbose
verbose: ## Build with verbose output
	cmake --build $(BUILD_DIR) --verbose

# ==============================
# Build specific target
# Usage: make target TARGET=master_app
# ==============================
.PHONY: target
target: ## Build specific target only (make target TARGET=logx)
	cmake --build $(BUILD_DIR) --target $(TARGET)

# ==============================
# Clean build directory
# ==============================
.PHONY: clean
clean: clean_docs ## Clean and rebuild everything
	rm -rf $(BUILD_DIR)
	rm -rf $(RELEASES_DIR)

# ==============================
# Rebuild from scratch
# ==============================
.PHONY: rebuild
rebuild: clean build ## Clean and rebuild everything

# ==============================
# Formatting
# ==============================
.PHONY: format
format: ## Run clang-format on apps/ and lib/
	@echo "Checking $(CLANG_FORMAT)..."
	@if ! command -v $(CLANG_FORMAT) >/dev/null 2>&1; then \
		echo "$(CLANG_FORMAT) not found. Installing LLVM 14..."; \
		CODENAME=$$(lsb_release -cs); \
		sudo apt update; \
		sudo apt install -y wget gnupg software-properties-common; \
		if ! grep -q "llvm-toolchain-$$CODENAME-14" /etc/apt/sources.list.d/* 2>/dev/null; then \
			echo "Adding LLVM repository for $$CODENAME..."; \
			wget -qO - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -; \
			echo "deb http://apt.llvm.org/$$CODENAME/ llvm-toolchain-$$CODENAME-14 main" | \
			sudo tee /etc/apt/sources.list.d/llvm14.list; \
			sudo apt update; \
		fi; \
		sudo apt install -y clang-format-14; \
	fi
	@if [ -z "$(FORMAT_FILES)" ]; then \
        echo "No .c/.h files found - nothing to format."; \
        exit 1; \
    fi
	@echo "Running $(CLANG_FORMAT)..."
	@$(CLANG_FORMAT) -i $(FORMAT_FILES)
	@echo "Formatting complete."

.PHONY: install-clang-format
install-clang-format:
	@if command -v apt >/dev/null 2>&1; then \
		sudo apt update; \
		sudo apt install -y clang-format-$(REQUIRED_CLANG_VERSION); \
		sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-$(REQUIRED_CLANG_VERSION) 100; \
	elif command -v dnf >/dev/null 2>&1; then \
		sudo dnf install -y clang-tools-extra; \
	elif command -v yum >/dev/null 2>&1; then \
		sudo yum install -y clang-tools-extra; \
	else \
		echo "Unsupported package manager. Please install clang-format >= $(REQUIRED_CLANG_VERSION) manually."; \
		exit 1; \
	fi

# ==============================
# Clean Documentation
# ==============================
.PHONY: clean_docs
clean_docs: ## Clean docs folder
	@echo "🧹 Cleaning existing docs..."
	@rm -rf $(DOCS_DIR)

# ==============================
# Documentation
# ==============================
.PHONY: fresh_docs
fresh_docs: clean_docs ## Generate documentation
	@if ! command -v doxygen >/dev/null 2>&1; then \
		echo "doxygen not found. Installing..."; \
		sudo apt-get install -y doxygen; \
	fi
	@if ! command -v dot >/dev/null 2>&1; then \
		echo "graphviz not found. Installing..."; \
		sudo apt-get install -y graphviz; \
	fi
	@if ! dpkg -l texlive-base 2>/dev/null | grep -q "^ii"; then \
		echo "texlive-base not found. Installing..."; \
		sudo apt-get install -y texlive-base texlive-latex-extra; \
	fi
	@echo "📄 Generating HTML and LaTeX docs..."
	@doxygen $(DOXYFILE)
	@if [ -d "$(LATEX_DIR)" ]; then \
		echo "📚 Building refman.pdf..."; \
		$(MAKE) -C $(LATEX_DIR); \
		echo "✅ Docs generation complete (HTML + PDF)."; \
	else \
		echo "ℹ️  LaTeX output disabled — HTML docs at $(DOCS_DIR)/html/index.html"; \
		echo "   To enable PDF: set GENERATE_LATEX = YES in Doxyfile, install texlive-latex-extra"; \
		echo "✅ Docs generation complete (HTML only)."; \
	fi

# ==============================
# Create .deb package
# ==============================
.PHONY: deb
deb: ## Generate .deb package
	@echo "🗜️  Generating Debian package..."
	@cd build && cpack -G DEB

	@echo "✅ Package generation completed successfully!"

# ==============================
# Install library
# ==============================
.PHONY: install
install: ## Install the deb package
	@echo "📦 Installing Debian package..."
	@sudo apt install --reinstall ./build/$(LIB_NAME)-$(VERSION).deb
	@echo "✅ Installation complete."

# ==============================
# Uninstall library
# ==============================
.PHONY: uninstall
uninstall: ## Uninstall the library
	@echo "🗑️  Uninstalling $(LIB_NAME) version $(VERSION)..."
	@sudo apt remove --purge -y $(LIB_NAME)
	@echo "✅ Uninstallation complete."

# ==============================
# Create releases folder
# ==============================
.PHONY: release
release: format rebuild ## Create releases folder
	@echo "📁 Creating releases folder..."
	@mkdir -p $(RELEASES_DIR)
	@mkdir -p $(RELEASES_DIR)/$(VERSION)
	@mkdir -p $(RELEASES_EXAMPLES_DIR)
	@mkdir -p $(RELEASES_BENCHMARKS_DIR)
	@cp build/logx/$(LIB_NAME).so* $(RELEASES_DIR)/$(VERSION)/
	@cp build/logx/$(LIB_NAME).a* $(RELEASES_DIR)/$(VERSION)/
	@echo "📁 Copying example executables..."
	$(call install_exes_from,$(BUILD_DIR)/examples,$(RELEASES_EXAMPLES_DIR))
	@echo "📁 Copying benchmarks executables..."
	$(call install_exes_from,$(BUILD_DIR)/benchmarks,$(RELEASES_BENCHMARKS_DIR))
	@echo "✅ Release folder created at $(RELEASES_DIR)/$(VERSION)/"

# ==============================
# Help
# ==============================
.PHONY: help
help: ## Show this help message
	@echo ""
	@echo "Available targets:"
	@echo "--------------------------------------------"
	@grep -E '^[a-zA-Z_-]+:.*?## ' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*?## "}; {printf "  %-20s %s\n", $$1, $$2}'
	@echo ""