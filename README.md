*This project has been created as part of the 42 curriculum by bsamy & chrrodri.*

# ft_irc — Internet Relay Chat Server

## Description

`ft_irc` is a 42 group project where we build our own IRC server in **C++98**.

The goal is not to create an IRC client, but to create a server that real IRC clients can connect to.

The server must handle multiple clients at the same time using non-blocking sockets and `poll()` or an equivalent system call.

The executable must run like this:

```bash
./ircserv <port> <password>
````

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
* [x] Basic `Server.hpp` created
* [x] Basic `Client.hpp` created
* [x] Basic `Channel.hpp` created
* [x] Basic `main.cpp` created
* [x] Program checks for correct argument count
* [x] Program creates a `Server` object with port and password
* [x] Program calls `server.run()`
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
* [x] Basic `removeClient()` added
* [x] Disconnected clients are closed and removed from the `poll()` list

### Client handling

* [x] `Client.cpp` added
* [x] `Client` constructors/destructor added
* [x] `Client` objects are created when clients connect
* [x] Connected clients are stored in the `_clients` map
* [x] Received data is appended to each client's buffer
* [x] Full IRC commands are extracted from the buffer using `\r\n`
* [x] Multiple commands can be processed from one received buffer
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
* [x] Client becomes registered only after:

  * correct `PASS`
  * valid `NICK`
  * valid `USER`
* [x] Wrong password detected
* [x] Missing nickname detected
* [x] Duplicate nickname detected
* [x] `PASS` after registration rejected
* [x] `USER` after registration rejected
* [x] Welcome numeric `001` sent after registration

### Replies

* [x] `sendReply()` added
* [x] `sendNumeric()` added
* [x] `sendUnknownCommand()` added
* [x] `sendNotRegistered()` added
* [x] Numeric reply `001` sent after successful registration
* [x] Numeric reply `421` sent for unknown commands
* [x] Numeric reply `451` sent when restricted commands are used before registration
* [x] Numeric reply `461` sent for not enough parameters
* [x] Numeric reply `431` sent for missing nickname
* [x] Numeric reply `433` sent for nickname already in use
* [x] Numeric reply `464` sent for wrong password
* [x] Numeric reply `462` sent when trying to register again
* [x] Numeric reply `403` sent for invalid channel name in `JOIN`

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
* [x] `JOIN` sends names list reply `353`
* [x] `JOIN` sends end of names reply `366`
* [x] `JOIN` tested with one registered client
* [x] `JOIN` error `461` tested
* [x] `JOIN` error `403` tested

---

## In Progress

* [ ] Test `JOIN` with two clients
* [ ] Broadcast `JOIN` message to existing channel members
* [ ] Prepare helper to send messages to all channel members
* [ ] Improve port validation
* [ ] Improve error handling for `poll()` / `recv()` edge cases
* [ ] Improve `send()` handling for partial sends
* [ ] Add full cleanup/destructor logic if needed

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
* [ ] Avoid crashing or quitting unexpectedly in all cases

---

## Mandatory IRC Features

The server must support:

* [x] `PASS` — connection password
* [x] `NICK` — set/change nickname at basic registration level
* [x] `USER` — set username/realname at basic registration level
* [x] `JOIN` — join a channel at basic level
* [ ] `PRIVMSG` — send private or channel messages

Channel behavior:

* [x] Users can join channels at basic level
* [ ] Messages sent to a channel are forwarded to all other users in that channel
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

## Planned Architecture

### Server

The `Server` class is responsible for:

* Opening the server socket
* Setting the server socket to non-blocking mode
* Binding the socket to the chosen port
* Listening for incoming connections
* Running the main `poll()` loop
* Accepting new clients
* Setting client sockets to non-blocking mode
* Receiving data from clients
* Storing connected clients
* Removing disconnected clients
* Rebuilding full IRC commands from client buffers
* Dispatching commands to the correct command handler
* Sending replies to clients
* Sending numeric IRC replies
* Managing created channels
* Sending messages to channel members

### Client

The `Client` class stores:

* Client file descriptor
* Nickname
* Username
* Real name
* Input buffer
* Password status
* Registration status

The input buffer is important because IRC commands can arrive in pieces.

The server waits until a full command ending in `\r\n` is received before processing it.

### Channel

The `Channel` class stores:

* Channel name
* Topic
* Password/key
* Members
* Operators
* Invite-only mode
* Topic restriction mode
* User limit

The current `Channel` class already contains placeholders for future channel modes, but not all of them are active yet.

Current implemented channel functions:

* Add member
* Remove member
* Check if a client is a member
* Add operator
* Remove operator
* Check if a client is an operator
* Build a names list

---

## Why We Use `vector` for Channel Members

In the `Server` class, connected clients are stored in a map:

```cpp
std::map<int, Client*> _clients;
```

This makes sense because the server receives a file descriptor from `poll()`, and the file descriptor is the key used to find the correct client quickly.

For channels, we use vectors:

```cpp
std::vector<Client*> _members;
std::vector<Client*> _operators;
```

This is because a channel is naturally a list of users.

A channel needs to answer questions like:

* Who is inside this channel?
* Who is an operator?
* Send this message to everyone in the channel.
* Build the names list for this channel.

For this project, a vector is simple, clear, and easy to explain.

A map could also work, for example:

```cpp
std::map<int, Client*> _members;
```

That would make lookups by file descriptor faster. However, it is a little more technical and not necessary for the size of this project.

Our rule is:

```text
Server:
Use map because clients are found by fd.

Channel:
Use vector because members and operators are simple user lists.
```

This keeps the project easier to understand, easier to defend, and easier to maintain while still being correct for the expected scale of the 42 project.

---

## Server Errors vs Client Replies

For evaluation, the most important thing is that the client receives the correct IRC replies.

Server-side messages like this:

```text
JOIN error: not enough parameters
```

are useful while debugging, but they are not required behavior for the IRC protocol.

Client-side numeric replies are more important. For example:

```irc
:ircserv 461 roger JOIN :Not enough parameters
:ircserv 403 roger test :No such channel
:ircserv 451 * :You have not registered
```

Our approach:

```text
During development:
Keep useful std::cout debug messages.

For protocol behavior:
Send correct numeric replies to the client.

Before final evaluation:
Reduce unnecessary debug output if it makes the server messy.
```

This keeps the project clean while still giving us enough information during testing.

---

## Development Plan

### Phase 1 — Server Foundation

* [x] Create Makefile
* [x] Create basic server object
* [x] Create socket
* [x] Set socket options
* [x] Set socket to non-blocking
* [x] Bind socket
* [x] Listen on socket
* [x] Add server socket to poll list
* [x] Create main poll loop
* [x] Accept new clients
* [x] Set accepted clients to non-blocking mode
* [x] Add accepted clients to poll list
* [x] Receive raw data from clients
* [x] Remove disconnected clients from poll list

### Phase 2 — Client Handling

* [x] Store connected clients as `Client` objects
* [x] Add `Client.cpp`
* [x] Add constructors/destructor for `Client`
* [x] Append received data to client buffer
* [x] Extract full IRC commands from buffer
* [x] Handle partial messages at buffer level
* [x] Handle client disconnects safely at basic level

### Phase 3 — Command Dispatcher

* [x] Parse received IRC command lines
* [x] Extract command name
* [x] Extract command arguments
* [x] Convert command name to an enum type
* [x] Route recognized commands through a dispatcher
* [x] Detect unknown commands
* [x] Send numeric `421` for unknown commands
* [x] Block restricted commands before registration
* [x] Send numeric `451` when restricted commands are used before registration

### Phase 4 — Registration

* [x] Implement `PASS`
* [x] Implement `NICK`
* [x] Implement `USER`
* [x] Mark client as registered only after required data is valid
* [x] Send welcome reply `001`
* [x] Send error replies for registration problems

### Phase 5 — Channel Foundation

* [x] Create `Channel.cpp`
* [x] Add channel constructors/destructor
* [x] Store members
* [x] Store operators
* [x] Add member functions
* [x] Add operator functions
* [x] Build names list
* [x] Test channel logic through `JOIN`

### Phase 6 — Basic IRC Commands

* [x] Implement basic `JOIN`
* [ ] Test `JOIN` with two clients
* [ ] Broadcast `JOIN` to existing channel members
* [ ] Implement `PRIVMSG`
* [ ] Send messages to channels
* [ ] Send messages to individual users

### Phase 7 — Channel Operators

* [ ] Implement `KICK`
* [ ] Implement `INVITE`
* [ ] Implement `TOPIC`
* [ ] Implement `MODE`

### Phase 8 — Testing

* [x] Test server startup
* [x] Test connection with `nc`
* [x] Test raw message reception with `nc`
* [x] Test full command extraction with `PASS`, `NICK`, and `USER`
* [x] Test dispatcher with known and unknown commands
* [x] Test numeric `421` for unknown commands
* [x] Test wrong password
* [x] Test duplicate nicknames
* [x] Test registration success
* [x] Test restricted commands before registration
* [x] Test numeric `451` for unregistered restricted commands
* [x] Test basic `JOIN`
* [x] Test `JOIN` error `461`
* [x] Test `JOIN` error `403`
* [ ] Test partial messages manually
* [ ] Test multiple clients
* [ ] Test joining channels with multiple clients
* [ ] Test private messages
* [ ] Test channel messages
* [ ] Test operator commands
* [ ] Test with real IRC client

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

`127.0.0.1` means the client connects to the server running on the same computer.

`-C` makes `nc` send lines using proper IRC-style `\r\n` endings.

### Registration test

Send:

```irc
PASS pass
NICK roger
USER roger 0 * :Roger
```

Expected client output:

```irc
:ircserv 001 roger :Welcome to the ft_irc server, roger
```

Expected server output:

```text
PASS accepted for fd <number>
Nickname set for fd <number>: roger
Username set for fd <number>: roger
Client fd <number> is now registered as roger
```

### Unknown command test

Send before registration:

```irc
hello
Nick roger
```

Expected client output:

```irc
:ircserv 421 * hello :Unknown command
:ircserv 421 * Nick :Unknown command
```

After registration, the target changes from `*` to the nickname.

Example:

```irc
:ircserv 421 roger hello :Unknown command
```

### Restricted command before registration test

Send before registration:

```irc
JOIN #test
PRIVMSG #test :hello
MODE #test +i
```

Expected client output:

```irc
:ircserv 451 * :You have not registered
:ircserv 451 * :You have not registered
:ircserv 451 * :You have not registered
```

Expected server output:

```text
Command blocked: client is not registered
```

### Basic JOIN test

After registration, send:

```irc
JOIN #test
```

Expected client output:

```irc
:roger!roger@localhost JOIN #test
:ircserv 353 roger = #test :@roger
:ircserv 366 roger #test :End of /NAMES list
```

Expected server output:

```text
roger joined #test
```

### JOIN missing parameter test

Send:

```irc
JOIN
```

Expected client output:

```irc
:ircserv 461 roger JOIN :Not enough parameters
```

### JOIN invalid channel name test

Send:

```irc
JOIN test
```

Expected client output:

```irc
:ircserv 403 roger test :No such channel
```

---

## Why `USER roger 0 * :Roger`?

The IRC `USER` command has this general structure:

```irc
USER <username> <hostname> <servername> :<realname>
```

Example:

```irc
USER roger 0 * :Roger
```

Breakdown:

```text
USER      = command
roger     = username
0         = hostname placeholder
*         = servername placeholder
:Roger    = real name / full name
```

The colon `:` means that everything after it is treated as one final parameter.

Example:

```irc
USER roger 0 * :Roger Pastor Gomez
```

The real name becomes:

```text
Roger Pastor Gomez
```

without splitting it into separate parameters.

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
./ircserv 6667 pass
```

Then connect with:

```bash
nc -C 127.0.0.1 6667
```

The password is not part of the `nc` command. It is sent after connecting:

```irc
PASS pass
```

---

## Current Git Checkpoint

Recommended commit message for the current stage:

```bash
git add .
git commit -m "Implement basic JOIN command"
```

Alternative:

```bash
git add .
git commit -m "Add channel creation and JOIN replies"
```

---

## Later Cleanup / Improvement List

### General cleanup

* [ ] Reduce debug `std::cout` messages before final evaluation
* [ ] Keep functions short and focused
* [ ] Split `Server.cpp` if it becomes too large
* [ ] Move reply helpers to a separate file later if needed
* [ ] Move command handlers to separate files later if needed
* [ ] Keep helper functions private when they are only used inside `Server`
* [ ] Keep data members private
* [ ] Use public getters/setters only when another class truly needs them

### Server cleanup

* [ ] Add/verify proper `Server` destructor
* [ ] Close all open fds safely
* [ ] Delete all `Client` objects safely
* [ ] Delete all `Channel` objects safely
* [ ] Prevent `Server` copying
* [ ] Improve `poll()` handling:

  * `POLLHUP`
  * `POLLERR`
  * `POLLNVAL`
* [ ] Improve `recv()` error handling:

  * `EAGAIN`
  * `EWOULDBLOCK`
  * `EINTR`
* [ ] Improve `send()` handling:

  * partial send
  * failed send
  * disconnected client during send
* [ ] Later use output buffers and `POLLOUT` for stricter non-blocking send handling

### Input validation

* [ ] Replace `atoi()` with safer port parsing
* [ ] Reject non-numeric ports
* [ ] Reject negative ports
* [ ] Reject port `0` unless intentionally allowed
* [ ] Reject ports above `65535`
* [ ] Decide if privileged ports under `1024` should be rejected or just allowed to fail

### IRC behavior

* [ ] Decide final behavior for lowercase commands
* [ ] Improve `USER` parsing
* [ ] Improve `NICK` validation
* [ ] Handle nickname change after registration
* [ ] Make sure wrong `PASS` behavior matches expected IRC behavior
* [ ] Verify numeric replies with the chosen real IRC client
* [ ] Possibly add more welcome numerics:

  * `002`
  * `003`
  * `004`

### Channels

* [ ] Broadcast `JOIN` to existing channel members
* [ ] Remove disconnected client from all channels
* [ ] Delete empty channels if needed
* [ ] Activate invite-only mode logic
* [ ] Activate topic restriction logic
* [ ] Activate password/key logic
* [ ] Activate user limit logic
* [ ] Decide if `broadcast()` should stay inside `Channel` or be handled by `Server`

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

AI was used to help understand the subject, plan the work, explain errors, and review code ideas.
