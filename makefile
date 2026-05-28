# 编译器设置
CXX = g++
CXXFLAGS = -std=c++20 -O2
COVFLAGS = -fprofile-arcs -ftest-coverage -g

# 目标文件名
TARGET = bmssp
SOURCES = main.cpp
COV_DIR = coverage_report

# 默认目标：正常编译
all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

# 覆盖率分析目标
coverage: clean_coverage
	@echo "=== 编译（带覆盖率选项） ==="
	$(CXX) $(CXXFLAGS) $(COVFLAGS) -o $(TARGET) $(SOURCES)
	@echo "=== 运行程序 ==="
	./$(TARGET)
	@echo "=== 生成 HTML 报告 ==="
	lcov --capture --directory . --output-file coverage.info --no-external
	genhtml coverage.info --output-directory $(COV_DIR)
	@echo "=== 完成 ==="
	@echo "报告位置: $(COV_DIR)/index.html"
	@echo "用浏览器打开: firefox $(COV_DIR)/index.html"

# 清理覆盖率相关文件
clean_coverage:
	rm -f *.gcda *.gcno *.gcov coverage.info
	rm -rf $(COV_DIR)

# 清理所有（包括可执行文件）
clean: clean_coverage
	rm -f $(TARGET)
	rm -f bmssp.res

# 查看报告（如果有图形界面）
view:
	firefox $(COV_DIR)/index.html

run:
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)
	./$(TARGET)

# 帮助信息
help:
	@echo "可用命令："
	@echo "  make              - 正常编译程序"
	@echo "  make coverage    - 编译、运行、生成覆盖率HTML报告"
	@echo "  make view        - 在浏览器中打开报告（需要图形界面）"
	@echo "  make clean       - 删除所有生成的文件"
	@echo "  make clean_coverage - 只删除覆盖率相关文件"

.PHONY: all coverage clean clean_coverage view help run
