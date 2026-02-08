git rm -r extern/zxing-cpp
git submodule add https://github.com/zxing-cpp/zxing-cpp extern/zxing-cpp
git submodule update --init --recursive
git commit -m "Use zxing-cpp as submodule"
