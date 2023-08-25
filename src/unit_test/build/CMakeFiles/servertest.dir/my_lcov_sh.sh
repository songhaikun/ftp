lcov -c -d ./ -o cover.info
lcov -a init.info -a cover.info -o total.info
lcov --remove cover.info '*/include/*' '*/test/*' '*/usr/include/*' '*/usr/lib/*' '*/usr/lib64/*' '*/usr/local/include/*' '*/usr/local/lib/*'  -o final.info
genhtml -o report final.info
