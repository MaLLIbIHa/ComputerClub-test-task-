#bin/bash
if [ $# -lt 1 ]; then
  printf "Usage: %s <application>\n" "$0" >&2
  exit 1
fi

filesCount=$(ls -1q tests/test*.in | wc -l)
bin="$1"           
diff="diff -ciadw"  

for i in $(seq 1 $filesCount); do
  file_in="tests/test${i}.in"             # The in file
  file_out_val="tests/test${i}.out"       # The out file to check against
  file_out_tst="tests/test${i}.out.tst"   # The outfile from test application
  if [ ! -f "$file_in" ]; then
    printf "In file %s is missing\n" "$file_in"
    continue;
  fi
  if [ ! -f "$file_out_val" ]; then
    printf "Validation file %s is missing\n" "$file_out_val"
    continue;
  fi

  printf "Testing against %s\n" "$file_in"

  "./$bin" "$file_in" > "$file_out_tst"
  $diff "$file_out_tst" "$file_out_val"

  e_code=$?
  if [ $e_code != 0 ]; then
    printf "TEST FAIL : %d\n" "$e_code"
  else
    printf "TEST OK!\n"
  fi

done

exit 0