=======================================================
PseuWoWConsole - Version 2.2.0
=======================================================
"If at first you don't succeed, sky diving is not for you"

=======================================================
Greetings from TheTourist
=======================================================

This app is a wrapper for PseuWow.
What it does is open a port on 8095 on the machine, and anything written to this port will treated as if you inputted it in the console. 

Try this from HyperTerm:
SAY Hello Bitches
to port 8095, your Bot will say
Hello Bitches!

Thus from code, script or external app you can make calls to your bot.

=======================================================
Code
=======================================================

I can't profess to be a trained developer, I am entirely self taught. So any advice or corrections are welcome. Code was written in Delphi 6 with the JEDI VCL suite installed. Feel Free to modify and share as needed.

This is all released under the GNU General Public License, which you review at: http://www.gnu.org/copyleft/gpl.html
As I feel it's better to share, we all get along better that way.

=======================================================
Changes
=======================================================
Version 2.0.0
	* 	Added Nifty Icon Feature - Basically If you run more than one session you now tell which one is which!
	* 	Display of Char Name
	* 	Started Work on reading the colors from input	

Version 2.1.0	
	* 	Fixed a problem due to failed recompile

Version 2.2.0
	* 	Redirected Input capture is now Handled in a Thread. Meaning a more responsive output
	* 	Checking for |r in sysmessages, if |r is not found then don't clear our buffer
	* 	There is a Check Box for Clean Up - which basically toggles all text post formatting. If you're
		having problem disable it. Else it will highlight in a very pretty manner!
	* 	Started Adding Colour Coding for .lookup* MaNGOS commands