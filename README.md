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

## Current Project Status

### Done

* [x] Basic project structure created
* [x] `include/` folder created
* [x] `src/` folder created
* [x] `src/commands/` folder created
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
* [x] Basic `Server::run()` created
* [x] Server currently compiles and runs
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
* [x] Basic `handleData()` added
* [x] Data is received with `recv()` after `poll()` reports the fd as ready
* [x] Basic `removeClient()` added
* [x] Disconnected clients are closed and removed from the `poll()` list
* [x] Basic manual test done with `nc`

### In Progress

* [ ] Store connected clients in the `_clients` map
* [ ] Create real `Client` objects when clients connect
* [ ] Add `Client.cpp`
* [ ] Add client buffer handling
* [ ] Append received data to each client buffer
* [ ] Extract full IRC commands ending in `\r\n`
* [ ] Add command dispatcher

## Mandatory Requirements

According to the subject, the server must:

* [x] Be written in C++98
* [x] Compile with `-Wall -Wextra -Werror`
* [x] Run as `./ircserv <port> <password>`
* [x] Use TCP/IP sockets
* [x] Use non-blocking file descriptors
* [x] Use `poll()` or equivalent for I/O handling
* [x] Avoid `fork()`
* [ ] Accept multiple clients at the same time
* [ ] Work with a real IRC client
* [ ] Correctly handle partial messages
* [ ] Avoid crashing or quitting unexpectedly in all cases

## Mandatory IRC Features

The server must support:

* [ ] PASS — connection password
* [ ] NICK — set/change nickname
* [ ] USER — set username/realname
* [ ] JOIN — join a channel
* [ ] PRIVMSG — send private or channel messages

Channel behavior:

* [ ] Users can join channels
* [ ] Messages sent to a channel are forwarded to all other users in that channel
* [ ] Channels have regular users and operators

Operator commands:

* [ ] KICK — remove a user from a channel
* [ ] INVITE — invite a user to a channel
* [ ] TOPIC — view/change channel topic
* [ ] MODE — change channel modes

Required channel modes:

* [ ] `+i` / `-i` — invite-only channel
* [ ] `+t` / `-t` — restrict topic changes to operators
* [ ] `+k` / `-k` — set/remove channel password
* [ ] `+o` / `-o` — give/remove operator privilege
* [ ] `+l` / `-l` — set/remove user limit

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
* Removing disconnected clients
* Later: dispatching commands to the correct command handler

### Client

The `Client` class should store:

* Client file descriptor
* Nickname
* Username
* Real name
* Input buffer
* Password status
* Registration status

The input buffer is important because IRC commands can arrive in pieces.

The server must wait until a full command ending in `\r\n` is received before processing it.

### Channel

The `Channel` class should store:

* Channel name
* Topic
* Password/key
* Members
* Operators
* Invited clients
* Invite-only mode
* Topic restriction mode
* User limit

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

* [ ] Store connected clients as `Client` objects
* [ ] Add `Client.cpp`
* [ ] Add constructors/destructor for `Client`
* [ ] Append received data to client buffer
* [ ] Extract full IRC commands from buffer
* [ ] Handle partial messages correctly
* [ ] Handle client disconnects safely

### Phase 3 — Command Dispatcher

* [ ] Parse received IRC command lines
* [ ] Extract command name
* [ ] Extract command parameters
* [ ] Route commands to the correct handler

### Phase 4 — Registration

* [ ] Implement `PASS`
* [ ] Implement `NICK`
* [ ] Implement `USER`
* [ ] Mark client as registered only after required data is valid

### Phase 5 — Basic IRC Commands

* [ ] Implement `JOIN`
* [ ] Implement `PRIVMSG`
* [ ] Send messages to channels
* [ ] Send messages to individual users

### Phase 6 — Channel Operators

* [ ] Implement `KICK`
* [ ] Implement `INVITE`
* [ ] Implement `TOPIC`
* [ ] Implement `MODE`

### Phase 7 — Testing

* [x] Test server startup
* [x] Test connection with `nc`
* [x] Test raw message reception with `nc`
* [ ] Test partial messages
* [ ] Test multiple clients
* [ ] Test wrong password
* [ ] Test duplicate nicknames
* [ ] Test joining channels
* [ ] Test private messages
* [ ] Test channel messages
* [ ] Test operator commands
* [ ] Test with real IRC client

Example test:

```bash
nc -C 127.0.0.1 6667
```

Then send:

```irc
hello
```

Expected current behavior:

```text
Client connected with fd <number>
Data received from fd <number>: hello
```

Later expected behavior after client buffer handling:

```irc
PASS pass
NICK roger
USER roger 0 * :Roger
```

Expected future server output:

```text
Full IRC command from fd <number>: PASS pass
Full IRC command from fd <number>: NICK roger
Full IRC command from fd <number>: USER roger 0 * :Roger
```

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

## Execution

Run the server with:

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 pass
```

## Current Git Checkpoint

Recommended commit message for the current stage:

```bash
git add .
git commit -m "Set up non-blocking server socket with poll and client accept"
```

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
