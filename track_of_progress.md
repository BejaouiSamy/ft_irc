---

# 1. `track_of_progress.md` completo

Copia y pega esto entero en `track_of_progress.md`:

````md
# ft_irc — Master Progress / Cleanup / TODO List

## Current estimated mandatory progress

```text
Estimated mandatory completion: 62% to 65%
````

The networking base, registration, channels, JOIN, channel messages, private messages, output buffering, safer port parsing, server refactor, TOPIC, and KICK are done.

INVITE has been started/prepared, but final testing is still pending.

The remaining mandatory work is mainly finishing/testing INVITE, implementing MODE and channel modes, disconnect cleanup, and real IRC client testing.

---

## Current status: done

### Server foundation

```text
[x] Makefile works
[x] Executable is named ircserv
[x] Program runs as ./ircserv <port> <password>
[x] main.cpp uses try/catch for clean error reporting
[x] Safer port parsing added
[x] Invalid ports rejected
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
[x] handleData() reads with recv() after POLLIN
[x] handleWrite() writes with send() after POLLOUT
[x] send() appears only in handleWrite()
[x] Client output buffer added
[x] sendReply() queues outgoing replies instead of sending directly
[x] POLLOUT enabled when output is pending
[x] POLLOUT disabled when output buffer is empty
[x] Partial send handling started
[x] Basic POLLHUP / POLLERR / POLLNVAL handling added
[x] removeClient() closes and removes disconnected clients
[x] Basic Server destructor added
[x] Destructor deletes Client objects
[x] Destructor deletes Channel objects
[x] Destructor closes fds
```

### Refactor

```text
[x] Server.cpp split into smaller files
[x] Server.cpp keeps constructor, destructor, run()
[x] ServerNetwork.cpp added
[x] ServerCommands.cpp added
[x] ServerRegistration.cpp added
[x] ServerChannels.cpp added
[x] ServerReplies.cpp added
[x] Makefile updated for new .cpp files
[x] Project compiles after refactor
[x] Existing registration / JOIN / PRIVMSG behavior retested after refactor
```

### Client handling

```text
[x] Client.hpp created
[x] Client.cpp created
[x] Client constructors/destructor added
[x] Client objects created when clients connect
[x] Clients stored in _clients map using fd as key
[x] Client input buffer added
[x] Client output buffer added
[x] Received data appended to client input buffer
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
[x] sendReply() queues data in client output buffer
[x] sendNumeric() added
[x] sendUnknownCommand() added
[x] sendNotRegistered() added
[x] sendToChannel() helper added
[x] Welcome reply 001 sent after registration
[x] Unknown command error 421 sent
[x] Not registered error 451 sent
[x] Password error 464 sent
[x] Not enough parameters 461 sent
[x] Nickname missing 431 sent
[x] Nickname already used 433 sent
[x] Reregister error 462 sent
[x] No such channel 403 sent
[x] No such nick/channel 401 sent
[x] No recipient 411 sent
[x] No text to send 412 sent
[x] Not on channel 442 sent
[x] User not in channel 441 sent for KICK target errors
[x] User already on channel 443 prepared for INVITE
[x] Channel operator privilege error 482 sent
[x] Topic numeric 331 sent when no topic is set
[x] Topic numeric 332 sent when topic exists
[x] Invite confirmation numeric 341 prepared
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
[x] Channel stores invited users using std::vector<Client*>
[x] Channel stores invite-only placeholder for mode +i
[x] Channel stores topic restriction placeholder for mode +t
[x] Channel stores user limit placeholder for mode +l
[x] Channel exposes members through getMembers()
[x] Channel can add members
[x] Channel can remove members
[x] Channel can check if a client is a member
[x] Channel can add operators
[x] Channel can remove operators
[x] Channel can check if a client is an operator
[x] Channel can add invited users
[x] Channel can remove invited users
[x] Channel can check if a client is invited
[x] Channel can set topic
[x] Channel can check if topic exists
[x] Channel can build names list
[x] Channel.cpp added to Makefile
[x] Project compiles with Channel.cpp
```

---

# JOIN

```text
[x] Implement JOIN command
[x] Connect CMD_JOIN to handleJoin()
[x] Validate channel name
[x] Create channel if it does not exist
[x] Add client to channel
[x] First user becomes channel operator
[x] Send JOIN message to joining client
[x] Send JOIN message to all channel members
[x] Send names list reply 353
[x] Send end of names reply 366
[x] Test JOIN with one client
[x] Test JOIN with two clients
[x] Test JOIN error 461
[x] Test JOIN error 403
[ ] Check invite-only mode
[ ] Check channel key/password
[ ] Check user limit
[ ] Send topic reply if topic exists
```

---

# PRIVMSG

```text
[x] Implement PRIVMSG to user
[x] Implement PRIVMSG to channel
[x] Check missing target
[x] Check missing message
[x] Check target exists
[x] Check sender is in channel before sending to channel
[x] Forward channel messages to all members except sender
[x] Send private messages directly to target user
[x] Test PRIVMSG to channel
[x] Test PRIVMSG to user
[x] Test PRIVMSG missing target 411
[x] Test PRIVMSG missing message 412
[x] Test PRIVMSG no such channel 403
[x] Test PRIVMSG not on channel 442
[x] Test PRIVMSG no such nick/channel 401
```

---

# Operator commands

```text
[x] Implement TOPIC
[x] Implement KICK
[~] Implement INVITE
[ ] Implement MODE
```

## TOPIC

```text
[x] handleTopic() added
[x] CMD_TOPIC connected to handleTopic()
[x] TOPIC can view topic when no topic is set
[x] TOPIC can set topic
[x] TOPIC can view existing topic
[x] TOPIC broadcasts topic change to channel members
[x] TOPIC checks missing parameters
[x] TOPIC checks channel exists
[x] TOPIC checks user is in channel
[x] TOPIC numeric 331 implemented
[x] TOPIC numeric 332 implemented
[x] TOPIC numeric 403 implemented
[x] TOPIC numeric 442 implemented
[x] TOPIC tested
```

## KICK

```text
[x] handleKick() added
[x] CMD_KICK connected to handleKick()
[x] parseKickArgs() helper added
[x] KICK parses channel name
[x] KICK parses target nickname
[x] KICK parses optional reason
[x] KICK uses nickname as default reason if reason is missing
[x] Only channel operators can KICK
[x] KICK broadcasts to channel members
[x] Kicked user receives KICK message
[x] Kicked user is removed from channel
[x] KICK checks missing parameters
[x] KICK checks channel exists
[x] KICK checks kicker is in channel
[x] KICK checks kicker is operator
[x] KICK checks target exists / is in channel
[x] KICK numeric 461 implemented
[x] KICK numeric 403 implemented
[x] KICK numeric 442 implemented
[x] KICK numeric 482 implemented
[x] KICK numeric 441 implemented
[x] KICK tested
```

## INVITE

```text
[x] Add invited users list to Channel
[x] Add addInvited()
[x] Add removeInvited()
[x] Add isInvited()
[x] Add parseInviteArgs()
[x] parseInviteArgs() extracts target nickname
[x] parseInviteArgs() extracts channel name
[x] parseInviteArgs() ignores extra text after channel name
[x] Add handleInvite()
[x] CMD_INVITE connected to handleInvite()
[x] INVITE stores invited user in Channel
[x] INVITE sends numeric 341 to inviter
[x] INVITE sends INVITE message to invited user
[ ] Test INVITE success
[ ] Test INVITE missing parameters
[ ] Test INVITE no such nick
[ ] Test INVITE no such channel
[ ] Test INVITE inviter not in channel
[ ] Test INVITE inviter not operator
[ ] Test INVITE target already in channel
```

---

# MODE requirements

```text
[ ] Implement MODE
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
[x] Test with multiple nc clients
[x] Test JOIN
[x] Test JOIN with two clients
[x] Test PRIVMSG user
[x] Test PRIVMSG channel
[x] Test port validation
[x] Test compile after refactor
[x] Test TOPIC
[x] Test KICK
[ ] Test INVITE
[ ] Test partial messages manually
[ ] Test multiple commands in one packet
[ ] Test USER with missing parameters
[ ] Test MODE
[ ] Test disconnect while inside channel
[ ] Test with real IRC client
[ ] Choose one reference IRC client for evaluation
```

---

# Later cleanup / improvement list

## General code cleanup

```text
[ ] Reduce debug std::cout messages before final evaluation
[ ] Keep only useful debug messages during development
[x] Split Server.cpp
[x] Move reply helpers to ServerReplies.cpp
[x] Move command routing to ServerCommands.cpp
[x] Move registration handlers to ServerRegistration.cpp
[x] Move channel handlers to ServerChannels.cpp
[x] Move network logic to ServerNetwork.cpp
[x] Keep KICK function shorter with parseKickArgs()
[x] Keep INVITE function shorter with parseInviteArgs()
[ ] Keep functions short and focused
[ ] Keep helper functions private when they are only used inside Server
[ ] Keep data members private
[ ] Use public getters/setters only when another class truly needs them
```

## Server cleanup

```text
[x] Add/verify basic Server destructor
[x] Close all open fds safely
[x] Delete all Client objects safely
[x] Delete all Channel objects safely
[ ] Prevent Server copying
[x] Improve basic poll() handling:
    - POLLHUP
    - POLLERR
    - POLLNVAL
[ ] Improve recv() error handling:
    - EAGAIN
    - EWOULDBLOCK
    - EINTR
[x] Improve send() handling:
    - partial send
    - failed send
    - disconnected client during send
[x] Use an output buffer and POLLOUT for stricter non-blocking send handling
```

## Input validation

```text
[x] Replace atoi() with safer port parsing
[x] Reject non-numeric ports
[x] Reject negative ports
[x] Reject port 0
[x] Reject ports above 65535
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
[ ] Verify numeric replies with chosen real IRC client
```

## Registration replies

```text
[ ] Possibly add more welcome numerics after 001:
    - 002 Your host
    - 003 Server created
    - 004 Server info
[x] Format basic replies consistently with :ircserv prefix
[x] Make sure target is * before nickname exists
[x] Make sure target is nickname after nickname exists
```

## Channels

```text
[ ] Remove disconnected client from all channels
[ ] Delete empty channels if needed
[ ] Decide if unused Channel::broadcast() should stay or be removed
[ ] Activate invite-only mode logic
[ ] Activate topic restriction logic
[ ] Activate password/key logic
[ ] Activate user limit logic
[x] Add invited clients list for INVITE / +i
[ ] Connect invited clients list to JOIN when +i is active
```

---

# Next recommended implementation order

```text
1. Finish and test INVITE
2. MODE +i / -i
3. Connect JOIN to invite-only mode
4. MODE +t / -t
5. Connect TOPIC restriction to +t
6. MODE +k / -k
7. Connect JOIN to channel key/password
8. MODE +l / -l
9. Connect JOIN to user limit
10. MODE +o / -o
11. Disconnect cleanup from channels
12. Real IRC client testing
```

---

# Current git checkpoint

```bash
git add .
git commit -m "Implement TOPIC KICK and start INVITE"
```
