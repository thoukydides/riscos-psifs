#   File        : clean.pl
#   Date        : 19-Sep-02
#   Author      : © A.Thoukydides, 1998-2002, 2019
#   Description : Clean the directory structure ready for release.
#
#   License     : PsiFS is free software: you can redistribute it and/or
#                 modify it under the terms of the GNU General Public License
#                 as published by the Free Software Foundation, either
#                 version 3 of the License, or (at your option) any later
#                 version.
#   
#                 PsiFS is distributed in the hope that it will be useful,
#                 but WITHOUT ANY WARRANTY; without even the implied warranty
#                 of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
#                 the GNU General Public License for more details.
#   
#                 You should have received a copy of the GNU General Public
#                 License along with PsiFS. If not, see
#                 <http://www.gnu.org/licenses/>.

# Use strict checking
use strict;

# Import useful packages
use FileHandle;
use Getopt::Std;
use RISCOS::Filespec;
use RISCOS::File ('gettype', 'settype');

# Process options
use vars qw($opt_d);
getopts('d');

# Construct date string in the format "dd-mmm-yy"
my ($mday, $mon, $year) = (localtime(time))[3.. 5];
my $date = sprintf '%02d-%s-%02d', $mday, ('Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec')[$mon], $year % 100;
my $date_pattern = '\d\d-\w\w\w-\d\d';

# Perform procesing recursively starting from directory containing the script
recurse($0 =~ /^(.*)\./g);
exit;

# Subroutine to recursively process directory structure
sub recurse
{
    my ($dir) = @_;
    
    # Process all objects within the directory
    foreach (glob "$dir.*")
    {
        if (-d $_)
        {
            # Suppress processing of special directories
            next if /\.(Extra|Old)$/i;
            next if /\.(!ChangePSI|!Psionconv|!SIBOConv)$/i;
            next if /\.!PsiFS\.(Docs|Resources\.Modules)$/i;
        
            # Recurse through directories
            recurse($_);
        }
        elsif (/\.[~o]\./i
               or /\.(tmp|ViaFile)$/i
               or /\/bak$/i
               or /\.(h|s|Hdr)\.(gdraw|module|psifs)$/i)
        {
            # Delete backup, automatically generated, and object files
            print "Deleting '$_'\n";
            unlink;
        }
        elsif (/\.(MakeFile|ReadMe|Messages|BuildDocs)$/i
               or /\.(!PsiFS|!Boot|!Help|!Run|Configure|Command|Desktop|Load)$/i
               or /\.(FindFSI|Capture|Convert|Install|Print)$/i
               or /\.(c|c\+\+|h|s|Hdr|cmhg|swi)\./i
               or /\/hsc$/i)
        {
            # Suppress date setting unless enabled
            next unless $opt_d;
            
            # Set current date in file
            print "Updating '$_'\n";
            edit($_);
        }
    }
}

# Subroutine to edit a file
sub edit
{
    my ($orig) = @_;
    my $temp = '<Wimp$Scrap>';
    my $in = (new FileHandle);
    my $out = (new FileHandle);
    
    # Open the input and ouptut files
    open $in, "<$orig";
    open $out, ">$temp";
    
    # Process every line
    while ($_ = <$in>)
    {
        # Update the date
        s/(Date\s+: )$date_pattern$/$1$date/o;
        s/\($date_pattern\)$/($date)/o;
        
        # Write the modified line to the output
        print $out "$_";
    }
    
    # Replace the original file, preserving filetype
    close $in;
    close $out;
    settype gettype($orig), $temp;
    unlink $orig;
    rename $temp, $orig;
}
