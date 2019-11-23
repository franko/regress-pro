python kramers-sympy.py ../src/kramers-template.cpp > ../src/kramers-kernel.cpp || exit 1
clang-format -i -style="{BasedOnStyle: google, IndentWidth: 4}" ../src/kramers-kernel.cpp

python kramers-diff-sympy.py ../src/kramers-diff-template.cpp > ../src/kramers-diff-kernel.cpp || exit 1
clang-format -i -style="{BasedOnStyle: google, IndentWidth: 4}" ../src/kramers-diff-kernel.cpp

