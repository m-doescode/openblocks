[ "$2" = "-debug" ] && CMAKE_OPTS=-DCMAKE_BUILD_TYPE=Debug
[ "$2" = "-release" ] && CMAKE_OPTS=-DCMAKE_BUILD_TYPE=Release
[ "$2" = "-reldbg" ] && CMAKE_OPTS=-DCMAKE_BUILD_TYPE=RelWithDebInfo

[ "$3" = "-gdb" ] && PRE_COMMAND="gdb -ex run "

cmake $CMAKE_OPTS . && cmake --build . && $PRE_COMMAND ./bin/$1
