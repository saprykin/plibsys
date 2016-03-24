cd /c/projects/plib-build
cmake -DCMAKE_BUILD_TYPE=$configuration -G"$CMAKE_GENERATOR" $BOOST_ARGS /c/projects/plib
make
ctest
