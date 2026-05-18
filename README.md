

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
* [x] Basic `Server::run()` function added
* [x] Server currently compiles and runs
* [x] Basic socket creation started in `Server.cpp`
* [x] `setsockopt()` added with `SO_REUSEADDR`
* [x] Server socket set to non-blocking mode with `fcntl()`

### In Progress

* [ ] Complete `Server` constructor
* [ ] Add `bind()`
* [ ] Add `listen()`
* [ ] Add server socket to `poll()` list
* [ ] Replace temporary `Server::run()` with real server loop
* [ ] Add `acceptClient()`
* [ ] Add `handleClient()`
* [ ] Add proper client disconnection handling

## Mandatory Requirements

According to the subject, the server must:

* [ ] Be written in C++98
* [ ] Compile with `-Wall -Wextra -Werror`
* [ ] Run as `./ircserv <port> <password>`
* [ ] Accept multiple clients at the same time
* [ ] Use non-blocking file descriptors
* [ ] Use only one `poll()` or equivalent loop for I/O handling
* [ ] Never use `fork()`
* [ ] Communicate through TCP/IP
* [ ] Work with a real IRC client
* [ ] Correctly handle partial messages
* [ ] Avoid crashing or quitting unexpectedly

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
* Binding the socket to the chosen port
* Listening for connections
* Running the main `poll()` loop
* Accepting new clients
* Receiving data from clients
* Sending responses
* Dispatching commands

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
* [ ] Bind socket
* [ ] Listen on socket
* [ ] Add socket to poll list
* [ ] Create main poll loop
* [ ] Accept new clients

### Phase 2 — Client Handling

* [ ] Store connected clients
* [ ] Receive data using `recv()`
* [ ] Append received data to client buffer
* [ ] Extract full IRC commands from buffer
* [ ] Handle client disconnects safely

### Phase 3 — Registration

* [ ] Implement `PASS`
* [ ] Implement `NICK`
* [ ] Implement `USER`
* [ ] Mark client as registered only after required data is valid

### Phase 4 — Basic IRC Commands

* [ ] Implement `JOIN`
* [ ] Implement `PRIVMSG`
* [ ] Send messages to channels
* [ ] Send messages to individual users

### Phase 5 — Channel Operators

* [ ] Implement `KICK`
* [ ] Implement `INVITE`
* [ ] Implement `TOPIC`
* [ ] Implement `MODE`

### Phase 6 — Testing

* [ ] Test with `nc`
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
PASS pass
NICK roger
USER roger 0 * :Roger
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