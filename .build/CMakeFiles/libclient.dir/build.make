# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

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
CMAKE_COMMAND = /opt/cmake-3.17.2-Linux-x86_64/bin/cmake

# The command to remove a file.
RM = /opt/cmake-3.17.2-Linux-x86_64/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/jeff/Desktop/Multiprocess-filesystem

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jeff/Desktop/Multiprocess-filesystem/.build

# Include any dependencies generated for this target.
include CMakeFiles/libclient.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/libclient.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/libclient.dir/flags.make

CMakeFiles/libclient.dir/libraries/client/lib.cpp.o: CMakeFiles/libclient.dir/flags.make
CMakeFiles/libclient.dir/libraries/client/lib.cpp.o: ../libraries/client/lib.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jeff/Desktop/Multiprocess-filesystem/.build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/libclient.dir/libraries/client/lib.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/libclient.dir/libraries/client/lib.cpp.o -c /home/jeff/Desktop/Multiprocess-filesystem/libraries/client/lib.cpp

CMakeFiles/libclient.dir/libraries/client/lib.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/libclient.dir/libraries/client/lib.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jeff/Desktop/Multiprocess-filesystem/libraries/client/lib.cpp > CMakeFiles/libclient.dir/libraries/client/lib.cpp.i

CMakeFiles/libclient.dir/libraries/client/lib.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/libclient.dir/libraries/client/lib.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jeff/Desktop/Multiprocess-filesystem/libraries/client/lib.cpp -o CMakeFiles/libclient.dir/libraries/client/lib.cpp.s

# Object files for target libclient
libclient_OBJECTS = \
"CMakeFiles/libclient.dir/libraries/client/lib.cpp.o"

# External object files for target libclient
libclient_EXTERNAL_OBJECTS =

liblibclient.a: CMakeFiles/libclient.dir/libraries/client/lib.cpp.o
liblibclient.a: CMakeFiles/libclient.dir/build.make
liblibclient.a: CMakeFiles/libclient.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/jeff/Desktop/Multiprocess-filesystem/.build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library liblibclient.a"
	$(CMAKE_COMMAND) -P CMakeFiles/libclient.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/libclient.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/libclient.dir/build: liblibclient.a

.PHONY : CMakeFiles/libclient.dir/build

CMakeFiles/libclient.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/libclient.dir/cmake_clean.cmake
.PHONY : CMakeFiles/libclient.dir/clean

CMakeFiles/libclient.dir/depend:
	cd /home/jeff/Desktop/Multiprocess-filesystem/.build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jeff/Desktop/Multiprocess-filesystem /home/jeff/Desktop/Multiprocess-filesystem /home/jeff/Desktop/Multiprocess-filesystem/.build /home/jeff/Desktop/Multiprocess-filesystem/.build /home/jeff/Desktop/Multiprocess-filesystem/.build/CMakeFiles/libclient.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/libclient.dir/depend

