import sys
import os
import re
import subprocess

test_dir = "tests"
regpro_exec = "./fox-gui/regress"

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
            out_tst = None
            run_error, run_msg = False, None
            devnull = open(os.devnull, 'w')
            try:
                out_tst = subprocess.check_output([regpro_exec, "--script", fullname], stderr= devnull)
            except subprocess.CalledProcessError:
                run_error, run_msg = True, "fail to run"
            out_ref = None
            try:
                out_ref = open("tests/expect/%s.txt" % test_name).read()
            except IOError:
                run_error, run_msg = True, "missing expect"
            led, msg = None, None
            if run_error:
                led, msg = "*", "\x1B[31m%s\x1B[0m" % run_msg
            elif out_tst == out_ref:
                led, msg = " ", "\x1B[32mpass\x1B[0m"
            else:
                led, msg = "*", "\x1B[31mfail\x1B[0m"
                log = open("tests/log/%s.output.diff" % test_name, "w")
                log.write("*** reference ***\n%s\n" % out_ref)
                log.write("*** test program ***\n%s\n" % out_tst)
                log.close()

            print("%s %-24s %s" % (led, test_name, msg))
