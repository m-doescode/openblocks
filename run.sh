if [ $# -eq 0 ] || ([ "$1" != "editor" ] && [ "$1" != "client" ]); then echo "Argument missing, must be 'client' or 'editor'"; exit; fi

[ "$2" = "-debug" ] && CMAKE_OPTS=-DCMAKE_BUILD_TYPE=Debug
[ "$2" = "-release" ] && CMAKE_OPTS=-DCMAKE_BUILD_TYPE=Release
[ "$2" = "-reldbg" ] && CMAKE_OPTS=-DCMAKE_BUILD_TYPE=RelWithDebInfo

[ "$3" = "-gdb" ] && PRE_COMMAND="gdb -ex run "

cmake -Bbuild $CMAKE_OPTS . && (cd build; cmake --build .; cd ..) && $PRE_COMMAND ./build/bin/$1
