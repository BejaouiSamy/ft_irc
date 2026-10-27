# ft_irc — Internal Oral Defense and Code Modification Masterclass

**Team:** `bsamy` and `chrrodri`  
**Updated:** October 27, 2026  
**Study language:** English throughout, including technical explanations and sample oral answers.  
**Project:** IRC server in C++98, **mandatory part only**, based on subject **11.0**.  
**Goal:** Both team members should be able to locate, explain, safely modify, and test the important parts of the project without memorizing code they do not understand.

> This guide is based on the architecture, code excerpts, and test outputs shared during the project review. It is not an exhaustive audit of every line in the latest repository. If a behavior depends on source code that has not been fully inspected, it is labeled **verify in the submitted version**.

---

## 1. Your 90-second introduction (practice aloud)

**Evaluator:** “Can you explain your project?”

**Suggested answer:**

> Our project is an IRC server written in C++98. It accepts multiple TCP clients and allows them to register with a password, nickname and username. Users can join channels, exchange private and channel messages, and perform channel operations depending on their privileges.
>
> We use a single poll-based event loop with non-blocking sockets, so one slow client does not block everyone else. Each client has an input buffer to rebuild complete IRC commands and an output buffer to queue data until the socket is writable.
>
> Our Server manages connections and commands, Client stores one user's state, and Channel manages members, operators, invitations, topics and modes. We tested the mandatory commands with netcat, used HexChat as a real IRC client, and checked networking robustness and memory behavior with stress tests and Valgrind.

**Three mistakes to avoid:** (1) Saying that `poll()` reads data: **`recv()`** actually reads it. (2) Saying clients are created after `PASS/NICK/USER`: they are created **after `accept()`**. (3) Saying commands run in parallel: our server handles **concurrent I/O in one event loop**; we have not shown parallel execution.

---

## 2. Complete mental map of the server

```text
./ircserv <port> <password>
       |
       v
     main.cpp
       |
       v
  Server constructor
  socket -> setsockopt -> fcntl(O_NONBLOCK) -> bind -> listen
       |
       v
  _fds contains the listening fd
       |
       v
  while (...) -> poll(&_fds[0], _fds.size(), -1)
       |
       +---- listening fd + POLLIN ----> accept() -> new Client(fd)
       |
       +---- client fd + POLLIN ------> recv()
       |                                 |
       |                                 v
       |                            input buffer
       |                                 |
       |                            extractMessage()
       |                                 |
       |                            dispatchCommand()
       |                                 |
       |                         PASS / NICK / USER /
       |                         JOIN / PRIVMSG / MODE ...
       |                                 |
       |                            queue reply/data
       |                                 |
       +---- client fd + POLLOUT -----> send() output buffer
       |                                 |
       |                              partial send? keep remainder
       |
       +---- disconnect/error -------> removeClientFromChannels()
                                         delete Client
                                         remove map and pollfd
                                         close(fd)
```

**Accurate explanation:** This diagram represents the logical flow. TCP may split or combine received data, and the operating system determines when `poll()` reports readiness events.

---

## 3. Files, classes, and what to explain during the evaluation

### `src/main.cpp`

**Purpose:** Entry point. Receives `argc/argv`, validates arguments according to our implementation, constructs the `Server` object, and starts its main loop.

**Show in the source:** How `<port>` and `<password>` reach the constructor.

**Question:** “What does `./ircserv 6667 pass` mean?”

> It starts our server listening on port 6667. Every client must authenticate using the server password pass before completing registration.

**Verify in the final code:** The exact behavior for an incorrect argument count, invalid port, or bind failure. Do not invent an answer.

### `src/Server.cpp`

**Purpose:** Creates the socket and listening descriptor; its destructor destroys owned objects and closes descriptors if a normal exit occurs; implements the main `Server::run()` loop.

**Known section of the loop:**

```cpp
int ret = poll(&_fds[0], _fds.size(), -1);
if (ret == -1)
    throw std::runtime_error("poll() failed");
for (size_t i = 0; i < _fds.size(); ) {
    int fd = _fds[i].fd;
    short revents = _fds[i].revents;
    // Decide what to do using the returned event bits.
}
```

**Concepts:** `poll()` returns the number of descriptors with events, or `-1` on error. A timeout of `-1` waits indefinitely for an event or interruption. **One `poll()` call site does not mean poll only runs once: that same call is executed repeatedly inside the loop.**

**Known risk:** The code we inspected prioritizes `POLLHUP | POLLERR | POLLNVAL` over `POLLIN`. If both occur together, bytes received before closing could be discarded. The final-message test passed, but does not cover every possible timing case.

**Another risk:** A signal interruption can make `poll()` return `-1`; the observed code throws an exception for every `-1`. Do not introduce changes without checking the requirements, tests, and signal handling implications.

### `src/ServerNetwork.cpp`

**Purpose:** `acceptClient()`, `handleData(fd)`, `handleWrite(fd)`, `removeClient(fd)`, `enablePollOut(fd)`, and `disablePollOut(fd)`.

**What has been seen and verified:**

- `acceptClient()` uses `accept()`, makes the client socket non-blocking, adds a `pollfd` with `events = POLLIN`, and stores `new Client(client_fd)` in `_clients`.
- `handleData()` calls `recv()` and appends incoming bytes to the `Client` input buffer to reconstruct commands.
- `handleWrite()` uses `send(..., MSG_NOSIGNAL)` on Linux and erases only the number of bytes actually sent.
- `removeClient()` removes channel references first, deletes the `Client`, removes it from the map and poll vector, then closes the fd.
- `enablePollOut()` uses `events |= POLLOUT`; `disablePollOut()` uses `events &= ~POLLOUT`.

**Subject 11.0:** Forbids relying on `errno` to choose the next action after `read/recv` or `write/send`. The team reported removing those branches, then rerunning basic and flood tests successfully. **Compare the exact submitted file before claiming compliance.**

### `src/ServerCommands.cpp`

**Known purpose:** Parse an IRC line into a command and arguments, then dispatch to the relevant handler. Minimal `CAP`, `PING`, and `USERHOST` compatibility was added for real IRC clients.

**Explanation:** An incoming command is **not executed once per TCP packet**. The program waits for a complete IRC line, then parses and dispatches it.

**Find during the defense:** `dispatchCommand()`, argument parser, and handling of an unknown command (`421`). Check whether command names are normalized for uppercase/lowercase; do not assume unless tested.

### `src/ServerRegistration.cpp`

**Known purpose:** `PASS`, `NICK`, `USER`, completion of registration, and numeric `001`.

**Different states:** Having a `Client` object, knowing its nickname and username, and being fully registered **are not the same thing**. The TCP socket is accepted before IRC registration.

**Tested case:** `PASS wrong` produces `464`. Sending NICK/USER still does not permit JOIN (`451`). A later correct `PASS pass` causes `001`; repeating PASS after registration causes `462`.

### `src/ServerChannels.cpp`

**Known purpose:** Channel creation and joining, message distribution, KICK/INVITE/TOPIC/MODE, and membership cleanup.

**Locate in the code:** Each command handler, membership checks, operator checks, and `_channels` access.

**Caution:** This file contains several handlers. Locate the function with `grep -n` before modifying it. Do not paste a patch into an arbitrary location without understanding the variables and return paths.

### `src/ServerReplies.cpp`

**Known purpose:** Construct numeric IRC replies and queue them rather than calling `send()` directly from every command handler.

**Defense concept:** Centralizing replies helps preserve the format `<prefix> <numeric> <nick> <params> :trailing`.

### `src/Client.cpp` and `include/Client.hpp`

**Purpose:** Store a TCP client's fd, nickname, username, registration state, input/output buffers, and relevant methods.

**Known ownership:** The `_clients` map in `Server` stores `Client*` pointers. When a connection is removed, **Server** deletes the object. The `Client` is created when TCP is accepted, not when USER is received.

**Buffers:** The input buffer stores incomplete bytes; the output buffer holds bytes not yet transmitted. A `std::string` can contain several complete messages or part of one.

### `src/Channel.cpp` and `include/Channel.hpp`

**Known purpose:** Store channel name, topic, members, operators, invited users, join key, and member limit.

**Observed design:** Vectors contain `Client*` pointers representing members, operators, and invited users. Those vectors **do not own** the `Client` objects. If a client is deleted without first removing its pointers from these vectors, dangling pointers remain.

**Question:** “Why does the first user become operator?”

> When a channel is created, it has no operator yet. Our implementation makes its first member an operator so the channel can be managed immediately.

**Verify if asked:** What happens when the last member leaves, and what happens when the last operator disconnects. Do not claim uninspected behavior is guaranteed.

---

## 4. Networking fundamentals: understand these, do not merely memorize them

### 4.1 Socket, port, and file descriptor

- **TCP socket:** One endpoint of reliable, ordered, byte-stream communication between processes.
- **Listening socket:** Bound to a port and used to accept connection requests. It is not Roger's individual socket.
- **`accept()`:** Returns a **new fd** for one client connection, which can be added to the `poll()` vector.
- **File descriptor (`fd`):** An integer meaningful **within this process**, identifying an open resource; `close(fd)` releases that descriptor, and the OS may reuse its number.
- **Port:** Identifies the TCP service at the destination. Port 6667 is an example, not a fixed requirement.
- **Concurrency vs. parallelism:** The server interleaves work across connections; it does not follow that multiple handlers execute physically at once.

### 4.2 `fcntl()`

```cpp
fcntl(fd, F_SETFL, O_NONBLOCK);
```

- `fcntl` stands for file control.
- `F_SETFL` means set file status flags.
- `O_NONBLOCK` means that an appropriate operation does not wait when it cannot proceed immediately.
- Previously we used `F_GETFL` followed by `flags | O_NONBLOCK` to preserve existing flags; we changed it to the exact form in the supplied evaluation scale.

**Defense answer:**

> We configure both the listening socket and each accepted client socket as non-blocking, using the exact fcntl form required by our evaluation scale.

### 4.3 `pollfd` and event flags

```cpp
struct pollfd {
    int fd;
    short events;
    short revents;
};
```

**`events`** are the events we request. **`revents`** are the events the kernel reports.

| Flag | Practical meaning | Typical server action |
|---|---|---|
| `POLLIN` | A read or accept can normally be attempted without waiting | `accept()` on listening fd; `recv()` on client fd |
| `POLLOUT` | A write can normally be attempted | `send()` some or all queued output |
| `POLLHUP` | Hangup; the peer has closed or the connection is no longer usable in the relevant direction | Clean up, but do not assume unread bytes never remain |
| `POLLERR` | Error condition on the fd | Handle failure/disconnect |
| `POLLNVAL` | Invalid descriptor in the poll set | Investigate/remove it |

`POLLRDHUP` is available on Linux for certain half-closed connections. We have not seen it used in the current implementation, and it is **not an additional mandatory feature**.

**Question:** “Does poll receive IRC messages?”

> No. Poll reports which file descriptors are ready. Then our server calls recv to actually receive bytes.

### 4.4 `recv()`

The known form in the project:

```cpp
char buffer[1024];
ssize_t bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
```

- `fd`: The connection descriptor.
- `buffer`: Destination for received bytes.
- `sizeof(buffer)-1`: Maximum requested bytes (1,023), leaving room for a C terminator in the current implementation.
- `0`: No special flags requested by this call.
- `ssize_t`: Signed type, capable of storing a positive byte count or `-1`.

**Return values:**

| Return | Explanation |
|---|---|
| `> 0` | This number of bytes arrived; **not necessarily one complete IRC command** |
| `== 0` | For a positive-size TCP read, the peer orderly closed its sending direction |
| `-1` | An operating-system error or unavailable operation; **not automatically a disconnect** |

**Subject 11.0 caution:** The wording prohibits relying on `errno` after `recv`/`send` to decide what to do next. Do not propose an immediate `EAGAIN`-based retry loop. The team's reported revision returns to the event loop for negative results and uses `poll()` errors/hangups; **permanent failures still require robustness verification**.

### 4.5 `send()` and the output buffer

```cpp
ssize_t n = send(fd, client->getOutputBuffer().c_str(),
                 client->getOutputBuffer().length(), MSG_NOSIGNAL);
```

If 1,000 bytes are pending and `send()` accepts 300, **erase only 300**. The other 700 remain queued for a future writable event. Partial writes are normal; they do not mean data was lost.

On Linux, `MSG_NOSIGNAL` prevents the default `SIGPIPE` termination for that send call when the socket is broken. It does not prove the socket is valid; errors still need handling.

Each client is monitored for `POLLIN`. The server enables `POLLOUT` **only while output is queued** and disables it when the queue is empty, avoiding unnecessary wakeups on sockets that are nearly always writable.

**Known risk:** If a negative `send()` always preserves output but the error is permanent and `poll()` repeatedly reports writability, the event loop may **spin without useful progress**. Do not claim this is ruled out without a targeted test.

### 4.6 An IRC line, not a TCP packet, is the processing unit

`PASS pass\r\n` uses CRLF (`\r\n`) as an IRC line terminator. One `recv()` might return `PA`, another `SS pass\r`, and a third `\n`; or it might return several complete commands at once. That is why the code uses `appendBuffer()` and `extractMessage()`.

`extractMessage()` is called until there are no more complete lines in the input buffer. Incomplete bytes remain stored while other clients continue working.

### 4.7 `std::string` and embedded NUL bytes

The reviewed code contained:

```cpp
buffer[bytes] = '\0';
client->appendBuffer(std::string(buffer));
```

This works for ordinary textual IRC commands, but `std::string(buffer)` stops at an embedded NUL byte. An alternative that preserves exactly `bytes` received bytes is `std::string(buffer, bytes)` if the team decides to investigate. **Do not change untested code unnecessarily**; understand the limitation.

---

## 5. The complete journey of a message

**Example:** Roger sends `PRIVMSG #42 :Hello` and Titi receives it.

1. Roger has an accepted TCP connection, represented by a `Client` object.
2. The kernel reports that Roger's fd is readable; `poll()` returns a `POLLIN` event.
3. `Server::handleData(fd)` calls `recv(fd, ...)`.
4. Received bytes are appended to **Roger's Client input buffer**.
5. When a line terminator arrives, `extractMessage()` returns one complete IRC command.
6. `dispatchCommand()` recognizes `PRIVMSG` and identifies `#42` as the destination.
7. The handler finds `Channel #42`, validates the relevant conditions, and selects the recipient members.
8. A correctly prefixed IRC message is added to **each recipient's output buffer**.
9. `enablePollOut(recipient_fd)` requests `POLLOUT` readiness events.
10. Later, `poll()` reports that writing is possible, and `handleWrite()` calls `send()`.
11. After a partial write, only the transmitted bytes are removed; when the queue becomes empty, `POLLOUT` is disabled.
12. Titi's IRC client reads the TCP data and displays the message in the channel.

**Suggested oral defense answer:**

> When Roger sends a message, poll reports POLLIN for his socket. We call recv and append the bytes to his input buffer. After a complete IRC line is available, we parse and dispatch PRIVMSG. We find the destination channel and queue the formatted message in each recipient's output buffer. Poll later reports POLLOUT and send transmits the queued bytes. If the write is partial, we keep the remaining bytes for a later event.

**Follow-up:** “Do you send the channel message back to Roger?”

> In our implementation, channel PRIVMSG is forwarded to the other channel members, not duplicated back to the sender. That's why the evaluator uses at least two clients.

---

## 6. IRC registration: PASS, NICK, USER

### Understanding the USER line

```irc
USER roger 0 * :Roger Example
```

- `USER`: IRC command name.
- `roger`: Username.
- `0`: Historical mode/host-related parameter; our implementation expects a four-parameter form.
- `*`: Historical server-name parameter.
- `:Roger Example`: Final (*trailing*) parameter; the leading `:` allows spaces in the real name.

**Do not type** `USER roger 0 *:Roger`; the two parameters become joined and this implementation may return `461`.

### Numeric replies to recognize

| Code | Meaning or observed context |
|---|---|
| `001` | Welcome after registration is complete |
| `401` | Private-message target does not exist |
| `403` | Requested channel does not exist |
| `421` | Unknown command |
| `433` | Nickname already in use; also investigate the same-nick edge case |
| `451` | Client is not yet registered |
| `461` | Not enough parameters |
| `462` | Attempt to register again |
| `464` | Incorrect password |
| `471` | Channel full under `+l` |
| `473` | Invite-only channel (`+i`) |
| `475` | Incorrect channel key |
| `482` | Not a channel operator |
| `442` | User is not on that channel (observed after KICK) |
| `324` | Channel mode query reply |
| `353` / `366` | NAMES listing and end of NAMES |

**Remember:** A reply number only makes sense alongside its parameters and the client's/channel's current state. Explain *why* a reply occurred rather than just memorizing codes.

### Why did a second, correct PASS complete registration?

The implementation stores NICK and USER even before a password is accepted, but **does not allow JOIN without the correct PASS**. `PASS wrong` returned `464` and JOIN returned `451`; after `PASS pass`, the server sent `001`. This is **not equivalent to accepting a wrong password**.

---

## 7. Channels and mandatory modes

### Conceptual Channel structure

- `_members`: Users currently in the channel.
- `_operators`: Users authorized to perform restricted channel actions.
- `_invited`: Users allowed into restricted channels.
- `_topic`: Channel topic text.
- `_password` / key: Required join key when `+k` is active.
- Mode flags and `_userLimit`: Controls over channel access, permissions, and capacity.

These identifiers come from known snippets and summaries; **verify the actual declarations in `Channel.hpp`** during practice.

### Modes

| Mode | Behavior | Short oral explanation |
|---|---|---|
| `+i` / `-i` | Invitation required / unrestricted entry | “It restricts channel entry to invited users.” |
| `+t` / `-t` | Only operators can change the topic / regular members can too | “It restricts topic changes to operators.” |
| `+k` / `-k` | Set / remove the channel key | “It protects a channel with a join key.” |
| `+o nick` / `-o nick` | Grant / revoke operator privileges for a member | “It changes one member's operator privilege.” |
| `+l N` / `-l` | Set / remove the member limit | “It controls how many members can join.” |

**Why did `MODE #ops` return `324 ... +` after `+o titi`?** Because `+o` is a privilege attached to Titi, not a general channel setting. When `i/t/k/l` are off, the channel mode string can consist of `+` alone.

### Operators versus regular members

- `KICK`: An operator removes a user from a channel.
- `INVITE`: Allows another user into a channel; check permissions, especially under `+i`.
- `TOPIC`: Reads or changes the topic; `+t` restricts changes to operators.
- `MODE`: Changes channel restrictions or privileges. First ensure the requester has sufficient rights.

**Do not confuse KICK with disconnecting from the server.** A user kicked from one channel may remain connected to other channels and still exchange direct messages.

---

## 8. Disconnection, explained clearly

**Closing a window, Ctrl+C in nc, FIN, RST, or a socket error:** The operating system changes the socket state. `poll()` can report readability, an error, or a hangup, and `recv()` can return zero. There is no single universal flag for every kind of disconnection.

Based on the code we inspected, `removeClient(fd)` does the following:

```text
1. Find Client* in _clients
2. removeClientFromChannels(client)   [remove references]
3. delete client                       [free the object]
4. _clients.erase(iterator)            [erase the map entry]
5. _fds.erase(iterator)                [stop monitoring the fd]
6. close(fd)                           [close the TCP descriptor]
7. Print a disconnect log message
```

**Essential definitions:**

- **Dangling pointer:** A channel still holds the address of a deleted `Client`.
- **Memory leak:** Dynamically allocated memory becomes unreachable and cannot be freed through its owner.
- **FD leak:** The program fails to close an open descriptor.
- **Use-after-free:** Using a `Client*` **after** `delete`.
- **Undefined behavior:** C++ gives no guarantees about the outcome; it might crash, corrupt data, or seem to work.

**Defense answer:**

> We remove the client's references from channels first, so no channel keeps pointers to a deleted object. Then we delete the Client, remove it from the server's containers, and close the socket descriptor. Other clients remain connected.

### Why our poll for loop does not automatically use `++i`

Imagine `_fds = [server, Roger, Titi, Katia]`. At index 1, Roger is removed, so Titi shifts to index 1. A `for (...; ++i)` would jump to index 2 and **skip Titi during that pass**. Instead, we increment only when `_fds[i].fd` still matches the fd we just handled.

**This does not mean poll is skipped or the loop stops making progress:** If the fd is removed, the next element shifts into place; otherwise, the index advances.

---

## 9. Valgrind and Ctrl+C: an accurate defense explanation

**Most recent reported result (PID 108518):**

```text
ERROR SUMMARY: 0 errors from 0 contexts
       definitely lost: 0 bytes
       indirectly lost: 0 bytes
         possibly lost: 0 bytes
       still reachable: 73,760 bytes in 3 blocks
```

Known breakdown: `32 bytes` from `std::vector<pollfd>`, `1,024 bytes` from a C stdio buffer, and `72,704 bytes` from libstdc++.

**Why it happened:** The server was stopped with `Ctrl+C`, and the process died from `SIGINT` while waiting in `poll()`. That path bypassed normal destruction of the `Server` object.

**Distinction:** `still reachable` means a reference path still exists; `definitely lost` means no reachable path remains. Do not claim that 73,760 bytes were leaked. Equally, do not claim “all memory was correctly freed”: that run did not execute the normal destructor. **The evidence is favorable, not a guarantee of passing.**

**Evaluator:** “Why did Valgrind show still reachable allocations?”

> We stopped the server with Ctrl+C, which terminated it while the main poll loop was running. The normal destructor was not executed. Valgrind reported zero errors and zero lost bytes, but it still found reachable allocations, including a small poll-vector block and C/C++ library buffers. We can explain the difference between reachable memory and leaked memory.

**Should we add a signal handler for cleanup?** It is technically possible, but a signal handler must not directly manipulate `std::vector`/`std::map` or perform complex destruction. A signal-safe mechanism should notify the event loop, with cleanup outside the handler. **Do not add it to the final project without a clear need and extensive testing.**

---

## 10. HexChat, optional commands, and the subject's scope

At 42 School, HexChat connected, joined channels, exchanged messages with `nc` clients, changed `+o` privileges, and executed KICK. The client also displayed unknown-command replies (`421`) for `WHO`/`WHOIS`.

**What to say:**

> Our subject requires a working reference IRC client for registration, channels, messages, and the mandatory operator commands. We used HexChat successfully for those interactions. WHO and WHOIS are standard IRC queries outside the explicit mandatory list; our server currently does not implement the full IRC RFC.

**Important nuance:** The subject also says the reference client must connect “without encountering any error,” which can prompt questions about optional-command warnings. Acknowledge those warnings accurately; do not claim they were caused by incorrect login without evidence. Mandatory features working is positive evidence, not an automatic exemption from every compatibility concern.

**QUIT:** It is not explicitly listed among required commands, but standard IRC clients commonly use it. It makes a useful defense exercise; do not add it carelessly without considering pointers and the dispatcher.

---

## 11. Oral question bank: practice as a pair

**Practice method:** One person asks; the other answers in English within 20–45 seconds. Then open the relevant file and point to the function. Switch roles. Suggested scores: 0–4 incorrect concepts, 5–6 partial, 7–8 good, 9–10 clear reasoning supported by source code.

1. What is the difference between an IRC server and an IRC client?
2. What is the difference between a listening socket and an accepted client socket?
3. At what exact moment do we create `Client`?
4. Why use `poll()` instead of one blocking `recv()` per client?
5. Does non-blocking I/O mean parallel execution?
6. What do `.events` and `.revents` represent in `pollfd`?
7. Explain POLLIN, POLLOUT, POLLHUP, POLLERR and POLLNVAL.
8. Why do we monitor POLLOUT only when output is pending?
9. Can one recv contain three IRC commands? Can one command require three recv calls?
10. Why must IRC commands be correctly terminated with `\r\n`?
11. What happens if recv returns 0? What about -1?
12. Why does subject 11.0 constrain the use of `errno`?
13. What do we do when send writes only 300 bytes out of 1,000?
14. What is SIGPIPE and why do we use MSG_NOSIGNAL on Linux?
15. Why do we remove channel references before deleting Client?
16. What is the difference between a dangling pointer and a memory leak?
17. Why can file descriptor numbers be reused after close?
18. What is the purpose of PASS/NICK/USER and when is registration complete?
19. Why can a client have a nickname but still be unregistered?
20. What does `USER roger 0 * :Roger Example` mean?
21. What happens if a regular member tries KICK?
22. How does `+i` change JOIN behavior? How do you test it?
23. Why doesn't `+o` necessarily appear in a general MODE query?
24. Why is a user kicked from a channel still connected to the server?
25. What does `353` mean? Why do we send `366`?
26. What is the benefit of having separate Server, Client and Channel classes?
27. Why doesn't Ctrl+Z normally close a TCP socket?
28. What did the 3,000-message flood prove, and what did it *not* prove?
29. What exactly did Valgrind report? Is still reachable a lost allocation?
30. How would you locate a handler quickly and safely make a one-line change?

**Tip:** If you don't know, locate the responsible file and reason through the flow. Never invent comprehensive RFC behavior that your implementation does not provide.

---

## 12. Code modification exercises (temporary practice branch)

**Essential rule:** These are **proposed training exercises**, not confirmed official evaluation questions. The supplied five-page scale specifies no live coding task. Never apply them to `main` or the submission branch without discussing it with your partner.

### Create a practice branch

```bash
git status -sb
git switch -c practice/defense-irc
```

If the 42 workstation has an older Git version that does not support `git switch`, use `git checkout -b practice/defense-irc`. Do not push this branch to the submission remote. After practice, switch back to the original branch and check `git status`.

### Exercise 1 — Change the welcome reply (very easy)

**Evaluator task:** “Please include the current nickname in the welcome message and show the result.”

**Steps:** Search for `001` or `Welcome` in `src/ServerRegistration.cpp` and `src/ServerReplies.cpp`; identify the responsible function; change **only the text**; compile; verify with `nc`. Revert your practice change with Git so it does not enter the mandatory version.

```bash
grep -Rn 'Welcome\|001' src include
make
```

**What to explain:** Where registration completion is checked, how the reply is constructed, and why command handlers do not call `send()` directly.

### Exercise 2 — Add a validation (easy)

**Evaluator task:** “Reject a malformed USER with a clear error.”

**Find:** `ServerRegistration.cpp`, the USER handler, parameter count, and numeric `461`. On your practice branch, modify one validation case **without altering the basic architecture**. Test both invalid and valid USER on fresh connections.

**What to explain:** Tokenization, trailing parameters, and when `checkRegistration()` runs.

### Exercise 3 — Find an operator permission check (easy/medium)

**Evaluator task:** “Show me where your code refuses TOPIC changes from non-operators when +t is active.”

**Find:** `handleTopic`, `isOperator`, the topic-restriction mode (confirm exact identifier names). An evaluator might only ask you to identify or alter one condition. Demonstrate `482` from a nonoperator and success from an operator.

**What to explain:** A member is not necessarily an operator; channel state determines permissions, and validation must happen before changing state.

### Exercise 4 — Minimal QUIT, practice only (medium)

**Evaluator task:** “Add a basic QUIT command that closes the client connection.”

**Learning goal:** Recognize a complete IRC command, clean up the client, and avoid touching a destroyed pointer. A simple place in the architecture we reviewed is in `handleData()`'s loop over complete messages, **before** dispatch:

```cpp
message = client->extractMessage();
if (message.empty())
    break ;
if (message == "QUIT" || message.compare(0, 5, "QUIT ") == 0)
{
    removeClient(fd);
    return ;  // Important: client is now destroyed.
}
dispatchCommand(client, message);
```

**Important limitations:**

- This is only a **minimal practice implementation**, not a fully RFC-compliant QUIT command. It does not notify other channel members of the QUIT or comprehensively handle all reasons, spaces, and command-case variations.
- **Never** call `removeClient(fd)` and then continue using `client` in the loop: that would be use-after-free.
- If QUIT is implemented inside a handler invoked by `dispatchCommand()`, the caller needs a way to know the `Client` was deleted; otherwise `handleData()` could continue dereferencing it. That is why this placement is shown for learning rather than presented as a finished architecture.
- Check that `extractMessage()` returns a line without its trailing `\r\n`; a reasoned QUIT takes the form `QUIT :reason`.

**Manual test:** Connect two clients, JOIN `#quitpractice`, send `QUIT :Bye` from one. The other should remain connected, the fd should close, and a new client should be able to reuse the nickname. The minimal implementation **does not announce QUIT to other users**; never claim it does.

**Defense answer:**

> I parse QUIT as a complete IRC command, call the existing cleanup function, and return immediately because removeClient deletes the Client object. Continuing to use the pointer would be a use-after-free bug.

### Exercise 5 — Validate the `+l` limit (medium)

**Evaluator task:** “What would you change to reject an invalid user limit?”

**Find:** MODE `+l` inside `ServerChannels.cpp`; locate number parsing, operator checks, and storing the limit on Channel. Explain what happens with zero, letters, or negative values. **Compare actual behavior with the subject before writing a change.**

### Exercise 6 — Reconsider poll event order (advanced, reasoning exercise)

**Evaluator question:** “What if POLLIN and POLLHUP are set together while unread bytes remain?”

**Goal:** Explain why prioritizing HUP might discard incoming data; how to investigate without calling `recv()` outside `poll()` readiness; and how to test a last message before disconnection. **Do not propose a patch without reviewing all event-loop branches** and cleanup rules.

---

## 13. Mini mock evaluation (45–60 minutes)

**Partner A evaluates B; then switch roles.**

**Part I — Architecture (10 minutes)**

- Explain the project in 90 seconds.
- Find `main`, `socket`, `bind`, `listen`, `poll`, and `accept` in the code.
- Explain who owns `Client*` and `Channel*`.

**Part II — Messages (10 minutes)**

- Register two users.
- Send one channel message and one direct message.
- Trace `recv` -> input -> dispatcher -> output -> `send`.

**Part III — Channel operators (10 minutes)**

- Demonstrate `+i`: first a rejected JOIN, then INVITE and successful JOIN.
- Demonstrate `+t`/TOPIC, then `+o`/KICK.
- Explain why `MODE #channel` can return only `+`.

**Part IV — Networking and memory (10 minutes)**

- Explain a command received in fragments.
- Explain Ctrl+Z and output buffering.
- Interpret Valgrind: zero errors, zero lost bytes, and still-reachable allocations.

**Part V — Modification (10–20 minutes)**

- Choose **one** of exercises 1–5.
- Locate the function unaided.
- Make a minimal change on the practice branch.
- Compile and test positive and negative cases.
- Explain in English why the change did not create a leak or break command processing.

### Internal grading rubric /10

| Criterion | Points |
|---|---:|
| Accurate architecture and ownership explanation | 2 |
| Understanding `poll`, `recv`, `send`, buffers, non-blocking | 3 |
| Commands, states, and privileges | 2 |
| Finding and explaining actual source, not only theory | 1 |
| Safe modification, compilation, and testing | 2 |

An internal score **does not predict the official grade**. It shows which topics require more study.

---

## 14. Final checklist before defending

- [ ] Both can deliver the 90-second introduction and find the related source files.
- [ ] Both know what `poll()` does and **does not** do.
- [ ] Both can trace a complete PRIVMSG through the actual code.
- [ ] Both understand why `Client` is created **before** PASS/NICK/USER.
- [ ] Both can distinguish the input and output buffers.
- [ ] Both can explain dangling pointers, memory leaks, and fd leaks.
- [ ] Both understand the five modes and numeric `482`.
- [ ] Both can explain why Valgrind reported reachable memory after Ctrl+C.
- [ ] Both know the subject 11.0 warning about `errno`.
- [ ] Both can make a small change on a **practice branch**, compile, and demonstrate it.
- [ ] The public README and submitted Git code are exactly the version tested.

**Suggested next session:** Open `src/Server.cpp` and `src/ServerNetwork.cpp`; trace a connection and a PRIVMSG line by line; ask 30-second oral questions; then practice QUIT on a temporary branch.

---

## 15. Sources and limits

**Formal requirements:** Supplied `ft_irc` subject version 11.0 (especially pages 6, 8–10, and 12) and supplied five-page `evaluation_sheet.pdf`. The current evaluation scale on 42 Intra takes precedence during the defense.

**Server-specific facts:** `Server.cpp` and `ServerNetwork.cpp` excerpts pasted during the conversation, grep searches, and actual test outputs. Other functions are described only to the level supported by testing or reports; this is not a claim of a complete source-code audit.

**Valgrind evidence:** Result shared by `chrrodri`, process PID 108518, including zero errors and `73,760 bytes still reachable`. The server exited via `SIGINT`.

**Networking reference material for studying:** Linux manual pages for `poll`, `recv`, `send`, and `fcntl`; RFC 1459 and RFC 2812 (background, not a requirement to implement every command).

**Companion document:** `ft_irc_EVALUATION_TESTS_EN.md` contains exact test commands and pass conditions.
