
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
./ircserv 6667 pass
```

`6667` is used in our tests because it is the traditional IRC port, but the server must work with any valid port passed as an argument.

---

## Current Status

Estimated mandatory progress:

```text
62% to 65%
```

The project currently has a solid mandatory base implemented.

Implemented so far:

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
* JOIN broadcast to channel members
* `PRIVMSG` to channels
* `PRIVMSG` to users
* `TOPIC`
* `KICK`
* `INVITE` started/prepared
* safer port validation
* `Server.cpp` refactored into smaller files

Still pending:

* final `INVITE` testing
* `MODE`
* channel modes: `+i`, `+t`, `+k`, `+o`, `+l`
* disconnect cleanup from channels
* real IRC client testing

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
* [ ] Work correctly with a real IRC client
* [ ] Avoid crashing or quitting unexpectedly in all tested cases

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
* [x] Channel operators at basic level
* [x] Channel `PRIVMSG`
* [x] User-to-user `PRIVMSG`

### Operator commands

* [x] `TOPIC`
* [x] `KICK`
* [~] `INVITE`
* [ ] `MODE`

### Required channel modes

* [ ] `+i` / `-i` — invite-only channel
* [ ] `+t` / `-t` — restrict topic changes to operators
* [ ] `+k` / `-k` — set/remove channel password
* [ ] `+o` / `-o` — give/remove operator privilege
* [ ] `+l` / `-l` — set/remove user limit

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

Current responsibilities:

```text
Server.cpp              constructor, destructor, run()
ServerNetwork.cpp       accept, recv, queued send, disconnect, POLLOUT helpers
ServerCommands.cpp      command parsing and dispatcher
ServerRegistration.cpp  PASS, NICK, USER, registration helpers
ServerChannels.cpp      JOIN, PRIVMSG, TOPIC, KICK, INVITE, channel helpers
ServerReplies.cpp       reply helpers and numeric replies
Client.cpp              client state, input buffer, output buffer
Channel.cpp             channel members, operators, topic, invited users
```

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
* password/key placeholder
* members
* operators
* invited users
* invite-only mode placeholder
* topic restriction mode placeholder
* user limit placeholder

Channels currently use:

```cpp
std::vector<Client*> _members;
std::vector<Client*> _operators;
std::vector<Client*> _invited;
```

This keeps the channel logic simple and easy to explain.

---

## Why We Use Output Buffers

The project requires non-blocking I/O and one main `poll()` loop.

For incoming data:

```text
poll() reports POLLIN
recv() reads data
data is stored in the input buffer
full IRC commands are extracted using \r\n
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

The only real `send()` call should be inside `handleWrite()`.

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

Then connect with netcat:

```bash
nc -C 127.0.0.1 6667
```

The password is not part of the `nc` command. It is sent after connecting:

```irc
PASS pass
```

---

## Manual Tests

### Registration

```irc
PASS pass
NICK roger
USER roger 0 * :Roger
```

Expected:

```irc
:ircserv 001 roger :Welcome to the ft_irc server, roger
```

---

### JOIN with two clients

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

INVITE is implemented/prepared, but final testing is pending.

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

---

### Port validation

These should fail with a clear error message:

```bash
./ircserv hello pass
./ircserv 0 pass
./ircserv 99999 pass
```

This should work:

```bash
./ircserv 6667 pass
```

---

## Next Steps

Recommended implementation order:

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

## Current Git Checkpoint

Recommended commit message:

```bash
git add .
git commit -m "Implement TOPIC KICK and start INVITE"
```

Alternative:

```bash
git add .
git commit -m "Add TOPIC KICK and invite preparation"
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

AI was used to help understand the subject, plan the work, explain errors, organize the implementation, and review code ideas.

All code added to the project should be reviewed, tested, and understood by the team before final evaluation.