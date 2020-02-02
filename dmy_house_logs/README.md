House logs for dummies
======================

This is the code and an example setup for "House logs for dummies."
But what is that?  Typically, tech geek homeowners keep an electronic
logbook of maintenance activities in their house.  However, when you
start involving non-technical folks who are disinterested in
maintaining documentation on the state of the house, you'll find out
that it is practically impossible to involve them in participating in
the system.  Hence, we have this "dumbed down" system that only asks
for the bare minimum involvement.

Since this system is so minimalistic, the idea is that you will
combine it with additional IoT sensors to gain a more detailed picture
of what is going on.  Typically, IoT sensors can observe _what_ is
changing about the environment, but not necessarily _who_ is doing it.
Therefore, by getting folks to minimally "sign their name" that they
performed a specific task, that is all that is needed to complete the
picture that is presented by IoT sensors.

Please note that there are two versions of the main user-facing CGI
dynamic web code, one written in Perl and one in C.  The Perl version
was the original implementation.  Alas, it turned out to be
unacceptably slow on the Raspberry Pi B+, so I rewrote it in C and was
able to regain the needed performance.  For development, the Perl code
can sometimes be more convenient as it is architecture-independent, so
you can keep a single web directory rsync'ed across computer systems
running different processor architectures.

Dependencies
============

* Perl YAML::Syck
* A web server with CGI support
* A Markdown renderer such as Kramdown
