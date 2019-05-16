This program is conceived for having 2 publisher publishing the following
message lower_case_letter+publisher_id sent to the mediator through a pipe

The mediator stores the data into the corrensponding circular queue

The 3 subscribers at each cycle ask for one of the datas published to which they
are subscribed

One possible output could be

sub0: a0
sub1: b0
sub2: a1
sub0: c0
sub1: b1
sub2: c1

The architecture is hard-coded this means that the number of publishers, the number
of subscribers, the periods of publishers, mediator and subscribers and the subscriptions
are all declared in the init.cpp

But actually it is thought to be easily made dynamic with very few modifications in the init.cpp
where all the datas can be loaded from a configuration file, moreover since in the pipes the
length of the message is transferred before the message itself this architecture can be used also
to send and print strings of whichever length and pattern

One more remark, we have one LOGFILE reporting errors when present and one other for viewing the
state of the mediator's buffer

compile everything exploiting the script "make", execute using the "run" one

./make.sh
./run.sh

press ctrl+c to kill every process and exiting the program
