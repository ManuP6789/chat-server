
#!/bin/bash
gcc -o client us_xfr_cl.c
gcc -o server us_xfr_sv.c
./server > b &
ls -lF /tmp/us_xfr
cat us_xfr_cl.c > a
cat us_xfr_sv.c > a
./client < a
kill %1
diff a b
