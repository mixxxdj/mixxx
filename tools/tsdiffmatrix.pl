sub help {
    my $err = shift(@_);
    if ($err) {
        print "Error: $err\n\n";
    }
    print<<EOF;
Script to display a matrix with the amount of differences between translation files.

Usage:
  Specify the list of .ts translation files to compare, e.g:

  $ cd res/translations/
  $ perl ../../tools/tsdiffmatrix.pl mixxx_es*.ts

  $ for base in en es fr pt zh; do perl ../../tools/tsdiffmatrix.pl mixxx_\$base*.ts; done

EOF
exit(-1);
}

sub pad {
    my $s = shift(@_);
    return substr("$s     ",0,5);
}

help() if $#ARGV == -1;
foreach $a (@ARGV) {
    $a =~ m/mixxx.*_(\w+)\.ts/ || help("Unexpected argument $a");
}

print pad("");
foreach $a (@ARGV) {
    if ($a =~ m/mixxx.*_(\w+)\.ts/) {
        print pad($1);
    }
}
print "\n";

foreach $b (@ARGV) {
    $b =~ m/mixxx.*_(\w+)\./;
    print pad($1);
    foreach $a (@ARGV) {
        $n = 0;
        open(DIFFOUT,"diff $a $b |");
        while (<DIFFOUT>) {
            if (m/^(\d+),?(\d*)c(\d+),?(\d*)/) {
                $na = $2 ? $2 - $1 : 1;
                $nb = $4 ? $4 - $3 : 1;
                $n += $na > $nb ? $na : $nb;
            }
        }
        close(DIFFOUT);
        print pad($a eq $b ? "" : $n);
    }
    print "\n";
}

print "--------------------------------------------------------------------\n";
