tar.exe -cvf HistoryTests.tar HistoryTests
.\Kanzi64.exe -c -j 6 -v 2 -i HistoryTests.tar -l 9 -o HistoryTests.tar.kanzi
rm HistoryTests.tar
