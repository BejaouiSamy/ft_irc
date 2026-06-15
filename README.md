*This project has been created as part of the 42 curriculum by bsamy & chrrodri.*

# ft_irc — Internet Relay Chat Server

## Description

`ft_irc` is a 42 group project where we build our own IRC server in **C++98**.

The goal is not to create an IRC client, but to create a server that real IRC clients can connect to.

The server handles multiple clients at the same time using non-blocking sockets and one `poll()` loop.

The executable runs like this:

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 pass
```

`6667` is used in our tests because it is the traditional IRC port, but the server must work with any valid port passed as an argument.

---

## Current Project Status

### Done

* [x] Basic project structure created
* [x] `include/` folder created
* [x] `src/` folder created
* [x] `Server`, `Client`, and `Channel` classes created
* [x] Program checks for correct argument count
* [x] Program creates a `Server` object with port and password
* [x] Program uses `try/catch` in `main.cpp` for clean error handling
* [x] Safer port parsing added
* [x] Invalid ports are rejected before server startup
* [x] Makefile created
* [x] Makefile compiles with `c++`
* [x] Makefile uses `-Wall -Wextra -Werror -std=c++98`
* [x] Makefile has required rules: `all`, `clean`, `fclean`, `re`
* [x] Executable name fixed to `ircserv`

### Server foundation

* [x] Server socket created with `socket()`
* [x] `setsockopt()` added with `SO_REUSEADDR`
* [x] Server socket set to non-blocking mode with `fcntl()`
* [x] Server socket bound to the selected port with `bind()`
* [x] Server starts listening with `listen()`
* [x] Server socket added to the `poll()` file descriptor list
* [x] Main `poll()` loop added in `Server::run()`
* [x] New client connections detected through `poll()`
* [x] `acceptClient()` added
* [x] Accepted client sockets are set to non-blocking mode
* [x] Accepted clients are added to the `poll()` list
* [x] Data is received with `recv()` after `poll()` reports the fd as ready
* [x] Output replies are queued in each client output buffer
* [x] `POLLOUT` is enabled only when a client has pending output
* [x] `send()` is only called from `handleWrite()` after `poll()` reports the fd writable
* [x] Partial send handling started by erasing only sent bytes from the output buffer
* [x] `POLLHUP`, `POLLERR`, and `POLLNVAL` are handled at basic level
* [x] Basic `removeClient()` added
* [x] Disconnected clients are closed and removed from the `poll()` list
* [x] Basic destructor cleanup added for clients, channels, and fds

### Refactor

`Server.cpp` was split into smaller files:

```text
src/Server.cpp
src/ServerNetwork.cpp
src/ServerCommands.cpp
src/ServerRegistration.cpp
src/ServerChannels.cpp
src/ServerReplies.cpp
```

This keeps the project easier to explain and prepares the code for operator commands and modes.

### Client handling

* [x] `Client.cpp` added
* [x] `Client` constructors/destructor added
* [x] `Client` objects are created when clients connect
* [x] Connected clients are stored in the `_clients` map by fd
* [x] Input buffer added
* [x] Output buffer added
* [x] Received data is appended to each client's input buffer
* [x] Full IRC commands are extracted from the buffer using `\r\n`
* [x] Multiple commands can be processed from one received buffer
* [x] Replies are stored in the client output buffer before sending
* [x] Disconnected clients are deleted from the `_clients` map

### Command dispatcher

* [x] Basic command dispatcher added
* [x] `getCommandName()` added
* [x] `getCommandArgs()` added
* [x] Command enum added
* [x] Dispatcher recognizes mandatory command names using an enum
* [x] Dispatcher currently detects:
  * `PASS`
  * `NICK`
  * `USER`
  * `JOIN`
  * `PRIVMSG`
  * `KICK`
  * `INVITE`
  * `TOPIC`
  * `MODE`
* [x] Unknown commands are detected
* [x] Unknown commands send numeric `421`
* [x] Commands are currently strict uppercase
* [x] Lowercase or mixed-case commands are currently treated as unknown
* [x] Commands that require registration are blocked before registration
* [x] Numeric `451` is sent when restricted commands are used before registration

### Registration

* [x] `PASS` handler implemented
* [x] `NICK` handler implemented
* [x] `USER` handler implemented
* [x] Client stores password status
* [x] Client stores nickname
* [x] Client stores username
* [x] Client stores real name
* [x] Client stores registration status
* [x] Client becomes registered only after correct `PASS`, valid `NICK`, and valid `USER`
* [x] Wrong password detected
* [x] Missing nickname detected
* [x] Duplicate nickname detected
* [x] `PASS` after registration rejected
* [x] `USER` after registration rejected
* [x] Welcome numeric `001` sent after registration

### Replies

* [x] `sendReply()` added
* [x] `sendReply()` now queues replies instead of calling `send()` directly
* [x] `sendNumeric()` added
* [x] `sendUnknownCommand()` added
* [x] `sendNotRegistered()` added
* [x] `sendToChannel()` helper added
* [x] Numeric reply `001` sent after successful registration
* [x] Numeric reply `421` sent for unknown commands
* [x] Numeric reply `451` sent when restricted commands are used before registration
* [x] Numeric reply `461` sent for not enough parameters
* [x] Numeric reply `431` sent for missing nickname
* [x] Numeric reply `433` sent for nickname already in use
* [x] Numeric reply `464` sent for wrong password
* [x] Numeric reply `462` sent when trying to register again
* [x] Numeric reply `403` sent for invalid or missing channel target cases
* [x] Numeric reply `401` sent when private message target nick does not exist
* [x] Numeric reply `411` sent when `PRIVMSG` has no recipient
* [x] Numeric reply `412` sent when `PRIVMSG` has no text
* [x] Numeric reply `442` sent when sending to a channel the client has not joined

### Channel foundation

* [x] `Channel.cpp` added
* [x] `Channel` constructors/destructor added
* [x] `Channel` stores channel name
* [x] `Channel` stores topic
* [x] `Channel` stores password/key placeholder for mode `+k`
* [x] `Channel` stores members using `std::vector<Client*>`
* [x] `Channel` stores operators using `std::vector<Client*>`
* [x] `Channel` stores invite-only mode placeholder for mode `+i`
* [x] `Channel` stores topic restriction placeholder for mode `+t`
* [x] `Channel` stores user limit placeholder for mode `+l`
* [x] `Channel` can add members
* [x] `Channel` can remove members
* [x] `Channel` can check if a client is a member
* [x] `Channel` can add operators
* [x] `Channel` can remove operators
* [x] `Channel` can check if a client is an operator
* [x] `Channel` can expose its members through `getMembers()`
* [x] `Channel` can build a names list for `353` replies
* [x] `Channel.cpp` compiles successfully

### JOIN

* [x] Basic `JOIN` handler implemented
* [x] `CMD_JOIN` connected to `handleJoin()`
* [x] `JOIN` validates missing channel name
* [x] `JOIN` validates that channel names must start with `#`
* [x] `JOIN` creates a channel if it does not exist
* [x] `JOIN` adds the client to the channel
* [x] First user in a new channel becomes channel operator
* [x] `JOIN` sends the join reply
* [x] `JOIN` broadcasts the join message to existing channel members
* [x] `JOIN` sends names list reply `353`
* [x] `JOIN` sends end of names reply `366`
* [x] `JOIN` tested with one registered client
* [x] `JOIN` tested with two clients
* [x] Second client joining is visible to the first client
* [x] `JOIN` error `461` tested
* [x] `JOIN` error `403` tested

### PRIVMSG

* [x] Basic `PRIVMSG` handler implemented
* [x] `PRIVMSG` to channel implemented
* [x] Channel messages are forwarded to all other channel members
* [x] Sender does not receive their own channel message
* [x] `PRIVMSG` to user/nickname implemented
* [x] Missing recipient error `411` tested
* [x] Missing text error `412` tested
* [x] Unknown channel error `403` tested
* [x] Not on channel error `442` tested
* [x] Unknown nick/channel error `401` tested

---

## In Progress / Next Priorities

* [ ] Implement `TOPIC`
* [ ] Implement `KICK`
* [ ] Implement `INVITE`
* [ ] Implement `MODE`
* [ ] Activate channel modes: `+i`, `+t`, `+k`, `+o`, `+l`
* [ ] Remove disconnected clients from all joined channels
* [ ] Delete empty channels if needed
* [ ] Improve `USER` parsing
* [ ] Improve `NICK` validation
* [ ] Test partial messages manually
* [ ] Test multiple commands in one packet
* [ ] Test with a real IRC client
* [ ] Choose one reference IRC client for evaluation

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
* [ ] Work correctly with a real IRC client
* [x] Correctly rebuild full commands from buffered data ending in `\r\n`
* [ ] Avoid crashing or quitting unexpectedly in all tested cases

---

## Mandatory IRC Features

The server must support:

* [x] `PASS` — connection password
* [x] `NICK` — set nickname at basic registration level
* [x] `USER` — set username/realname at basic registration level
* [x] `JOIN` — join a channel at basic level
* [x] `PRIVMSG` — send private or channel messages at basic level

Channel behavior:

* [x] Users can join channels at basic level
* [x] Messages sent to a channel are forwarded to all other users in that channel
* [x] Channels have regular users and operators at basic level

Operator commands:

* [ ] `KICK` — remove a user from a channel
* [ ] `INVITE` — invite a user to a channel
* [ ] `TOPIC` — view/change channel topic
* [ ] `MODE` — change channel modes

Required channel modes:

* [ ] `+i` / `-i` — invite-only channel
* [ ] `+t` / `-t` — restrict topic changes to operators
* [ ] `+k` / `-k` — set/remove channel password
* [ ] `+o` / `-o` — give/remove operator privilege
* [ ] `+l` / `-l` — set/remove user limit

---

## Current Architecture

### Server

The `Server` class is split across several files:

```text
Server.cpp              constructor, destructor, run()
ServerNetwork.cpp       accept, recv, queued send, disconnect, pollout helpers
ServerCommands.cpp      command parsing and dispatcher
ServerRegistration.cpp  PASS, NICK, USER, registration helpers
ServerChannels.cpp      JOIN, PRIVMSG, channel helpers
ServerReplies.cpp       reply helpers and numeric replies
```

### Client

The `Client` class stores:

* Client file descriptor
* Nickname
* Username
* Real name
* Input buffer
* Output buffer
* Password status
* Registration status

The input buffer is used because IRC commands can arrive in pieces. The output buffer is used so replies are only sent after `poll()` reports the fd writable.

### Channel

The `Channel` class stores:

* Channel name
* Topic
* Password/key
* Members
* Operators
* Invite-only mode placeholder
* Topic restriction mode placeholder
* User limit placeholder

---

## Current Manual Tests

Start the server:

```bash
./ircserv 6667 pass
```

Connect with netcat:

```bash
nc -C 127.0.0.1 6667
```

### Registration test

```irc
PASS pass
NICK roger
USER roger 0 * :Roger
```

Expected:

```irc
:ircserv 001 roger :Welcome to the ft_irc server, roger
```

### JOIN test with two clients

Client 1:

```irc
PASS pass
NICK roger
USER roger 0 * :Roger
JOIN #test
```

Client 2:

```irc
PASS pass
NICK titi
USER titi 0 * :Titi
JOIN #test
```

Client 1 should receive:

```irc
:titi!titi@localhost JOIN #test
```

Client 2 should receive names list including:

```irc
:ircserv 353 titi = #test :@roger titi
```

### Channel PRIVMSG test

From Roger:

```irc
PRIVMSG #test :hello channel
```

Titi should receive:

```irc
:roger!roger@localhost PRIVMSG #test :hello channel
```

### Private PRIVMSG test

From Roger:

```irc
PRIVMSG titi :hello directly
```

Titi should receive:

```irc
:roger!roger@localhost PRIVMSG titi :hello directly
```

### Port validation tests

```bash
./ircserv hello pass
./ircserv 0 pass
./ircserv 99999 pass
```

These should all fail with a clear error message.

---

## Current Git Checkpoint

Recommended commit message for the current stage:

```bash
git add .
git commit -m "Refactor server and implement queued replies with PRIVMSG"
```

Alternative:

```bash
git add .
git commit -m "Add POLLOUT replies and basic PRIVMSG"
```

---

## Later Cleanup / Improvement List

### General cleanup

* [ ] Reduce debug `std::cout` messages before final evaluation
* [ ] Keep functions short and focused
* [x] Split `Server.cpp` into smaller files
* [x] Move reply helpers to a separate file
* [x] Move command/registration/channel handlers into separate files
* [ ] Keep helper functions private when they are only used inside `Server`
* [ ] Keep data members private
* [ ] Use public getters/setters only when another class truly needs them

### Server cleanup

* [x] Add basic `Server` destructor
* [x] Close open fds safely in destructor
* [x] Delete `Client` objects safely in destructor
* [x] Delete `Channel` objects safely in destructor
* [ ] Prevent `Server` copying
* [x] Improve basic `poll()` handling for `POLLHUP`, `POLLERR`, `POLLNVAL`
* [ ] Improve `recv()` error handling for `EAGAIN`, `EWOULDBLOCK`, `EINTR`
* [x] Improve `send()` handling with output buffer and `POLLOUT`
* [x] Handle partial sends by keeping unsent output in the buffer

### Input validation

* [x] Replace `atoi()` with safer port parsing
* [x] Reject non-numeric ports
* [x] Reject negative ports
* [x] Reject port `0`
* [x] Reject ports above `65535`
* [ ] Decide if privileged ports under `1024` should be rejected or just allowed to fail

### IRC behavior

* [ ] Decide final behavior for lowercase commands
* [ ] Improve `USER` parsing
* [ ] Improve `NICK` validation
* [ ] Handle nickname change after registration
* [ ] Make sure wrong `PASS` behavior matches expected IRC behavior
* [ ] Verify numeric replies with the chosen real IRC client
* [ ] Possibly add more welcome numerics: `002`, `003`, `004`

### Channels

* [x] Broadcast `JOIN` to existing channel members
* [ ] Remove disconnected client from all channels
* [ ] Delete empty channels if needed
* [ ] Activate invite-only mode logic
* [ ] Activate topic restriction logic
* [ ] Activate password/key logic
* [ ] Activate user limit logic
* [ ] Decide if unused `Channel::broadcast()` should be removed or kept as placeholder

---

## Resources

Useful references:

* RFC 1459 — Internet Relay Chat Protocol
* Modern IRC documentation
* Linux manual pages for: `socket`, `setsockopt`, `bind`, `listen`, `accept`, `fcntl`, `poll`, `recv`, `send`, `close`

---

## AI Usage

AI was used to help understand the subject, and review code ideas.
