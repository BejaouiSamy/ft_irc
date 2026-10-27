# ft_irc — Masterclass interna de defensa oral y modificaciones de código

**Equipo:** `bsamy` y `chrrodri`  
**Actualizado:** 27 de octubre de 2026  
**Idioma de estudio:** explicaciones en español; respuestas cortas de defensa en inglés.  
**Proyecto evaluado:** servidor IRC en C++98, **mandatory only** según subject **11.0**.  
**Objetivo:** que **ambos miembros** puedan localizar, explicar, modificar y probar cada parte importante sin memorizar código incomprendido.

> Esta guía está basada en la arquitectura, los fragmentos de código y las salidas que se compartieron durante las pruebas. No representa una auditoría exhaustiva del código fuente actual. Cuando una estructura o comportamiento dependa del código no adjuntado, se señala como **verificar en la versión entregada**.

---

## 1. Tu presentación de 90 segundos (practicar en voz alta)

**Pregunta del evaluador:** “Can you explain your project?”

**Respuesta modelo en inglés:**

> Our project is an IRC server written in C++98. It accepts multiple TCP clients and allows them to register with a password, nickname and username. Users can join channels, exchange private and channel messages, and perform channel operations depending on their privileges.
>
> We use a single poll-based event loop with non-blocking sockets, so one slow client does not block everyone else. Each client has an input buffer to rebuild complete IRC commands and an output buffer to queue data until the socket is writable.
>
> Our Server manages connections and commands, Client stores one user's state, and Channel manages members, operators, invitations, topics and modes. We tested the mandatory commands with netcat, used HexChat as a real IRC client, and checked networking robustness and memory behavior with stress tests and Valgrind.

**Tres errores a evitar:** (1) decir que `poll()` lee datos — los lee `recv()`; (2) decir que los clientes se crean después de `PASS/NICK/USER` — se crean **tras `accept()`**; (3) decir que los comandos se ejecutan en paralelo — el servidor gestiona I/O **concurrente en un solo loop**, no se ha demostrado paralelismo de ejecución.

---

## 2. El mapa mental completo del servidor

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
  _fds incluye el listening fd
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

**Descripción correcta:** las flechas indican un flujo lógico; TCP puede fragmentar o reunir datos, y el sistema operativo decide en qué momento `poll()` devuelve eventos.

---

## 3. Archivos, clases y qué explicar frente al evaluador

### `src/main.cpp`

**Rol:** punto de entrada. Recibe `argc/argv`, valida los parámetros según nuestra implementación, crea el objeto `Server` y llama a su ciclo de ejecución.

**Mostrar en vivo:** cómo llegan `<port>` y `<password>` hasta el constructor.

**Pregunta:** “What does `./ircserv 6667 pass` mean?”

> It starts our server listening on port 6667. Every client must authenticate using the server password pass before completing registration.

**Verificar en código final:** comportamiento exacto ante número incorrecto de argumentos, puerto inválido o error al hacer bind. No improvisar respuestas.

### `src/Server.cpp`

**Rol:** constructor de socket y listening fd; destructor que destruye los objetos y cierra fds si sale normalmente; ciclo principal `Server::run()`.

**Fragmento conocido del loop:**

```cpp
int ret = poll(&_fds[0], _fds.size(), -1);
if (ret == -1)
    throw std::runtime_error("poll() failed");
for (size_t i = 0; i < _fds.size(); ) {
    int fd = _fds[i].fd;
    short revents = _fds[i].revents;
    // Decide qué hacer según los bits devueltos.
}
```

**Conceptos:** `poll()` retorna cantidad de descriptores con eventos o `-1` si falla; timeout `-1` espera indefinidamente hasta un evento o interrupción. **Un `poll()` en el código no significa una sola llamada total: el mismo poll se ejecuta repetidamente en un bucle.**

**Punto de riesgo reconocido:** en el código compartido, el branch `POLLHUP | POLLERR | POLLNVAL` tiene prioridad sobre `POLLIN`; revisar si pueden descartarse bytes que llegaron antes del cierre. Un test de último mensaje pasó, pero no prueba todas las condiciones de carrera.

**Otro riesgo:** un `poll()` interrumpido por señal puede devolver `-1`; el código observado lanza excepción para todo `-1`. No añadir cambios sin revisar requisitos, las pruebas y gestión de señales.

### `src/ServerNetwork.cpp`

**Rol:** `acceptClient()`, `handleData(fd)`, `handleWrite(fd)`, `removeClient(fd)`, `enablePollOut(fd)` y `disablePollOut(fd)`.

**Fragmentos vistos y verificados:**

- `acceptClient()` usa `accept`, configura cliente no bloqueante, añade un `pollfd` con `events = POLLIN` y guarda `new Client(client_fd)` en `_clients`.
- `handleData()` usa `recv()` y agrega bytes a `Client` para reconstruir comandos.
- `handleWrite()` usa `send(..., MSG_NOSIGNAL)` en Linux y elimina solamente los bytes enviados.
- `removeClient()` elimina primero referencias en canales, luego `delete client`, borra mapa y vector de poll, cierra fd.
- `enablePollOut` usa `events |= POLLOUT`; `disablePollOut` usa `events &= ~POLLOUT`.

**Subject 11.0:** prohíbe basarse en `errno` para decidir qué hacer después de `read/recv` o `write/send`. El equipo reportó haber eliminado esos branches y luego ejecutado pruebas normales y de flood con éxito. **Comparar el archivo exacto del Git final antes de decir “cumple”.**

### `src/ServerCommands.cpp`

**Rol conocido:** transformar la línea IRC en comando y parámetros y dirigirla al handler correspondiente. También se añadieron funciones de compatibilidad `CAP`, `PING`, `USERHOST` para clientes gráficos.

**Explicación:** un comando entrante **no se ejecuta por cada paquete TCP**. Primero se espera una línea completa y después se analiza y despacha.

**Buscar en vivo:** `dispatchCommand`, parser de argumentos, cómo maneja comandos desconocidos (`421`). Comprobar si normaliza mayúsculas/minúsculas en comandos; no suponerlo si no se ha probado.

### `src/ServerRegistration.cpp`

**Rol conocido:** `PASS`, `NICK`, `USER`, verificación de registro y emisión de `001`.

**Estados distintos:** tener un objeto `Client`, conocer su nickname/username y estar registrado **no son lo mismo**. La aceptación del socket ocurre antes del registro.

**Caso efectivamente probado:** `PASS wrong` produce `464`; introducir NICK/USER no habilita JOIN (responde `451`). `PASS pass` posterior produce `001`; luego PASS repetido produce `462`.

### `src/ServerChannels.cpp`

**Rol conocido:** creación y entrada en canales, distribución de mensajes, KICK/INVITE/TOPIC/MODE y limpieza de membresías.

**Buscar en vivo:** handler de cada comando, comprobación de membership, comprobación de operator, acceso a `_channels`.

**Advertencia:** el archivo contiene varios handlers; localizar la función con `grep -n` antes de intentar una modificación. No copiar parches en una parte arbitraria sin entender variables y retornos.

### `src/ServerReplies.cpp`

**Rol conocido:** construir y encolar respuestas numéricas IRC, en vez de llamar a `send()` directamente desde cada handler.

**Defensa:** por qué centralizar las respuestas ayuda a mantener la estructura `<prefix> <numeric> <nick> <params> :trailing`.

### `src/Client.cpp` y `include/Client.hpp`

**Rol:** almacenar un cliente TCP, su `fd`, nick/usuario, estado de registro, buffers y métodos de acceso.

**Ownership conocido:** el mapa `_clients` en `Server` almacena punteros `Client*`. Al eliminar la conexión, **Server** destruye el objeto. El `Client` se crea al aceptar TCP, no al recibir USER.

**Buffers:** input guarda bytes incompletos; output guarda bytes aún no transmitidos. Un `std::string` puede contener varios mensajes juntos o fracciones de uno.

### `src/Channel.cpp` y `include/Channel.hpp`

**Rol conocido:** nombre, topic, miembros, operadores, invitados, key y límite de usuarios del canal.

**Modelo visto:** vectores de punteros `Client*` a miembros/operadores/invitados. **No deben ser propietarios** de esos objetos. Si se destruye Client sin retirarlo de los vectores, quedan punteros colgantes.

**Pregunta:** “Why does the first user become operator?”

> When a channel is created, it has no operator yet. Our implementation makes its first member an operator so the channel can be managed immediately.

**Verificar si lo piden:** qué sucede cuando el último miembro sale y qué hace el servidor si el último operador se desconecta. No presentar comportamiento no inspeccionado como garantía.

---

## 4. La base de networking: conceptos que debes entender, no memorizar

### 4.1 Socket, puerto y fd

- **Socket TCP**: extremo de una comunicación fiable, ordenada y basada en bytes entre dos procesos.
- **Listening socket**: está asociado al puerto y recibe solicitudes de conexión. No es el socket particular de Roger.
- **`accept()`**: devuelve un **fd nuevo** para esa conexión, que ya puede registrarse en el vector de `poll()`.
- **File descriptor (`fd`)**: entero válido **dentro de ese proceso** que identifica un recurso abierto; `close(fd)` libera ese descriptor, y su número puede reutilizarse posteriormente.
- **Puerto**: identifica el servicio TCP en el sistema destino; 6667 es ejemplo, no necesariamente fijo.
- **Concurrencia vs paralelismo**: el servidor puede atender conexiones intercalando operaciones; no implica que varios handlers se ejecuten físicamente a la vez.

### 4.2 `fcntl()`

```cpp
fcntl(fd, F_SETFL, O_NONBLOCK);
```

- `fcntl` = file control.
- `F_SETFL` = establecer banderas de estado del fd.
- `O_NONBLOCK` = hacer que una operación adecuada no espere cuando no puede progresar inmediatamente.
- Antiguamente se usaba `F_GETFL` seguido de `flags | O_NONBLOCK` para preservar banderas; se cambió a la forma literal que pide la escala aportada.

**Defensa en inglés:**

> We configure both the listening socket and each accepted client socket as non-blocking, using the exact fcntl form required by our evaluation scale.

### 4.3 `pollfd` y banderas

```cpp
struct pollfd {
    int fd;
    short events;
    short revents;
};
```

**`events`** = lo que deseamos vigilar. **`revents`** = lo que el kernel reporta que ocurrió.

| Bandera | Significado práctico | Qué suele hacer nuestro programa |
|---|---|---|
| `POLLIN` | Se puede intentar leer/aceptar sin esperar normalmente | `accept` si listening fd; `recv` si client fd |
| `POLLOUT` | Se puede intentar escribir | `send` parte o toda la cola de salida |
| `POLLHUP` | Hangup; el peer cerró o dejó de estar disponible en el sentido relevante | Limpiar, sin asumir que siempre no quedan bytes |
| `POLLERR` | Error asociado al fd | Manejar error/desconectar |
| `POLLNVAL` | Descriptor inválido en el conjunto | Investigar/retirarlo |

`POLLRDHUP` es un indicador disponible en Linux para ciertos half-close; no se observó utilizado en el código actual y **no es una obligación adicional del proyecto**.

**Pregunta:** “Does poll receive IRC messages?”

> No. Poll reports which file descriptors are ready. Then our server calls recv to actually receive bytes.

### 4.4 `recv()`

Forma conocida en el proyecto:

```cpp
char buffer[1024];
ssize_t bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
```

- `fd`: descriptor de conexión.
- `buffer`: destino de los bytes.
- `sizeof(buffer)-1`: máximo solicitado (1023), dejando espacio para terminador de C en el código actual.
- `0`: ninguna opción especial en esa llamada.
- `ssize_t`: tipo firmado que permite números positivos o `-1`.

**Retornos:**

| Retorno | Explicación |
|---|---|
| `> 0` | Se han recibido esa cantidad de bytes, **no necesariamente un comando IRC completo** |
| `== 0` | Para una lectura TCP de tamaño positivo, cierre ordenado del sentido de envío por el peer |
| `-1` | Error de operación; no significa automáticamente desconexión |

**Cuidado subject 11.0:** el texto prohíbe depender de `errno` después de `recv`/`send` para decidir la acción siguiente. No proponer reintentos inmediatos basados en `EAGAIN`. El servidor reportado devuelve al ciclo central en caso de resultado negativo y deja eventos de error/hangup al `poll()`; **hay que verificar estabilidad ante errores permanentes**.

### 4.5 `send()` y buffer de salida

```cpp
ssize_t n = send(fd, client->getOutputBuffer().c_str(),
                 client->getOutputBuffer().length(), MSG_NOSIGNAL);
```

Si hay 1000 bytes pendientes y `send` acepta 300, **solo se eliminan 300**. Quedan 700 para un evento de escritura futuro. Esto es una **escritura parcial normal**, no pérdida de datos.

`MSG_NOSIGNAL`: en Linux evita que esa llamada genere la terminación por defecto de `SIGPIPE` si la conexión está rota. No demuestra que el socket sea válido; aún hay que tratar errores.

`POLLIN` se vigila para cada cliente; `POLLOUT` se activa **solo si hay cola de salida** y se desactiva al quedar vacía, evitando un ciclo inútil sobre sockets continuamente escribibles.

**Riesgo a conocer:** si un `send` negativo siempre deja la cola intacta, pero el error es permanente y `poll` reporta continuamente posibilidad de escritura, puede ocurrir **busy-loop**. No afirmar que está descartado sin una prueba específica.

### 4.6 La unidad correcta es una línea IRC, no un paquete TCP

`PASS pass\r\n` usa CRLF (`\r\n`) como separador IRC. Un `recv()` puede devolver `PA`, otro `SS pass\r`, y otro `\n`; o devolver varios comandos completos juntos. Por eso existe `appendBuffer` + `extractMessage()`.

`extractMessage()` se repite hasta que no quede una línea completa en el buffer. Las partes sin terminador permanecen almacenadas, mientras otros clientes siguen conectados.

### 4.7 `std::string` y el NUL

En el fragmento examinado se hacía:

```cpp
buffer[bytes] = '\0';
client->appendBuffer(std::string(buffer));
```

Esto funciona con comandos IRC de texto normales, pero `std::string(buffer)` se detiene ante un NUL embebido. Una alternativa para conservar exactamente `bytes` bytes sería `std::string(buffer, bytes)` si el equipo decidiera investigarlo. **No cambiar sin necesidad ni pruebas**; es un detalle técnico para comprender limitaciones.

---

## 5. El recorrido de un mensaje de principio a fin

**Ejemplo:** Roger escribe `PRIVMSG #42 :Hello` y Titi lo recibe.

1. Roger tiene una conexión TCP aceptada; Server le asignó un `Client`.
2. El kernel indica que el fd de Roger está listo para lectura; `poll()` devuelve `POLLIN`.
3. `Server::handleData(fd)` llama a `recv(fd, ...)`.
4. Los bytes recibidos se agregan al input buffer **del Client de Roger**.
5. Cuando aparece una terminación de línea, `extractMessage()` devuelve un mensaje completo.
6. `dispatchCommand()` reconoce `PRIVMSG` y determina su destino `#42`.
7. El handler busca `Channel #42`, valida contexto y recorre los miembros a los que hay que entregarlo.
8. Para cada destinatario, se agrega una respuesta IRC con prefijo de Roger al **output buffer de ese destinatario**.
9. `enablePollOut(dest_fd)` pide eventos `POLLOUT`.
10. En iteraciones posteriores `poll()` reporta que se puede intentar escribir y `handleWrite()` llama a `send()`.
11. Si `send` escribe una parte, se elimina solamente esa parte; si termina toda la cola, se desactiva `POLLOUT`.
12. El IRC client de Titi lee la línea del socket y la presenta en el canal.

**Respuesta de defensa (inglés):**

> When Roger sends a message, poll reports POLLIN for his socket. We call recv and append the bytes to his input buffer. After a complete IRC line is available, we parse and dispatch PRIVMSG. We find the destination channel and queue the formatted message in each recipient's output buffer. Poll later reports POLLOUT and send transmits the queued bytes. If the write is partial, we keep the remaining bytes for a later event.

**Pregunta de seguimiento:** “Do you send the channel message back to Roger?”

> For our implementation, channel PRIVMSG is forwarded to the other channel members, not duplicated back to the sender. That's why the evaluator uses at least two clients.

---

## 6. Registro IRC: PASS, NICK, USER

### Interpretación de la línea USER

```irc
USER roger 0 * :Roger Example
```

- `USER`: nombre del comando.
- `roger`: username.
- `0`: campo histórico de modo/host tradicional; nuestra implementación requiere la estructura de cuatro parámetros.
- `*`: campo histórico de servidor.
- `:Roger Example`: último parámetro (*trailing*): el `:` inicial permite espacios.

**No escribir** `USER roger 0 *:Roger`, porque se unirían parámetros y nuestra versión puede devolver `461`.

### Respuestas numéricas que ya conocemos

| Código | Significado o contexto probado |
|---|---|
| `001` | Welcome tras completar registro |
| `401` | Destinatario de mensaje privado inexistente |
| `403` | No existe el canal solicitado |
| `421` | Comando desconocido |
| `433` | Nickname ya usado (también observar la anomalía de mismo nick propio) |
| `451` | Cliente todavía no registrado |
| `461` | Parámetros insuficientes |
| `462` | Intento de registrarse otra vez |
| `464` | Password incorrecto |
| `471` | Canal lleno por `+l` |
| `473` | Canal invite-only `+i` |
| `475` | Canal con clave incorrecta |
| `482` | No es channel operator |
| `442` | No pertenece al canal (según las pruebas con KICK) |
| `324` | Respuesta a consulta de modos del canal |
| `353` / `366` | NAMES y fin de NAMES |

**Atención:** un código no puede interpretarse sin los argumentos y el estado de registro/canal. En la defensa, explicar *por qué* se emitió y no solo memorizar el número.

### ¿Por qué el segundo PASS correcto permitió registrarse?

El código permitía guardar NICK y USER antes de que la contraseña fuera aceptada, pero **no permitía JOIN antes de PASS correcto**. `PASS wrong` produjo `464` y JOIN `451`; tras `PASS pass` apareció `001`. **No es equivalente a aceptar password incorrecto**.

---

## 7. Canales y modos obligatorios

### Estructura conceptual de Channel

- `_members` -> usuarios unidos al canal.
- `_operators` -> usuarios a los que se permite acciones reservadas.
- `_invited` -> usuarios habilitados para entrar en un canal restringido.
- `_topic` -> texto del tema.
- `_password` / key -> clave de ingreso si `+k`.
- flags de modo y `_userLimit` -> controles de permisos/aforo.

Los nombres anteriores se conocen de fragmentos y resúmenes; **verificar la declaración real de `Channel.hpp`** durante la práctica.

### Modos

| Modo | Función | Defensa oral breve |
|---|---|---|
| `+i` / `-i` | Invitar obligatorio / acceso libre | “It restricts channel entry to invited users.” |
| `+t` / `-t` | Solo operadores cambian topic / regular también | “It restricts topic changes to operators.” |
| `+k` / `-k` | Definir / quitar clave | “It protects a channel with a join key.” |
| `+o nick` / `-o nick` | Dar / quitar operador a un miembro | “It changes one member's operator privilege.” |
| `+l N` / `-l` | Definir / quitar límite de usuarios | “It controls how many members can join.” |

**¿Por qué `MODE #ops` respondió `324 ... +` después de `+o titi`?** Porque `+o` pertenece al miembro Titi. Cuando `i/t/k/l` están desactivados, el string de modos de canal puede constar solo de `+`.

### Operadores y usuarios normales

- `KICK`: un operador expulsa a un usuario de un canal.
- `INVITE`: permitir a un usuario entrar en canal; comprobar permisos especialmente en `+i`.
- `TOPIC`: ver o cambiar tema, bajo `+t` restringir cambios.
- `MODE`: cambiar restricciones y privilegios. Primero comprobar que solicitante tiene permiso.

**No confundir KICK con desconexión del servidor.** Un usuario expulsado de un canal puede seguir conectado a otros canales y recibir mensajes privados.

---

## 8. La desconexión, explicada muy claramente

**Cierre de ventana, Ctrl+C en nc, FIN, RST o error:** el sistema operativo cambia el estado del socket; `poll()` señala disponibilidad/error/hangup o `recv()` devuelve 0. No hay una bandera universal única para todos los cierres.

`removeClient(fd)` según el código observado:

```text
1. Buscar Client* en _clients
2. removeClientFromChannels(client)  [eliminar referencias]
3. delete client                       [liberar objeto]
4. _clients.erase(iterator)            [eliminar entrada del mapa]
5. _fds.erase(iterator)                [dejar de hacer poll del fd]
6. close(fd)                           [cerrar descriptor TCP]
7. imprimir log de desconexión
```

**Definiciones fundamentales:**

- **Dangling pointer:** un canal conserva una dirección a `Client` ya destruido.
- **Memory leak:** memoria dinámica que ya no se puede liberar porque se ha perdido su referencia propietaria.
- **FD leak:** recurso de descriptor que el programa no cierra.
- **Use-after-free:** utilizar el `Client*` **después** de `delete`.
- **Undefined behavior:** el estándar no garantiza cómo se comportará el programa; podría crashear, mostrar datos incorrectos o aparentar funcionar.

**Respuesta de defensa:**

> We remove the client's references from channels first, so no channel keeps pointers to a deleted object. Then we delete the Client, remove it from the server's containers, and close the socket descriptor. Other clients remain connected.

### Por qué nuestro `for` de poll no usa `++i` automáticamente

Imagina `_fds = [server, Roger, Titi, Katia]`. En índice 1 se elimina Roger; Titi se mueve a índice 1. Un `for (...; ++i)` pasaría al índice 2, **saltándose Titi durante esta vuelta**. Por eso incrementamos solo cuando `_fds[i].fd` sigue siendo el fd que estábamos procesando.

**No significa que se salte `poll()` o que el bucle no avance:** si se elimina el fd, el siguiente ocupa su lugar; si se conserva, avanzamos.

---

## 9. Valgrind y Ctrl+C: explicación defensible

**Resultado más reciente comunicado (PID 108518):**

```text
ERROR SUMMARY: 0 errors from 0 contexts
       definitely lost: 0 bytes
       indirectly lost: 0 bytes
         possibly lost: 0 bytes
       still reachable: 73,760 bytes in 3 blocks
```

Desglose conocido: `32 bytes` de `std::vector<pollfd>`, `1,024 bytes` del buffer C stdio, `72,704 bytes` de libstdc++.

**Por qué ocurre:** se usó `Ctrl+C` y el proceso murió por SIGINT mientras esperaba en `poll()`; con esa ruta no hubo destrucción normal del objeto `Server`.

**Diferencia:** `still reachable` = sigue existiendo una ruta de referencias; `definitely lost` = ya no hay ruta que permita liberar. No decir que 73.760 bytes “son pérdidas”. Tampoco decir “la memoria quedó libre correctamente”: esa prueba no observó el destructor normal. **Esta evidencia es favorable, no una garantía de evaluación**.

**Pregunta:** “Why did Valgrind show still reachable allocations?”

> We stopped the server with Ctrl+C, which terminated it while the main poll loop was running. The normal destructor was not executed. Valgrind reported zero errors and zero lost bytes, but it still found reachable allocations, including a small poll-vector block and C/C++ library buffers. We can explain the difference between reachable memory and leaked memory.

**¿Añadir un signal handler para limpiar?** Es técnicamente posible, pero un handler no debe manipular directamente `std::vector`/`std::map` ni llamar destructores complejos. Una señal debería notificar al bucle de forma segura y el cleanup ocurrir fuera del handler. **No introducirlo en la versión final sin pruebas y sin necesidad explícita**.

---

## 10. HexChat, comandos opcionales y el alcance del subject

En la escuela HexChat logró conectar, unirse al canal, comunicarse con clientes `nc`, cambiar privilegios `+o` y ejecutar KICK. Se observaron avisos `WHO`/`WHOIS` desconocidos (`421`).

**Qué decir:**

> Our subject requires a working reference IRC client for registration, channels, messages, and the mandatory operator commands. We used HexChat successfully for those interactions. WHO and WHOIS are standard IRC queries outside the explicit mandatory list; our server currently does not implement the full IRC RFC.

**Matiz:** la condición del subject de que el cliente conecte “without encountering any error” puede dar lugar a preguntas por los avisos opcionales. Reconocerlos honestamente: no describirlos como errores de login sin evidencia. Los mandatorios funcionando es evidencia positiva, no una exención automática de cualquier problema de compatibilidad.

**QUIT:** no aparece en lista explícita de comandos obligatorios, pero se usa frecuentemente en clientes IRC estándar. Es buen ejercicio de defensa; no debe incorporarse sin cuidar las referencias y el parser.

---

## 11. Banco de preguntas: estudiar en pareja

**Modo de práctica:** uno pregunta, el otro responde en inglés en 20–45 segundos. Después, abrir archivo y mostrar función. Cambiar roles. Puntuación: 0–4 conceptos incorrectos, 5–6 parcial, 7–8 bien, 9–10 explica causa y demuestra la línea de código.

1. What is the difference between an IRC server and an IRC client?
2. What is the difference between a listening socket and an accepted client socket?
3. At what exact moment do we create `Client`?
4. Why use `poll()` and not one blocking `recv()` per client?
5. Does non-blocking I/O mean parallel execution?
6. What do `.events` and `.revents` represent in `pollfd`?
7. Explain POLLIN, POLLOUT, POLLHUP, POLLERR and POLLNVAL.
8. Why do we monitor POLLOUT only when output is pending?
9. Can one recv contain three IRC commands? Can one command require three recv calls?
10. Why must IRC commands be terminated correctly (`\r\n`)?
11. What happens if recv returns 0? What about -1?
12. Why does the subject 11.0 constrain the use of `errno`?
13. What do we do when send writes only 300 bytes out of 1000?
14. What is SIGPIPE and why do we use MSG_NOSIGNAL on Linux?
15. Why do we remove channel references before deleting Client?
16. What is the difference between a dangling pointer and a memory leak?
17. Why do file descriptor numbers get reused after close?
18. What is the purpose of PASS/NICK/USER and when is registration complete?
19. Why can a client have a nickname but still be unregistered?
20. What does `USER roger 0 * :Roger Example` mean?
21. What happens if a regular member tries KICK?
22. How does `+i` change JOIN behavior? How do you test it?
23. Why doesn't `+o` necessarily appear in a general MODE query?
24. Why is a user kicked out of a channel still connected to the server?
25. What does `353` mean? Why do we send `366`?
26. What is the benefit of having separate Server, Client and Channel classes?
27. Why doesn't Ctrl+Z normally close a TCP socket?
28. What did the 3,000-message flood prove, and what did it *not* prove?
29. What exactly did Valgrind report? Is still reachable a lost allocation?
30. How would you locate a handler quickly and make a one-line change safely?

**Consejo:** si no conoces la respuesta, señala el archivo y razona a partir del flujo; no inventes comportamiento RFC completo que el proyecto no tiene.

---

## 12. Ejercicios de modificación de código (rama temporal)

**Regla imprescindible:** estas son **prácticas propuestas**, no preguntas oficiales confirmadas. La escala subida de 5 páginas no especifica el cambio en vivo. No las aplicar a `main` ni a la rama que vais a entregar sin acordarlo con el compañero.

### Preparar una rama de prácticas

```bash
git status -sb
git switch -c practice/defense-irc
```

Si el Git de la escuela no admite `git switch`, usar `git checkout -b practice/defense-irc`. No hacer `git push` de esta rama a la entrega. Al terminar, volver a rama original y verificar `git status`.

### Ejercicio 1 — Cambiar la respuesta de bienvenida (nivel muy fácil)

**Consigna:** “Please include the current nickname in the welcome message and show the result.”

**Pasos:** buscar `001` o `Welcome` en `src/ServerRegistration.cpp` y `src/ServerReplies.cpp`; identificar función; modificar **solo el texto**; compilar; comprobar respuesta con `nc`. Volver a versión inicial con Git sin arrastrar el cambio al obligatorio.

```bash
grep -Rn 'Welcome\|001' src include
make
```

**Qué explicas:** dónde se comprueba registro, cómo se construye la respuesta y por qué no se llama `send()` directamente desde ese handler.

### Ejercicio 2 — Añadir una validación (nivel fácil)

**Consigna:** “Reject a malformed USER with a clear error.”

**Buscar:** `ServerRegistration.cpp`, handler `USER`, número de argumentos y respuesta `461`. En una rama de ensayo, cambiar un caso de validación **sin alterar la estructura principal**. Probar USER inválido y válido en conexiones nuevas.

**Qué explicas:** tokenización, trailing parameter y momento de checkRegistration.

### Ejercicio 3 — Localizar permiso de operador (nivel fácil/medio)

**Consigna:** “Show me where your code refuses TOPIC changes by a non-operator when +t is active.”

**Buscar:** `handleTopic`, `isOperator`, modo `topicRestricted` (nombres reales a confirmar). Puede que solo pidan indicar/cambiar una condición. Mostrar `482` desde un cliente sin privilegios y aceptación desde operador.

**Qué explicas:** miembro ≠ operador, estado almacenado en Channel, validación previa a mutación.

### Ejercicio 4 — QUIT mínimo, solo en práctica (nivel medio)

**Consigna:** “Add a basic QUIT command that closes the client connection.”

**Objetivo del ejercicio:** identificar el final de un comando IRC, cerrar el cliente y evitar utilizar un puntero destruido. Una ubicación simple en la arquitectura que vimos está en el bucle que extrae mensajes completos de `handleData()` **antes** del dispatch:

```cpp
message = client->extractMessage();
if (message.empty())
    break ;
if (message == "QUIT" || message.compare(0, 5, "QUIT ") == 0)
{
    removeClient(fd);
    return ;  // Obligatorio: client queda destruido.
}
dispatchCommand(client, message);
```

**Advertencias importantes:**

- Solo una **simulación mínima**, no IRC QUIT conforme a RFC. Falta posible anuncio `QUIT` a otros miembros y manejo más completo de razones, espacios y normalización de comando.
- **No** hacer `removeClient(fd)` y continuar el `while`: sería uso de `Client*` destruido.
- Si implementas QUIT dentro de un handler llamado por `dispatchCommand`, debe existir una manera de avisar al llamador que el Client ya fue eliminado; de lo contrario `handleData()` puede seguir usando el puntero. Por eso se muestra la alternativa anterior para estudiar, no como arquitectura definitiva.
- Revisar si `extractMessage()` devuelve una línea sin `\r\n`; el caso `QUIT :reason` se compara con el prefijo `QUIT `.

**Test manual:** conectar dos clientes, `JOIN #quitpractice`, mandar `QUIT :Bye` desde uno; el otro debe seguir conectado, el fd cerrarse, nuevo cliente reutiliza el nick. Ejecución mínima **no anuncia QUIT a terceros**, no atribuirle esa característica.

**Defensa en inglés:**

> I parse QUIT as a complete IRC command, call the existing cleanup function, and return immediately because removeClient deletes the Client object. Continuing to use the pointer would be a use-after-free bug.

### Ejercicio 5 — Ajustar un límite `+l` (nivel medio)

**Consigna:** “What would you change to reject an invalid user limit?”

**Buscar:** `MODE +l` en `ServerChannels.cpp`; localizar parseo del número, comprobación de usuario operador, guardado en Channel. Explicar qué pasa con límite 0, letras o números negativos. **Antes de escribir, comparar comportamiento real con el subject.**

### Ejercicio 6 — Cambio de orden dentro de poll (nivel avanzado, no necesariamente escribir)

**Consigna de razonamiento:** “What if POLLIN and POLLHUP are set together while unread bytes remain?”

**Objetivo:** explicar por qué priorizar HUP puede perder bytes; cómo investigar sin introducir `recv` fuera de poll; cómo probar último mensaje antes de disconnect. **No ofrecer un parche sin revisar todas las ramas** del loop y la política de cleanup.

---

## 13. Mini-simulacro de evaluación (45–60 minutos)

**Persona A evalúa a B; después cambiar roles.**

**Parte I — Arquitectura (10 min)**

- Explicar 90 segundos el proyecto.
- Mostrar en código `main`, `socket`, `bind`, `listen`, `poll`, `accept`.
- Decir quién posee `Client*` y quién posee `Channel*`.

**Parte II — Mensajes (10 min)**

- Completar registro de dos usuarios.
- Enviar un mensaje canal y otro privado.
- Rastrear `recv` -> input -> dispatcher -> output -> `send`.

**Parte III — Operadores (10 min)**

- Demostrar `+i` con rechazo y posterior INVITE.
- Demostrar `+t` y TOPIC; un `+o` + KICK.
- Explicar por qué `MODE #canal` puede mostrar solo `+`.

**Parte IV — Red y memoria (10 min)**

- Explicar comando cortado en dos.
- Explicar Ctrl+Z y buffer de salida.
- Interpretar Valgrind con 0 errores, 0 perdidos y still reachable.

**Parte V — Modificación (10–20 min)**

- Elegir **uno** de los ejercicios 1–5.
- Localizar función sin ayuda.
- Hacer cambio mínimo en rama de prácticas.
- Compilar, probar caso positivo y negativo.
- Explicar en inglés por qué no se creó una fuga ni se rompió el flujo.

### Rúbrica interna /10

| Dimensión | Puntos |
|---|---:|
| Explicación correcta de arquitectura y ownership | 2 |
| Comprender `poll`, `recv`, `send`, buffers y non-blocking | 3 |
| Comandos, estados y permisos | 2 |
| Mostrar y explicar código real, no solo teoría | 1 |
| Modificación segura + compilación y test | 2 |

Una calificación interna **no predice la nota oficial**, pero señala qué temas hay que estudiar.

---

## 14. Última revisión antes de la defensa

- [ ] Los dos podéis recitar la introducción de 90 segundos y localizar cada archivo.
- [ ] Los dos sabéis responder qué es `poll()` y qué **no** hace.
- [ ] Los dos podéis explicar un `PRIVMSG` completo siguiendo el código.
- [ ] Los dos sabéis por qué el Client se crea **antes** de PASS/NICK/USER.
- [ ] Los dos sabéis distinguir output buffer de input buffer.
- [ ] Los dos podéis explicar puntero colgante, leak y FD leak.
- [ ] Los dos entendéis los cinco modos y el error `482`.
- [ ] Los dos entendéis por qué Valgrind mostró memoria aún accesible tras Ctrl+C.
- [ ] Los dos tenéis claro que en la versión 11.0 hay una advertencia sobre `errno`.
- [ ] Los dos podéis hacer un cambio pequeño en **rama de ensayo**, compilar y demostrarlo.
- [ ] El README público y el Git de entrega corresponden exactamente a la versión probada.

**Próxima sesión propuesta:** abrir `src/Server.cpp` y `src/ServerNetwork.cpp`, seguir línea por línea una conexión y un PRIVMSG, hacer preguntas orales de 30 segundos, y después practicar el ejercicio QUIT en rama temporal.

---

## 15. Fuentes y límites

**Exigencias formales:** subject `ft_irc` versión 11.0 (especialmente páginas 6, 8–10 y 12) y escala `evaluation_sheet.pdf` aportada (5 páginas). La escala activa del Intra de 42 será la autoridad durante la defensa.

**Hechos del servidor:** fragmentos de `Server.cpp` y `ServerNetwork.cpp` pegados en la conversación, búsquedas `grep` y salidas de pruebas. Las otras funciones se describen al nivel probado o reportado, no como afirmación de auditoría exhaustiva.

**Datos Valgrind:** resultado facilitado por `chrrodri` con `PID 108518`, incluidos 0 errores y `73,760 bytes still reachable`. El resultado corresponde a terminación por `SIGINT`.

**Teoría de red para estudio:** páginas de manual de Linux sobre `poll`, `recv`, `send`, `fcntl`; RFC 1459 y RFC 2812 (como contexto, no obligación de implementar todos los comandos).

**Documento compañero:** `ft_irc_EVALUATION_TESTS_INTERNAL.md` contiene los comandos exactos y las condiciones de paso para cada test.
