cd "$(dirname "$0")"

if ! python -c "import sympy; print('')" &> /dev/null; then
    echo "The sympy module is not available."
    echo "You may install sympy and try again."
    exit 1
fi

format_code () {
    clang-format -i -style="{BasedOnStyle: google, IndentWidth: 4}" "$1"
}

echo "Kramers oscillators model"

echo "Generating C++ kernel for n evaluation..."
python kramers-sympy.py ../src/kramers-template.cpp > ../src/kramers-kernel.cpp || exit 1
format_code ../src/kramers-kernel.cpp

echo "Generating C++ kernel for n evaluation and its derivatives..."
python kramers-diff-sympy.py ../src/kramers-diff-template.cpp > ../src/kramers-diff-kernel.cpp || exit 1
format_code ../src/kramers-diff-kernel.cpp

echo "Done"
