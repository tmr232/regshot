
Readme file of Regshot 1.8.2    (20071103)

-----------------
Introduction:
-----------------
Regshot is a small, free and open-source registry compare utility that allows you to quickly
take a snapshot of your registry and then compare it with a second one - done after doing
system changes or installing a new software product. The changes report can be produced in
text or HTML format and contains a list of all modifications that have taken place between
snapshot1 and snapshot2. In addition, you can also specify folders (with sub filders) to be
scanned for changes as well.
(Most of above text was copied from webattack.com,thanks :)



-----------------
Usage:
-----------------
(1)CLICK "1st shot" BUTTON
It pops up a menu which contains several items:
 (A)"Shot"  to take a snapshot only,and it will not be kept if you exit regshot program;
 (B)"Shot and save..." to take a snapshot of your registry and save the whole registry to
    a "hive" file and you can keep it in your harddisk for future use;
 (C)"Load..." to load a "hive" file previous saved.
If you want to monitor your file system ,just check the "Scan Dir [dir..]" checkbox
and input the folder names below it. Note: Regshot has the ability to scan multiple
folders,Just separate them with ";",Regshot also scan the subfolders of the current
folders you entered.

(2)RUN SOME PROGRAMS which may change your windows registry,or it may change the file system

(3)CLICK "2nd shot" BUTTON

(4)Select your output LOG file type,"text" or "HTML,default is "text"

(5)INPUT YOUR COMMENT for this action into the "comment field",eg:"Changes made after
winzip started". COMMENT will only be saved into compare log files not into "hive" files

(6)CLICK "compare" BUTTON
Regshot will do the compare job now(auto detect which shot is newer),when it is finished,
Regshot will automatically load the compare LOG as you defined above,the log files are
saved  in the directory where "Output path" is defined,default is your Windows Temp Path
,it was named as the "comment" you input,if the "comment field" is empty or invalid, the
LOG will be name as "~resxxxx.txt" or "~resxxxx.htm" where "xxxx" is 0000-9999.

(7)CLICK "Clear" BUTTON
You  will clear the two snapshots(or separately) previous made and begin a new job.
Note:"Clear" does not erase the log files!

(8)TO QUIT Regshot,just hit "Quit" button

(9)You can change the language of the regshot at main window,all words are saved in the
file "language.ini". View it for details!

(10)New to 1.7: regshot.dat now changed to regshot.ini, skipdirs and skip registry keys
are included. exe size are smaller!
(11)New to 1.8:
File shots are now saved in hive file.
"UseLongRegHead" option added in regshot.ini to compatible with undoReg(1.46)
"UseLongRegHead=1" means using "HKEY_LOCAL_MACHINE" instead of "HKLM",default 0
Do not compare shots saved with different "UseLongRegHead" option!

-----------------
Thanks:
-----------------
Special thanks:
Alexander Romanenko   -- Former space provider! http://www.ist.md/
Ivan                  -- Former space provider! http://www.digitalnuke.com/
Toye                  -- Release!
zhangl                -- Debug!
firstk                -- Debug!
mssoft                -- Test!
dreamtheater          -- Test!
Gonzalo               -- Spainish
ArLang°¢ÀÉ            -- Chinese
Mikhail A.Medvedev    -- Russian[Thanks!]
Kenneth Aarseth       -- Norsk
Marcel Drappier       -- French
Vittorio "Capoccione" -- Italian
Gnatix                -- Deutsch
Murat KASABOGLU       -- Türkçe
Paul Lowagie          -- Nederlands
ondra                 -- Èesky
AVE7                  -- Fixed Deutsch!
Pau Bosch i Crespo    -- Catalan
Michael Papadakis     -- Greek
Per Bryldt            -- Danish
Rajah                 -- Latvian
Leandro               -- Portuguese
Roland Turcan         -- Slovak
Kentaro Okude         -- Japanese
Adam Mikusiñski       -- Polish
Computer Wizard[Wiz]  -- Serbian
Jack Gorji             -- Hebrew
Xos?Antón Vicente Rodríguez --Galego
Obelix                -- Hrvatski
Alex Simidchiev       -- Bulgarian
joup@algonet.se       -- Svenska
Bata György           -- Hungarian
Dmitry P.             -- Ukrainian
Mr Anonyme,Charles    -- French Update
kazakh                -- Erzhan Erbolatuly

Nick Reid             -- Advice
tongjiawang           -- Many help!
Belogorokhov Youri    -- Coder

Franck Uberto, Patrick Whitted, Walter Bergner, Jim McMahon, Fred Bailey,
Dchenka , itschy, HANDLE and all those we fogot to mention!!

-----------------
LICENSE:
-----------------

Copyright 1999-2003,2007 TiANWEi
Copyright 2004 tulipfan
Copyright 2007 Belogorokhov Youri

Multi-language translation are the property of their respective owner.

Regshot is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Regshot is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Regshot; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

-----------------
Contact:
-----------------
TiANWEi    spring_w@163.com
tulipfan   tulipfan@163.com
Youri      handle@wgapatcher.ru

http://code.google.com/p/regshot/

Original webpages:
http://sourceforge.net/projects/regshot/
