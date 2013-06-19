The scripts in this directory allow you to generate all the bit depths and sample rates of the file types that Mixxx supports.

You must have sox, lame and bzip2 installed on your system for them to work.

Then run ./generateFiles on OSX and Linux, or generateFiles.cmd on Windows. Then load each of the resulting files into Mixxx and verify for each:

1) it loads without problems
2) the large and summary waveforms eventually display something (just looks like a green bar)
3) it plays without problems at the correct pitch (440Hz for the left channel, 1000Hz for the right, combined for mono)
4) you can click & drag the mouse over the waveform to vary the pitch

If all of these are good, please add a "Yes" in the "Does it work?" column for your OS and the file type on this page: http://mixxx.org/wiki/doku.php/sound_file_testing_matrix

If any of these fails, please report a bug using the instructions on this page: http://mixxx.org/wiki/doku.php/reporting_bugs

When you're done, you can run ./generateFiles clean to delete all the generated files