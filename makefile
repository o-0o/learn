# 指定编译器
CC=g++

all:create_build build/discoveryhost build/discoverydevice

create_build:
	@mkdir -p build
# 编译目标1
# $(TARGET1):$(OBJECTS)
build/discoveryhost:include/discoveryhost.h src/discoveryhost.cpp src/discoveryhost_main.cpp
	$(CC)$(CFLAGS) -o $@ $^

# 编译目标2
build/discoverydevice:include/discoverydevice.h src/discoverydevice.cpp src/discoverydevice_main.cpp
	$(CC)$(CFLAGS) -o $@ $^
# $(TARGET2):$(OBJECTS)
# 	$(CC)$(CFLAGS) -o $@$(BINDIR)/sender_main.o $(BINDIR)/sender.o

# 编译规则
# $(BINDIR)/%.o:$(SRCDIR)/%.cpp
# 	echo "$(CC)$(CFLAGS) -c $< -o$@"
# 	$(CC)$(CFLAGS) -c $< -o$@

# 清理目标
# clean:
# 	rm -rf $(BINDIR)/*

# .PHONY是一个伪目标，防止与文件名冲突
.PHONY: all clean
