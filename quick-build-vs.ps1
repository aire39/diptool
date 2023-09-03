
git submodule update --init --recursive

if ( -not (Test-Path -Path "build-vs") ) { 
mkdir build-vs
}

pushd build-vs
cmake -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
popd
