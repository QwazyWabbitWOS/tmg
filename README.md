### What is this repository for? ###

* This is TMG for Quake II, Railwarz CTF with Zigbot support.
* Version 2.21

### How do I get set up? ###

* On Linux, clone the repo and 'make all' then copy the dynamic library to your quake2/tmg folder. 
* The code compiles on Linux and OS X as a 32-bit shared library. Run it with the 32-bit Q2 engine. No guarantees about portability to 64-bits. Performance may actually be impaired since this code is so ancient that 64 bits might actually slow it down.
* Use the make file to build on Linux/OS X.
* Use the VS2010 project to build on Windows.
* TMG is CTF by default and can be played with or without player bots.
* Configuration: The usual Quake2 setup for your platform.
* Dependencies: None.
* Database configuration: none

### Contribution guidelines ###

* Writing tests: If you write tests for this, please contribute them. 
* Code review: currently a work in progress. There were several portability issues.
* If you spot a bug or some feature doesn't work, please describe the problem and the platform(s) you're testing on and how to reproduce the bug.
* Transition to VS2015 is feasible. 

### Who do I talk to? ###

* Josh Waggoner is repo owner. 
* Contact QwazyWabbit for current status or bugs. Post issues and I'll see what I can do. (QW)

* [Learn Markdown](https://bitbucket.org/tutorials/markdowndemo)