#!/usr/bin/perl

my @files = (glob("*.vert"),glob("*.frag"));

open(GENERATED,">generated_shaders_gl.cmake");
print(GENERATED "set(generated_shaders_gl\n");
for $file (@files)
{
    system("qsb","--glsl","120",$file,"-o","/tmp/$$-$file.qsb");
    open(INFILE,"qsb --dump /tmp/$$-$file.qsb|");
    open(OUTFILE,">$file.gl");
    $ok = 0;
    $comment_added = 0;
    print "Generating $file.gl from $file\n";
    while (<INFILE>)
    {
        if ($in_shader_block == 2)
        {
            if (m/^\*\*/)
            {
                $in_shader_block = 0;
                $ok = 1;
            }
            else
            {
                if (!$comment_added)
                {
                    if (!m/^#/)
                    {
                        print(OUTFILE "//// GENERATED - EDITS WILL BE OVERWRITTEN\n");
                        $comment_added = 1;
                    }
                }
                print OUTFILE "$_";
            }
        }
        elsif ($in_shader_block == 1)
        {
            chomp($_);
            if ($_ eq "Contents:")
            {
                $in_shader_block = 2;
            }
        }
        else
        {
            chomp($_);
            if ($_ eq "Shader 1: GLSL 120 [Standard]")
            {
                $in_shader_block = 1;
            }
        }
    }
    close INFILE;
    close OUTFILE;
    if($ok)
    {
        print(GENERATED "    $file.gl\n");
    }
    else
    {
        print STDERR "Failed to generated $file.gl";
        unlink("$file.gl")
    }
    unlink("/tmp/$$-$file.qsb");
}
print(GENERATED ")\n");
close GENERATED;
