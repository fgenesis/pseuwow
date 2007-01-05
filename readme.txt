####################################################################################
################################# - PseuWoW ReadMe - ########## 09.11.2006 #########
####################################################################################
##                                                                                ##
## Web: http://my.opera.com/PseuWoW                                               ##
## Email/MSN: hoden386@lycos.de                                                   ##
##                                                                                ##
## 1.) What is PseuWoW?                                                           ##
## 2.) Installation & configuration                                               ##
## 3.) The DefScript engine [to be updated]                                       ##
## 4.) The controller [*availible soon*]                                          ##
## 5.) Interaction with PseuWoW                                                   ##
## 6.) Compiling your own version                                                 ##
## 7.) A word to Blizzard                                                         ##
## ...                                                                            ##
## 8.) Appendix: [*availible soon*]                                               ##
##   A) The DefScript interface                                                   ##
##   B) Macro reference                                                           ##
##   C) Function reference                                                        ##
##                                                                                ##
##                                                                                ##
####################################################################################
####################################################################################


####################################################
########## 1.) What is PseuWoW? ####################
####################################################

A custom wow client, or at least it is supposed to be a "real" client later on.
For now it is thougt to be a bot that can help GMs/admins to manage their *private(!)* WoW server.
It does not need an installation of real WoW to run, and you do not need any
good hardware to run it, since it has no graphics (yet!) and not such extensive
memory usage like the bloated WoW.exe has.
it uses only the console for now, but since it runs on SDL, you *could* easily build up
your own gui using OpenGL. buts it's not designed for that purpose in this early state.

DO NOT use this program to connect to official servers!
It does NOT work due to Blizzard's IP-Spoof filter, which PseuWoW cant bypass yet.
( YES this IS intended to be that way! For your own safety and for the legality of PseuWoW!)
If you should try anyway, the only thing you can win is the ban/loss of your account.



Performance:
============
Tested Build 8 on a 300MHz Intel Celeron notebook, took a bit longer to Auth then on a
3GHz Pentium 4 PC, but worked well so far; more benchmarks on slower hardware will follow =)



####################################################
########## 2.) Installation & Configuration ########
####################################################

Required DLL files:
===================

- SDL.dll
- SDL_net.dll
- msvcp71(d).dll
- msvcr71(d).dll

Be sure you have them put into the same folder as PseuWoW.exe (or your windows/system32 folder).

How to make it run properly (main configuration):
=================================================

Open the file "conf/PseuWoW.conf" in a text editor and edit it to your needs.
There should be comments inside the file explaining what the options are good for.

User permission configuration:
==============================

Open the file "conf/users.conf".
For every player you want to assign a permission level to enter the name of the player (note correct spelling!!),
followed by a "=" and the permission level ( a number from 0 - 255; 0=least; 255=all permissions ).
The lines should look like this:

Master=255
Operator=100
Playerx=1
...

If a player is not listed in this file he will get permission 0 automatically.

Note that special unicode chars in playernames like óäöüßêýà etc. DO NOT work.


Saving the cache:
=================
If you close PseuWoW using the [X] in the top-right corner it will be instantly closed
and NO data will be saved. To save the data you can either quit PseuWoW pressing Ctrl+C,
giving the "-quit" command ingame or just save without quitting giving the ingame command
"-savecache". (explained further down this file)



####################################################
########## 3.) The DefScript engine ################
####################################################

Information:
============

PseuWoW uses a scripting language exclusively designed for it - i admit i didn't know
how to implement Python or LUA :P
DefScript has no controlling structures like IF, FOR, WHILE, etc. yet, this will come in future.

The Scripts have the extension ".def" and are stored in the "/scripts/" folder of PseuWoW.
Every file represents a single function.

Syntax:
=======
A typical function call looks like this:

command,arg1,arg2,arg3,argx defaultarg

command: name of the script or function you want to execute (UPPER- or lowercase does not matter)
arg1 - argx: OPTIONAL(!) arguments, seperated by commas.
defaultarg: the last ( or better called "typical" ) argument of the function,
seperated by [space].
Everything after the first space char will be treated as defaultarg!
To prevent this, you can use curly brackets {} to express that the given text is connected.
e.g.:
"SENDCHATMESSAGE,6,{hi there},Keks" -> correct, works. Whispers "hi there" to player "Keks".

"SENDCHATMESSAGE,6,hi there,Keks"
-> works too, BUT:
"SENDCHATMESSAGE,6,hi" -> this will be the command with its args
"there,Keks" -> this will be the defaultarg
result: the call will not produce the expected result!


Variables:
==========

Setting variables:
"x = 1" -> WRONG! DefScript knows only functions!!
You have tu use this:
"SET,x 1" -> sets the LOCAL variable x to 1. [x will be stored as "scriptname::x" internally]
"SET,#x 1" -> sets the GLOBAL variable x to 1. [#x will be stored as "x" internally]
"SET,#welcomemsg PseuWoW logged on, be careful!" -> sets the GLOBAL variable "welcomemsg" to PseuWoW logged on, be careful!".
Note that the variable name is the first arg, preceded with a comma, and its value is the defaultarg.
Variable names with a preceding "#"  will be stored as GLOBAL vars, with the preceding "#" removed.

"DEFAULT,x 1" -> sets x only if it has not been set or x has no value (=is empty)
"UNSET x" -> removes the variable. you have to remove every variable you do no longer need yourself!


A special kind of variables are macros, which values can only be changed by the program itself.
Macros begin with "@".

Accessing variables:
====================

SET,x 1
SET,mystr hello world
OUT ${mystr}, x has the value ${x}
PAUSE 1000
SET,mystr ${mystr} goodbye
OUT ${mystr}

pretty obvious:
${mystr} gets replaced with "hello world"
${x} gets replaced with 1.
and the text "hello world, x has the value 1" will be written to the console.
after one second, mystr will be set to "hello world goodbye" and again written to console.

accessing globals: OUT ${#myglobal}
accessing macros: OUT ${mymacro}

[[* more explanations of the variable system will follow *]]

Load Instructions:
==================
load instructions begin with "#" and are put at the beginning of every script.
You can not use variables in them!
The following are availible:
#permission=x   -> sets the permission required to execute this script. 0 by default! x=0...255
#debug          -> produce more output
#load_debug     -> lists every line that is loaded
#load_notify    -> notifies you once the script has been loaded and is ready for use
...more to come...





####################################################
########## 5.) Interaction with PseuWoW ############
####################################################


You can either say or whisper the command to PseuWoW; put a "-" before the command name
that PseuWoW recognizes the chat message as a command, like:
"-say hello"
"-say,orcish hello"
"-say,8 hello"
"-emote 12"
"-yell LOL"
"-yell,darnassian LOL"
"-savecache"
"-follow Keks"
"-quit"
etc.

There is also some VERY basic chat AI, say hi,lol,omg,wtf,download,... when PseuWoW will hear you and it will respond :)
A full (and useful) chat AI will be implemented later.

All trade and party invite rquests are automatically declined.
Note the "_onwhisper" script that is run everytime a whisper is recieved.




####################################################
########## 6.) Compiling your own version ##########
####################################################

- i used Microsoft Visual Studio.NET 2003 for compiling
- download the include/lib package (see URL on top). copy the content into the
  include/lib directory of the compiler.
- if needed, copy the reqired DLL files into the /bin directory (where the project files are)
- open the project and compile.



####################################################
########## 7.) A word to Blizzard ##################
####################################################

I always liked blizz... until WoW came out.
The management is shitty, almost no customer support, and bans in case they dont know what
to do with players.
Blizzard programmers and designers, YOU ARE GREAT and you made great games before
Blizzard got eaten up by vivendi universal. KEEP UP THE GOOD WORK!
After that, vivendi management fucked it all up. the result is WoW, a always-pay game like
Diablo3 will probably be.
So here are my greetings to the vivendi management:






                         Eat shit and die, bitches!

                                        _gm__gz,
                                      ,Z\/`   'c\.
                                      /,!      '[W
                                     ]!] .      []
                                     ] ] .-==-  b]
                                     ]i ~ -==== !]-
                                     ]    ~~    -![
                                     W           d`
                                    'b           W
                                    i[          'M
                                    ].         ~ 4.
                                    ]           -'[
                                    d          -= b
                                    M!            @
                                    @           `-D
                                   ]@           -]A
                                   @P   , ,   .  '@
                                   M|   ~`  '`  /]@
                       _._.        Y            ciP
                    ,g/G/~Vi       ![          '-V[
                  ,Z~_/`- b@_      ,[          -` [
               _v4Tv/ .m/TGs,2V~-i ]           \/.[
            ,vf     ,(f       `'iY.A           / -[,z --=-__
         ,gY`      v)`         \-.@!          ,--]Wf   =/`,~N
      ,gY`        id           / b@[          -/ ]M         (b.
    vf|         - t[          !,i@P!          '  ][.       /`YZ~V,c.
  if, '       , ,(]            ]'@`              ]b        `'/@. `'*W.
  K          ,)`-'Z   _     _ -/i@ i           ' @Y          |4t    /Vmv.
  b          gz/-Z`'` @   - ]  -P] `  ,     i   ,!W           ]@    ',/M.-
  [        gZ\  ]!    *  i[ !  Z!d,   ]-   [W     ~          \ ]       X8.\
 ,[       '7v`  A        'i `  - !'   ]    [@./   ,           ,]        4/c-
 ]        !i- `][           ,]        '    e!]    i    - . m   !        'W =
 W        s-=` ][           !A              /-          ]  W=            ][!
 A        -Z-  @-           -!              P             ,A,    ,    ]  ,@
 [        '- ,/4.                          !'             Z`      [ . W g`/[
 Y.-        '  /[             .                         ,/            !.'c/b-
  Vi'-        ' !             !                         /             ' `` @[
   'e.                        ]         ,     .                     v      @[
     Vs.                      '.                        .-           `   / @`
      '5c.                     \             !         ,                 --@.
        \Zs  `-.               '-       \             -                   -A
          '\.   - .             '      '     -        `         .-, =    'i[
            'e.   -'         -           ,         -` ,/      !vi\ v     i*`
              Vs   -!                    -- '        v`     ,'.-'/     _/`\`
               'G.   '           !      '-          /      '.c\,`    vfm/ -
                 !i   `          -.    ,`    `  ,          `'c    ,z7!.` -
                  Yi  ' -         !  . ,``  -       -      !`   ,zL'`
                   b    i     , -  - '  -   ,        - -      ,Z|!`
                   Y    `      \-   ,  ,      '- `     ,  . ,Z7
                   8    --     .      `           -` .'.-  -P
                   ]     ]       -,      .           -  ,  ,[
                   ].    Y-    '                       /   8
                   i|               ,'   -       `  . - '  W
                           `     ,      -  -    ,   ,      K



(c) SnowStorm Software, 2006
(and the finger belogs to SSG cracking group :P )

