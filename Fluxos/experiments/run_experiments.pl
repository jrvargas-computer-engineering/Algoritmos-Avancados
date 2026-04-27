#!/usr/bin/perl
use strict;
use warnings;
use POSIX qw(strftime); # Import the time formatting function
use Getopt::Long; 

# --- CLI Arguments ---
my $run_density = 0;
my $run_scaling = 0;
my $run_all     = 0;
my $help        = 0;

GetOptions(
    "density" => \$run_density,
    "scaling" => \$run_scaling,
    "all"     => \$run_all,
    "help|h"  => \$help
) or die "Error parsing command line arguments. Try --help\n";

if ($help) {
    print "Usage: ./run_experiments.pl [options]\n";
    print "Options:\n";
    print "  --density   Run only the Density Impact experiment\n";
    print "  --scaling   Run only the Size Scaling experiment\n";
    print "  --all       Run both experiments\n";
    print "  --help, -h  Show this help message\n";
    exit;
}

if (!$run_density && !$run_scaling && !$run_all) {
    print "No specific flags provided. Defaulting to running ALL experiments...\n";
    $run_all = 1;
}


my $timestamp = strftime "%Y%m%d_%H%M%S", localtime;
# --- Configuration ---
my $output_csv = "../experiments/reports/final_complexity_report_$timestamp.csv";
my $temp_file = "temp_graph.max";
my $generator = "./washington";
my $solver = "./FlowExperiment";
my $capacity_range = 10000;
my $header_written = 0;

die "Error: $solver not found. Run 'make experiments' first.\n" if (! -e $solver);
die "Error: $generator not found. Run 'make experiments' first.\n" if (! -e $generator);

my $file_mode = ($run_all) ? '>' : '>>';
open(my $out_fh, $file_mode, $output_csv) or die "Could not open '$output_csv': $!";

print "==================================================\n";
print " Starting Maximum Flow Complexity Experiments\n";
print "==================================================\n\n";

my %graph_families = (
    6 => "BasicLine",
    8 => "DoubleExpLine"
);

foreach my $type (keys %graph_families) {
    my $family_name = $graph_families{$type};
    print ">>> Testing Graph Family: $family_name (Type $type) <<<\n\n";

    # FIXED: Moved dim1 here so both Experiment A and B can use it!
    my $dim1 = 2; 

    # =================================================================
    # EXPERIMENT A: DENSITY IMPACT
    # =================================================================
    if ($run_density || $run_all) {
        print "  [Experiment A] Density Impact (Fixed n ~= 1000)\n";
        my $dim2_fixed = 2500; 
        my @degrees = (2, 5, 10, 20, 40); 

        foreach my $deg (@degrees) {
            print "    -> Generating: n~=1000, deg=$deg...\n";
            system("$generator $type $dim1 $dim2_fixed $deg $capacity_range $temp_file");
            my @results = qx($solver --benchmark < $temp_file);
            process_results(\@results, $family_name, "Density");
        }
        print "\n";
    }

    # =================================================================
    # EXPERIMENT B: SCALING IMPACT
    # =================================================================
    if ($run_scaling || $run_all) {
        print "  [Experiment B] Scaling Impact (Fixed density deg = 5)\n";
        my $deg_fixed = 10;
        my @dim2_values = (500, 1000, 2500, 5000, 10000);

        foreach my $dim2 (@dim2_values) {
            print "    -> Generating: dim2=$dim2 (Increasing n), deg=5...\n";
            system("$generator $type $dim1 $dim2 $deg_fixed $capacity_range $temp_file");
            my @results = qx($solver --benchmark < $temp_file);
            process_results(\@results, $family_name, "Scaling");
        }
        print "\n";
    }
}

close($out_fh);
unlink $temp_file if -e $temp_file;

print "==================================================\n";
print " Experiments Complete! Data saved to: $output_csv\n";
print "==================================================\n";

sub process_results {
    my ($results_ref, $family, $exp_type) = @_;
    
    if (-s $output_csv > 0 && !$header_written && $file_mode eq '>>') {
        $header_written = 1; 
    }

    foreach my $line (@$results_ref) {
        if ($line =~ /^Algorithm,/) {
            if (!$header_written) {
                print $out_fh "Family,ExpType," . $line;
                $header_written = 1;
            }
        } 
        elsif ($line =~ /^\w+,/) {
            chomp $line;
            print $out_fh "$family,$exp_type,$line\n";
        }
    }
}