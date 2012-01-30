#!/usr/bin/env perl

use strict;
use warnings;

open(VM,'VBoxManage list runningvms |') or die "Couldn't open VBoxManage: $!";

my %vms = ();

while(<VM>){
    chomp;
    if(m/"(.*)" {([^-]*)-.*/){
        my ($vm,$id) = ($1,$2);
        $vms{ $vm } = $id;
    }
}

close(VM);

if( (scalar keys %vms) > 0 ){
    open(PS,'ps -ef | grep VBoxHeadless | grep -v grep |') or die "Couldn't list VBoxHeadless processes: $!";

    while(<PS>){
        chomp;
        if(m/(\d+).* --comment (.*) --startvm ([^-]*)-.*/){
            my ($vmpid,$vm,$id) = ($1,$2,$3);
            if($vms{ $vm } eq $id) { $vms{ $vm } = $vmpid; }
        }
    }

    close(PS);

    my $pidlist = join(',',values %vms);
    open(PRS,"prstat -p $pidlist 1 1 |") or die "Couldn't prstat processes";

    my %vmls = ();

    while(<PRS>){
        chomp;
        if(m/(\d+) .* (\d?\.?\d+%) VBoxHeadless/){
            my ($vmpid,$load) = ($1,$2);
            $vmls { $vmpid } = $load;
        }
    }

    close(PRS);

    print "VM:";
    while(my ($vm,$vmpid) = each %vms){
        print " $vm:$vmls{$vmpid}";
    }
}

