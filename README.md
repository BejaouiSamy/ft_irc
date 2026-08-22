*This project has been created as part of the 42 curriculum by bsamy & chrrodri.*

# ft_irc — Internet Relay Chat Server

## Description

`ft_irc` is a 42 group project where we build our own IRC server in **C++98**.

The goal of the project is not to create an IRC client, but to create a server that real IRC clients can connect to.

The server handles multiple clients at the same time using non-blocking sockets and one main `poll()` loop.

The executable runs like this:

```bash
./ircserv <port> <password>
````

Example:

```bash
./ircserv 6667 con
```

`6667` is used in our tests because it is the traditional IRC port, but the server works with any valid port passed as an argument.

---

## Current Status

Mandatory part status:

```text
99% complete
```

The mandatory IRC server is implemented and manually tested with both `nc` and a real IRC client.

Implemented:

* server socket setup
* non-blocking server and client file descriptors
* one main `poll()` loop
* multiple client connections
* client input buffering using `\r\n`
* client output buffering using `POLLOUT`
* registration with `PASS`, `NICK`, and `USER`
* numeric replies for common errors
* channel creation with `JOIN`
* first user in a channel becomes channel operator
* channel member list with `353` / `366`
* channel messages with `PRIVMSG`
* user-to-user messages with `PRIVMSG`
* `TOPIC`
* `KICK`
* `INVITE`
* all required channel modes:

    * `+i` / `-i`
    * `+t` / `-t`
    * `+k` / `-k`
    * `+l` / `-l`
    * `+o` / `-o`
* invite-only JOIN behavior
* channel key/password behavior
* user limit behavior
* topic restriction behavior
* operator privilege handling
* disconnect cleanup from channels
* compatibility helpers for real IRC clients:

    * `CAP`
    * `PING`
    * `USERHOST`
* testing with AdiIRC
* safer port validation
* final debug output cleanup

---

## Mandatory Requirements

According to the subject, the server must:

* [x] Be written in C++98
* [x] Compile with `-Wall -Wextra -Werror`
* [x] Run as `./ircserv <port> <password>`
* [x] Use TCP/IP sockets
* [x] Use non-blocking file descriptors
* [x] Use `poll()` or equivalent for I/O handling
* [x] Avoid `fork()`
* [x] Accept multiple clients through the same `poll()` loop
* [x] Correctly rebuild full commands from buffered data ending in `\r\n`
* [x] Work correctly with a real IRC client
* [x] Avoid crashing or quitting unexpectedly in all tested cases

---

## Mandatory IRC Features

### Registration

* [x] `PASS`
* [x] `NICK`
* [x] `USER`

### Channels and messages

* [x] `JOIN`
* [x] Channel creation
* [x] Channel members
* [x] Channel operators
* [x] Channel `PRIVMSG`
* [x] User-to-user `PRIVMSG`
* [x] Channel message forwarding to other members

### Operator commands

* [x] `KICK`
* [x] `INVITE`
* [x] `TOPIC`
* [x] `MODE`

### Required channel modes

* [x] `+i` / `-i` — invite-only channel
* [x] `+t` / `-t` — restrict topic changes to operators
* [x] `+k` / `-k` — set/remove channel password
* [x] `+o` / `-o` — give/remove operator privilege
* [x] `+l` / `-l` — set/remove user limit

---

## Architecture

The project is split into several files to keep the code easier to understand and defend.

```text
src/Server.cpp
src/ServerNetwork.cpp
src/ServerCommands.cpp
src/ServerRegistration.cpp
src/ServerChannels.cpp
src/ServerReplies.cpp
src/Client.cpp
src/Channel.cpp
```

Responsibilities:

```text
Server.cpp              constructor, destructor, run()
ServerNetwork.cpp       accept, recv, queued send, disconnect, POLLOUT helpers
ServerCommands.cpp      command parsing, command type detection, dispatcher, small client helpers
ServerRegistration.cpp  PASS, NICK, USER, registration helpers
ServerChannels.cpp      JOIN, PRIVMSG, TOPIC, KICK, INVITE, MODE, channel helpers
ServerReplies.cpp       reply helpers and numeric replies
Client.cpp              client state, input buffer, output buffer
Channel.cpp             channel members, operators, topic, invited users, modes
```

This separation keeps the network layer, command dispatching, registration, replies, and channel logic easier to explain.

---

## Server

The `Server` class is responsible for:

* creating the server socket
* setting sockets to non-blocking mode
* binding and listening on the selected port
* running the main `poll()` loop
* accepting clients
* receiving data
* sending queued replies
* dispatching commands
* managing clients
* managing channels

The server stores clients like this:

```cpp
std::map<int, Client*> _clients;
```

The file descriptor is used as the key because `poll()` tells the server which fd is ready.

Channels are stored like this:

```cpp
std::map<std::string, Channel*> _channels;
```

---

## Client

The `Client` class stores:

* file descriptor
* nickname
* username
* real name
* input buffer
* output buffer
* password status
* registration status

The input buffer is used because IRC commands can arrive in pieces.

The output buffer is used because replies must only be sent after `poll()` reports that the fd is writable.

---

## Channel

The `Channel` class stores:

* channel name
* topic
* password/key
* members
* operators
* invited users
* invite-only mode
* topic restriction mode
* user limit

Channels use:

```cpp
std::vector<Client*> _members;
std::vector<Client*> _operators;
std::vector<Client*> _invited;
```

This keeps the channel logic simple and easy to explain.

---

## Why We Use Input and Output Buffers

The project requires non-blocking I/O and one main `poll()` loop.

For incoming data:

```text
poll() reports POLLIN
recv() reads data
data is stored in the input buffer
full IRC commands are extracted using \r\n
each complete command is dispatched
```

For outgoing data:

```text
sendReply() queues the message in the client output buffer
POLLOUT is enabled for that fd
poll() reports the fd writable
handleWrite() calls send()
only sent bytes are removed from the buffer
POLLOUT is disabled when the buffer is empty
```

Command handlers do not call `send()` directly.

The only real `send()` call is inside `handleWrite()`.

The only real `recv()` call is inside the network read handler.

---

## Compilation

Compile the project with:

```bash
make
```

Clean object files:

```bash
make clean
```

Remove object files and executable:

```bash
make fclean
```

Recompile from zero:

```bash
make re
```

---

## Execution

Run the server with:

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 con
```

Then connect with netcat:

```bash
nc -C 127.0.0.1 6667
```

The password is not part of the `nc` command. It is sent after connecting:

```irc
PASS con
```

---

## Real IRC Client

The reference IRC client used for testing was:

```text
AdiIRC
```

Connection settings used:

```text
Server: 127.0.0.1
Port: 6667
SSL: off
Password: con
Nick: roger
Username: roger
Fullname: roger
```

AdiIRC was tested with:

```irc
/join #test
/mode #test +i
/mode #test
/topic #test hello from AdiIRC
```

Normal channel messages were also tested from AdiIRC and received by another client connected with `nc`.

---

## Manual Tests

### Registration

```irc
PASS con
NICK roger
USER roger 0 * :roger
```

Expected:

```irc
:ircserv 001 roger :Welcome to the ft_irc server, roger
```

Malformed `USER` is rejected:

```irc
USER titi -0 :titi
```

Expected:

```irc
:ircserv 461 titi USER :Not enough parameters
```

---

### JOIN with two clients

Client 1:

```irc
PASS con
NICK roger
USER roger 0 * :roger
JOIN #test
```

Client 2:

```irc
PASS con
NICK titi
USER titi 0 * :titi
JOIN #test
```

Client 1 should receive:

```irc
:titi!titi@localhost JOIN #test
```

Client 2 should receive:

```irc
:ircserv 353 titi = #test :@roger titi
:ircserv 366 titi #test :End of /NAMES list
```

---

### Channel PRIVMSG

From Roger:

```irc
PRIVMSG #test :hello channel
```

Titi should receive:

```irc
:roger!roger@localhost PRIVMSG #test :hello channel
```

---

### Private PRIVMSG

From Roger:

```irc
PRIVMSG titi :hello directly
```

Titi should receive:

```irc
:roger!roger@localhost PRIVMSG titi :hello directly
```

---

### TOPIC

View topic:

```irc
TOPIC #test
```

If no topic exists:

```irc
:ircserv 331 roger #test :No topic is set
```

Set topic:

```irc
TOPIC #test :Welcome to our channel
```

Expected broadcast:

```irc
:roger!roger@localhost TOPIC #test :Welcome to our channel
```

View topic again:

```irc
TOPIC #test
```

Expected:

```irc
:ircserv 332 roger #test :Welcome to our channel
```

---

### MODE +t / -t

Activate topic restriction:

```irc
MODE #test +t
```

Expected:

```irc
:roger!roger@localhost MODE #test +t
```

When `+t` is active, only channel operators can change the topic.

Deactivate topic restriction:

```irc
MODE #test -t
```

Expected:

```irc
:roger!roger@localhost MODE #test -t
```

---

### KICK

Roger is operator in `#test`.

Titi joins `#test`.

From Roger:

```irc
KICK #test titi :Too noisy
```

Expected in both clients:

```irc
:roger!roger@localhost KICK #test titi :Too noisy
```

After that, Titi should no longer be in the channel.

From Titi:

```irc
PRIVMSG #test :am I still here?
```

Expected:

```irc
:ircserv 442 titi #test :You're not on that channel
```

---

### INVITE

Roger is operator in `#test`.

Titi is registered but not in `#test`.

From Roger:

```irc
INVITE titi #test
```

Expected in Roger:

```irc
:ircserv 341 roger titi #test
```

Expected in Titi:

```irc
:roger!roger@localhost INVITE titi #test
```

If invite-only mode is active, the invited user can join after receiving the invite.

---

### MODE +i / -i

Activate invite-only mode:

```irc
MODE #test +i
```

Expected broadcast:

```irc
:roger!roger@localhost MODE #test +i
```

Query modes:

```irc
MODE #test
```

Expected:

```irc
:ircserv 324 roger #test +i
```

Deactivate invite-only mode:

```irc
MODE #test -i
```

Expected broadcast:

```irc
:roger!roger@localhost MODE #test -i
```

Query modes again:

```irc
MODE #test
```

Expected:

```irc
:ircserv 324 roger #test +
```

---

### Invite-only JOIN behavior

Roger creates `#test` and activates invite-only mode:

```irc
JOIN #test
MODE #test +i
```

A user who is not invited tries to join:

```irc
JOIN #test
```

Expected:

```irc
:ircserv 473 titi #test :Cannot join channel (+i)
```

The channel operator invites that user:

```irc
INVITE titi #test
```

Expected for the invited user:

```irc
:roger!roger@localhost INVITE titi #test
```

Now the invited user joins:

```irc
JOIN #test
```

Expected:

```irc
:titi!titi@localhost JOIN #test
:ircserv 353 titi = #test :@roger titi
:ircserv 366 titi #test :End of /NAMES list
```

---

### MODE +k / -k

Set channel key:

```irc
MODE #test +k secret
```

Expected:

```irc
:roger!roger@localhost MODE #test +k secret
```

A user who tries to join without the key should receive:

```irc
:ircserv 475 titi #test :Cannot join channel (+k)
```

A user can join with:

```irc
JOIN #test secret
```

Remove key:

```irc
MODE #test -k
```

Expected:

```irc
:roger!roger@localhost MODE #test -k
```

---

### MODE +l / -l

Set user limit:

```irc
MODE #test +l 1
```

Expected:

```irc
:roger!roger@localhost MODE #test +l 1
```

If the channel is full, another user should receive:

```irc
:ircserv 471 titi #test :Cannot join channel (+l)
```

Remove limit:

```irc
MODE #test -l
```

Expected:

```irc
:roger!roger@localhost MODE #test -l
```

---

### MODE +o / -o

Give operator privilege:

```irc
MODE #test +o titi
```

Expected:

```irc
:roger!roger@localhost MODE #test +o titi
```

After that, Titi can use operator commands such as:

```irc
KICK #test roger :test
```

Remove operator privilege:

```irc
MODE #test -o titi
```

Expected:

```irc
:titi!titi@localhost MODE #test -o titi
```

After `-o`, Titi should no longer be able to use operator commands:

```irc
KICK #test roger :test
```

Expected:

```irc
:ircserv 482 titi #test :You're not channel operator
```

---

### Partial command test

The server rebuilds commands received in pieces.

Example with `nc`:

```text
com
Ctrl+D
man
Ctrl+D
d
Enter
```

Expected:

```irc
:ircserv 421 roger command :Unknown command
```

The important behavior is that the server waits until it receives the final `\r\n` before dispatching the full command.

---

### Multiple commands in one packet

```bash
printf "PASS con\r\nNICK roger\r\nUSER roger 0 * :roger\r\nJOIN #final\r\nMODE #final +it\r\nMODE #final\r\n" | nc -C 127.0.0.1 6667
```

Expected:

```irc
:ircserv 001 roger :Welcome to the ft_irc server, roger
:roger!roger@localhost JOIN #final
:ircserv 353 roger = #final :@roger
:ircserv 366 roger #final :End of /NAMES list
:roger!roger@localhost MODE #final +it
:ircserv 324 roger #final +it
```

---

### Port validation

These should fail with a clear error message:

```bash
./ircserv hello con
./ircserv 0 con
./ircserv 99999 con
```

This should work:

```bash
./ircserv 6667 con
```

---

## Final Checks

Final checks performed:

```bash
grep -R "send(" -n src include
grep -R "recv(" -n src include
grep -R "std::cout" -n src include
```

Expected:

```text
send() appears only in ServerNetwork.cpp
recv() appears only in ServerNetwork.cpp
only minimal std::cout output remains
```

---

## Resources

Useful references:

* RFC 1459 — Internet Relay Chat Protocol
* Modern IRC documentation
* Linux manual pages for:

    * `socket`
    * `setsockopt`
    * `bind`
    * `listen`
    * `accept`
    * `fcntl`
    * `poll`
    * `recv`
    * `send`
    * `close`

---

## AI Usage

AI was used as a learning and support tool during this project.

It helped us:
- understand parts of the subject and IRC protocol behavior;
- plan the implementation order;
- debug compilation and runtime errors;
- create manual test scenarios with `nc` and AdiIRC;
- review the mandatory requirements before final cleanup;
- improve the wording and structure of this README.

All code was reviewed, adapted, compiled, tested, and defended by the project members.s used to help understand the subject, plan the implementation, explain errors, create test scenarios, and review the final mandatory requirements.