#!/usr/bin/perl
require ("flush.pl");
use File::Copy;
use Cwd;

$templatedir = "/opt/kde3//share/apps/kdevelop/templates";
$kde_icondir = "/opt/kde3//share/icons";

my $entriesfilename= shift || "entries";

my %changings;

my $homedirectory = $ENV{KDEHOME};
$homedirectory = "$ENV{HOME}/.kde2"    if (!$homedirectory);

my $local_datadir     = shift || ($homedirectory  . "/share/apps/kdevelop");
my $kde_miniicondir   = $kde_icondir    . "/locolor/16x16/apps/";

printflush (STDOUT,"Starting with installation\n");

#open file "entries" for reading the parameters from kAppWizard and put it in a hash
open (PROCESSLIST, $local_datadir . "/" . $entriesfilename)
  || die "can not open file \"$entriesfilename\" in your local app-data-dir:\n  $!";

while (defined($name = <PROCESSLIST>)) {
  chomp ($name);
  chomp ($process = <PROCESSLIST>);

  $processes{$name} = $process;
}
close (PROCESSLIST);
printflush (STDOUT,"App \[$processes{NAME}\] Type \[$processes{APPLICATION}\]\n");

$nameLittle = $processes{NAME};
$nameLittle =~ tr/A-Z/a-z/;
$nameBig = $processes{NAME};
$nameBig =~ tr/a-z/A-Z/;
$name =  $processes{NAME};

#$template_dir= $processes{TEMPLATESDIR} if ($processes{TEMPLATESDIR});
if ($processes{KDEICONDIR})
{
  $kde_icondir= $processes{KDEICONDIR} ."/locolor/32x32/apps/";
  $kde_miniicondir = $processes{KDEICONDIR} . "/locolor/16x16/apps/";
}

if ($processes{VSSUPPORT} eq "none"){
  $overDirectory = $processes{DIRECTORY} . "/";
  $underDirectory = $overDirectory . $nameLittle;
}
else{
  $overDirectory = $local_datadir . "/kdeveloptemp/";
  $underDirectory = $overDirectory . $nameLittle;
}


# change to work/project directory
#printflush (STDOUT, "changing directory...\n");
chdir ($overDirectory) || die "Couldn't change to directory $overDirectory: $!";

$date = `date`;
chomp($date);
@time = localtime();
$year = 1900 + $time[5];

setupDefaultChangings();

#printflush (STDOUT,"changing directory...\n");
chdir ($overDirectory);

if ($processes{APPLICATION} =~/customproj/){
  changeCustomApp();
}
else{
  unpackFiles ("template.tar");
#  removed because almost all projects except custom and gnome should
#  use admin-dir (W. Tasin 2001-11-02)
#  if ($processes{APPLICATION} =~ /kde2mini|kde2normal|kde2mdi|qt2normal|qt2mdi|qextmdi|kickerapp|^kpart$|kioslave|kcmodule|ktheme|sharedlib/)
   unless ($processes{APPLICATION} =~ /gnomenormal/)
   {
    unpackFiles ("admin.tar");
   }

  if ($processes{APPLICATION} =~/kde2mini|kickerapp|^kpart$|kioslave|kcmodule|ktheme/) {
	changeMiniApp();
  }
  elsif ($processes{APPLICATION} =~ /kde2normal|kde2mdi/)
  {
    changeNormalApp();
  }
  elsif ($processes{APPLICATION} =~ /qt2normal|qt2mdi|qextmdi/) {
    changeQtApp();
  }
  elsif ($processes{APPLICATION} =~ /qtobjcnormal/) {
    changeQtObjcApp();
  }
  elsif ($processes{APPLICATION} =~  /^cpp$|^sharedlib$/) {
    changeTerminalCPPApp();
  }
  elsif ($processes{APPLICATION} =~ /^c$/) {
    changeTerminalCApp();
  }
  elsif ($processes{APPLICATION} =~ /gnomenormal/) {
    changeGnomeApp();
  }
  elsif ($processes{APPLICATION} eq "objc") {
    changeTerminalObjcApp();
  }
}

if ($processes{VSSUPPORT} eq "cvs") {

  $copysrc = $templatedir . "/cvsignore_template";
  $copydes = $overDirectory . "/.cvsignore";
  copy ("$copysrc", "$copydes");

  $copydes = $underDirectory . "/templates/.cvsignore";
  copy ("$copysrc", "$copydes");

  $copydes = $underDirectory . "/.cvsignore";
  copy ("$copysrc", "$copydes");

  if ($processes{APPLICATION} ne "customproj")
  {
    $copydes = $underDirectory . "/docs/.cvsignore";
    copy ("$copysrc", "$copydes");
    
    $copydes = $underDirectory . "/docs/en/.cvsignore";
    copy ("$copysrc", "$copydes");

    if ($processes{APPLICATION} =~ /kde2mini|kde2normal|kde2mdi/)
    {

      $copydes = $underDirectory . "-api/.cvsignore";
      copy ("$copysrc", "$copydes");

      $copydes = $underDirectory . "/po/.cvsignore";
      copy ("$copysrc", "$copydes");
    }
  }
}

exit;

# this subroutine sets up the changing hash
sub setupDefaultChangings
{
  $changings{"|AUTHOR|"}=$processes{AUTHOR};
  $changings{"|EMAIL|"}=$processes{EMAIL};
  $changings{"|VERSION|"}=$processes{VERSION};
  $changings{"|KDE_QTVER|"}=$processes{KDE_QTVER};
  $changings{"|DATE|"}=$date;
  $changings{"|YEAR|"}=$year;
  $changings{"|SUBDIR|"}= $nameLittle;
  $changings{"|PRJNAME|"}=$name;
  $changings{"|NAME|"}= $name;
  $changings{"|NAMELITTLE|"}= $nameLittle;
  $changings{"|NAMEBIG|"}= $nameBig;
  $changings{"gnomehello"}= $nameLittle;
  $changings{"hello_"}= $nameLittle . "_";
  $changings{"gnome-hello"}= $nameLittle;
  $changings{"GNOMEHELLO"}= $nameBig;
  $changings{"LOCATION"}= "Internet"; ############# FIX ME
}

#the subroutine for replacing words in files
sub replaceValuesInFile {

  my $hashref = shift;
  my $oldfile = shift;
  my $newfilename = shift || "";

  my $where = 0;
  my $wordlength = 0;
  my $word = "";
  my $replace = "";

  if (-f $oldfile)
  {
    open (INPUT,"<$oldfile") || die "cannot open file: $oldfile";
  }
  else
  {
    printflush (STDERR, "Warning: Input file doesn't exist: $oldfile\n");
    return;
  }

  # setup filename replacement...
  $hashref->{"|FILENAME|"}=$oldfile;
  $hashref->{"|FILENAME|"}=$newfilename if ($newfilename);
  my @lines;
  my $i=0;
  while ( defined ($line = <INPUT> ))
  {
    # scan the hash
    foreach $word (keys %$hashref)
    {

      $where=0;
      $wordlength = length($word);
      $replace="";
      $replace=$hashref->{$word} if ($hashref->{$word});

      while ($where != -1)
      {
        $where = index($line,$word,$where);
        if ($where != -1)
        {
          substr($line,$where,$wordlength) = $replace;
          ++$where;
        }
      }
    }

    $lines[$i++]=$line;
  }
  close (INPUT);

  open (OUTPUT,">$oldfile") || die "cannot open file: $oldfile";
  foreach $line (@lines) {
      print OUTPUT $line;
  } 
  close (OUTPUT);
}


sub unpackFiles {

    my ( $tarfile ) = @_;

#    chdir ($overDirectory);
    $dir = cwd();
    printflush (STDOUT, "unpacking <$tarfile.gz> in <$dir>...\n");
    system ("gunzip $tarfile.gz");
#    printflush (STDOUT, "untar file <$tarfile.gz> in <$dir>...\n");
    system ("tar -xf $tarfile");
    unlink $tarfile;
}


sub condCopyTemplateFiles {

  chdir ($underDirectory);
  mkdir ("templates", 0777);

  #copying the templates in the templatedirectoy
  if ($processes{CPP} eq "no" and $processes{HEADER} eq "no") {}
  else {
    if ($processes{HEADER} eq "yes") {
        copy ("$local_datadir/header",
        "$underDirectory/templates/header_template");
    }
  
    if ($processes{CPP} eq "yes") {
        copy ("$local_datadir/cpp",
        "$underDirectory/templates/cpp_template");
    }
  }
}


sub condCopyGNUFiles {

  #if GNU-Files was chosen in kAppWizard
  if ($processes{GNU} eq "yes") {
    #copying the GNU-Files and renamed these
    chdir ($templatedir);
    copy ("AUTHORS_template",   "$overDirectory/AUTHORS"  );
    copy ("COPYING_template",   "$overDirectory/COPYING"  );
    copy ("ChangeLog_template", "$overDirectory/ChangeLog");
    copy ("INSTALL_template",   "$overDirectory/INSTALL"  );
    copy ("README_template",    "$overDirectory/README"   );
    copy ("TODO_template",      "$overDirectory/TODO"     );

    # replace AUTHOR and EMAIL
    chdir($overDirectory);
    replaceValuesInFile(\%changings, "AUTHORS");
  }
}


sub condCopyLsmFile {

  #if LSM-Files was chosen in kAppWizard
  if ($processes{LSM} eq "yes") {
    #copying, rename and replace in the lsm-template
    chdir ($templatedir);
    copy ("lsm_template", "$overDirectory/$nameLittle.lsm");
    chdir ($overDirectory);
    replaceValuesInFile(\%changings, "$nameLittle.lsm");
  }
}


sub condCopyUserDocFiles {  

  #if USER-Docs was chosen in kAppWizard
  if ($processes{USER} eq "yes")
  {
    #copying, rename and replace in the handbook-en-template
    chdir ($templatedir);
    if ($processes{APPLICATION} =~ /kde2mini|kde2normal|kde2mdi|kickerapp|^kpart$|kioslave|kcmodule/)
    {
      copy ("docbook_en_template", "$overDirectory/doc/en/index.docbook");
      chdir ("$overDirectory/doc/en");
      replaceValuesInFile(\%changings, "index.docbook");
    }
    else
    {
      copy ("handbook_en_template", "$underDirectory/docs/en/index.sgml");
      chdir ("$underDirectory/docs/en");
      replaceValuesInFile(\%changings, "index.sgml");
    }
  }
  
  #if USER-Docs was not chosen in kAppWizard
  if ($processes{USER} eq "no") {
    my %emptyIT = (
      "$nameLittle/docs/Makefile" => "",
      "$nameLittle/docs/en/Makefile" => "" );

    # "SUBDIRS = docs" => ""    do you really need this???
    #                    );
    #  chdir ($underDirectory);
    #  replaceValuesInFile(\%emptyIT, "Makefile.am");
    chdir ($overDirectory);

    # The "in.in" will generate the "in" file, so make sure we
    # look for that first
    if ( -f "configure.in.in")
    {
      replaceValuesInFile(\%emptyIT, "configure.in.in");
    }
    elsif ( -f "configure.in")
    {
      replaceValuesInFile(\%emptyIT, "configure.in");
    }
  }
}


sub condCopyKdelnkFile {

  #if .desktop-file was chosen in kAppWizard
  if ($processes{KDELNK} eq "yes") {

    #copying, rename and replace in the kdelnk-file
    chdir ($templatedir);

    if ($processes{APPLICATION} =~ /kde2mini|kde2normal|kde2mdi/)
    {
      copy ("desktop_template", "$underDirectory/$nameLittle.desktop");
      chdir ($underDirectory);
      replaceValuesInFile(\%changings, "$nameLittle.desktop");
    }
  }
}


sub condCopyProgIcons {

  #if no ProgIcon was chosen in kAppWizard
  if ($processes{PROGICON} eq "no") {}

  #if the default ProgIcon was chosen in kAppWizard
  elsif ($processes{PROGICON} eq "") {
    chdir ($templatedir);
    copy ("lo32-app-appicon.png", "$underDirectory/lo32-app-$nameLittle.png");
    chdir ($underDirectory);
    chmod (0666, "lo32-app-$nameLittle.png");
  }
    
  #if a new ProgIcon was chosen in kAppWizard
  else {
    $icon = "$kde_icondir/$processes{PROGICON}.png";
    copy ($icon, "$underDirectory/lo32-app-$nameLittle.png");
    chdir ($underDirectory);
    chmod (0666, "lo32-app-$nameLittle.png");
  }
    
  #if no MiniIcon was chosen in kAppWizard
  if ($processes{MINIICON} eq "no") {}

  #if the default MiniIcon was chosen in kAppWizard
  elsif ($processes{MINIICON} eq "") {
    chdir ($templatedir);
    copy ("lo16-app-appicon.png", "$underDirectory/lo16-app-$nameLittle.png");
    chdir ($underDirectory);
    chmod (0666, "lo16-app-$nameLittle.png");
  }
    
  #if a new MiniIcon was chosen in kAppWizard
  else {
    $icon = "$kde_icondir/$processes{MINIICON}.png";
    copy ($icon, "$underDirectory/lo16-app-$nameLittle.png");
    chdir ($underDirectory);
    chmod (0666, "lo16-app-$nameLittle.png");
  }
}


sub condCreateApiDoc {

#  chdir ($underDirectory);
#  mkdir ("api", 0777);

  # has been moved to processesend.pl

}


sub processCppTemplate {

  my ( $file ) = @_;

  copy ("$local_datadir/cpp", $underDirectory);
  chdir ($underDirectory);
  replaceValuesInFile(\%changings, "cpp", $file);

  if (-f $file)
  {
    open (INPUT,"$file") || die "cannot open file: $file; $!";
  }
  else
  {
    printflush (STDERR, "Warning: Template CPP file does not exist: $file\n");
    return;
  }

  open (OUTPUT,">>cpp");
  while ( defined ($line = <INPUT> )) {
    print OUTPUT $line;
  }
  close (INPUT);
  close (OUTPUT);
  rename ("cpp" , $file);
  unlink ("cpp");

}

sub processHeaderTemplate {

  my ( $file ) = @_;

  copy ("$local_datadir/header", $underDirectory);
  chdir ($underDirectory);
  replaceValuesInFile(\%changings, "header", $file);

  if (-f $file)
  {
    open (INPUT,"$file") || die "cannot open file: $file; $!";
  }
  else
  {
    printflush (STDERR, "Warning: Template Header file does not exist: $file\n");
    return;
  }

  open (OUTPUT,">>header");
  while ( defined ($line = <INPUT> )) {
    print OUTPUT $line;
  }
  close (INPUT);
  close (OUTPUT);
  rename ("header" , $file);
  unlink ("header");
}


sub changeNormalApp {

#    unpackFiles("normal.tar");
#
  #renamed the directory
  printflush (STDOUT, "changing files [$processes{APPLICATION}]...\n");

  chdir ($overDirectory);
  rename ("skel", $nameLittle);

  #create the templatedirectory
  condCopyTemplateFiles();

  #Replacements in overDirectory
  chdir ($overDirectory);
  replaceValuesInFile(\%changings, "Makefile.am");
  if ($processes{APPLICATION} =~ /kdenormal/)
  {
    replaceValuesInFile(\%changings, "configure.in");
  }
  if ($processes{APPLICATION} =~ /kde2normal|kde2mdi|kickerapp|kpart|kioslave|kcmodule/)
  {
    replaceValuesInFile(\%changings, "configure.in.in");
    replaceValuesInFile(\%changings, "doc/en/Makefile.am");
  }

#    replaceOldFile("SUBDIRS = |NAMELITTLE|", "SUBDIRS = $nameLittle.po",
#       "Makefile.am");        not needed anymore!!
#                  the template should have
#              SUBDIRS = |NAMELITTLE|.po
#              but in this case it's made
#              by the binary itself
#    replaceOldFile("$nameLittle/Makefile", "$nameLittle/Makefile po/Makefile",
#       "configure.in");    now fixed entry in template of normal.tar.gz

  #Replacements and name changing in underDirectory
  chdir ($underDirectory);

  rename ("kbase.cpp",     $nameLittle.".cpp");
  rename ("kbase.h",       $nameLittle.".h");
  rename ("kbasedoc.cpp",  $nameLittle."doc.cpp");
  rename ("kbasedoc.h",    $nameLittle."doc.h");
  rename ("kbaseview.cpp", $nameLittle."view.cpp");
  rename ("kbaseview.h",   $nameLittle."view.h");
  if ($processes{APPLICATION} =~ /kde2normal|kde2mdi/)
  {
    rename ("kbaseui.rc",  $nameLittle."ui.rc");
  }

  replaceValuesInFile(\%changings, "Makefile.am");
  replaceValuesInFile(\%changings, "main.cpp");
  replaceValuesInFile(\%changings, $nameLittle.".cpp");
  replaceValuesInFile(\%changings, $nameLittle.".h");
  replaceValuesInFile(\%changings, $nameLittle."doc.cpp");
  replaceValuesInFile(\%changings, $nameLittle."view.cpp");
  replaceValuesInFile(\%changings, $nameLittle."doc.h");
  replaceValuesInFile(\%changings, $nameLittle."view.h");
  if ($processes{APPLICATION} =~ /kdenormal/)
  {
    replaceValuesInFile(\%changings, "resource.h");
  }
  if ($processes{APPLICATION} =~ /kde2normal|kde2mdi/)
  {
    replaceValuesInFile(\%changings, $nameLittle."ui.rc");
  }

  #Replacements in documentation directory
  chdir ($underDirectory."/docs/en");
  replaceValuesInFile(\%changings, "Makefile.am");

  condCopyGNUFiles();
  condCopyLsmFile();
  condCopyUserDocFiles();

  if ($processes{CPP} eq "yes") {
    processCppTemplate("main.cpp");
    processCppTemplate("$nameLittle.cpp");
    processCppTemplate("${nameLittle}view.cpp");
    processCppTemplate("${nameLittle}doc.cpp");
  }

  if ($processes{HEADER} eq "yes") {
    processHeaderTemplate("$nameLittle.h");
    processHeaderTemplate("${nameLittle}view.h");
    processHeaderTemplate("${nameLittle}doc.h");
    if ($processes{APPLICATION} =~ /kdenormal/)
    {
      processHeaderTemplate("resource.h");
    }
  }
  condCopyKdelnkFile();
  condCopyProgIcons();
  condCreateApiDoc();

}


sub changeMiniApp {

#   unpackFiles("mini.tar");
#
  #renamed the directory
  printflush (STDOUT, "changing files [$processes{APPLICATION}]...\n");
  chdir ($overDirectory);
  rename ("skel", $nameLittle);

  #create the templatedirectory
  condCopyTemplateFiles();

  #Replacements in overDirectory
  chdir ($overDirectory);
  replaceValuesInFile(\%changings, "Makefile.am");
  if ($processes{APPLICATION} =~ /kde2mini|kickerapp|^kpart$|kioslave|kcmodule/)
  {
    replaceValuesInFile(\%changings, "configure.in.in");
    replaceValuesInFile(\%changings, "doc/en/Makefile.am");
  }
  if ($processes{APPLICATION} =~ /ktheme/)
  {
    replaceValuesInFile(\%changings, "configure.in.in");
  }

  #Replacements and name changing in underDirectory
  chdir ($underDirectory);


  if ($processes{APPLICATION} =~ /kickerapp|kcmodule/)
  {
    rename ("skel.desktop", $nameLittle.".desktop");
    replaceValuesInFile(\%changings, "$nameLittle.desktop");
  }
  if ($processes{APPLICATION} =~ /^kpart$|kioslave|kcmodule|ktheme|kickerapp/)
  {
    if ($processes{APPLICATION} =~ /^kpart$/)
        {
                rename("skel.rc", $nameLittle.".rc");
                replaceValuesInFile(\%changings, "$nameLittle.rc");
        }
    if ($processes{APPLICATION} =~ /kioslave/)
        {
                rename("skel.protocol", $nameLittle.".protocol");
                replaceValuesInFile(\%changings, "$nameLittle.protocol");
        }
    if ($processes{APPLICATION} =~ /ktheme/)
        {
                rename("skel.themerc", $nameLittle.".themerc");
                replaceValuesInFile(\%changings, "$nameLittle.themerc");
        }
    rename ("skel.cpp",  $nameLittle.".cpp");
    rename ("skel.h",   $nameLittle.".h");
    replaceValuesInFile(\%changings, "$nameLittle.cpp");
    replaceValuesInFile(\%changings, "$nameLittle.h");
    if ($processes{CPP} eq "yes") {
      processCppTemplate("$nameLittle.cpp");
    }

    if ($processes{HEADER} eq "yes") {
       processHeaderTemplate("$nameLittle.h");
    }
  }
  else
  {
     rename ("skel.cpp", $nameLittle.".cpp");
     rename ("skel.h",   $nameLittle.".h");
     replaceValuesInFile(\%changings, "$nameLittle.cpp");
     replaceValuesInFile(\%changings, "$nameLittle.h");
     replaceValuesInFile(\%changings, "main.cpp");
    if ($processes{CPP} eq "yes") {
      processCppTemplate("main.cpp");
      processCppTemplate("$nameLittle.cpp");
    }

    if ($processes{HEADER} eq "yes") {
       processHeaderTemplate("$nameLittle.h");
    }
  }

  replaceValuesInFile(\%changings, "Makefile.am");

#  chdir ($underDirectory."/docs/en");
#  replaceValuesInFile(\%changings, "Makefile.am");

  condCopyGNUFiles();
  condCopyLsmFile();
  condCopyUserDocFiles();
  if ($processes{APPLICATION} =~ /kde2mini/){
    condCopyKdelnkFile();
  }
  condCopyProgIcons();

  condCreateApiDoc();
}


sub changeTerminalCPPApp {

#    unpackFiles("cpp.tar");
#
    #renamed the directory
    printflush (STDOUT, "changing files [$processes{APPLICATION}]...\n");
    chdir ($overDirectory);
    rename ("skel", $nameLittle);

    #create the templatedirectory
    condCopyTemplateFiles();

    #replaced skel with the projectname in different files
    chdir ($overDirectory);
    replaceValuesInFile(\%changings, "Makefile.am");

    # The "in.in" will generate the "in" file, so make sure we
    # look for that first
    if ( -f "configure.in.in")
    {
      replaceValuesInFile(\%changings, "configure.in.in");
    }
    elsif ( -f "configure.in")
    {
      replaceValuesInFile(\%changings, "configure.in");
    }

    chdir ($underDirectory);
    replaceValuesInFile(\%changings, "Makefile.am");
    replaceValuesInFile(\%changings, "main.cpp");

    chdir ($underDirectory . "/docs/en");
    replaceValuesInFile(\%changings, "Makefile.am");

    condCopyGNUFiles();
    condCopyLsmFile();
    condCopyUserDocFiles();

    if ($processes{CPP} eq "yes") {
        processCppTemplate("main.cpp");
    }

    condCopyKdelnkFile();
    condCopyProgIcons();
    condCreateApiDoc();

}


sub changeTerminalCApp {

#    unpackFiles("c.tar");
#
    #renamed the directory
    printflush (STDOUT, "changing files [$processes{APPLICATION}]...\n");
    chdir ($overDirectory);
    rename ("skel", $nameLittle);

    #create the templatedirectory
    condCopyTemplateFiles();

    #replaced skel with the projectname in different files
    chdir ($overDirectory);
    replaceValuesInFile(\%changings, "Makefile.am");

    # The "in.in" will generate the "in" file, so make sure we
    # look for that first
    if ( -f "configure.in.in")
    {
      replaceValuesInFile(\%changings, "configure.in.in");
    }
    elsif ( -f "configure.in")
    {
      replaceValuesInFile(\%changings, "configure.in");
    }

    chdir ($underDirectory);
    replaceValuesInFile(\%changings, "Makefile.am");
    replaceValuesInFile(\%changings, "main.c");

    chdir ($underDirectory . "/docs/en");
    replaceValuesInFile(\%changings, "Makefile.am");

    condCopyGNUFiles();
    condCopyLsmFile();
    condCopyUserDocFiles();

    if ($processes{CPP} eq "yes") {
        processCppTemplate("main.c");
    }

    condCopyKdelnkFile();
    condCopyProgIcons();
    condCreateApiDoc();

}

sub changeTerminalObjcApp {

#    unpackFiles("objc.tar");
#
    #renamed the directory
    printflush (STDOUT, "change files...\n");
    chdir ($overDirectory);
    rename ("skel", $nameLittle);

    #create the templatedirectory
    condCopyTemplateFiles();

    #replaced skel with the projectname in different files
    chdir ($overDirectory);
    replaceValuesInFile(\%changings, "Makefile.am");
    replaceValuesInFile(\%changings, "configure.in");

    chdir ($underDirectory);
    replaceValuesInFile(\%changings, "Makefile.am");
    replaceValuesInFile(\%changings, "main.m");

    chdir ($underDirectory . "/docs/en");
    replaceValuesInFile(\%changings, "Makefile.am");

    condCopyGNUFiles();
    condCopyLsmFile();
    condCopyUserDocFiles();

    if ($processes{CPP} eq "yes") {
        processCppTemplate("main.m");
    }

    condCopyKdelnkFile();
    condCopyProgIcons();
    condCreateApiDoc();

}

sub changeQtApp {

#    unpackFiles("qt.tar");
#
  #renamed the directory
  printflush (STDOUT, "changing files [$processes{APPLICATION}]...\n");
  chdir ($overDirectory);
  rename ("skel", $nameLittle);

  #create the templatedirectory
  condCopyTemplateFiles();

  #Replacements in overDirectory
  chdir ($overDirectory);
  replaceValuesInFile(\%changings, "Makefile.am");
  replaceValuesInFile(\%changings, "configure.in");

  #Replacements and name changing in underDirectory
  chdir ($underDirectory);
  rename ("bank.dsp",     $nameLittle.".dsp") if ( -e "bank.dsp" );
  rename ("bank.dsw",     $nameLittle.".dsw") if ( -e "bank.dsw" );
  rename ("bank.pro",     $nameLittle.".pro") if ( -e "bank.pro" );

  rename ("bank.cpp",     $nameLittle.".cpp");
  rename ("bank.h",       $nameLittle.".h");
  rename ("bankdoc.cpp",  $nameLittle."doc.cpp");
  rename ("bankdoc.h",    $nameLittle."doc.h");
  rename ("bankview.cpp", $nameLittle."view.cpp");
  rename ("bankview.h",   $nameLittle."view.h");

  replaceValuesInFile(\%changings, "Makefile.am");
  replaceValuesInFile(\%changings, "main.cpp");
  replaceValuesInFile(\%changings, "$nameLittle.cpp");
  replaceValuesInFile(\%changings, "$nameLittle.h");

  replaceValuesInFile(\%changings, "$nameLittle.dsp") if ( -e "$nameLittle.dsp" );
  replaceValuesInFile(\%changings, "$nameLittle.dsw") if ( -e "$nameLittle.dsw" );
  replaceValuesInFile(\%changings, "$nameLittle.pro") if ( -e "$nameLittle.pro" );

  replaceValuesInFile(\%changings, "${nameLittle}doc.cpp");
  replaceValuesInFile(\%changings, "${nameLittle}view.cpp");
  replaceValuesInFile(\%changings, "${nameLittle}doc.h");
  replaceValuesInFile(\%changings, "${nameLittle}view.h");

  if ($processes{APPLICATION} =~ /qtnormal|qextmdi/)
  {
    replaceValuesInFile(\%changings, "resource.h");
  }
#    replaceOldFile($name."AppDoc",  $name."Doc",   "skel.cpp");
#    replaceOldFile($name."AppDoc",  $name."Doc",   "skel.h");
#    replaceOldFile($name."AppView", $name."View",  "skel.cpp");
#    replaceOldFile($name."AppView", $name."View",  "skel.h");
#
#    should be inserted in the template as used with
#    |NAME|Doc, |NAME|View
#

  chdir ($underDirectory."/docs/en");
  replaceValuesInFile(\%changings, "Makefile.am");

  condCopyGNUFiles();
  condCopyLsmFile();
  condCopyUserDocFiles();

  if ($processes{CPP} eq "yes") {
    processCppTemplate("main.cpp");
    processCppTemplate("$nameLittle.cpp");
    processCppTemplate("${nameLittle}view.cpp");
    processCppTemplate("${nameLittle}doc.cpp");
  }

  if ($processes{HEADER} eq "yes") {
    processHeaderTemplate("$nameLittle.h");
    processHeaderTemplate("${nameLittle}view.h");
    processHeaderTemplate("${nameLittle}doc.h");
    if ($processes{APPLICATION} =~ /qtnormal|qextmdi/) {
      processHeaderTemplate("resource.h");
    }
  }
  condCopyKdelnkFile();
  condCopyProgIcons();
  condCreateApiDoc();

}

sub changeQtObjcApp {

#    unpackFiles("qtobjc.tar");
#
    #renamed the directory
    printflush (STDOUT, "change files...\n");
    chdir ($overDirectory);
    rename ("skel", $nameLittle);

    #create the templatedirectory
    condCopyTemplateFiles();

    #Replacements in overDirectory
    chdir ($overDirectory);
    replaceValuesInFile(\%changings, "Makefile.am");
    replaceValuesInFile(\%changings, "configure.in");

    #Replacements and name changing in underDirectory
    chdir ($underDirectory);

    rename ("bank.m",     $name.".m");
    rename ("bank.h",       $name.".h");
    rename ("bankdoc.m",  $name."Doc.m");
    rename ("bankdoc.h",    $name."Doc.h");
    rename ("bankview.m", $name."View.m");
    rename ("bankview.h",   $name."View.h");

    replaceValuesInFile(\%changings, "Makefile.am");
    replaceValuesInFile(\%changings, "main.m");
    replaceValuesInFile(\%changings, "$name.m");
    replaceValuesInFile(\%changings, "$name.h");
    replaceValuesInFile(\%changings, "${name}Doc.m");
    replaceValuesInFile(\%changings, "${name}View.m");
    replaceValuesInFile(\%changings, "${name}Doc.h");
    replaceValuesInFile(\%changings, "${name}View.h");
    replaceValuesInFile(\%changings, "resource.h");

#    replaceOldFile($name."AppDoc",  $name."Doc",   "skel.cpp");
#    replaceOldFile($name."AppDoc",  $name."Doc",   "skel.h");
#    replaceOldFile($name."AppView", $name."View",  "skel.cpp");
#    replaceOldFile($name."AppView", $name."View",  "skel.h");
#
#    should be inserted in the template as used with
#    |NAME|Doc, |NAME|View
#

    chdir ($underDirectory."/docs/en");
    replaceValuesInFile(\%changings, "Makefile.am");

    condCopyGNUFiles();
    condCopyLsmFile();
    condCopyUserDocFiles();

    if ($processes{CPP} eq "yes") {
        processCppTemplate("main.m");
	processCppTemplate("$name.m");
	processCppTemplate("${name}View.m");
	processCppTemplate("${name}Doc.m");
    }

    if ($processes{HEADER} eq "yes") {
	processHeaderTemplate("$name.h");
	processHeaderTemplate("${name}View.h");
	processHeaderTemplate("${name}Doc.h");
	processHeaderTemplate("resource.h");
    }

    condCopyKdelnkFile();
    condCopyProgIcons();
    condCreateApiDoc();

}

sub changeCustomApp {

  mkdir ($nameLittle, 0777);

  #create the templatedirectory
  condCopyTemplateFiles();

  condCopyGNUFiles();
  condCopyLsmFile();

}

sub changeGnomeApp {

#    unpackFiles("c.tar");
#
  #renamed the directory
  printflush (STDOUT, "changing files [$processes{APPLICATION}]...\n");
  chdir ($overDirectory);
  rename ("src", $nameLittle);
  rename ("gnome-hello.desktop",$nameLittle . ".desktop");

  chdir ("pixmaps");
  rename ("gnome-hello-logo.png",$nameLittle . "-logo.png");

  chdir ($underDirectory);
  rename ("hello.h", "main.h");
  rename ("hello.c", "main.c");

  #create the templatedirectory
  condCopyTemplateFiles();

  #replaced skel with the projectname in different files
  chdir ($overDirectory);
  replaceValuesInFile(\%changings, "Makefile.am");
  replaceValuesInFile(\%changings, "configure.in");
  replaceValuesInFile(\%changings, $nameLittle . ".desktop");

  chdir ($underDirectory);
  replaceValuesInFile(\%changings, "Makefile.am");
  replaceValuesInFile(\%changings, "main.c");
  replaceValuesInFile(\%changings, "app.c");
  replaceValuesInFile(\%changings, "app.h");
  replaceValuesInFile(\%changings, "menus.c");
  replaceValuesInFile(\%changings, "menus.h");

#    chdir ($underDirectory . "/docs/en");
#    replaceValuesInFile(\%changings, "Makefile.am");

  condCopyGNUFiles();
  condCopyLsmFile();
  condCopyUserDocFiles();

  if ($processes{CPP} eq "yes") {
    processCppTemplate("main.c");
    processCppTemplate("app.c");
    processCppTemplate("menus.c");
  }
  if ($processes{HEADER} eq "yes") {
    processHeaderTemplate("main.h");
    processHeaderTemplate("app.h");
    processHeaderTemplate("menus.h");
 }

  condCopyKdelnkFile();
  condCopyProgIcons();
  condCreateApiDoc();

}
