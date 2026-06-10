# ft_irc — Master Progress / Cleanup / TODO List

## Current status: done

### Server foundation

```text
[x] Makefile works
[x] Executable is named ircserv
[x] Program runs as ./ircserv <port> <password>
[x] Server class created
[x] main.cpp creates Server object and calls run()
[x] Server socket created with socket()
[x] SO_REUSEADDR added with setsockopt()
[x] Server fd set to non-blocking with fcntl()
[x] Server bound to chosen port with bind()
[x] Server starts listening with listen()
[x] Server fd added to poll list
[x] Main poll loop working
[x] New clients detected through poll()
[x] acceptClient() accepts new clients
[x] Client fds set to non-blocking
[x] Client fds added to poll list
[x] handleData() reads with recv()
[x] removeClient() closes and removes disconnected clients
[x] Basic JOIN implemented
[x] CMD_JOIN connected to handleJoin()
[x] JOIN validates channel name
[x] JOIN creates channel if it does not exist
[x] JOIN adds client to channel
[x] First user becomes channel operator
[x] JOIN sends JOIN reply
[x] JOIN sends 353 names list reply
[x] JOIN sends 366 end of names reply
[x] JOIN tested with one registered client
[x] JOIN error 461 tested
[x] JOIN error 403 tested
````

### Client handling

```text
[x] Client.hpp created
[x] Client.cpp created
[x] Client constructors/destructor added
[x] Client objects created when clients connect
[x] Clients stored in _clients map using fd as key
[x] Client buffer added
[x] Received data appended to client buffer
[x] Full IRC commands extracted using \r\n
[x] Multiple commands can be processed from one recv()
[x] Disconnected clients are deleted from _clients
```

### Dispatcher

```text
[x] getCommandName() added
[x] getCommandArgs() added
[x] Command enum added
[x] getCommandType() added
[x] dispatchCommand() added
[x] Dispatcher recognizes:
    - PASS
    - NICK
    - USER
    - JOIN
    - PRIVMSG
    - KICK
    - INVITE
    - TOPIC
    - MODE
[x] Unknown commands detected
[x] Unknown commands send numeric 421 reply to the client
[x] Commands that require registration are blocked before registration
[x] Numeric 451 is sent when unregistered clients try restricted commands
[x] Commands are strict uppercase for now
[x] Lowercase / mixed-case commands are treated as unknown
```

### Registration

```text
[x] Client stores pass status
[x] Client stores nickname
[x] Client stores username
[x] Client stores realname
[x] Client stores registered status
[x] handlePass() implemented
[x] handleNick() implemented
[x] handleUser() implemented
[x] checkRegistration() implemented
[x] Client becomes registered only after:
    - correct PASS
    - valid NICK
    - valid USER
[x] Duplicate nickname detected
[x] Missing nickname detected
[x] Wrong password detected
[x] PASS after registration rejected
[x] USER after registration rejected
[x] Welcome numeric 001 sent after registration
```

### Replies

```text
[x] sendReply() added
[x] sendNumeric() added
[x] sendUnknownCommand() added
[x] sendNotRegistered() added
[x] Basic numeric replies started
[x] Welcome reply 001 sent after registration
[x] Unknown command error 421 sent
[x] Not registered error 451 sent
[x] Password error 464 sent
[x] Not enough parameters 461 sent
[x] Nickname missing 431 sent
[x] Nickname already used 433 sent
[x] Reregister error 462 sent
```

### Channel foundation

```text
[x] Channel.hpp created
[x] Channel.cpp created
[x] Channel constructors/destructor added
[x] Channel stores name
[x] Channel stores topic
[x] Channel stores password/key placeholder for mode +k
[x] Channel stores members using std::vector<Client*>
[x] Channel stores operators using std::vector<Client*>
[x] Channel stores invite-only placeholder for mode +i
[x] Channel stores topic restriction placeholder for mode +t
[x] Channel stores user limit placeholder for mode +l
[x] Channel can add members
[x] Channel can remove members
[x] Channel can check if a client is a member
[x] Channel can add operators
[x] Channel can remove operators
[x] Channel can check if a client is an operator
[x] Channel can build names list
[x] Channel.cpp added to Makefile
[x] Project compiles with Channel.cpp
```

---

# Why we use vector for Channel members

In the `Server` class, connected clients are stored in a map:

```cpp
std::map<int, Client*> _clients;
```

This makes sense because the server receives a file descriptor from `poll()`, and the fd is the key used to find the correct client quickly.

For channels, we use vectors:

```cpp
std::vector<Client*> _members;
std::vector<Client*> _operators;
```

This is because a channel is naturally a list of users.

A channel needs to answer simple questions:

```text
Who is inside this channel?
Who is an operator?
Can this client join?
Can this client change the topic?
Who should receive this channel message?
What names should be shown in the names list?
```

A `std::map<int, Client*>` could also work for channel members, but for this project a vector is easier to understand and easier to explain.

Our rule for now:

```text
Server:
Use map because clients are found by fd.

Channel:
Use vector because members and operators are simple user lists.
```

This keeps the project simple, readable, and defense-friendly.

If the project later becomes more complex, we can change channel storage to maps, but for the expected size of the 42 project, vector is completely fine.

---

# Later cleanup / improvement list

Keep this list and remove items as we fix them.

## General code cleanup

```text
[ ] Reduce debug std::cout messages before final evaluation
[ ] Keep only useful debug messages during development
[ ] Split Server.cpp if it becomes too large
[ ] Keep functions short and focused
[ ] Keep helper functions private when they are only used inside Server
[ ] Keep data members private
[ ] Use public getters/setters only when another class truly needs them
[ ] Avoid stuffing all logic into one giant function
[ ] Keep one clear responsibility per function
[ ] Move reply helpers to a separate file later if needed
[ ] Move command handlers to separate files later if needed
```

## Server cleanup

```text
[ ] Add/verify proper Server destructor
[ ] Close all open fds safely
[ ] Delete all Client objects safely
[ ] Delete all Channel objects safely
[ ] Prevent Server copying
[ ] Improve poll() handling:
    - POLLHUP
    - POLLERR
    - POLLNVAL
[ ] Improve recv() error handling:
    - EAGAIN
    - EWOULDBLOCK
    - EINTR
[ ] Improve send() handling:
    - partial send
    - failed send
    - disconnected client during send
[ ] Later use an output buffer and POLLOUT for stricter non-blocking send handling
```

## Input validation

```text
[ ] Replace atoi() with safer port parsing
[ ] Reject non-numeric ports
[ ] Reject negative ports
[ ] Reject port 0 unless we intentionally allow it
[ ] Reject ports above 65535
[ ] Decide if privileged ports under 1024 should be rejected or just allowed to fail
```

## IRC command behavior

```text
[ ] Decide final behavior for lowercase commands
    Current decision: strict uppercase only
[ ] Improve USER parsing
[ ] Improve NICK validation
[ ] Handle nickname change after registration
[ ] Make sure wrong PASS behavior matches expected IRC behavior
[ ] Add real handlers for recognized but not implemented commands
[ ] Verify numeric replies with chosen real IRC client
```

## Registration replies

```text
[ ] Possibly add more welcome numerics after 001:
    - 002 Your host
    - 003 Server created
    - 004 Server info
[ ] Format replies consistently with :ircserv prefix
[ ] Make sure target is * before nickname exists
[ ] Make sure target is nickname after nickname exists
```

---

# Channels

## Channel foundation

```text
[x] Implement Channel.cpp
[x] Channel stores name
[x] Channel stores topic
[x] Channel stores members
[x] Channel stores operators
[x] Channel stores password/key placeholder
[x] Channel stores user limit placeholder
[x] Channel stores invite-only mode placeholder
[x] Channel stores topic-restricted mode placeholder
[x] Add/remove members
[x] Add/remove operators
[x] Check if user is member
[x] Check if user is operator
[x] Build names list
```

## Channel cleanup / later

```text
[ ] Remove disconnected client from all channels
[ ] Delete empty channels if needed
[ ] Decide if broadcast() should stay inside Channel or be handled by Server
[ ] Activate invite-only mode logic
[ ] Activate topic restriction logic
[ ] Activate password/key logic
[ ] Activate user limit logic
[ ] Add invited clients list for INVITE / +i
```

---

# JOIN

```text
[ ] Implement JOIN command
[ ] Connect CMD_JOIN to handleJoin()
[ ] Validate channel name
[ ] Create channel if it does not exist
[ ] Add client to channel
[ ] First user becomes channel operator
[ ] Check invite-only mode
[ ] Check channel key/password
[ ] Check user limit
[ ] Send JOIN message to joining client
[ ] Later send JOIN message to all channel members
[ ] Send topic reply if topic exists
[ ] Send names list reply 353
[ ] Send end of names reply 366
[ ] Test JOIN with one client
[ ] Test JOIN with two clients
```

---

# PRIVMSG

```text
[ ] Implement PRIVMSG to user
[ ] Implement PRIVMSG to channel
[ ] Check missing target
[ ] Check missing message
[ ] Check target exists
[ ] Check sender is in channel before sending to channel
[ ] Forward channel messages to all members except sender
[ ] Send private messages directly to target user
```

---

# Operator commands

```text
[ ] Implement KICK
[ ] Implement INVITE
[ ] Implement TOPIC
[ ] Implement MODE
```

## MODE requirements

```text
[ ] +i / -i invite-only
[ ] +t / -t topic restricted to operators
[ ] +k / -k channel key/password
[ ] +o / -o give/take operator privilege
[ ] +l / -l set/remove user limit
```

---

# Testing

```text
[x] Test server startup
[x] Test connection with nc
[x] Test raw message reception with nc
[x] Test full command extraction with PASS, NICK, and USER
[x] Test dispatcher with known and unknown commands
[x] Test wrong password
[x] Test PASS after registration
[x] Test missing NICK
[x] Test duplicate NICK
[x] Test USER registration
[x] Test unknown command
[x] Test numeric 421 for unknown command
[x] Test restricted commands before registration
[x] Test numeric 451 for restricted command before registration
[x] Test Channel.cpp compiles
[ ] Test with multiple nc clients
[ ] Test partial messages
[ ] Test multiple commands in one packet
[ ] Test USER with missing parameters
[ ] Test JOIN
[ ] Test PRIVMSG user
[ ] Test PRIVMSG channel
[ ] Test KICK
[ ] Test INVITE
[ ] Test TOPIC
[ ] Test MODE
[ ] Test disconnect while inside channel
[ ] Test with real IRC client
[ ] Choose one reference IRC client for evaluation
```

---

# Code style: should we keep functions small?

Yes. The 42 Norm is not checked for ft_irc the same way as C projects, but we should still keep a clean structure.

Not because the subject forces it, but because:

```text
- It is easier to debug
- It is easier to explain in defense
- It avoids huge unreadable files
- It helps the team work in parallel
- It makes future JOIN / PRIVMSG / MODE much easier
```

## Good style rule for this project

Use this idea:

```text
main.cpp
    only parses arguments and starts Server

Server.cpp
    server setup, poll loop, accept, recv, disconnect

Client.cpp
    client state and buffer

Channel.cpp
    channel members, operators, topic, modes

ServerCommands.cpp or src/commands/*.cpp
    command handlers

ServerReplies.cpp
    sendReply(), sendNumeric(), reply helpers
```

You do not have to split immediately, but if `Server.cpp` becomes too big, split it.

A clean split later could be:

```text
src/Server.cpp
src/ServerNetwork.cpp
src/ServerCommands.cpp
src/ServerReplies.cpp
src/Client.cpp
src/Channel.cpp
```

Or use the existing commands folder:

```text
src/commands/Pass.cpp
src/commands/Nick.cpp
src/commands/User.cpp
src/commands/Join.cpp
src/commands/Privmsg.cpp
src/commands/Mode.cpp
```

For now, it is acceptable to continue in `Server.cpp` while the logic is still small. Once JOIN and PRIVMSG grow, splitting command handlers will make the project cleaner.

---

# Reply helper functions

We use helper functions because they avoid repeating the same message-building code everywhere.

Instead of writing this everywhere:

```cpp
std::string response = ":ircserv 001 roger :Welcome\r\n";
send(client->getFd(), response.c_str(), response.length(), 0);
```

we use:

```cpp
sendNumeric(client, "001", ":Welcome to the ft_irc server, " + client->getNickname());
```

This is cleaner and easier to maintain.

Current helpers:

```text
[x] sendReply()
[x] sendNumeric()
[x] sendUnknownCommand()
[x] sendNotRegistered()
```

Useful helpers to add later:

```cpp
void sendNeedMoreParams(Client *client, const std::string& command);
void sendNicknameInUse(Client *client, const std::string& nickname);
void sendWelcome(Client *client);
void sendJoinReply(Client *client, const std::string& channelName);
void sendNamesList(Client *client, Channel *channel);
void sendEndOfNames(Client *client, const std::string& channelName);
```

Example later:

```cpp
if (args.empty())
{
    sendNeedMoreParams(client, "PASS");
    return;
}
```

This is much cleaner than writing the full numeric reply every time.

---

# Why port 6667?

It does not have to be 6667.

The subject says the program runs like:

```bash
./ircserv <port> <password>
```

So the port is whatever we pass as argument.

Examples:

```bash
./ircserv 6667 pass
./ircserv 4242 pass
./ircserv 8080 pass
./ircserv 12345 pass
```

We use `6667` because it is the traditional/default IRC port.

For testing, `6667` is normal and easy to remember.

Important:

```text
- Ports 0–65535 exist
- Ports under 1024 often require admin/root permissions
- 6667 is common for IRC
- The evaluator may test with another port
```

So the server must not hardcode `6667`.

It must use the port received from the command line.

---

# Why IP 127.0.0.1?

`127.0.0.1` means “this same computer.”

It is called the loopback address.

When we run:

```bash
nc -C 127.0.0.1 6667
```

we are saying:

```text
Connect to port 6667 on my own machine.
```

We can also use:

```bash
nc -C localhost 6667
```

This usually means the same thing.

If another computer on the same network wants to connect, it would use the local network IP of the server machine, for example:

```bash
nc -C 192.168.1.50 6667
```

For development, we usually use:

```bash
127.0.0.1
```

because both server and test client are running on the same computer.

---

# Why `nc -C 127.0.0.1 6667`?

The command is:

```bash
nc -C 127.0.0.1 6667
```

Meaning:

```text
nc          = netcat, a simple TCP client
-C          = send lines with CRLF endings
127.0.0.1   = connect to this computer
6667        = connect to port 6667
```

The password is not part of the `nc` command.

This is wrong:

```bash
nc -C 127.0.0.1 pass
```

because `pass` is not a port.

The password is sent after connecting, as an IRC command:

```irc
PASS pass
```

Full flow:

Terminal 1:

```bash
./ircserv 6667 pass
```

Terminal 2:

```bash
nc -C 127.0.0.1 6667
```

Then inside `nc`:

```irc
PASS pass
NICK roger
USER roger 0 * :Roger
```

## Why `-C`?

IRC messages end with:

```text
\r\n
```

That is carriage return + newline.

Normal terminal Enter may send only:

```text
\n
```

`nc -C` helps send proper CRLF line endings.

That matches IRC better and helps us test partial/full IRC command handling correctly.

---

# Why `USER roger 0 * :Roger`?

IRC `USER` command has this general shape:

```irc
USER <username> <hostname> <servername> :<realname>
```

Example:

```irc
USER roger 0 * :Roger
```

Breakdown:

```text
USER        = command
roger       = username
0           = hostname placeholder
*           = servername placeholder
:Roger      = real name / full name
```

The colon `:` is important.

It means:

```text
Everything after this is one trailing parameter.
```

So this:

```irc
USER roger 0 * :Roger Pastor Gomez
```

means the realname is:

```text
Roger Pastor Gomez
```

Without the colon, spaces would be treated as separate parameters.

For the current simple parser:

```text
username = first word after USER
realname = everything after :
```

So:

```irc
USER roger 0 * :Roger
```

gives:

```text
username = roger
realname = Roger
```

Later we should improve USER parsing to verify enough parameters properly.

---

# Why `Nick roger` is unknown

This test:

```irc
Nick roger
```

currently gives:

```text
Unknown command: Nick
```

That is intentional for now.

We decided to keep commands strict uppercase:

```irc
NICK roger
```

Current behavior:

```text
PASS -> recognized
pass -> unknown
NICK -> recognized
Nick -> unknown
USER -> recognized
user -> unknown
```

This keeps the dispatcher simple while we build the mandatory behavior.

Later, if our chosen real IRC client sends lowercase or mixed-case commands, we can make command detection case-insensitive.

---

# Next coding steps

```text
1. Implement handleJoin()
2. Connect CMD_JOIN to handleJoin()
3. Test JOIN with one registered client
4. Send JOIN reply
5. Send 353 names list
6. Send 366 end of names
7. Test JOIN with two registered clients
8. Later broadcast JOIN to other channel members
```

```
```
