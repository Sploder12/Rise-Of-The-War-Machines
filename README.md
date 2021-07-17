# Rise-Of-The-War-Machines
Game that interacts with IBM Watson and makes AIs fight

To run this clone the thing in Visual Studio 2019 (or later). Then you'll want to replace the API keys, player stats, and player names with what you have for your students.
These are the constants at the top of the cpp file. runModel.py is not meant to be run by itself so don't worry about it. 

Client.py needs the address changed to the IPv4 of the host machine then distributed to the students. However, it cannot be run on any online IDEs/interpreters. They need to use IDLE, VSCode, PyCharm, etc. NO REPL.IT OR TRINKET.

Run the cpp file then run client.py on any computer you want to view the game on. They must all be on the same local network since LAN server.

The game takes a while and can also have infinte loops where the AIs keep doing the same thing and no damage. Feel free to modify the battle system.


**IMPORTANT**
When making the ML4K models make sure you have "Attack", "Defend", "Advance", and "Retreat" labels. they must be spelled correctly but case doesn't matter.
the model will recognize numbers attack, defense, spy power, hp, and shield in that order. The name doesnt matter this time but now the order does. When training the model keep in mind that you are putting in your opponents stats and not your own.
