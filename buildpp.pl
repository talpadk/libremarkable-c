#!/usr/bin/perl -w

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


use strict;
use Fcntl ':mode';
use Cwd;
use Getopt::Long;
use threads;
use threads::shared;
use Thread::Semaphore;

#=================== Setup of internal global variables ================
#ToDo refactor the code to have all the global variables suffixed by '_' to make it easier to see that they are globals
#May cause backwards compatibility issues for the ones that are changeable by localbuild.pl

#mutex for all variables
my $globalMutex = Thread::Semaphore->new(1);

#an array of all builder threads
my @threads= ();

#the number of the next job to build
my $nextBuildJob:shared = 0;

#an array of build jobs, sorted by importance
my @sortedBuildJobs:shared = ();


my $workingDir:shared = &Cwd::cwd();

#a mapping of the pathes not to search for source files
my %dontSearchForFiles=();

#a mapping to find the path of a file
my %fileMapping:shared=();

#a cache of level 1 dependencies
my %deps=();

#a cache of the recursive dependencies (also stored i .d files)
my %depCache=();

#a cache of file times -1 = file dos't exist, -2 = cache entry invalid
my %timeStamps:shared=();

#a hash used to remember files pased for this run, also speeds up parsing
my %recurivePassedFiles:shared=();

#a hash from files to modification times, for the O files that needs to be
#rebuild this run.
my %rebuildOFiles:shared=();
#a hash from filenames to compiler arguments for O files
my %rebuildOFilesArguments:shared=();
#The O file currently being build
my $currentOFileNumber:shared = 1;
#the total number of O files needing to be build
my $numberOfOFiles:shared = -1;

#The time we started to compile files
my $compileStartTime:shared = 0;

#If true buildpp will emit warnings about duplicate files
my $warnDuplicateFiles = 1;

#a hash from files to modification times, for the exe files that needs to be
#rebuild this run.
my %rebuildExeFiles:shared=();
#a has from filenames to linker arguments for exefiles
my %rebuildExeFilesArguments:shared=();

my @exeFiles:shared=();
my $doClean:shared = 0;
my $doBuild:shared = 0;
my $buildFailed:shared = 0;
my $doTest:shared = 0;
my $testArguments:shared = "!¤/)=(Noonewritesthis!!IHope";
my @targets:shared = ();
my $ignoreMissingCodeFiles = 0;

#set to true when options and arguments are read
my $argumentsRead:shared = 0;

#if set to true "" include pathes will have there path removed
my $stripIncludePath:shared = 0;


#============ localbuild.pl changeable variables below this point =============

#
# The localbuild.pl file may modify the variables below
#          (those it makes sense to change)
#



#A hash from regEx's for OS names to $target values
my %osHash:shared = ("linux" => "linux", "cygwin" => "windows", "darwin" => "macosx");

my $incDirs:shared="";



#the number of threats to use for compiling
my $numberOfThreads:shared = 0;


#The current build target, empty for no specific target
my $target:shared = "";

#Use lazy linking or not
my $lazyLinking:shared = 0;

#Use distcc for compileing?
my $useDistcc:shared = 0;

#The program to use as a compiler
my $compiler:shared = "g++";
#The default flags used when compiling code into object files
my $cflags:shared = "-c -Wall ";
#Defines the suffixes that if included with #include ""
#will become part of the dependencies. Used in a regEx therefore
#the weird syntax.
my $includeSuffix:shared = "\\.h|\\.cpp";
#The suffix that your code files have, DON'T put a dot in front 
my $codeSuffix:shared = "cpp";
#The suffix to put after object files, DON'T put a dot in front 
my $objectSuffix:shared = "o";

#The program to use as a linker
my $linker:shared = "g++";
#The default flags used when linking object files together
my $ldflags:shared = "";
#The suffix to put after executable files, DON'T put a dot in front 
my $exeSuffix:shared = "";
#The program used to test the exe, this could be "gdb " or "wine " if you are 
#xcompiling to windows. It's just put in front so remember a trailing space
my $testProgram:shared = "";

#The dir where buildpp generates files (.o and .d files)
#if it dos't exist it will be created. WARNING ALL files here
#will be deleted on clean! 
my $buildDir:shared = "build/";

#The dir where buildpp generates output files (.exe files)
#if it dos't exist it will be created. WARNING ALL files here
#will be deleted on clean! 
#If $outputDir is left empty the value of $buildDir will be used instead
my $outputDir:shared = "";

#If true you are requested to confirm when cleaning
my $cleanConfirm:shared = 1;


#Colour definitions for different types of output
my $colourVerbose:shared = "\033[33m";
my $colourNormal:shared = "\033[37;40m";
my $colourError:shared = "\033[33;41m";
my $colourAction:shared = "\033[32m";
my $colourExternal:shared = "\033[36m";
my $colourWarning:shared = "\033[33m";
my $colourGirlie:shared = "\033[35m"; 

#If false the colour entrys will not be used.
my $useColours:shared = 0;

#Displays lots of information if true.
my $verbose:shared = 0;

#If true displays information that you normally dont need nor want.
my $girlie:shared = 0;

#If true show when we rebuild .d files.
my $showDBuild:shared = 0;

#If true always shows the command used for compilation (else just on error)
my $showCompilerCommand:shared = 0;

#If true always shows the command used for linking (else just on error)
my $showLinkerCommand:shared = 0;

#If true buildpp will perform a unconditional clean opperation if the last target isn't the same as the current target
#The functionality is intended to be used if the target may influence how other .o files are build
my $newTargetClean = 0;

#default sub for post processing.
#1. argument = path + filename of executable to post process
sub defaultPostProcessing {}
my $postProcessingRef = \&defaultPostProcessing;

my $rescanFiles = 0;

#default action for .auto files
#1. argument = path to .auto file
#2. argument = filename of .auto file
#3. argument = current path (you must return here at the end of the function
#Set $rescanFiles = 1, if your auto process may generate new files.
sub defaultAutoProcessing
{
  my $dirName = $_[0];
  my $name = $_[1];
  my $workingDir = $_[2];
  
  chdir($dirName);
  print `./$name`;
  chdir($workingDir);
  $rescanFiles = 1;
}
my $autoProcessingRef = \&defaultAutoProcessing;

sub defaultBeforeCompileRunFunction {}
my $beforeCompileRunRef = \&defaultBeforeCompileRunFunction;

#The hash of arguments for use with GetOptions
my %argumentHash = 
(
"<>" => \&handleArguments,
"clean" => \$doClean,
"usecolours|usecolors|c!" => \$useColours,
"verbose|v!" => \$verbose,
"girlie!" => \$girlie,
"showdbuild!" => \$showDBuild,
"test:s" => \$testArguments,
"j=i" => \$numberOfThreads,
"distcc" => \$useDistcc
);


#============= Functions below this point ============


sub printWarning {
	print $colourWarning.$_[0].$colourNormal;
}

sub printError {
	print $colourError.$_[0].$colourNormal;
}

sub printFatal {
	die $colourError.$_[0].$colourNormal;
}

sub printGirlie {
	my $string = $_[0];
	if ($girlie){
		print $colourGirlie.$string.$colourNormal;
	}
}

#Sub to handle arguments, this is called by 'GetOptions' and updates the list of build targets
sub handleArguments
{
  if (!$argumentsRead){
    my $argument = $_[0];
    #Strip path component from the argument 
    if ($argument =~ /^.*\/([^\/]+)$/){
			$argument = $1;
    }
    push(@targets, "$argument");
  }
}



#Tries to reads an extra file and parse its contents, it will only
#generate a warning if it can't find it.
sub readConfigFile
{
  my ($filename) = @_;
  my $contents = "";
  my $config;
  if (open ($config, "$filename")) {
	print "reading $filename\n";
	while (<$config>) {
	    $contents .= $_;
	}
	close ($config);
    }
    else {
	print "Warning: unable to open $filename\n";
    }
    eval $contents;
    
    if (!($@ eq "")){
      die $@;
    }
}

#Tries to determine the OS we are running under and sets $target accordingly
sub autoTarget
{
  my $targetFound = 0;
  my $osMatch;
  my $osString;
  my $osName = $^O;  
  while (($osMatch, $osString) = each %osHash){
    if ($osName =~ /$osMatch/){
      $targetFound = 1;
      $target = $osString;
      last;
    }
  }
  
  
  if ($targetFound==0){
    print "WARNING: autoTarget failed to acquire a target for '$osName'\n";
    print "Try fixing %osHash or switch to manual targeting\n"
  }
}


#reads the commandline arguments
sub parseArguments
{
  GetOptions(%argumentHash);
  if (scalar(@targets) != 0 || ($doClean == 0)){
	  $doBuild = 1;
  }
  if (!($testArguments eq "!¤/)=(Noonewritesthis!!IHope")){
    $doTest = 1;
  }
  $argumentsRead = 1;
}

#Recursively searches a dir for files that may be used to generate .o and exe files with
#Any .auto files will also be passed to the $autoProcessingRef function
#This function appends/populates the globalVariables $incDirs and %fileMapping
sub findFilesInDir
{
  my $dirName =  $_[0];
  if (exists($dontSearchForFiles{$dirName})){
    return;
  }
  
  
  my $dir;
  opendir ($dir, $dirName)
    || die "Can't find directory '$dirName' specified in modulelist";
  
  my @entrys = readdir $dir;
  
  my $usableFiles = 0;
  
  foreach(@entrys){
    if ($_ !~ /^\.+$/){
      #it was not .. , . or a linux hidden file
      my $fileName = $dirName."/".$_;
      my $mode = (stat($fileName))[2];
      if (S_ISDIR($mode)){
        findFilesInDir($fileName);
      }
      else {
        if ($_ =~ /(^.*)($includeSuffix)$/){
          $usableFiles = 1;
          my $entry = $_;
          if (!exists($dontSearchForFiles{$fileName})){
            if (!exists($fileMapping{$entry})){
	      $fileMapping{$entry} = $dirName."/";
              #print "found $entry in $dirName with filename $fileName\n";
	    }
            elsif ($warnDuplicateFiles) {
	      my $duplicateFolder = $fileMapping{$entry};
	      printWarning("Ignoring duplicate file $fileName, as it already was found in\n                        $duplicateFolder\n");
	    }
	  }
        }
        elsif ($_ =~ /\.auto$/){
          my $name = $_;
          print $colourExternal."Updating $name in $dirName$colourNormal\n";
          &$autoProcessingRef($dirName, $name, $workingDir);
        }
      }
    }
  }
  closedir $dir;
  if ($usableFiles == 1){
    $incDirs = $incDirs."-I$dirName ";
  }
}

#reads the modulelist file and generates the search tables for building
sub readModuleList
{
  $incDirs="";
  print $colourAction."Reading module list and compiling file list$colourNormal\n";
  open (moduleList, "<modulelist")
    || die "Can't open modulelist";
    
  while (<moduleList>){
    my $dirName = $_;
    $dirName =~ s/[\n\r]+$//;
    if (substr($dirName,0,1) eq "!"){
      $dontSearchForFiles{substr($dirName,1)}=1;
    }
    else {
      findFilesInDir($dirName);
    }
  }

  close moduleList;
}


#generates a string that can be used to search the file list for files that
#the user has requested a build of
sub makeMatchString
{
  my $match="";
  if (scalar(@targets) == 0){
    $match = ".*\.$codeSuffix";	
  }
  else {
    foreach (@targets){
      $match = $match.$_.".$codeSuffix|";
    }
    $match =~ s/\|$//;
  }
  return $match;
}

#removes the specified object files
sub removeObjectFiles
{
    print $colourAction."Removing object files$colourNormal\n";
    my $match = makeMatchString();
    foreach(keys(%fileMapping)){
	unlink(($fileMapping{$_}.$_.".".$objectSuffix));
    }
}



#gets the time stamp of the first argument (file (-path + sufix))
sub getTime
{
  my $filename = $_[0];
  my $result = -1;

  if (exists($timeStamps{$filename}) and $timeStamps{$filename} != -2){
    $result = $timeStamps{$filename};
  }
  else {
   my @fileStatus = stat($filename);
    if (scalar(@fileStatus) == 0){
      $result = -1;
    }
    else {
      $result = $fileStatus[9];    
    }
  }
  $timeStamps{$filename} = $result;

  return $result;
}

#parses a file and generates a parsed version in memory (only 1. level deps are
#generated, code files will also generate linker level 1. information
#first argument is the name of the file without path
#returns nothing, however the result can be found in the memory cache (%deps)
sub parseFile
{
  my $filename = $_[0];

  if ($stripIncludePath){
      $filename =~ s/^.+\///;
  }

  if (exists($deps{$filename})){
    return;
  }

  if ($verbose){
    print $colourVerbose."Parsing file $filename".$colourNormal."\n";
  }
  print  $colourVerbose."Parsing file $filename".$colourNormal."\n";
  
  if (!exists($fileMapping{$filename})){
      if ($ignoreMissingCodeFiles){
	my @emptyDeps;
        printWarning("Ignoreing file $filename\n");
	$deps{$filename} = \@emptyDeps;
	return;
      }
      else {
	die $colourError."Unable to find file '$filename'".$colourNormal."\n";
      }
  }
  my $filePath = $fileMapping{$filename}.$filename;

  #parse it 
  my $file;
  my @includeList=();
  #all files start out with generic all target
  my $currentTarget = "all";
  #use project wide setting for lazyLinking
  my $currentLazyLinking = $lazyLinking;

  open ($file, "<$filePath") 
		|| die "Unexpected error unable to read $filePath";

  while (<$file>){
    my $line = $_;  
    #detect local target changes
    if ($line =~ /\/{2,3}\#target\s+(\S+)/){
      $currentTarget = $1;
    }
    #detect local lazy linking changes
    if ($line =~ /\/{2,3}#lazylinking\s+(\S+)/){
      my $arg = $1;
      if ($arg =~ /off/i){
        $currentLazyLinking = 0;
      }
      if ($arg =~ /on/i){
        $currentLazyLinking = 1;
      }
    }
    if ($currentTarget eq "all" or $target eq $currentTarget){
      #curent target matches, now read other options
      if ($line =~ /^\s*(\#include\s*\"\s*)(\S+)(\s*\")/){
	my $inc = $1;
	my $lineFile = $2;
	my $suffix = $3;
	if ($stripIncludePath){
	    $lineFile =~ s/^.+\///;
	}
        push (@includeList, $inc.$lineFile.$suffix);
        if ($currentLazyLinking){
          my $linkName = $lineFile;
          #remove file suffix
          $linkName =~ s/\.[^\.]+$//;
	  #only push the link requirements if the file exists or we don't allow missing code files
	  if ((!$ignoreMissingCodeFiles) || (exists($fileMapping{$lineFile}) && -e $fileMapping{$lineFile}."$linkName.$codeSuffix")){
	    push (@includeList, "\#link $linkName");
	  }
        }
      }
      if ($line =~ /\/{2,3}(\#cflags|\#ldflags|\#exe|\#target|\#link|\#lazylinking|\#global_cflags)(\s?[^\r^\n]*)/){
        push (@includeList, ($1.$2));
      }
    } 
  }
  close($file);
  
  $deps{$filename} = \@includeList;
}


#Parses a given file recursively returning a list of all include and link dependencies.
#
#The parsing is done by calling parseFile(), and calling it self for the recursive element.
#
#The following tokens triggers a recursion #link and #include
#From the recursion into #link dependencies only #link, #ldflags and #global_cflags are used
#
#returns a list of all dependencies
#first argument is the name of the file without path
sub parseFileRecurcive
{
  my $file = $_[0];

  if ($stripIncludePath){
      $file =~ s/^.+\///;
  }

  my @result=();

  if (exists($recurivePassedFiles{$file})){
    return @result;
  }
  
  $recurivePassedFiles{$file} = "";
  
  parseFile($file);
  
  if (!exists($deps{$file})){
    die $colourError.
      "Trying to get recursive dependencie information for $file, ".
      "it has not been created$colourNormal\n";
  }

  my @dep = @{$deps{$file}};
  my %unique = ();
  @result = @dep;
  for my $item (@dep){
    if ($item =~ /\#link\s+(\S+)/){
      my $newFile = $1;
      parseFile($newFile.".$codeSuffix");
      my @subItem = parseFileRecurcive($newFile.".$codeSuffix");
      for my $subSubItem (@subItem){
        if ($subSubItem =~ /\#link/ || $subSubItem =~ /\#ldflags/ || $subSubItem =~ /\#global_cflags/){
          if (!exists($unique{$subSubItem})){
            push (@result, $subSubItem);
            $unique{$subSubItem} = " ";
          }
        }
      }
    }
    else{ if ($item =~ /\#include\s+\"\s*(\S+)\s*\"/){
      my $newFile = $1;
      parseFile($newFile);
      my @subItem = parseFileRecurcive($newFile);
      for my $subSubItem (@subItem){
        if (!exists($unique{$subSubItem})){
          push (@result, $subSubItem);
          $unique{$subSubItem} = " ";
        }
      }
    }}
  }
  return @result;
}


#generate .d file on disk (cached version of parseFile)
#includes all levels, created using parseFileRecurcive
#first argument is the name of the file without path
sub parseFileCached{
  my $filename = $_[0];
  
  if ($stripIncludePath){
      $filename =~ s/^.+\///;
  }

  #test for a cached(mem) version and return it if found 
  if (exists($depCache{$filename})){
    my @result = @{$depCache{$filename}};
    return @result;
  }
  
  if ($verbose){
    print $colourVerbose."Testing $filename.d$colourNormal\n";
  }
  
  #if source (.cpp) is newer that .d file we need to refresh disk version
  my $rebuild = 0;
  my $dFilename = $buildDir.$filename.".d";
  my $dTime = getTime($dFilename);
  my $fileTime = getTime($fileMapping{$filename}.$filename);

  if ($dTime == -1 or $dTime <= $fileTime){
    $rebuild = 1;
  }
  else {
    #dfile newer that source, now we need to test .dfiles that this file depends on
    my $dFile;
    open ($dFile, "<$dFilename") ||
      die ($colourError."Unexpected error unable to open $dFilename for input".
           $colourNormal."\n");
    while (<$dFile>){
      my $line = $_;
      #depends on a "header" file
      if ($line =~ /\#include\s+\"([^"]+)\"/){
        my $depName = $1;
        if (!exists($fileMapping{$depName})){
          die ($colourError."Unknown file $depName$colourNormal\n");
        }
        my $depPath = $fileMapping{$depName};
        my $depTime = getTime($depPath.$depName);
        if ($depTime >= $dTime){
          $rebuild = 1;
          last();
        }
      }
      #depends on a source file
      if ($line =~ /\#link\s+([^[\r\n]]+)/){
        my $depName = $1.".$codeSuffix";
        if (!exists($fileMapping{$depName})){
          die ($colourError."Unknown file $depName$colourNormal\n");
        }
        my $depPath = $fileMapping{$depName};
        my $depTime = getTime($depPath.$depName);
        if ($depTime >= $dTime){
          $rebuild = 1;
          last();
        }
      }
    }
  }
  
  my @deps = ();
  
  if ($rebuild){
    #rebuild was needed
    if ($showDBuild){
      print $colourAction."Building $filename.d$colourNormal\n";
    }
    
    @deps = parseFileRecurcive($filename);
    
    my $outFile;
    open ($outFile, ">$dFilename") ||
      die $colourError."Unable to write to $dFilename$colourNormal\n";
    
    for my $fileLine (@deps){
      print $outFile $fileLine."\n";
    }
    
    close ($outFile);
    $depCache{$filename} = \@deps;
    #invalidate timestamp of .d file
    $timeStamps{$dFilename} = -2;
  }
  else {
    #rebuild was not needed, use disk file instead
    my $dFile;
    open ($dFile, "<$dFilename") ||
      die ($colourError."Unexpected error unable to open $dFilename for input".
           $colourNormal."\n");
    
    while (<$dFile>){
      push (@deps, $_);
    }
    close($dFile);
  }
  
  return @deps;
}






#returns a string containing progress info (only valid when compiling O files) !!! THREAD WARNING !!!
sub getProgressInfo
{
  my $timePassed = time()-$compileStartTime;
  my $result;
  if ($currentOFileNumber == 1){
    $result = "(0%"
  }
  else {
    my $progress = ($currentOFileNumber-1)*100/$numberOfOFiles;
    my $timeString;
    if (!($progress==0)){
      my $timeLeft = ($timePassed/($progress/100)-$timePassed);
      $timeLeft = int $timeLeft;
      if ($timeLeft > 60){
        my $minutes = int ($timeLeft/60);
        $timeString.=$minutes."m ";
        $timeLeft -= $minutes*60;
      }
      $timeString.=$timeLeft."s";
    }
    $progress = int ($progress);
    $result = "($progress% $timeString";
  }
  $result.=")";
  return $result;
}

#builds an object file !!! THREAD WARNING !!!
#first argument is the file without a path and without suffix
sub buildObjectFile
{
  my $filename = $_[0];  

  if ($stripIncludePath){
      $filename =~ s/^.+\///;
  }
  
  my $comileArguments = $cflags.$rebuildOFilesArguments{$filename};
  my $path = $fileMapping{$filename.".$codeSuffix"};
  
  $globalMutex->down;
  if ($buildFailed){
    $globalMutex->up;
    return;
  }
  my $progress = getProgressInfo();
  
  my $compilerCommand = $compiler;
  if ($useDistcc){
    $compilerCommand = "distcc $compiler";
  }
  my $command = "$compilerCommand -o $buildDir$filename.$objectSuffix $comileArguments $path$filename.$codeSuffix";

  print $colourAction."$currentOFileNumber/$numberOfOFiles $progress Compiling ".
        "$filename.o$colourNormal\n";

  $currentOFileNumber++;
  
  if ($showCompilerCommand == 1){
      print "$command\n";
  }
  $globalMutex->up;
  
  `$command`;
  if ($? != 0) {
    $globalMutex->down;
    print $colourError."Compiling of $filename.$objectSuffix failed$colourNormal\n";
    if (!$showCompilerCommand){
      $buildFailed = 1;
      print "$command\n";
    }
    $globalMutex->up;
    unlink($filename.".".$objectSuffix);
    print $colourError."   Sorry   $colourNormal\n";
  }
  #invalidate object files timestamp cache
  $globalMutex->down;
  $timeStamps{$buildDir.$filename.".$objectSuffix"} = -2;
  $globalMutex->up;
}

#builds an object file
#first argument is the code file to use
sub findObjectFiles
{
  my $filename = $_[0];

  if ($stripIncludePath){
      $filename =~ s/^.+\///;
  }

  if ($stripIncludePath){
      $filename =~ s/^.+\///;
  }

  if ($verbose){
    print $colourVerbose."Generating $filename.$objectSuffix$colourNormal\n";
  }

  if (!exists($fileMapping{$filename.".$codeSuffix"})){
    die $colourError."Tryed to compile $filename.$codeSuffix file not found".
      "$colourNormal\n";
  }

  my $codeTime = getTime($fileMapping{$filename.".$codeSuffix"}.
                         $filename.".$codeSuffix");
  my $objTime = getTime($buildDir.$filename.".$objectSuffix");
  
  my $needsRebuild = 0;

  if ($objTime == -1 or $objTime <= $codeTime){
    $needsRebuild = 1;
    if ($girlie){
      print $colourGirlie."You have rearranged $filename.$codeSuffix now I ".
      "need to recompile $filename.$objectSuffix!$colourNormal\n";
    }
  }


  my $path = $fileMapping{$filename.".$codeSuffix"};
 
  %recurivePassedFiles = ();
  my @includeList = parseFileCached($filename.".$codeSuffix");

  for my $incFile (@includeList){
    if ($incFile =~ /\#include\s+\"\s*(\S+)\s*\"/){
      $incFile = $1;
      if (!exists($fileMapping{$incFile})){
        die $colourError."Could not find $incFile$colourNormal\n";
      }
      my $incTime = getTime($fileMapping{$incFile}.$incFile);
      if ($incTime >= $objTime){
        $needsRebuild = 1;
        if ($girlie){
          print $colourGirlie."You keep changing $incFile now I ".
          "need to recompile $filename.$objectSuffix!$colourNormal\n";
        }
        last();
      }
    }
  }


  if ($needsRebuild){
    %recurivePassedFiles = ();
    my @linkList = parseFileCached($filename.".$codeSuffix");
    my $linkLine = "$filename.$objectSuffix";
    my %linkMap = ();
    my $myCFlags = "";
  
    for my $item (@linkList){
      if ($item =~ /^#cflags\s+([^\n^\r]+)/){
        $myCFlags = $myCFlags." $1";
      }
    }

    $rebuildOFiles{$filename}=0;
    $rebuildOFilesArguments{$filename}="$myCFlags $incDirs";
    #buildObjectFile($filename);
  }
}

#builds an exe file
#first argument is the file without a path and without suffix
sub buildExeFile
{
  my $filename = $_[0];
  my $arguments = $rebuildExeFilesArguments{$filename};

  if ($outputDir eq ""){
    $outputDir = $buildDir;
  }
  my $command = "$linker -o $outputDir$filename$exeSuffix $arguments";
  
  print $colourAction."Linking $filename$exeSuffix$colourNormal\n";
  
  if ($showLinkerCommand == 1){
    print "$command\n";
  }
  `$command`;
  if ($? != 0) {
    print $colourError."Linking of $filename$exeSuffix failed$colourNormal\n";
    if (!$showLinkerCommand){
      print "$command\n";
    }
    unlink($filename.$exeSuffix);
      die $colourError."   Sorry   $colourNormal\n";
  }
  &$postProcessingRef($outputDir.$filename."$exeSuffix");
  $timeStamps{$outputDir.$filename."$exeSuffix"} = -2;
}

#finds the files required for an output file, from a file + path without suffix
sub findBuildFilesForFile
{
  my $filename = $_[0];

  findObjectFiles($filename);
  

  my $exeTime = getTime($outputDir.$filename."$exeSuffix");
  my $objTime = getTime($buildDir.$filename.".$objectSuffix");
  
  my $needsRebuild = 0;

  if ($exeTime == -1 or $exeTime <= $objTime){
    $needsRebuild = 1;
    if ($girlie){
      print $colourGirlie.
      "Uhh $filename$exeSuffix is older than ".
      "$filename.$objectSuffix relinking will be required $colourNormal\n";
    }
  }

  %recurivePassedFiles = ();
  my @linkList = parseFileCached($filename.".$codeSuffix");
  my $isExe = 0;
  my $linkLine = "$buildDir$filename.$objectSuffix";
  my %linkMap = ();
  $linkMap{"$filename.$objectSuffix"} = " ";
  my $myLdFlags = $ldflags;
  #filter list to contain only valid data
  my @filterList;
  for my $item (@linkList){
    if ($item =~ /^#link\s+(\S+)\s*/){
      my $oFile = $1;
      my $objFile = $oFile.".$objectSuffix";
      if (!exists($linkMap{$objFile})){
        $linkMap{$objFile}=" ";
        findObjectFiles($oFile);
        $objTime = getTime($buildDir.$objFile);
        if ($exeTime <= $objTime){
          $needsRebuild = 1;
          if ($girlie){
            print $colourGirlie."Do you like this new object file $objFile?".
            " Well lets relink $filename$exeSuffix$colourNormal\n";
          }
        }        
        $linkLine = $linkLine." $buildDir$objFile"
      }
    }
    if ($item =~ /^#ldflags\s+(.*)\s*/){
      $myLdFlags = $myLdFlags." $1";
    }
    if ($item =~ /^#exe/){
      $isExe = 1;
    }
  }

  if ($needsRebuild and $isExe){
    $rebuildExeFilesArguments{$filename}=" $linkLine $myLdFlags";
    $rebuildExeFiles{$filename}=-1;
    #buildExeFile($filename);
  }
}
    

#ensures that the build directories exists
sub buildDirTest
{
  my @fileStatus = stat($buildDir);
  if (scalar(@fileStatus) == 0){
    print $colourAction."Creating build dir$colourNormal\n";
    mkdir($buildDir);
  }
  
  if ($outputDir eq ""){
    $outputDir = $buildDir;
  }
  @fileStatus = stat($outputDir);
  if (scalar(@fileStatus) == 0){
    print $colourAction."Creating output dir$colourNormal\n";
    mkdir($outputDir);
  }
}

#tries to find the files that needs rebuild. For each file it finds that
#matches the list of user requested files it calls findBuildFilesForFile.
sub scanForRebuildFiles
{
  #gennerate a regular expression that matches the names the user requested to
  #have build
  my $match = makeMatchString();
  
  #iterate over all known files
  foreach(keys(%fileMapping)){
    if ($_ =~ /^($match)$/){
      my $theFile = $_;
      $theFile =~ s/\.$codeSuffix$//;
      findBuildFilesForFile($theFile);
    }
  }

}


#the threaded sub that builds the .o files !!! THREAD WARNING !!!
sub threadBuildFiles
{
  my $done = 0;
  while (!$done){
    my $file;
    $globalMutex->down;
    if ($nextBuildJob == $numberOfOFiles){
      $done = 1;
    }
    else {
      $file = $sortedBuildJobs[$nextBuildJob];
      $nextBuildJob++;
    }
    $globalMutex->up;

    if (!$done){    
      buildObjectFile($file);
    }
  }
}

#Finds the #global_cflags in the requested build taregets and atts them to $cflags
sub getGlobalFlags
{
	#gennerate a regular expression that matches the names the user requested to
  #have build
  my $match = makeMatchString();
  
	my %flagsFound = ();

  #iterate over all known files
  foreach(keys(%fileMapping)){
    if ($_ =~ /^($match)$/){
      my $theFile = $_;
			my @tokens = parseFileCached($theFile);
			foreach my $token (@tokens){
				if ($token =~ /#global_cflags\s+(.*)/){
					my $flag = $1;
					$flagsFound{$flag}=1;
					printGirlie("Oh Look in '$theFile' I found the global flag '$flag'\n") 
				}
			}
    }
  }
	my $globalFlags = join(' ', keys(%flagsFound));
	$cflags = "$globalFlags $cflags";
}

#builds the files requested
#Uses scanForRebuildFiles to find the .o files that needs to be rebuild
#Uses threadBuildFiles to perform the actual build
#Then uses scanForRebuildFiles to find the files that need to be relinked 
sub buildFiles
{
  print $colourAction."Finding files needing to be rebuild$colourNormal\n";

  scanForRebuildFiles();
  foreach (keys %rebuildOFiles){
    my $file=$_;
    my $path = $fileMapping{"$file.$codeSuffix"};
    #Get the modifications dates of the files that needs til be
    #rebuild (in order to re build the latest modifications fisrt)
    $rebuildOFiles{$file}=getTime("$path$file.$codeSuffix");
  }

	#Call the pre compile function just before we start to build files
	&$beforeCompileRunRef();
  my $buildMessage = "Building files";
  if ($numberOfThreads != 1){
    $buildMessage .= " (multi threaded using $numberOfThreads threads)";
  }
  print $colourAction.$buildMessage."$colourNormal\n";

  #sort the joblist by modification time
  @sortedBuildJobs = sort {$rebuildOFiles{$b} cmp  $rebuildOFiles{$a}}keys%rebuildOFiles;
  $numberOfOFiles=$#sortedBuildJobs+1;
  $compileStartTime = time();

  #create threads for building O files
  for (my $i=1; $i!=$numberOfThreads; $i++){
    my $thread = threads->new(\&threadBuildFiles);
    push @threads, $thread;
  }
  
  #do the actual building of the files for the main thread
  threadBuildFiles();
  
  #wait for all threads to terminate
  for my $thread (@threads){
    $thread->join;
  }
  
  if ($buildFailed){
    die $colourError."   There were errors!!!   $colourNormal\n";
  }
  
  scanForRebuildFiles();
  print $colourAction."Linking files$colourNormal\n";
  for my $file (keys %rebuildExeFiles){
    buildExeFile($file);
  }

}


sub testFiles
{
  if ($outputDir eq ""){
    $outputDir = $buildDir;
  }  
  for my $file (@targets){
    print $colourAction."Testing $file$colourNormal\n";
    print `$testProgram$outputDir$file $testArguments`;
  } 
}

#fixes variables so that they get there default values
sub fixVariablesWithDefaults
{
  if ($numberOfThreads == 0 and $useDistcc){
    my $distccSettings = $ENV{"DISTCC_HOSTS"};
    if (defined($distccSettings)){
      $distccSettings =~ s/^\s+//;
      $distccSettings =~ s/\s+$//;
      my @hostArray = split /\s+/, $distccSettings;
      $numberOfThreads = $#hostArray+1;
    }
  }
  
  if ($numberOfThreads < 1){
    $numberOfThreads = 1;
  }
  
  if ($outputDir eq ""){
    $outputDir = $buildDir;
  }
}

#removes ALL files from a directory, use with extreme care!
#first arg is the path of the dir
sub purgeDir
{
  my $dirName = $_[0];
  my $dir;
  if (!opendir ($dir, $dirName)) {
    print "Can't find directory $dirName, unable to clean it";
  }
  else {
    my @entrys = readdir $dir;
    
    foreach(@entrys){
      if ($_ !~ /^\.+$/){
        #it was not .. or .
        unlink($dirName."/$_");
      }
    }
    closedir $dir;
  }  
}

sub updateCheckForTargetCleanCache {
	my $targetCache = $buildDir."__lastTarget.cache";
	my $newTarget = join(',', @targets);
	my $targetCacheFile;

	open($targetCacheFile, ">$targetCache") || printFatal("Can't write to '$targetCache'\n");
	print $targetCacheFile $newTarget;
	close($targetCacheFile);
}

sub checkForTargetClean {
	my $targetCache = $buildDir."__lastTarget.cache";
	my $newTarget = join(',', @targets);
	my $oldTarget = '';
	
	my $targetCacheFile;
	local $/; # enable localized slurp mode
	
	if ($newTargetClean){
		if (open($targetCacheFile, "<$targetCache")){
			$oldTarget = <$targetCacheFile>;
			close($targetCacheFile);
		}

		if (!($oldTarget eq $newTarget)){
			printWarning("Target changed from '$oldTarget' to '$newTarget', clean forced!\n");
			$doClean = 1;
		}
	}
	updateCheckForTargetCleanCache();
}

#====================== Start of main function below this point =======================

readConfigFile("localbuild.pl");

#Patch the exe file suffix with dot
if (!$exeSuffix eq ""){
    $exeSuffix = ".$exeSuffix";
}

#$target can have been assigned a value by localbuild.pl (simple assignment or by calling autoTaget())
#$target is stored in the environment variables allowing external code to use the information if needed
$ENV{"target"}=$target;

if (!$argumentsRead){
  parseArguments();
}

if (!$useColours){
  $colourVerbose = "";
  $colourNormal = "";
  $colourError = "";
  $colourAction = "";
  $colourExternal = "";
  $colourWarning = "";
  $colourGirlie = "";
}

readModuleList();
#.auto files may generate new files if any is seen a file rescan is requested
if ($rescanFiles){
  %fileMapping=();
  readModuleList();
}

fixVariablesWithDefaults();

#ensure that the build directories exists
buildDirTest();

checkForTargetClean();

#Code for handling the cleaning of the output and build directories
if ($doClean == 1){
  my $proceedWithClean = 1;
    
  if ($outputDir eq ""){
    $outputDir = $buildDir;
  }

  
  if ($outputDir eq $buildDir){
    print $colourWarning."Cleaning build directry '$buildDir'";
  }
  else {
    print $colourWarning."Cleaning directries '$buildDir' and '$outputDir'";
  }
  if ($cleanConfirm){
    print ", are you sure? Yes or no: ";
    my $answer = <STDIN>;
    if (!($answer eq "Yes\n")){
      $proceedWithClean = 0;
      die $colourWarning."Clean aborted$colourNormal\n";
    }
    else {
      print "Proceeding with clean action"
      
    }
    print "$colourNormal\n"
  }
  else {
    print "$colourNormal\n";
  }
  
  if ($proceedWithClean){
    purgeDir($buildDir);
    purgeDir($outputDir);
  }
  updateCheckForTargetCleanCache();
}

if ($doBuild == 1){
	getGlobalFlags();
  buildFiles();
}
if ($doTest){
  testFiles();
}
