import sys
import os
import re
import subprocess

test_dir = "tests"
regpro_exec = "./fox-gui/regress"

def parse_fit_output(text):
    if not re.match(r'Recipe (\S+) :\s*$', text[0]): return None
    results = []
    i, n = 1, len(text)
    while i < n:
        m = re.match(r'\s*(\S+) : ([0-9.eE+\-]+)\s*$', text[i])
        if not m: break
        results.append( (m.group(1), float(m.group(2))) )
        i = i + 1
    m = re.match(r'Residual Chisq/pt: ([0-9.eE+\-]+)\s*$', text[i])
    if not m: return None
    results.append( ("ChiSquare", float(m.group(1))) )
    return results

def check_results_eq(results_a, results_b):
    for tpa, tpb in zip(results_a, results_b):
        name_a, value_a = tpa
        name_b, value_b = tpb
        if name_a != name_b or value_a != value_b: return False
    return True

if not os.path.isdir("tests/log"):
    try:
        print "Creating directory tests/log..."
        os.mkdir("tests/log")
    except:
        print "Error creating directory tests/log."
        sys.exit(1)

try:
    subprocess.check_call([regpro_exec, "-v"])
except:
    print "Error calling regress pro."
    print "Please make sure that regress pro executable is correct."
    sys.exit(1)


for dirpath, dirnames, filenames in os.walk(test_dir):
    for filename in sorted(filenames):
        m = re.match(r'([^.]+)\.script$', filename)
        if m:
            fullname = os.path.join(dirpath, filename)
            test_name = m.group(1)
            out_tst, results_test = None, None
            run_error, run_msg = False, None
            devnull = open(os.devnull, 'w')
            try:
                out_tst = subprocess.check_output([regpro_exec, "--script", fullname], stderr= devnull)
                # On windows replace \r\n with \n in the program output.
                out_tst = re.sub(r'(\r+)\n', "\n", out_tst)
                results_test = parse_fit_output(out_tst.split("\n"))
            except subprocess.CalledProcessError:
                run_error, run_msg = True, "fail to run"

            out_ref, results_ref = None, None
            try:
                out_ref = open("tests/expect/%s.txt" % test_name).read()
                results_ref = parse_fit_output(out_ref.split("\n"))
            except IOError:
                run_error, run_msg = True, "missing expect"
            led, msg = None, None
            if run_error:
                led, msg = "*", "\x1B[31m%s\x1B[0m" % run_msg
            elif results_test and results_ref and check_results_eq(results_test, results_ref):
                led, msg = " ", "\x1B[32mpass\x1B[0m"
            else:
                led, msg = "*", "\x1B[31mfail\x1B[0m"
                log = open("tests/log/%s.output.diff" % test_name, "w")
                log.write("*** expected output ***\n%s\n" % out_ref)
                log.write("*** test program ***\n%s\n" % out_tst)
                log.close()

            print("%s %-24s %s" % (led, test_name, msg))
