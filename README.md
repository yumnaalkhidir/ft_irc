Reference: https://www.rfc-editor.org/rfc/rfc2812#section-3

IRC is a way to communicate through channels over the internet using a client-server model, where I am the client (using irssi), and I connect to an IRC server (like Libera.Chat) which hosts channels.

a client is a program that requests a service or data

irssi is a client because it requests to join channels 

a server is a program/software that provides a service 

[libbera.chat](http://libbera.chat) is a server it hadles users, channels messages

### Client:

- Initiates communication
- Usually runs on your local machine
- Waits for responses

### Server:

- Listens for incoming requests
- Runs continuously (24/7 usually)
- Can handle multiple clients


Users:
each user is identified by a unique nickname. 9 characters max. (nickname)
the name of the host that the user  is running on  max of 63 chars (user hostname)
the username of the user on that host (username)
the server to which the client is connected to (server)

Channels
a channel is a group chat
a channel name can start with '#', '&', '+', '!' and it can be upto 50 characters
a channel name can't contain space ' ', comma ',', Control G(Ascii 7), colon ':'
Channel names are case insensitive (#chat = #CHAT = #Chat)
A comma is use for a list separator (JOIN #A, #B, #C)

Messages
Every message ends with Carriage Return and line Feed (CR-LF) "\r\n"
messages can be received like : 
NICK john\r\n
USER test 0 * :hello\r\n
or
"NICK jo"
"hn\r\nUSER te"
"st 0 * :hello\r\n"
also 
IRC has special case-insensitive system ({} = [], ^ = ~, | = \)
Format
A Message consist of three parts and separated (0x20 = 32 = space)
the prefix (optional),  the command, command parameters(15 parameters max)
the prefix is indicated by a leading colon (0x3a = 58 = :) with no gap in between
if parameters starts with colon(:) treat the following as one parameter(including the spaces)
all messages have CR-FL(\r\n) characters trailing
a line must not exceed 512 characters including CR-FL
NULL '\0' is forbidden

SO
receiving from client:
parse for:
optional prefix (ignore unless they are match to the nickname)
command
parameters
Format:
:prefix COMMAND [param1 param 2 :parameter 3]\r\n

Sending to client:
prefix
command
parameters

Format:
:nick!user@host COMMAND <param list>

Numeric Replies:
on errors server must reply with a numeric code 
format:
:server_name CODE target :message\r\n

examples:
NICK john\r\n (client->server)
PRIVMSG #chat :hello world\r\n (client->server)
:john!user@host PRIVMSG #chat :hello\r\n (server->clent)
:irc.local 433 john :Nickname is already in use\r\n

Summary: messages are separated by CR-LF "\r\n" and message components are separated by space except if led by a colon':', prefixes in most cases is the servername or nickname for users and: When receiving → ignore prefix
	   When sending → build full prefix:


COMMANDS

PASS <password>
used to set a connection password
this command must be the first line a user enter before registering for a nickname and  username and a user must enter it once only if more return an numeric error
Errors or Numerical returns: ERR_NEEDMOREPARAMS & ERR_ALREADYREGISTERED

NICK <nickname> or NICK <new_nickname>
gives a user a nickname or changes the existing one
Errors or Numerical returns: ERR_NONICKNAMEGIVEN, ERR_ERRONEUSNICKNAME, ERR_NICKNAMEINUSE, ERR_NICKCOLLISION, ERR_UNAVAILRESOURCE, ERR_RESTRICTED

USER <username> <mode> <unused> :<realname>
used to specify username, hostname and realname, mode is a numeric param used to set user modes
hostname is usually extracted from a function like getperrname();
Errors or Numerical returns: ERR_NEEDMOREPARAMS, ERR_ALREADYREGISTRED

When a user has entered these three command the user is marked as registered and a welcome message may be sent

QUIT [<quit message>]
used when a client want to disconnect from the server
No Errors or Numerical returns

Commands allowed before registration are PASS, NICK, USER, QUIT everything else is 451

Channel Operations:
JOIN <channel> [<key>]
JOIN 0
allows a user to join one or more channels or leave all channels (param = 0) if the user joined successful the server send a message in the channel announcing that the user has joined
Errors or Numerical returns: ERR_NEEDMOREPARAMS, ERR_BANNEDFROMCHAN, ERR_INVITEONLYCHAN, ERR_BADCHANNELKEY, ERR_CHANNELISFULL, ERR_BADCHANMASK, ERR_NOSUCHCHANNEL, ERR_TOOMANYCHANNELS, ERR_TOOMANYTARGETS, ERR_UNAVAILRESOURCE, RPL_TOPIC
 
PART <channel> [<message>]
allows a user to leave one or more channels and an optional message can be sent to all members of the channel
Errors or Numerical returns: ERR_NEEDMOREPARAMS, ERR_NOSUCHCHANNEL, ERR_NOTONCHANNEL

MODE <channel> +/- <modes> <modes parameter> 
i: Set/remove Invite-only channel
t: Set/remove the restrictions of the TOPIC command to channel
operators
k: Set/remove the channel key (password)
o: Give/take channel operator privilege
l: Set/remove the user limit to channel
used to change the modes of the channel
Errors and Numerical returns: ERR_NEEDMOREPARAMS, ERR_KEYSET, ERR_NOCHANMODES, ERR_CHANOPRIVSNEEDED, ERR_USERNOTINCHANNEL, ERR_UNKNOWNMODE, RPL_CHANNELMODEIS, RPL_BANLIST, RPL_ENDOFBANLIST, RPL_EXCEPTLIST, RPL_ENDOFEXCEPTLIST, RPL_INVITELIST, RPL_ENDOFINVITELIST, RPL_UNIQOPIS

TOPIC <channel> [<topic>]
this command manipulates the channel topic and has three roles one to 
TOPIC <channel> //view the channel topic
reply: RPL_NOTOPIC or RPL_TOPIC
TOPIC <channel> :new_topic //set channel topic
in this option the server will check if the user have permission to change the topic or not
TOPIC <channel> : //remove existing topic

Errors or Numeric returns: ERR_NEEDMOREPARAMS, ERR_NOTONCHANNEL, RPL_NOTOPIC, RPL_TOPIC, ERR_CHANOPRIVSNEEDED, ERR_NOCHANMODES


INVITE <nickname> <channel>
allows user to invite another user to a channel and the channel may be not existing yet then the channel will be created
if the channel exist and invide mode is on the members of the channel arent notified only the inviter and invitee are notified
Errors or Numerical returns: ERR_NEEDMOREPARAMS, ERR_NOSUCHNICK, ERR_NOTONCHANNEL, ERR_USERONCHANNEL, ERR_CHANOPRIVSNEEDED, RPL_INVITING, RPL_AWAY


KICK <channels> <users> [<comment>]
forces users to be removed from channel or channels
Either one channel with multiple users, or one-to-one mapping of channels and users. Multiple channels/users in one message are not sent to clients for backward compatibility.
Errors or Numerical returns: ERR_NEEDMOREPARAMS, ERR_NOSUCHCHANNEL, ERR_BADCHANMASK, ERR_CHANOPRIVSNEEDED, ERR_USERNOTINCHANNEL, ERR_NOTONCHANNEL

PRIVMSG <target> :<message>
target -> nickname or channel
allows to send messages to other user or channel
Errors or Numeric returns: ERR_NORECIPIENT                 ERR_NOTEXTTOSEND
           ERR_CANNOTSENDTOCHAN            ERR_NOTOPLEVEL
           ERR_WILDTOPLEVEL                ERR_TOOMANYTARGETS
           ERR_NOSUCHNICK
           RPL_AWAY

NOTICE <target> :<message>
similar to PRIVMSG it doesn't reply back or error respond

Command List:
Registration
PASS
NICK
USER
QUIT
OPER
MODE (user)
SERVICE
SQUIT

Channel
JOIN
PART
MODE
TOPIC
INVITE
KICK
NAMES
LIST

Send Message
PRIVMSG
NOTICE

Server Quires
MOTD
LUSER
VERSION
STATS
LINKS
TIME
CONNECT
TRACE
ADMIN
INFO

Service
SERVLIST
SQUERY

User based queries
WHO
WHOIS
WHOWAS

Miscellanous
KILL
PING
PONG
ERROR

Optional
AWAY
REHASH
DIE
RESTART
SUMMON
USERS
WALLOPS
USERHOST
ISON
