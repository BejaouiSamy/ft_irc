# ft_irc — Master Progress / Cleanup / TODO List

## Current estimated mandatory progress

```text
Estimated mandatory completion: 100%
````

The mandatory IRC server is implemented, cleaned up, and manually tested with both `nc` and a real IRC client.

The remaining work is documentation review, final `make fclean && make`, and final defense preparation.

---

## Final status: done

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
[x] send() appears only in ServerNetwork.cpp
[x] recv() appears only in ServerNetwork.cpp
[x] Client output buffer added
[x] sendReply() queues outgoing replies instead of sending directly
[x] POLLOUT enabled when output is pending
[x] POLLOUT disabled when output buffer is empty
[x] Partial send handling implemented
[x] Basic POLLHUP / POLLERR / POLLNVAL handling added
[x] removeClient() closes and removes disconnected clients
[x] removeClientFromChannels() removes disconnected clients from channels
[x] Empty channels are deleted
[x] Server destructor added
[x] Destructor deletes Client objects
[x] Destructor deletes Channel objects
[x] Destructor closes fds
[x] Debug std::cout messages cleaned up
```

---

## Refactor

```text
[x] Server.cpp split into smaller files
[x] Server.cpp keeps constructor, destructor, run()
[x] ServerNetwork.cpp added
[x] ServerCommands.cpp added
[x] ServerRegistration.cpp added
[x] ServerChannels.cpp added
[x] ServerReplies.cpp added
[x] Client.cpp added
[x] Channel.cpp added
[x] Makefile updated for all .cpp files
[x] Project compiles after refactor
[x] Empty src/commands folder removed
```

Current file responsibilities:

```text
Server.cpp              constructor, destructor, run()
ServerNetwork.cpp       accept, recv, queued send, disconnect, POLLOUT helpers
ServerCommands.cpp      command parsing, command type detection, dispatcher, small real-client helpers
ServerRegistration.cpp  PASS, NICK, USER, registration helpers
ServerChannels.cpp      JOIN, PRIVMSG, TOPIC, KICK, INVITE, MODE, channel helpers
ServerReplies.cpp       reply helpers and numeric replies
Client.cpp              client state, input buffer, output buffer
Channel.cpp             channel members, operators, topic, invited users, modes
```

---

## Client handling

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
[x] Partial commands are buffered until complete
[x] Disconnected clients are deleted from _clients
[x] Disconnected clients are removed from channels
```

---

## Dispatcher

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
    - CAP
    - PING
    - USERHOST
[x] Unknown commands detected
[x] Unknown commands send numeric 421 reply to the client
[x] Commands that require registration are blocked before registration
[x] Numeric 451 is sent when unregistered clients try restricted commands
```

Note:

```text
Commands are handled in uppercase when using nc.
Real IRC clients such as AdiIRC handle user input like /join or /mode and send proper IRC commands to the server.
```

---

## Registration

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
[x] Malformed USER rejected with numeric 461
[x] Welcome numeric 001 sent after registration
```

---

## Replies

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
[x] User not in channel 441 sent
[x] User already on channel 443 sent
[x] Channel operator privilege error 482 sent
[x] Topic numeric 331 sent when no topic is set
[x] Topic numeric 332 sent when topic exists
[x] Invite confirmation numeric 341 sent
[x] Channel mode numeric 324 sent
[x] Channel full numeric 471 sent
[x] Unknown mode numeric 472 sent
[x] Invite-only join error 473 sent
[x] Bad channel key numeric 475 sent
[x] USERHOST numeric 302 added for real IRC client compatibility
[x] PONG reply added for PING
[x] CAP minimal reply added for real IRC client compatibility
```

---

## Channel foundation

```text
[x] Channel.hpp created
[x] Channel.cpp created
[x] Channel constructors/destructor added
[x] Channel stores name
[x] Channel stores topic
[x] Channel stores password/key
[x] Channel stores members using std::vector<Client*>
[x] Channel stores operators using std::vector<Client*>
[x] Channel stores invited users using std::vector<Client*>
[x] Channel stores invite-only mode for +i
[x] Channel stores topic restriction mode for +t
[x] Channel stores user limit for +l
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
[x] Channel can set and clear password/key
[x] Channel can set and clear user limit
[x] Channel can set invite-only mode
[x] Channel can set topic restriction mode
[x] Channel can build names list with @ for operators
```

---

# Mandatory IRC commands

## PASS

```text
[x] Correct password accepted
[x] Missing password rejected with 461
[x] Wrong password rejected with 464
[x] PASS after registration rejected with 462
```

## NICK

```text
[x] NICK sets nickname
[x] Missing nickname rejected with 431
[x] Duplicate nickname rejected with 433
[x] NICK with leading ':' cleaned for real client compatibility
```

## USER

```text
[x] USER sets username and realname
[x] USER after registration rejected with 462
[x] USER missing parameters rejected with 461
[x] Malformed USER rejected with 461
[x] USER titi 0 * :titi accepted
[x] USER titi -0 :titi rejected
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
[x] Check invite-only mode
[x] Block non-invited user from invite-only channel
[x] Allow invited user to join invite-only channel
[x] Remove invited status after successful JOIN
[x] Check channel key/password
[x] Block user with numeric 475 when key is missing/wrong
[x] Allow user to join with correct key
[x] Check user limit
[x] Block user with numeric 471 when channel is full
[x] Test JOIN with one client
[x] Test JOIN with two clients
[x] Test JOIN error 461
[x] Test JOIN error 403
[x] Test invite-only JOIN blocked with numeric 473
[x] Test invited user can JOIN after INVITE
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
[x] Test channel messages between AdiIRC and nc
```

---

# Operator commands

```text
[x] Implement TOPIC
[x] Implement KICK
[x] Implement INVITE
[x] Implement MODE
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
[x] TOPIC respects +t topic restriction
[x] Non-operator cannot change topic when +t is active
[x] Operator can change topic when +t is active
[x] TOPIC tested with nc
[x] TOPIC tested with AdiIRC
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
[x] KICK tested through +o operator privilege
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
[x] INVITE allows invited user to join +i channel
[x] Test INVITE success
[x] Test INVITE sends 341
[x] Test INVITE sends message to invited user
[x] Test INVITE target already in channel with numeric 443
[x] Test INVITE with invite-only mode active
```

## MODE

```text
[x] Add handleMode()
[x] Connect CMD_MODE to handleMode()
[x] Add parseModeArgs()
[x] MODE can query current channel modes
[x] MODE query sends numeric 324
[x] MODE checks missing parameters
[x] MODE checks channel exists
[x] MODE checks user is in channel
[x] MODE checks user is operator before changing modes
[x] MODE handles combined modes such as +it
[x] MODE +i activates invite-only mode
[x] MODE -i disables invite-only mode
[x] MODE +t activates topic restriction
[x] MODE -t disables topic restriction
[x] MODE +k sets channel key/password
[x] MODE -k removes channel key/password
[x] MODE +l sets user limit
[x] MODE -l removes user limit
[x] MODE +o gives channel operator privilege
[x] MODE -o removes channel operator privilege
[x] MODE broadcasts mode changes to channel members
[x] MODE query shows active channel modes
[x] MODE unknown mode sends numeric 472
```

---

# MODE requirements

```text
[x] Implement MODE
[x] +i / -i invite-only
[x] +t / -t topic restricted to operators
[x] +k / -k channel key/password
[x] +o / -o give/take operator privilege
[x] +l / -l set/remove user limit
```

---

# Real IRC client compatibility

Reference client:

```text
AdiIRC
```

Connection settings tested:

```text
Server: 127.0.0.1
Port: 6667
SSL: off
Password: con
Nick: roger
Username: roger
Fullname: roger
```

Real client support added:

```text
[x] CAP LS 302 handled
[x] CAP LIST handled without error
[x] CAP END handled without error
[x] PING handled with PONG
[x] USERHOST handled with numeric 302
[x] AdiIRC connects without unknown command errors
[x] AdiIRC receives welcome message
[x] AdiIRC joins #test
[x] AdiIRC uses /mode
[x] AdiIRC uses /topic
[x] AdiIRC sends messages to channel
[x] nc client receives messages from AdiIRC
[x] AdiIRC receives messages from nc client
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
[x] Test malformed USER
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
[x] Test INVITE
[x] Test MODE +i
[x] Test MODE -i
[x] Test MODE +t
[x] Test MODE -t
[x] Test MODE +k
[x] Test MODE -k
[x] Test MODE +l
[x] Test MODE -l
[x] Test MODE +o
[x] Test MODE -o
[x] Test invite-only JOIN blocked
[x] Test invited user can JOIN invite-only channel
[x] Test JOIN with channel key/password
[x] Test JOIN with user limit
[x] Test partial messages manually with Ctrl+D
[x] Test multiple commands in one packet with printf
[x] Test disconnect while inside channel
[x] Test empty channel deletion
[x] Test operator disconnect cleanup
[x] Test with real IRC client
[x] Choose AdiIRC as reference IRC client
[x] Final grep for send()
[x] Final grep for recv()
[x] Final grep for std::cout
```

---

# Final checks performed

```bash
make re
grep -R "send(" -n src include
grep -R "recv(" -n src include
grep -R "std::cout" -n src include
printf "PASS con\r\nNICK roger\r\nUSER roger 0 * :roger\r\nJOIN #final\r\nMODE #final +it\r\nMODE #final\r\n" | nc -C 127.0.0.1 6667
```

Expected final grep status:

```text
send() appears only in ServerNetwork.cpp
recv() appears only in ServerNetwork.cpp
only minimal std::cout output remains
```

Final observed behavior:

```text
Registration works
JOIN works
First channel user becomes operator
MODE +it works
MODE query returns +it
```

---

# Final cleanup

```text
[x] Reduce debug std::cout messages before final evaluation
[x] Keep only useful server status output
[x] Split Server.cpp
[x] Move reply helpers to ServerReplies.cpp
[x] Move command routing to ServerCommands.cpp
[x] Move registration handlers to ServerRegistration.cpp
[x] Move channel handlers to ServerChannels.cpp
[x] Move network logic to ServerNetwork.cpp
[x] Keep KICK function shorter with parseKickArgs()
[x] Keep INVITE function shorter with parseInviteArgs()
[x] Keep MODE parsing separate with parseModeArgs()
[x] Remove unused empty src/commands folder
[x] Keep data members private
[x] Use public getters/setters only when another class needs them
```

---

# Known decisions / not implemented

```text
[x] No bonus part implemented
[x] No server-to-server communication implemented
[x] No IRC client implemented
[x] No fork used
[x] No external libraries used
[x] No Libft used
[x] No Boost used
[x] Lowercase commands in nc are not normalized by the server
[x] Real IRC client commands work because the client sends proper IRC commands
[x] Full RFC coverage is not the goal
[x] Channel can temporarily have no operator if all operators leave or remove their own privilege
[x] This is accepted for mandatory behavior because the subject only requires give/take operator privilege
```

---

# Remaining TODO before submission

```text
[ ] make fclean
[ ] make
[ ] Run ./ircserv 6667 con once more
[ ] Quick final nc test
[ ] Review README.md
[ ] Review this track_of_progress.md
[ ] Final git status
[ ] Final commit
```

---

# Suggested final git checkpoint

```bash
git add .
git commit -m "Complete mandatory IRC server"
```