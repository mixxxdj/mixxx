The scripts in this directory allow you to generate all the bit depths and sample rates of the file types that Mixxx supports.

You must have sox and bzip2 installed on your system for them to work.

Then run ./generateFiles on OSX and Linux, or generateFiles.cmd on Windows. Then load each of the resulting files into Mixxx and verify for each:

1) it loads without problems
2) the large and summary waveforms eventually display something (just looks like a green bar)
3) it plays without problems
4) you can click & drag the mouse over the waveform to vary the pitch

If any of these fails, please report a bug using the instructions on this page: http://mixxx.org/wiki/doku.php/reporting_bugs

When you're done, you can run ./generateFiles clean to delete all the generated files