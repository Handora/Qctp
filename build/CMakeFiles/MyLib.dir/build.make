# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/handora/QCTP

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/handora/QCTP/build

# Include any dependencies generated for this target.
include CMakeFiles/MyLib.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/MyLib.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/MyLib.dir/flags.make

../src/parser/lexer.c:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/handora/QCTP/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating lexer.c"
	/usr/bin/flex --outfile=/home/handora/QCTP/src/parser/lexer.c /home/handora/QCTP/src/parser/lexer.l

CMakeFiles/MyLib.dir/src/parser/parser.tab.c.o: CMakeFiles/MyLib.dir/flags.make
CMakeFiles/MyLib.dir/src/parser/parser.tab.c.o: ../src/parser/parser.tab.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/handora/QCTP/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/MyLib.dir/src/parser/parser.tab.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/MyLib.dir/src/parser/parser.tab.c.o   -c /home/handora/QCTP/src/parser/parser.tab.c

CMakeFiles/MyLib.dir/src/parser/parser.tab.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/MyLib.dir/src/parser/parser.tab.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/handora/QCTP/src/parser/parser.tab.c > CMakeFiles/MyLib.dir/src/parser/parser.tab.c.i

CMakeFiles/MyLib.dir/src/parser/parser.tab.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/MyLib.dir/src/parser/parser.tab.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/handora/QCTP/src/parser/parser.tab.c -o CMakeFiles/MyLib.dir/src/parser/parser.tab.c.s

CMakeFiles/MyLib.dir/src/parser/parser.tab.c.o.requires:

.PHONY : CMakeFiles/MyLib.dir/src/parser/parser.tab.c.o.requires

CMakeFiles/MyLib.dir/src/parser/parser.tab.c.o.provides: CMakeFiles/MyLib.dir/src/parser/parser.tab.c.o.requires
	$(MAKE) -f CMakeFiles/MyLib.dir/build.make CMakeFiles/MyLib.dir/src/parser/parser.tab.c.o.provides.build
.PHONY : CMakeFiles/MyLib.dir/src/parser/parser.tab.c.o.provides

CMakeFiles/MyLib.dir/src/parser/parser.tab.c.o.provides.build: CMakeFiles/MyLib.dir/src/parser/parser.tab.c.o


CMakeFiles/MyLib.dir/src/parser/lexer.c.o: CMakeFiles/MyLib.dir/flags.make
CMakeFiles/MyLib.dir/src/parser/lexer.c.o: ../src/parser/lexer.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/handora/QCTP/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/MyLib.dir/src/parser/lexer.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/MyLib.dir/src/parser/lexer.c.o   -c /home/handora/QCTP/src/parser/lexer.c

CMakeFiles/MyLib.dir/src/parser/lexer.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/MyLib.dir/src/parser/lexer.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/handora/QCTP/src/parser/lexer.c > CMakeFiles/MyLib.dir/src/parser/lexer.c.i

CMakeFiles/MyLib.dir/src/parser/lexer.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/MyLib.dir/src/parser/lexer.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/handora/QCTP/src/parser/lexer.c -o CMakeFiles/MyLib.dir/src/parser/lexer.c.s

CMakeFiles/MyLib.dir/src/parser/lexer.c.o.requires:

.PHONY : CMakeFiles/MyLib.dir/src/parser/lexer.c.o.requires

CMakeFiles/MyLib.dir/src/parser/lexer.c.o.provides: CMakeFiles/MyLib.dir/src/parser/lexer.c.o.requires
	$(MAKE) -f CMakeFiles/MyLib.dir/build.make CMakeFiles/MyLib.dir/src/parser/lexer.c.o.provides.build
.PHONY : CMakeFiles/MyLib.dir/src/parser/lexer.c.o.provides

CMakeFiles/MyLib.dir/src/parser/lexer.c.o.provides.build: CMakeFiles/MyLib.dir/src/parser/lexer.c.o


# Object files for target MyLib
MyLib_OBJECTS = \
"CMakeFiles/MyLib.dir/src/parser/parser.tab.c.o" \
"CMakeFiles/MyLib.dir/src/parser/lexer.c.o"

# External object files for target MyLib
MyLib_EXTERNAL_OBJECTS =

lib/libMyLib.a: CMakeFiles/MyLib.dir/src/parser/parser.tab.c.o
lib/libMyLib.a: CMakeFiles/MyLib.dir/src/parser/lexer.c.o
lib/libMyLib.a: CMakeFiles/MyLib.dir/build.make
lib/libMyLib.a: CMakeFiles/MyLib.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/handora/QCTP/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C static library lib/libMyLib.a"
	$(CMAKE_COMMAND) -P CMakeFiles/MyLib.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/MyLib.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/MyLib.dir/build: lib/libMyLib.a

.PHONY : CMakeFiles/MyLib.dir/build

CMakeFiles/MyLib.dir/requires: CMakeFiles/MyLib.dir/src/parser/parser.tab.c.o.requires
CMakeFiles/MyLib.dir/requires: CMakeFiles/MyLib.dir/src/parser/lexer.c.o.requires

.PHONY : CMakeFiles/MyLib.dir/requires

CMakeFiles/MyLib.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/MyLib.dir/cmake_clean.cmake
.PHONY : CMakeFiles/MyLib.dir/clean

CMakeFiles/MyLib.dir/depend: ../src/parser/lexer.c
	cd /home/handora/QCTP/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/handora/QCTP /home/handora/QCTP /home/handora/QCTP/build /home/handora/QCTP/build /home/handora/QCTP/build/CMakeFiles/MyLib.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/MyLib.dir/depend

