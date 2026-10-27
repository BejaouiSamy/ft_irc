*This project has been created as part of the 42 curriculum by chrrodri, bsamy.*

# ft_irc — Internet Relay Chat Server

## Description

`ft_irc` is a TCP/IP Internet Relay Chat (IRC) server developed in **C++98** for the 42 curriculum. It accepts multiple simultaneous clients, authenticates users with a server password, supports private and channel messages, and provides the mandatory channel-operator commands.

The project implements an **IRC server**, not an IRC client or server-to-server communication. It focuses on the features required by the `ft_irc` subject rather than every extension of the IRC protocol.

### Implemented features

- **Registration:** `PASS`, `NICK`, `USER`, including numeric replies for invalid credentials, incomplete parameters, and registration errors.
- **Channels:** `JOIN`, channel creation, membership and names replies (`353` and `366`). The first member of a newly created channel becomes its operator.
- **Messaging:** `PRIVMSG` to another registered user or to other members of a joined channel.
- **Operator commands:** `KICK`, `INVITE`, `TOPIC`, and `MODE`.
- **Required channel modes:**
  - `+i` / `-i` — enable or disable invite-only access.
  - `+t` / `-t` — restrict or permit topic changes by regular members.
  - `+k` / `-k` — set or remove a channel key.
  - `+o` / `-o` — grant or revoke channel-operator privileges for a member.
  - `+l` / `-l` — set or remove a channel member limit.
- **Client interoperability:** basic handling of `PING`, `CAP`, and `USERHOST`, in addition to the mandatory commands.

## Instructions

### Requirements

- A Unix-like environment with TCP/IP socket support (developed and tested on Linux/WSL and at 42 School).
- A C++98-compatible compiler and `make`.
- An IRC client such as HexChat, AdiIRC, or `nc` for manual testing.

### Compilation

From the repository root:

```bash
make
```

The Makefile builds `ircserv` using:

```text
c++ -Wall -Wextra -Werror -std=c++98
```

Available targets:

```bash
make          # Build ircserv
make clean    # Remove object files
make fclean   # Remove object files and the executable
make re       # Rebuild from scratch
```

### Running the server

```bash
./ircserv <port> <password>
```

For example:

```bash
./ircserv 6667 pass
```

Here, `6667` is an example listening port, and `pass` is the password clients must supply when connecting. The listening port is provided at runtime; it is not hard-coded to `6667`.

### Testing with netcat

In another terminal:

```bash
nc -C 127.0.0.1 6667
```

Then enter the IRC commands **inside the netcat connection**, not in the shell:

```irc
PASS pass
NICK roger
USER roger 0 * :Roger Example
JOIN #test
PRIVMSG #test :Hello from netcat!
```

In the netcat variant used for testing, `-C` sends CRLF (`\r\n`) line endings as required by IRC. Open another netcat connection with a different nickname to check channel delivery and private messages.

### Testing with a graphical IRC client

**HexChat** was used on a 42 School Linux computer to test actual IRC-client connections, channel joins, public and private messaging, operator promotion, and kicking users. **AdiIRC** was also used on Windows during development.

For a local HexChat connection, create a custom network with these settings:

| Setting | Value |
| --- | --- |
| Server | `127.0.0.1/6667` |
| Server password | `pass` (or the password used to launch the server) |
| Nickname / username | Any available nickname and username |
| TLS/SSL | Disabled for this local example |
| SASL/NickServ | Not required |

In a graphical IRC client, use its slash-command interface, for example:

```irc
/join #test
/topic #test Test topic
/mode #test +t
```

Some optional IRC commands and client extensions are outside the scope of this mandatory-only implementation.

## Architecture and technical choices

The server is organized into three primary classes:

| Class | Responsibility |
| --- | --- |
| `Server` | Listening socket, network events, client/channel registries, command dispatch, and queued replies. |
| `Client` | One accepted connection, its registration state, nickname, username, and input/output buffers. |
| `Channel` | Channel name, topic, members, operators, invitations, key, and channel modes. |

The source files separate networking from IRC command logic:

```text
include/Server.hpp
include/Client.hpp
include/Channel.hpp
src/main.cpp
src/Server.cpp
src/ServerNetwork.cpp
src/ServerCommands.cpp
src/ServerRegistration.cpp
src/ServerChannels.cpp
src/ServerReplies.cpp
src/Client.cpp
src/Channel.cpp
```

### Non-blocking event loop

The server uses **one main `poll()` loop** for the listening socket and connected clients. Listening and accepted sockets are configured in non-blocking mode with `fcntl(fd, F_SETFL, O_NONBLOCK)`.

- `POLLIN` indicates that a listening socket may accept a connection or that a client socket may have data to receive.
- Each client has an **input buffer**: TCP data may arrive in fragments, so complete IRC commands are extracted only once the line terminator has been received.
- Commands are parsed and dispatched to the relevant registration, messaging, or channel handler.
- Replies and forwarded messages are queued in each recipient's **output buffer**.
- `POLLOUT` is monitored when a client has pending output. If `send()` accepts only part of the buffer, only the transmitted bytes are removed and the remainder stays queued.
- `recv()` and `send()` handlers use their return values and return to the event loop without using `errno` to trigger additional network operations. `MSG_NOSIGNAL` is used for `send()` on Linux to avoid a `SIGPIPE` termination on a broken connection.
- On disconnection, the server removes channel references to the client before deleting the `Client` object, removing its poll descriptor, and closing its socket.

No `fork()` or external/Boost library is used.

## Testing and validation

The following manual checks have been performed during development, including tests on a 42 School computer:

- A clean C++98 build with the required compiler flags.
- Correct and incorrect passwords, duplicate nicknames, incomplete registration, and expected IRC error replies.
- Multiple concurrent clients, channel and private messaging, and channel membership checks.
- Operator and regular-user permissions for `KICK`, `INVITE`, `TOPIC`, and all five required `MODE` flags.
- Partial commands and multiple IRC commands sent over one connection.
- Disconnecting clients while others remain active; abrupt TCP resets; and a message sent immediately before closing a connection.
- Suspending netcat clients with `Ctrl+Z` and resuming them with `fg`.
- A controlled stress test that sent **3,000 numbered channel messages** followed by a `FLOOD_END` marker. The final marker was observed by receiving clients and the server remained responsive; an independent count of every message was not performed.
- Real IRC client interaction with HexChat on Linux and AdiIRC on Windows.

A Valgrind Memcheck session after the networking changes reported **0 memory errors** and **0 bytes definitely, indirectly, or possibly lost**. The server was interrupted with `Ctrl+C` (`SIGINT`), so some allocations were reported as **still reachable** rather than being released through a graceful shutdown. These results describe the runs performed, not a guarantee of correctness for every possible network condition.

## Resources

- [RFC 1459 — Internet Relay Chat Protocol](https://www.rfc-editor.org/rfc/rfc1459)
- [RFC 2812 — Internet Relay Chat: Client Protocol](https://www.rfc-editor.org/rfc/rfc2812)
- [Linux `poll(2)` manual](https://man7.org/linux/man-pages/man2/poll.2.html)
- [Linux `recv(2)` manual](https://man7.org/linux/man-pages/man2/recv.2.html)
- [Linux `send(2)` manual](https://man7.org/linux/man-pages/man2/send.2.html)
- [Linux `fcntl(2)` manual](https://man7.org/linux/man-pages/man2/fcntl.2.html)
- [HexChat documentation](https://hexchat.readthedocs.io/)
- [AdiIRC documentation](https://dev.adiirc.com/projects/adiirc/wiki)
- The official 42 `ft_irc` project subject and its evaluation requirements.

### AI usage

AI was used as a learning and development aid to explain IRC, TCP sockets, `poll()`, non-blocking I/O, buffers, and C++98 concepts; to explore implementation options and **suggest code changes**; to investigate compiler and runtime behavior; to prepare manual testing scenarios and evaluation checklists; and to review and improve project documentation.

AI-generated suggestions were reviewed, adapted, and tested during development. The team is responsible for understanding the submitted implementation, checking it against the applicable subject, and explaining and defending its design and behavior during peer evaluation.
