help:
    just -l

configure:
    cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug .

# Commented out configure because it takes unnecessarily long
# Just run configure manually if you've made any changes
build: #configure
    cmake --build build -j$(nproc)

editor: build
    ./build/bin/editor
    
test: build
    ctest --test-dir=build
    
test-v: build
    ctest --test-dir=build --rerun-failed --output-on-failure
    
test-dbg: build
    gdb -q ./build/bin/obtest