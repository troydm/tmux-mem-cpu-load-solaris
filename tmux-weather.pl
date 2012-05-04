#!/usr/bin/env perl

use strict;
use warnings;

# This is a tmux weather applet that uses 
# World Weather Online free weather api
# go to 
# http://www.worldweatheronline.com/feed-generater.aspx
# and create a request url for your city using csv format

# url generated using feeder page
my $url = '';
# interval to refresh weather info in seconds
my $interval = 21600;
# temporary file to save weather data into
my $temp = '/tmp/current_weather';

sub get_weather {
    open(PS,"curl -s \"$url\" |") or die "couldn't get weather data";
    open(WD,">$temp") or die "can't write into temporary file";
    print WD time()."\n";

    while(<PS>){
        if(!m/^#/){ 
            print WD $_;
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

my @weatherData = split(/,/,$weather);

my $tempreture = $weatherData[1];
my $condition = $weatherData[4];

close(WD);

my $symbol = '☀ ';

if($condition =~ /rain/i){
    $symbol = '☂ ';
}elsif($condition =~ /shower/i){
    $symbol = '☔ ';
}elsif($condition =~ /sun/i){
    $symbol = '☀ ';
}elsif($condition =~ /cloud/i){
    $symbol = '☁ ';
}elsif($condition =~ /snow/i){
    $symbol = '☃ ';
}

print $symbol.$tempreture . '°C ' . $condition;
