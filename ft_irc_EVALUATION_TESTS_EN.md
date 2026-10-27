# ft_irc — Internal Evaluation and Testing Guide

**Team:** `bsamy` and `chrrodri`  
**Updated:** October 27, 2026  
**Purpose:** Private working guide and mock evaluation; it does not replace the subject or the current evaluation scale on 42 Intra.  
**Reference version:** `en.subject.pdf`, **ft_irc 11.0**.  
**Additional supplied source:** `evaluation_sheet.pdf`, a five-page capture from **42 EvalHub**.  
**Status:** Extensive encouraging test results; **not a guarantee of passing**.

> **Before the evaluation:** Check the scale currently displayed on 42 Intra. The supplied scale is a dated copy and might not include newer requirements. Subject 11.0 contains a strict warning about using `errno` to choose the next action after `read`/`recv` or `write`/`send`. Never assume an external copy overrides Intra.

---

## 1. How to use this guide

1. One team member acts as the **evaluator**: reads the test, asks for an explanation, and watches the result; the other operates the project. Then switch roles.
2. Demonstrate a **fresh Git clone** and clean `make` **before** reusing any existing binary.
3. For each test, record the command, expected result, observed result, and **PASS / FAIL / NOT TESTED**.
4. Type `PASS`, `JOIN`, and other IRC commands **inside netcat**. Type `python3`, `git`, `make`, `ps`, etc. **in the shell** where the prompt shows `$` or `%`.
5. Use a fresh channel name for each mode test so prior settings do not affect the result.
6. Do not add bonuses or optional commands at the last minute if the mandatory implementation works.

### Status definitions

- **Verified:** Output or a successful test was shared by the team in this conversation.
- **Partial:** A basic check passed, but a more rigorous variation was not demonstrated.
- **Pending:** Needs review or confirmation on the exact version that will be submitted.

---

## 2. The evaluation scale, in practical terms

| Evaluation source | Requirement | Risk | Current status |
|---|---|---|---|
| Pages 1–2: rules | Work from the submitted Git repository, clone into an empty folder, avoid crashes, check leaks | Certain failures are disqualifying | **Clean remote clone pending** |
| Page 3: Basic checks | Makefile, C++98, `ircserv` executable, one `poll()` or equivalent | **Grade 0** if incorrect | Compilation and static search were favorable |
| Page 3: Basic checks | `poll()` controls `accept`, `recv`, and `send` | **Grade 0** if incorrect | Main paths reviewed; inspect the final submitted version |
| Page 3: Basic checks | `fcntl(fd, F_SETFL, O_NONBLOCK)` with no other `fcntl()` forms | **Grade 0** according to supplied scale | Two calls verified by grep |
| Subject v11, page 8 | Do not use `errno` to decide the next action after `recv`/`send` | **Grade 0** according to subject | Code updated and retested; confirm final grep |
| Pages 3–4: Networking | Listen, communicate with `nc`, use a real IRC client, multiple connections, channel messages | Mandatory | Favorable `nc` and HexChat tests |
| Page 4: Networking specials | Partial commands, unexpected disconnects, `Ctrl+Z` + flood + `fg` | Mandatory | Favorable; an independent count of all 3,000 messages was not performed |
| Page 4: Basic client | Authentication, nickname, username, JOIN, private PRIVMSG | Mandatory | Tested with `nc`; messaging also tested with HexChat |
| Page 4: Channel operators | KICK, INVITE, TOPIC, MODE `i,t,k,o,l`, regular-user restrictions | Mandatory | Tested with `nc`; HexChat `+o`/KICK verified; full operator matrix not repeated in HexChat |
| Page 5: Bonus | File transfer or bot | Optional | No bonus claimed |

**Important:** The supplied five-page scale **does not describe a specific live coding exercise**. Its general rules caution against editing source files during the defense unless agreed or allowed. Subject 11.0, page 12, says a small modification *may* be requested **if the applicable evaluation instructions require one**. Prepare for such an exercise, but never edit submitted code on your own initiative during an evaluation.

---

## 3. Environment preparation: first disqualifying checks

### A. Git status and a clean clone

In the original project directory:

```bash
git status -sb
git diff --check
git log -1 --oneline
git remote -v
git ls-files
```

Check your partner's changes before `git push`. Do not use `push --force` to resolve divergent histories.

Clone the **real remote URL** into a new, empty directory (replace the placeholder):

```bash
mkdir -p ~/ft_irc_eval_clean
cd ~/ft_irc_eval_clean
git clone <ACTUAL_REPOSITORY_URL> ft_irc
cd ft_irc
make fclean && make
```

**Pass condition:** The clone compiles without copying anything from the development folder. Confirm that `README.md`, Makefile, `include/`, and `src/` exist and that no source file is missing. Do not accidentally commit `.o` files, `ircserv`, Valgrind logs, or evaluation PDFs.

### B. Build, rules, and unnecessary relinking

```bash
make fclean && make
make
ls -l ircserv
grep -nE '(^|[[:space:]])(all|clean|fclean|re):' Makefile
```

**Expected:** Compilation with `c++ -Wall -Wextra -Werror -std=c++98` and an `ircserv` executable. A second `make` with no changes **must not relink** needlessly. Required targets include `$(NAME)`, `all`, `clean`, `fclean`, and `re`.

### C. Static networking checks

```bash
grep -RnE --include='*.cpp' --include='*.hpp' 'poll\(' src include
grep -RnE --include='*.cpp' --include='*.hpp' 'fcntl\(' src include
grep -RnE --include='*.cpp' --include='*.hpp' 'accept\(|recv\(|send\(' src include
grep -RnE --include='*.cpp' --include='*.hpp' 'errno|EAGAIN|EWOULDBLOCK|EINTR|fork\(' src include
grep -RnE --include='*.cpp' --include='*.hpp' 'MSG_NOSIGNAL' src include
```

**How to interpret the output:** Error strings such as `"poll() failed"` also match grep, but **they are not function calls**. Grep identifies locations; the evaluator may still ask you to trace `revents & POLLIN` to `accept`/`recv`, and `revents & POLLOUT` to `send`.

**Critical v11 check:** Verify that no decision after `recv()` or `send()` is based on `errno`. The team's reported revision keeps `recv()==0` as an orderly-close case and returns to the event loop on a negative result, without an immediate `errno`-driven retry. **Technical caution:** Confirm that permanent write failures cannot cause a busy loop and that `poll()` errors/hangups are handled.

**Another important detail:** The strict `fcntl()` form appears in the subject's **For MacOS only** subsection, while the supplied evaluation scale asks for it under Basic checks without restricting it to MacOS. We therefore used that exact form for every socket.

---

## 4. Functional test setup

Server, terminal S:

```bash
./ircserv 6667 pass
```

Clients A, B, and C, each in a separate terminal:

```bash
nc -C 127.0.0.1 6667
```

**Client A — Roger:**

```irc
PASS pass
NICK roger
USER roger 0 * :Roger
```

**Client B — Titi:**

```irc
PASS pass
NICK titi
USER titi 0 * :Titi
```

**Client C — Katia:**

```irc
PASS pass
NICK katia
USER katia 0 * :Katia
```

**Expected:** Numeric `001` for each user. In the netcat variant used during these tests, `-C` sends CRLF-terminated lines. Availability and behavior of `-C` depend on the netcat version.

> **If you see `zsh: command not found: PASS`,** you are typing in the shell instead of inside netcat. Launch `nc -C ...` again and type IRC commands there.

---

## 5. Complete mandatory-command test matrix

### T01 — Authentication and invalid registration

**Use a fresh client D** (not already registered):

```irc
PASS wrong
NICK wrongtest
USER wrongtest 0 * :Wrong Tester
JOIN #basic
```

**Expected:** `464` (incorrect password), no `001` welcome, and `451` when attempting JOIN.

In the **same** connection:

```irc
PASS pass
```

**Observed:** Registration completed (`001`) after correcting the password, because NICK and USER had already been stored. This **does not mean** the server allowed JOIN without a valid password. Repeating `PASS pass` after registration returned `462`.

Test other cases in the correct registration state or a fresh connection:

```irc
NICK roger
USER wrongtest 0 :Missing
UNKNOWNCOMMAND
```

**Expected where applicable:** `433` for another client's nickname already in use; `461` for incomplete USER; `421` for an unknown command. Confirm that the connection remains stable. Do not assume identical outcomes for an already registered user without examining that context.

**Follow-up case:** Repeating `NICK` with the client's **own current nickname** may return `433` in the tested code. This is a minor compatibility edge case to investigate, not an explicitly confirmed grading requirement.

### T02 — JOIN, NAMES, channel messages, and private messages

On A:

```irc
JOIN #basic
```

On B and C:

```irc
JOIN #basic
```

On A:

```irc
PRIVMSG #basic :Hello everyone
PRIVMSG titi :Private hello
PRIVMSG katia :Only for Katia
```

On B:

```irc
PRIVMSG #basic :Hello Roger and Katia
PRIVMSG roger :Private reply
```

**Expected:** `353` / `366`, `@roger` as the first operator; channel messages reach the other members; direct messages reach only their targets. Also test `PRIVMSG nobody123 :Hello` (`401` error) and `PRIVMSG #missing :Hello` without creating that channel.

### T03 — KICK and operator restrictions

On B:

```irc
KICK #basic roger :Not operator
```

**Expected:** `482`.

On A:

```irc
KICK #basic titi :Operator test
```

On B:

```irc
PRIVMSG #basic :Can I still write?
JOIN #basic
```

**Expected:** A KICK notification, `442` when a nonmember attempts to write (observed in this implementation), then rejoining is allowed if no restriction applies.

### T04 — INVITE, `+i` and `-i`

On A:

```irc
JOIN #inv
MODE #inv +i
MODE #inv
```

On C:

```irc
JOIN #inv
```

**Expected:** `473` (invite-only rejection).

On A:

```irc
INVITE katia #inv
```

On C:

```irc
JOIN #inv
```

**Expected:** JOIN is now allowed. To verify nonoperator permissions, register a fourth user, `auditfour`, in a fresh netcat session if necessary. Then:

On A:

```irc
INVITE titi #inv
```

On B:

```irc
JOIN #inv
INVITE auditfour #inv
```

**Expected:** B receives `482` for INVITE because `+i` requires operator privileges. On A: `INVITE auditfour #inv`; on D: `JOIN #inv` -> allowed. A disables invite-only with `MODE #inv -i`. If any user was already invited, use a different registered nickname to avoid a misleading result.

### T05 — TOPIC, `+t` and `-t`

A and B join `#top`, with A joining first.

On B:

```irc
TOPIC #top :Topic from regular user
```

On A:

```irc
MODE #top +t
```

On B:

```irc
TOPIC #top :Unauthorized change
```

**Expected:** `482`.

On A:

```irc
TOPIC #top :Official topic
TOPIC #top
MODE #top -t
```

On B:

```irc
TOPIC #top :Allowed again
```

**Expected:** The topic change works after `-t`.

### T06 — Channel key `+k` / `-k`

On A:

```irc
JOIN #key
MODE #key +k secret
```

On B:

```irc
JOIN #key
JOIN #key wrong
JOIN #key secret
```

**Expected:** Two `475` errors and then a successful JOIN. A removes the key with `MODE #key -k`; C should then join using `JOIN #key` with no password.

### T07 — User limit `+l` / `-l`

On A:

```irc
JOIN #limit
MODE #limit +l 1
```

On B: `JOIN #limit` -> `471`.

On A: `MODE #limit +l 2`; on B: `JOIN #limit` -> allowed.

On C: `JOIN #limit` -> `471`.

On A: `MODE #limit -l`; on C: `JOIN #limit` -> allowed.

### T08 — Member operator privilege `+o` / `-o`

A, B, and C join `#ops`, with A first.

On B:

```irc
KICK #ops katia :Before promotion
```

**Expected:** `482`.

On A:

```irc
MODE #ops +o titi
MODE #ops
```

On B:

```irc
KICK #ops katia :After promotion
```

**Expected:** KICK succeeds; `+o` changes Titi's privileges. A `MODE #ops` query can return `324 ... +` if no other channel-wide modes are active: `+o` is a **member privilege**, not a channel-wide mode flag.

On A:

```irc
MODE #ops -o titi
```

On B: `KICK #ops roger :After demotion` -> `482`.

### T09 — Combined modes and invalid parameters (extra test, not fully completed)

```irc
MODE #ops +it
MODE #ops
MODE #ops -it
MODE #ops
MODE #key +k
MODE #limit +l invalid
```

Check consistent state changes, parameter validation, no crashes, and appropriate errors. **Do not claim complete support for every parameterized combined mode, such as `+kl`,** without checking the parser: this was previously identified as an unverified case.

---

## 6. Networking specials: literal subject tests and robustness

### T10 — Fragmented message (`Ctrl+D`)

Open a fresh terminal:

```bash
nc -C 127.0.0.1 6667
```

Type `PA` **without Enter**, press `Ctrl+D` **once**, then type `SS pass` and press Enter. Continue with:

```irc
NICK partial
USER partial 0 * :Partial
```

**Expected:** Registration only occurs after the complete `PASS pass` line arrives. Verify your specific netcat and terminal behavior: `Ctrl+D` varies by terminal state and implementation. The example in the subject tests reconstruction of fragments `com`, `man`, `d\n`.

**Deterministic alternative in the shell** if `Ctrl+D` is confusing:

```bash
python3 - <<'PY'
import socket, time
s = socket.create_connection(('127.0.0.1', 6667))
for part in (b'PA', b'SS pass\r\n', b'NICK fragment\r\n',
             b'USER fragment 0 * :fragment\r\n'):
    s.sendall(part)
    time.sleep(.25)
s.settimeout(2)
try:
    print(s.recv(4096).decode(errors='replace'))
except socket.timeout:
    print('No response within timeout')
s.close()
PY
```

The Python script is for testing only; it is not part of the submitted C++98 project.

### T11 — Several commands in a single connection

In the **shell**, not inside `nc`:

```bash
(printf 'PASS pass\r\nNICK batchuser\r\nUSER batchuser 0 * :batchuser\r\nJOIN #batch\r\n'; sleep 2) | nc 127.0.0.1 6667
```

**Expected:** `001`, JOIN, `353`, and `366`. The test verifies **multiple commands over one TCP connection**; it does not prove that the kernel delivered them through exactly one `recv()`.

### T12 — Unexpected disconnect in the middle of a command

Connect another `nc` client, send PASS and NICK, and leave `USER half 0` unfinished without pressing Enter. Close netcat with `Ctrl+C`. Verify that another client still works and the same nickname can be registered on a new connection. Cleanup must remove channel references before destroying the `Client` object.

### T13 — Suspend, flood, and resume a client

A and B join `#stress`. On **B**, press `Ctrl+Z`. In a **different shell**:

```bash
python3 - <<'PY'
import socket, time
s = socket.create_connection(('127.0.0.1', 6667))
s.sendall(b'PASS pass\r\nNICK flooder\r\nUSER flooder 0 * :flooder\r\nJOIN #stress\r\n')
time.sleep(1)
for i in range(3000):
    line = 'PRIVMSG #stress :%05d %s\r\n' % (i, 'x' * 220)
    s.sendall(line.encode())
s.sendall(b'PRIVMSG #stress :FLOOD_END\r\n')
time.sleep(2)
s.close()
print('3000 messages sent and FLOOD_END')
PY
```

While B is suspended, check that A remains connected and C can send A a direct message. On **B**, type `fg`. Confirm that B receives buffered traffic, including `02999` and `FLOOD_END`, without the server hanging.

**Team evidence:** The final marker arrived and other clients continued working; a photo also showed `02999`. **Limitation:** We did not independently count every one of the 3,000 message lines or validate every sequence number.

### T14 — Final message just before an orderly TCP shutdown

Client A is Roger in `#last`. In a separate **shell**:

```bash
python3 - <<'PY'
import socket, time
s = socket.create_connection(('127.0.0.1', 6667))
s.sendall(b'PASS pass\r\nNICK lastcheck\r\nUSER lastcheck 0 * :lastcheck\r\nJOIN #last\r\n')
time.sleep(.5)
s.sendall(b'PRIVMSG #last :LAST_MESSAGE_BEFORE_DISCONNECT\r\n')
s.shutdown(socket.SHUT_WR)
time.sleep(1)
s.close()
print('Last message sent')
PY
```

**Expected and seen in a photo:** Roger receives `LAST_MESSAGE_BEFORE_DISCONNECT`. **Unresolved edge case:** The previously reviewed code checks HUP before POLLIN. When both occur together, unread bytes might be discarded. One successful test does not demonstrate every possible event ordering.

### T15 — Ten abrupt TCP resets (RST)

Keep A connected. Run in a shell:

```bash
python3 - <<'PY'
import socket, struct
for i in range(10):
    s = socket.create_connection(('127.0.0.1', 6667))
    msg = ('PASS pass\r\nNICK abrupt%d\r\nUSER abrupt 0 * :abrupt\r\n'
           'JOIN #last\r\nPRIVMSG #last :unfinished') % i
    s.sendall(msg.encode())
    s.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, struct.pack('ii', 1, 0))
    s.close()
print('10 abrupt connections closed')
PY
```

**Expected and reported:** The script finishes, the server continues accepting connections, and existing clients can still communicate. The test deliberately ends with an incomplete IRC command.

### T16 — Permanent write failure / CPU use (conclusive test pending)

The last reported code change avoids checking `errno` after `send()` and keeps the output queue if no progress is made. With a permanent failure, some systems may continue reporting `POLLOUT`, potentially causing a **busy loop**. Passing the flood test **does not rule this out**.

To observe the actual running process at idle:

```bash
pgrep -af 'ircserv|valgrind'
ps -eo pid,ppid,stat,pcpu,rss,comm | grep -E 'ircserv|valgrind'
```

Run these twice a few seconds apart **while the server is still running**. The earlier `ps -C ircserv` command found no process, so this is **not marked as passed**. If CPU usage stays high or broken connections are never cleaned up, investigate without adding an immediate error-based retry loop that conflicts with v11.

---

## 7. Real IRC clients: HexChat (mandatory) and AdiIRC

In HexChat (tested on a 42 Linux workstation), create a local network using `127.0.0.1/6667`, **server password** `pass` (not NickServ password), TLS/SASL disabled, and nickname `hexa42`. Start the server first.

| Test | Action | Result |
|---|---|---|
| Connect and register | Connect | **Verified** |
| JOIN | `/join #test` | **Verified** |
| Channel message | Enter plain text without `/` in the channel tab | **Verified** |
| Direct message | `/msg wrongtest hi` or open a nickname dialog | **Verified** |
| Operator privilege | `/mode #test +o hexa42` from an operator | **Verified** in reported tests |
| KICK | `/kick titi :test` from an operator | **Verified** in reported tests |
| TOPIC | `/topic #test :New topic` | **Not verified from HexChat**; previously tested with `nc` |
| INVITE with +i | `/invite katia #test` as applicable | **Not verified from HexChat**; tested with `nc` |

**Observed compatibility note:** HexChat displayed `WHO` and `WHOIS` as unknown commands (`421`). They are not explicitly listed in the mandatory command set; however, the subject expects a reasonably functional reference-client experience. Document this observation; **do not describe the client as having literally no errors if these notices appeared**. Discuss whether they affect evaluated workflows before adding optional support.

AdiIRC was tested earlier on Windows. **Name one reference client** during the evaluation (HexChat if that's available at 42), and explain that `nc` is a raw protocol-testing utility, not the graphical IRC reference client.

---

## 8. Valgrind: exact results and how to explain them

### Command used

```bash
valgrind --leak-check=full --show-leak-kinds=all \
  --track-origins=yes --log-file=valgrind.log \
  ./ircserv 6667 pass
```

Clients were closed, then the server was stopped with `Ctrl+C`. To inspect the evidence:

```bash
tail -n 90 valgrind.log
grep -E 'ERROR SUMMARY|definitely lost|indirectly lost|possibly lost|still reachable|Invalid read|Invalid write|Invalid free' valgrind.log
```

### Latest reported result — PID 108518

| Metric | Result |
|---|---:|
| `ERROR SUMMARY` | **0 errors from 0 contexts** |
| `definitely lost` | **0 bytes** |
| `indirectly lost` | **0 bytes** |
| `possibly lost` | **0 bytes** |
| `still reachable` | **73,760 bytes in 3 blocks** |
| `pollfd` vector | **32 bytes** |
| C standard output buffer | **1,024 bytes** |
| C++ library allocation | **72,704 bytes** |

**Careful interpretation:** `still reachable` means Valgrind could still locate references to that memory at termination; **it did not classify those blocks as lost**. The server was terminated by the default `SIGINT` action (`Ctrl+C`) while waiting in `poll()`, so it did not follow a normal exit path that destroys the `Server` object. The **32-byte vector allocation** would likely be freed by its destructor if normal destruction occurred. The **1,024 + 72,704 bytes** originate in library allocations and buffers. Do not promise that those exact numbers will disappear on every system.

**Could this cause a failure?** The scale expects leak checks and can penalize actual leaks. **This report found no lost allocations or memory errors**, which is encouraging, but is **not a guaranteed grade** or evidence of clean graceful shutdown. If asked, explain the distinction between `still reachable` and `definitely lost`, including the Ctrl+C termination.

**Short answer in English:**

> Valgrind reported zero memory errors and zero definitely, indirectly or possibly lost bytes. It reported 73,760 bytes as still reachable, mostly from the C/C++ runtime, plus 32 bytes for our poll vector. We stopped the server with Ctrl+C while it was inside poll, so the normal Server destructor did not run. Still reachable does not mean the memory was lost, but this run does not prove graceful shutdown cleanup.

**Optional FD check:** If an evaluator wants descriptor information, repeat with `--track-fds=yes` and interpret the signal termination carefully. Do not change signal handling at the last minute just to remove `still reachable` entries without thoroughly testing the effect.

---

## 9. Submission and defense checklist

### Submission

- [ ] The current subject and scale on Intra match what we are presenting.
- [ ] Builds from a **fresh, empty-directory Git clone** with the required flags.
- [ ] `make` with no changes does not relink.
- [ ] `poll` / `fcntl` / `errno` comply with the applicable rules.
- [ ] English README: italicized first line with exact logins, Description, Instructions, Resources, AI usage.
- [ ] No accidentally committed binaries, logs, private test guides, or unnecessary PDFs.
- [ ] Both partners know the final commit, branch, and remote.
- [ ] HexChat, as the reference client, performs the essential mandatory commands.
- [ ] CPU behavior / permanent send failures investigated if required.
- [ ] Both partners practiced oral explanations.

### Fast demonstration for the evaluator

1. `make fclean && make`.
2. `./ircserv 6667 pass`.
3. Open `nc -C` and HexChat.
4. Register a user, JOIN, exchange channel and direct messages.
5. Demonstrate KICK and MODE with appropriate privileges.
6. Explain `poll`, buffering, and disconnection cleanup.
7. Present the Valgrind report with appropriate qualifications.
8. Be ready for a small modification **only if the active scale calls for it**.

---

## 10. Questions the evaluator may ask after each test

- **Where is `Client` created?** After successful `accept()`, before PASS/NICK/USER registration is complete.
- **Why only one `poll()`?** One event loop monitors both the listening descriptor and client read/write readiness.
- **Why no `fork()`?** The subject forbids it; concurrency is achieved with non-blocking I/O.
- **Why check `POLLIN` before `recv()`?** It ensures a readiness event before reading; the non-blocking socket will not wait indefinitely.
- **Why enable `POLLOUT` only for pending data?** Otherwise writable sockets would continually wake the event loop.
- **What if `send` accepts 300 of 1,000 bytes?** Remove only the 300 bytes sent; keep 700 queued.
- **What if `recv` returns 0?** The peer closed its sending direction; clean up that client.
- **Why does `MODE #channel` show `+` after `+o`?** `+o` is a member privilege, not a channel-wide mode flag.
- **Why is `QUIT` not implemented?** It is not explicitly listed among mandatory subject features; TCP disconnects are handled.
- **What is `SIGPIPE`?** A signal that can occur when writing to a broken connection; `MSG_NOSIGNAL` on Linux prevents the default termination for that send call.
- **What is `still reachable`?** Memory that still has references, unlike lost allocations; Ctrl+C prevented normal destruction.

For longer answers, code-change exercises, and file-by-file walkthroughs, see **`ft_irc_DEFENSE_MASTERCLASS_EN.md`**.

---

## 11. Sources and scope

- **Supplied subject 11.0**, `en.subject.pdf`: Introduction/general rules (pages 3–5), mandatory networking (pages 6–9), README (page 10), evaluation/modifications (page 12).
- **Supplied subject 10.0**, `ft_irc.pdf`: Historical comparison; prioritize 11.0 and whichever current edition is shown on Intra.
- **Supplied scale**, `evaluation_sheet.pdf` (42 EvalHub, five pages): Basic checks, Networking, Networking specials, Client commands, Bonus.
- **Actual team evidence:** Compilation, `nc`, HexChat, Python tests, and Valgrind output shared during this conversation. Missing evidence is identified explicitly.
- **Technical study references (not extra subject requirements):** `man 2 poll`, `man 2 recv`, `man 2 send`, RFC 1459 / RFC 2812.
