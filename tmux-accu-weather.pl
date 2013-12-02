#!/usr/bin/env perl

use strict;
use warnings;

# This is a tmux weather applet that uses 
# Accu Weather Web page
# go to 
# http://www.accuweather.com
# and click on your local weather to get url

# url for extracting weather data
my $url = 'http://www.accuweather.com/en/ge/tbilisi/171705/weather-forecast/171705';
# interval to refresh weather info in seconds
my $interval = 3600;
# temporary file to save weather data into
my $temp = '/tmp/current_accu_weather';

sub get_weather {
    open(PS,"curl -s \"$url\" |") or die "couldn't get weather data";
    open(WD,">$temp") or die "can't write into temporary file";
    print WD time()."\n";

    my $next = 0;
    while(<PS>){
        if($next){
            my $cond = '';
            my $deg = '';
            if($_ =~ m/cond\"\>([^\<]*)\</){
                $cond = $1;
            }
            if($_ =~ m/temp\"\>([^\<]*)\</){
                $deg = $1;
            }
            print WD "$cond $deg";
            $next = 0;
        }
        if(m/\<h3 class="ac"\>Currently/){ 
            $next = 1;
        }
    }

    close(WD);
    close(PS);
}

if(!open(WD,$temp)){
    get_weather;
    open(WD,$temp) or die "can't open weather temporary data";
}

my $t = int(<WD>);

if((time()-$t) >= $interval){
    close(WD);
    get_weather;
    open(WD,$temp);        
    $t = int(<WD>);
}

my $weather = <WD>;

my $tempreture = $weather;
my $condition = $weather;

close(WD);

my $symbol = '☁ ';

if($condition =~ /shower/i){
    $symbol = '☔ ';
}elsif($condition =~ /rain/i){
    $symbol = '☂ ';
}elsif($condition =~ /sun/i){
    $symbol = '☀ ';
}elsif($condition =~ /cloud/i){
    $symbol = '☁ ';
}elsif($condition =~ /snow/i){
    $symbol = '☃ ';
}

print $symbol.$tempreture . '°C ';
