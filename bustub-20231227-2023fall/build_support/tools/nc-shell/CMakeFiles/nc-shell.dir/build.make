# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support

# Include any dependencies generated for this target.
include tools/nc-shell/CMakeFiles/nc-shell.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include tools/nc-shell/CMakeFiles/nc-shell.dir/compiler_depend.make

# Include the progress variables for this target.
include tools/nc-shell/CMakeFiles/nc-shell.dir/progress.make

# Include the compile flags for this target's objects.
include tools/nc-shell/CMakeFiles/nc-shell.dir/flags.make

tools/nc-shell/CMakeFiles/nc-shell.dir/nc-shell.cpp.o: tools/nc-shell/CMakeFiles/nc-shell.dir/flags.make
tools/nc-shell/CMakeFiles/nc-shell.dir/nc-shell.cpp.o: ../tools/nc-shell/nc-shell.cpp
tools/nc-shell/CMakeFiles/nc-shell.dir/nc-shell.cpp.o: tools/nc-shell/CMakeFiles/nc-shell.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tools/nc-shell/CMakeFiles/nc-shell.dir/nc-shell.cpp.o"
	cd /home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/tools/nc-shell && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT tools/nc-shell/CMakeFiles/nc-shell.dir/nc-shell.cpp.o -MF CMakeFiles/nc-shell.dir/nc-shell.cpp.o.d -o CMakeFiles/nc-shell.dir/nc-shell.cpp.o -c /home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/tools/nc-shell/nc-shell.cpp

tools/nc-shell/CMakeFiles/nc-shell.dir/nc-shell.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nc-shell.dir/nc-shell.cpp.i"
	cd /home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/tools/nc-shell && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/tools/nc-shell/nc-shell.cpp > CMakeFiles/nc-shell.dir/nc-shell.cpp.i

tools/nc-shell/CMakeFiles/nc-shell.dir/nc-shell.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nc-shell.dir/nc-shell.cpp.s"
	cd /home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/tools/nc-shell && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/tools/nc-shell/nc-shell.cpp -o CMakeFiles/nc-shell.dir/nc-shell.cpp.s

# Object files for target nc-shell
nc__shell_OBJECTS = \
"CMakeFiles/nc-shell.dir/nc-shell.cpp.o"

# External object files for target nc-shell
nc__shell_EXTERNAL_OBJECTS =

bin/bustub-nc-shell: tools/nc-shell/CMakeFiles/nc-shell.dir/nc-shell.cpp.o
bin/bustub-nc-shell: tools/nc-shell/CMakeFiles/nc-shell.dir/build.make
bin/bustub-nc-shell: lib/libbustub.a
bin/bustub-nc-shell: lib/libutf8proc.a
bin/bustub-nc-shell: /usr/lib/x86_64-linux-gnu/libdwarf.so
bin/bustub-nc-shell: /usr/lib/x86_64-linux-gnu/libelf.so
bin/bustub-nc-shell: /usr/lib/x86_64-linux-gnu/libdl.a
bin/bustub-nc-shell: lib/libbustub_murmur3.a
bin/bustub-nc-shell: lib/libduckdb_pg_query.a
bin/bustub-nc-shell: lib/libfmtd.a
bin/bustub-nc-shell: lib/libfort.a
bin/bustub-nc-shell: tools/nc-shell/CMakeFiles/nc-shell.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../bin/bustub-nc-shell"
	cd /home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/tools/nc-shell && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/nc-shell.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tools/nc-shell/CMakeFiles/nc-shell.dir/build: bin/bustub-nc-shell
.PHONY : tools/nc-shell/CMakeFiles/nc-shell.dir/build

tools/nc-shell/CMakeFiles/nc-shell.dir/clean:
	cd /home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/tools/nc-shell && $(CMAKE_COMMAND) -P CMakeFiles/nc-shell.dir/cmake_clean.cmake
.PHONY : tools/nc-shell/CMakeFiles/nc-shell.dir/clean

tools/nc-shell/CMakeFiles/nc-shell.dir/depend:
	cd /home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall /home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/tools/nc-shell /home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support /home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/tools/nc-shell /home/llb/LLB-windows/BUSTUB2023FALL/bustub-20231227-2023fall/build_support/tools/nc-shell/CMakeFiles/nc-shell.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tools/nc-shell/CMakeFiles/nc-shell.dir/depend
