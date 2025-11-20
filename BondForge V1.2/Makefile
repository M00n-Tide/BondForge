# BondForge V1.1 Makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -fPIC
MOC = moc
QT_CXXFLAGS = $(shell pkg-config --cflags Qt6Core Qt6Widgets 2>/dev/null || echo -I/usr/include/qt6/QtCore -I/usr/include/qt6/QtWidgets)
QT_LIBS = $(shell pkg-config --libs Qt6Core Qt6Widgets 2>/dev/null || echo -lQt6Core -lQt6Widgets)

TARGET = bondforge
SOURCE = bondforge.cpp
MOC_SOURCE = bondforge_moc.cpp

# 默认目标
all: $(TARGET)

# 链接目标文件生成可执行文件
$(TARGET): $(SOURCE) $(MOC_SOURCE)
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -o $(TARGET) $(SOURCE) $(MOC_SOURCE) $(QT_LIBS)

# 生成MOC文件
$(MOC_SOURCE): $(SOURCE)
	$(MOC) -o $(MOC_SOURCE) $(SOURCE)

# 清理目标文件和可执行文件
clean:
	rm -f $(TARGET) $(MOC_SOURCE)
	rm -rf bin build

# 构建并安装
install: all
	mkdir -p bin/
	cp $(TARGET) bin/

.PHONY: all clean install