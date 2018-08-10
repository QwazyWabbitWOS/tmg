### What is this repository for? ###

* This is TMG for Quake II, Railwarz CTF with Zigbot support.

### How do I get set up? ###

* On Linux, clone the repo and 'make all' then copy the dynamic library to your quake2/ctf folder. 
* The code compiles on Linux and OS X as a native shared library. (32 or 64 bit depending on your platform)
* As of 0.2.35 this code compiles as 64-bit code by default and will require a 64-bit engine.
  Use setarch i386 make all to build 32-bits on x64 platforms.
* See the makefile for how to build 32-bit target on 64-bit platform.
* Run the 64-bit library with a 64-bit engine. Run the 32-bit library with the 32-bit Q2 engine. 
* Use the make file to build on Linux/OS X. (GNUmakefile and BSDmakefile provided)
* Use the VS2010 project to build on Windows.
* The binary will be named gamei386.real.dll or gamex86_64.real.so depending on platform.
  This naming convention is used to allow cascade DLL such as q2admin, usually named gamei386.so (dll).
  Use of q2admin is optional, if you don't wish to use it, rename the TMG library to gamei386.dll or gamex86_64.so
  as needed. See https://github.com/QwazyWabbitWOS/q2admin-tsmod.git for Q2admin source.
* TMG is CTF by default and can be played with or without player bots.
* Configuration: The usual Quake2 setup for your platform.
* Dependencies: none
* Database configuration: none

### Contribution guidelines ###

* Writing tests: If you write tests for this, please contribute them. 
* Code review: currently a work in progress. There were several portability issues.
* If you spot a bug or some feature doesn't work, please describe the problem and the platform(s) you're testing on and how to reproduce the bug.
* Transition to VS2015 is feasible. 

### Who do I talk to? ###

* Josh Waggoner is repo owner. 
* Contact QwazyWabbit for current status or bugs. 
* Post issues and I'll see what I can do. (QW)

* [Learn Markdown](https://bitbucket.org/tutorials/markdowndemo)