mkdir c:\projects\plib-build

if "%APPVEYOR_REPO_BRANCH%"=="appveyor_test" (
        set "BOOST_ARGS=-DPLIB_TESTS_STATIC=ON -DBOOST_ROOT=C:\Libraries\boost_1_59_0"
)

if "%USE_MINGW%"=="1" (
        cd c:\projects\plib-build
        set "PATH=C:\MinGW\bin;C:\Program Files (x86)\CMake\bin"
        set BUILD_TYPE=-DCMAKE_BUILD_TYPE=%configuration%

        cmake %BUILD_TYPE% -G"%CMAKE_GENERATOR%" %BOOST_ARGS% c:\projects\plib
        mingw32-make
        ctest
) else (
        if "%USE_MSYS%"=="1" (
                set "PATH=C:\msys2\bin;C:\Program Files (x86)\CMake\bin"
                c:\msys2\bin\bash c:\projects\plib\contrib\appveyor\build_msys.sh
        )
)
