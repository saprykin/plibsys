cd /c/projects/plib-build
"/c/Program Files (x86)/CMake/bin/cmake" -DCMAKE_BUILD_TYPE=$configuration -G"$CMAKE_GENERATOR" $BOOST_ARGS /c/projects/plib
make
"/c/Program Files (x86)/CMake/bin/ctest"
