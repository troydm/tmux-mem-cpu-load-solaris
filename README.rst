====================================================
            tmux-mem-cpu-load-solaris
====================================================
----------------------------------------------------
 CPU, RAM memory, and load monitor for use with tmux_
----------------------------------------------------

A simple tmux applet that prints system monitoring information
on solaris operating system using kstat.

It's inspired by tmux-mem-cpu-load_ that is for linux systems

Installation
============
* make
* sudo make install

License
============
Licensed under GNU LESSER GENERAL PUBLIC LICENSE version 3 (see LICENSE file)

Usage
========
    
tmux-mem-cpu-load [options]
::
    -mem - disables memory output
    --interva=n - where n is number of seconds between cpu load measurement
    --graphs=n - where n is number of graphs to use (if n=0 detailed cpu load information is displayed instead of simple graphical output)
    -load - disables load average output

Configuring tmux_
=======================

Edit your ``$HOME/.tmux.conf`` to display the program's output in *status-left* or *status-right*.  For example::
    
    set -g status-interval 2
    set -g status-left "#(tmux-mem-cpu-load --interval=2 --graphs=10)#[default]"

Note that --interval=n to tmux-mem-cpu-load should be the same number of seconds that status-interval is set at.

My configuration
::
    set -g status-interval 2
    set -g status-right-length 100
    set -g status-right "#[bg=colour189] #(tmux-vbox-vm-load.pl) #(tmux-mem-cpu-load -load --interval=2 --graphs=20)  %m/%d/%Y %H:%M "

Author
======

Dmitry Geurkov (troydm) <d.geurkov@gmail.com>

.. _tmux: http://tmux.sourceforge.net/
.. _tmux-mem-cpu-load: https://github.com/thewtex/tmux-mem-cpu-load
    
