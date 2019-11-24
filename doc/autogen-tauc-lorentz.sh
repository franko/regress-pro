cd "$(dirname "$0")"

if ! python -c "import sympy; print('')" &> /dev/null; then
    echo "The sympy module is not available."
    echo "You may install sympy and try again."
    exit 1
fi

format_code () {
    clang-format -i -style="{BasedOnStyle: google, IndentWidth: 4}" "$1"
}

echo "Tauc-Lorentz model"

echo "Generating C++ kernel for n evaluation..."
python tauc-lorentz-sympy.py ../src/tauc-lorentz-template.cpp > ../src/tauc-lorentz-kernel.cpp || exit 1
format_code ../src/tauc-lorentz-kernel.cpp

echo "Generating C++ kernel for n evaluation and its derivatives..."
python tauc-lorentz-diff-sympy.py ../src/tauc-lorentz-diff-template.cpp > ../src/tauc-lorentz-diff-kernel.cpp || exit 1
format_code ../src/tauc-lorentz-diff-kernel.cpp

echo "Done"
