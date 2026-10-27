# ft_irc — Guía interna de evaluación y pruebas

**Equipo:** `bsamy` y `chrrodri`  
**Actualizado:** 27 de octubre de 2026  
**Uso:** guía privada de trabajo y simulación de defensa; no sustituye el subject ni la escala activa de Intra.  
**Versión de referencia:** `en.subject.pdf`, **ft_irc 11.0**.  
**Otra fuente aportada:** `evaluation_sheet.pdf`, captura de cinco páginas de **42 EvalHub**.  
**Estado:** pruebas extensas favorables; **no es una certificación de aprobado**.

> **Lectura antes de evaluar:** comprobar la escala disponible en 42 Intra. La escala que se aportó es una copia fechada y podría no incluir pruebas nuevas. El subject 11.0 incorpora una advertencia estricta sobre `errno` tras `read`/`recv` y `write`/`send`. Nunca suponer que una copia pública de la escala prevalece sobre Intra.

---

## 1. Cómo usar esta guía

1. Una persona es **evaluador**: lee la prueba, pide una explicación y observa la salida; la otra maneja el proyecto. Luego invertir papeles.
2. Mostrar el **repositorio clonado** y un `make` limpio **antes** de reutilizar binarios previos.
3. Para cada test, guardar: comando, resultado esperado, resultado observado y `PASÓ / FALLÓ / NO PROBADO`.
4. Escribir `PASS`, `JOIN`, etc. **dentro de netcat**; escribir `python3`, `git`, `make`, `ps`, etc. **en la shell** donde aparece `$` o `%`.
5. Para evitar interferencias entre los modos, utilizar nombres de canal nuevos por test.
6. No añadir bonus ni comandos opcionales a última hora si el obligatorio funciona.

### Leyenda de estado

- **Comprobado:** existe salida o prueba comunicada por el equipo en esta conversación.
- **Parcial:** pasó una comprobación básica, pero una variante rigurosa no está demostrada.
- **Pendiente:** requiere revisión o confirmación sobre la versión exacta que se entregue.

---

## 2. Traducción operativa de la escala de evaluación

| Fuente de la escala | Qué pide | Riesgo | Estado actual |
|---|---|---|---|
| Pág. 1–2: reglas | Trabajar desde el Git entregado, clonarlo en carpeta vacía, evitar crashes, comprobar leaks | Eliminatorio ante ciertos fallos | **Clon remoto limpio pendiente** |
| Pág. 3: Basic checks | Makefile, C++98, ejecutable `ircserv`, un `poll()`/equivalente | **Nota 0** si falla | Compilación y búsqueda estática favorables |
| Pág. 3: Basic checks | `poll()` controla `accept`, `recv`, `send` | **Nota 0** si falla | Ruta general revisada; auditar ejecución de la versión final |
| Pág. 3: Basic checks | `fcntl(fd, F_SETFL, O_NONBLOCK)`; no otras llamadas | **Nota 0** según escala aportada | Dos llamadas verificadas en grep |
| Subject v11, pág. 8 | No usar `errno` para decidir qué hacer tras `recv`/`send` | **Nota 0** según subject | Código modificado y retestado; comprobar grep final |
| Pág. 3–4: Networking | Escucha, `nc`, cliente IRC real, múltiples conexiones y mensajes por canal | Obligatorio | `nc` y HexChat favorables |
| Pág. 4: Specials | Comandos parciales, cierres inesperados, `Ctrl+Z` + flood + `fg` | Obligatorio | Favorable; conteo exacto de 3.000 mensajes no demostrado |
| Pág. 4: Basic client | Autenticación, nickname, username, JOIN, PRIVMSG privado | Obligatorio | Probado con `nc` y mensajería HexChat |
| Pág. 4: Operator | KICK, INVITE, TOPIC, MODE `i,t,k,o,l`, permisos de usuarios | Obligatorio | Probado con `nc`; HexChat: +o/KICK verificados; no toda la matriz en HexChat |
| Pág. 5: Bonus | Transferencia de ficheros o bot | Opcional | No se reclama bonus |

**Importante:** la escala de cinco páginas facilitada **no describe un ejercicio concreto de modificación en vivo**. Sus reglas iniciales desaconsejan editar archivos durante la defensa, salvo excepciones acordadas. El subject 11.0, pág. 12, dice que una modificación breve *puede* pedirse **si las instrucciones vigentes de evaluación la especifican**. Prepararse para ella, pero no cambiar el código entregado por iniciativa propia durante la defensa.

---

## 3. Preparación de entorno: prueba eliminatoria inicial

### A. Estado Git y clon limpio

En la carpeta original:

```bash
git status -sb
git diff --check
git log -1 --oneline
git remote -v
git ls-files
```

Verificar los cambios del compañero antes de hacer `git push`. No hacer `push --force` para solucionar divergencias.

Clonar la **URL real** del remoto en un directorio nuevo y vacío (sustituir el marcador):

```bash
mkdir -p ~/ft_irc_eval_clean
cd ~/ft_irc_eval_clean
git clone <URL_REAL_DEL_REPOSITORIO> ft_irc
cd ft_irc
make fclean && make
```

**Resultado:** el clon compila sin copiar ficheros desde la carpeta de desarrollo. Comprobar `README.md`, Makefile, `include/`, `src/` y que no faltan fuentes. No subir `.o`, `ircserv`, logs de Valgrind ni PDFs de evaluación por accidente.

### B. Compilación, reglas y relink

```bash
make fclean && make
make
ls -l ircserv
grep -nE '(^|[[:space:]])(all|clean|fclean|re):' Makefile
```

**Esperado:** `c++ -Wall -Wextra -Werror -std=c++98`, ejecutable `ircserv`. Un segundo `make` sin cambios **no relinka** innecesariamente. Las reglas obligatorias incluyen `$(NAME)`, `all`, `clean`, `fclean`, `re`.

### C. Comprobaciones estáticas de red

```bash
grep -RnE --include='*.cpp' --include='*.hpp' 'poll\(' src include
grep -RnE --include='*.cpp' --include='*.hpp' 'fcntl\(' src include
grep -RnE --include='*.cpp' --include='*.hpp' 'accept\(|recv\(|send\(' src include
grep -RnE --include='*.cpp' --include='*.hpp' 'errno|EAGAIN|EWOULDBLOCK|EINTR|fork\(' src include
grep -RnE --include='*.cpp' --include='*.hpp' 'MSG_NOSIGNAL' src include
```

**Interpretación:** las cadenas de error como `"poll() failed"` también aparecen en grep, pero **no son llamadas**. Con grep solo vemos ubicaciones: el evaluador puede pedir que sigamos la lógica `revents & POLLIN` antes de `accept/recv`, y `revents & POLLOUT` antes de `send`.

**Punto crítico v11:** verificar que no se consulta `errno` para decidir qué hacer tras `recv()` y `send()`. El cambio comunicado por el equipo deja `recv()==0` como cierre; una operación que devuelve negativo regresa al evento, sin reintento inmediato condicionado por `errno`. **Advertencia técnica:** esto obliga a revisar que los fallos permanentes de escritura no causen busy-loop y que se gestionen los errores/hangups de `poll()`.

**Punto crítico:** el subject coloca la forma restringida de `fcntl()` en la subsección **For MacOS only**; la hoja de evaluación aportada la solicita en Basic checks sin limitarla a MacOS. Por prudencia, se utiliza la forma estricta en todos los sockets.

---

## 4. Configuración de los tests funcionales

Servidor, terminal S:

```bash
./ircserv 6667 pass
```

Clientes A, B y C, cada uno en una terminal nueva:

```bash
nc -C 127.0.0.1 6667
```

**En A (Roger):**

```irc
PASS pass
NICK roger
USER roger 0 * :Roger
```

**En B (Titi):**

```irc
PASS pass
NICK titi
USER titi 0 * :Titi
```

**En C (Katia):**

```irc
PASS pass
NICK katia
USER katia 0 * :Katia
```

**Esperado:** respuesta `001` para cada usuario. En la variante de netcat usada por el equipo, `-C` transmite líneas terminadas en CRLF. La disponibilidad de `-C` depende de la variante instalada.

> **Si sale `zsh: command not found: PASS`**, estás en la shell, no en la conexión netcat. Vuelve a abrir `nc -C ...` y escribe ahí los comandos IRC.

---

## 5. Matriz completa de comandos obligatorios

### T01 — Autenticación y registros incorrectos

**En cliente nuevo D** (sin haberlo registrado):

```irc
PASS wrong
NICK wrongtest
USER wrongtest 0 * :Wrong Tester
JOIN #basic
```

**Esperado:** `464` (wrong password), no respuesta `001`, y `451` al intentar JOIN.

En esa **misma** conexión:

```irc
PASS pass
```

**Comportamiento observado:** registro completado (`001`) tras corregir contraseña, porque NICK y USER ya estaban almacenados. **No significa** que se haya permitido JOIN sin contraseña. Después de registrado, repetir `PASS pass` produjo `462`.

Otros casos, cada uno en estado apropiado o conexión nueva:

```irc
NICK roger
USER wrongtest 0 :Missing
UNKNOWNCOMMAND
```

**Esperados según caso:** `433` para nick de *otro* cliente ya usado; `461` para USER incompleto; `421` para comando desconocido. Confirmar que la conexión conserva estabilidad. No extrapolar estas respuestas a un cliente ya registrado sin revisar su contexto.

**Caso de seguimiento:** repetir `NICK` con el **mismo nick propio** puede devolver `433` en el código visto en pruebas: compatibilidad menor a analizar, sin afirmar que sea mandato explícito de la escala.

### T02 — JOIN, NAMES y mensajes privados/de canal

En A:

```irc
JOIN #basic
```

En B y C:

```irc
JOIN #basic
```

En A:

```irc
PRIVMSG #basic :Hello everyone
PRIVMSG titi :Private hello
PRIVMSG katia :Only for Katia
```

En B:

```irc
PRIVMSG #basic :Hello Roger and Katia
PRIVMSG roger :Private reply
```

**Esperado:** `353` / `366`, `@roger` como primer operador; mensajes de canal a los otros miembros; privado solo al destinatario. También probar `PRIVMSG nobody123 :Hello` (error `401`) y `PRIVMSG #missing :Hello` sin crear el canal.

### T03 — KICK y restricción de operador

En B:

```irc
KICK #basic roger :Not operator
```

**Esperado:** `482`.

En A:

```irc
KICK #basic titi :Operator test
```

En B:

```irc
PRIVMSG #basic :Can I still write?
JOIN #basic
```

**Esperado:** mensaje KICK, error `442` si el no miembro intenta escribir (comportamiento probado en nuestra implementación), luego reingreso permitido si no hay restricciones.

### T04 — INVITE, `+i` y `-i`

A:

```irc
JOIN #inv
MODE #inv +i
MODE #inv
```

C:

```irc
JOIN #inv
```

**Esperado:** rechazo `473`.

A:

```irc
INVITE katia #inv
```

C:

```irc
JOIN #inv
```

**Esperado:** entrada permitida. Para probar el permiso desde miembro no operador, registra un cuarto usuario `auditfour` en un netcat nuevo (si no estaba registrado), después:

En A:

```irc
INVITE titi #inv
```

En B:

```irc
JOIN #inv
INVITE auditfour #inv
```

**Esperado:** `482` en el INVITE de B, porque `+i` requiere permisos de operador. En A: `INVITE auditfour #inv`; en D: `JOIN #inv` -> permitido. A desactiva con `MODE #inv -i`. Si alguno ya había sido invitado antes, elegir otro nickname registrado para evitar falsas conclusiones.

### T05 — TOPIC, `+t` y `-t`

A y B entran en `#top` (A primero).

En B:

```irc
TOPIC #top :Topic from regular user
```

En A:

```irc
MODE #top +t
```

En B:

```irc
TOPIC #top :Unauthorized change
```

**Esperado:** `482`.

En A:

```irc
TOPIC #top :Official topic
TOPIC #top
MODE #top -t
```

En B:

```irc
TOPIC #top :Allowed again
```

**Esperado:** cambio aceptado tras `-t`.

### T06 — Clave de canal `+k` / `-k`

A:

```irc
JOIN #key
MODE #key +k secret
```

B:

```irc
JOIN #key
JOIN #key wrong
JOIN #key secret
```

**Esperado:** dos rechazos `475`, luego JOIN. A: `MODE #key -k`. C: `JOIN #key` sin contraseña -> permitido.

### T07 — Límite de usuarios `+l` / `-l`

A:

```irc
JOIN #limit
MODE #limit +l 1
```

B: `JOIN #limit` -> error `471`.

A: `MODE #limit +l 2`; B: `JOIN #limit` -> acepta.

C: `JOIN #limit` -> `471`.

A: `MODE #limit -l`; C: `JOIN #limit` -> acepta.

### T08 — Operadores individuales `+o` / `-o`

A, B y C entran en `#ops` (A primero).

B:

```irc
KICK #ops katia :Before promotion
```

**Esperado:** `482`.

A:

```irc
MODE #ops +o titi
MODE #ops
```

B:

```irc
KICK #ops katia :After promotion
```

**Esperado:** KICK permitido; `+o` cambia los privilegios de Titi. La consulta `MODE #ops` puede mostrar `324 ... +` si no hay otros modos de canal: `+o` es un privilegio **del miembro**, no un flag general de canal.

A:

```irc
MODE #ops -o titi
```

B: `KICK #ops roger :After demotion` -> `482`.

### T09 — Combinaciones y errores de modos (prueba adicional, no completada sistemáticamente)

```irc
MODE #ops +it
MODE #ops
MODE #ops -it
MODE #ops
MODE #key +k
MODE #limit +l invalid
```

Comprobar cambios consistentes, parámetros obligatorios, ausencia de crash y errores adecuados. **No afirmar soporte integral para todas las combinaciones `+kl` con parámetros** sin inspeccionar el parser: esa combinación se señaló anteriormente como caso no garantizado.

---

## 6. Networking specials: pruebas literales y de robustez

### T10 — Mensaje fragmentado (`Ctrl+D`)

Terminal nueva:

```bash
nc -C 127.0.0.1 6667
```

Escribir `PA` **sin Enter**, pulsar `Ctrl+D` **una vez**, escribir `SS pass` + Enter, luego:

```irc
NICK partial
USER partial 0 * :Partial
```

**Esperado:** solo se registra cuando se recibe el comando `PASS pass` completo. Verificar con la variante concreta de netcat: dependiendo de terminal, `Ctrl+D` puede comportarse distinto. La prueba del subject es la agregación de fragmentos `com`, `man`, `d\n`.

**Alternativa determinista en la shell** (si el comportamiento de `Ctrl+D` es confuso):

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

El script se utiliza solo para el test, no es parte del proyecto C++98.

### T11 — Varios comandos en la misma conexión

En la **shell**, no dentro de `nc`:

```bash
(printf 'PASS pass\r\nNICK batchuser\r\nUSER batchuser 0 * :batchuser\r\nJOIN #batch\r\n'; sleep 2) | nc 127.0.0.1 6667
```

**Esperado:** `001`, JOIN, `353`, `366`. La prueba verifica **varios comandos enviados por la misma conexión**; no demuestra que el kernel hiciera un único `recv()`.

### T12 — Desconexión imprevista con medio comando

Conecta otro `nc`, manda PASS/NICK y deja `USER half 0` sin Enter. Cierra con `Ctrl+C`. Comprueba que otro cliente sigue respondiendo y que el mismo nick puede utilizarse en una nueva conexión. La eliminación debe limpiar referencias a `Client` antes de destruirlo.

### T13 — Suspender cliente, inundar canal y reanudar

A y B entran en `#stress`. **B:** `Ctrl+Z`. En una **shell distinta**:

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

Mientras B está suspendido, comprobar que A sigue conectado y que C puede enviarle un privado. **B:** `fg`. Verificar que B recibe mensajes, incluido `02999` y `FLOOD_END`, sin colgar el servidor.

**Evidencia del equipo:** el marcador final llegó y otros clientes siguieron operando; en fotografía también se vio `02999`. **Limitación:** no se hizo un conteo independiente de las 3.000 líneas ni una prueba de garantía de entrega por número de secuencia.

### T14 — Último mensaje antes de cierre TCP normal

A conectado a `#last` como Roger. En otra **shell**:

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

**Esperado y observado en fotografía:** Roger recibe `LAST_MESSAGE_BEFORE_DISCONNECT`. **Riesgo no completamente cubierto:** el código conocido comprueba HUP antes de POLLIN; si ambos se señalan juntos, podrían descartarse bytes aún pendientes. Una prueba exitosa no demuestra que todas las intercalaciones se hayan cubierto.

### T15 — Diez cierres abruptos TCP/RST

A conserva conexión. En shell:

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

**Esperado y comunicado:** el script finaliza, el servidor sigue aceptando conexiones y los clientes existentes siguen comunicándose. El código termina con un comando IRC incompleto a propósito.

### T16 — Fallo permanente de escritura / CPU (pendiente de prueba concluyente)

El último cambio comunicado evita comprobar `errno` tras `send` y conserva la cola de salida cuando no hay progreso. En un error permanente algunos sistemas pueden volver a informar `POLLOUT`, lo que produciría un **busy-loop**. Un flood exitoso **no descarta** ese caso.

Para observar el proceso real al menos en reposo:

```bash
pgrep -af 'ircserv|valgrind'
ps -eo pid,ppid,stat,pcpu,rss,comm | grep -E 'ircserv|valgrind'
```

Ejecutar dos veces con intervalo breve **mientras el servidor esté vivo**. La prueba previa con `ps -C ircserv` no devolvió proceso, por lo que NO se marca como pasada. Ante CPU elevada persistente o desconexiones no limpiadas, revisar sin añadir bucles de reintento ni consultar `errno` como mecanismo de decisión prohibido por v11.

---

## 7. Cliente IRC real: HexChat (obligatorio) y AdiIRC

En HexChat (probado en Linux de 42): crear una red local con servidor `127.0.0.1/6667`, **server password** `pass` (no NickServ password), sin TLS/SASL y nick `hexa42`. El servidor debe estar ya ejecutándose.

| Prueba | Acción | Resultado |
|---|---|---|
| Conexión y registro | Connect | **Comprobado** |
| JOIN | `/join #test` | **Comprobado** |
| Canal | Mensaje normal sin slash en pestaña de canal | **Comprobado** |
| Privado | `/msg wrongtest hi` o diálogo abierto con nick | **Comprobado** |
| Privilegios | `/mode #test +o hexa42` desde operador | **Comprobado** en pruebas comunicadas |
| KICK | `/kick titi :test` desde operador | **Comprobado** en pruebas comunicadas |
| TOPIC | `/topic #test :New topic` | **Por verificar desde HexChat**, probado antes con `nc` |
| INVITE con +i | `/invite katia #test` cuando corresponda | **Por verificar desde HexChat**, probado con `nc` |

**Compatibilidad observada:** HexChat mostró `WHO` y `WHOIS` como comandos desconocidos (`421`). No son parte de los comandos obligatorios enumerados; sin embargo, el subject exige una experiencia de cliente de referencia razonablemente correcta. Registrar la observación; **no afirmar conexión “sin ningún error” si el cliente mostró esos avisos**. Decidir con el compañero si afectan los flujos efectivamente evaluados antes de añadir funcionalidades opcionales.

AdiIRC se probó anteriormente en Windows. **Nombrar un solo cliente de referencia** durante la evaluación (HexChat si es el disponible en 42) y explicar que `nc` es una herramienta de protocolo, no el cliente IRC gráfico de referencia.

---

## 8. Valgrind: resultado exacto y cómo defenderlo

### Comando usado

```bash
valgrind --leak-check=full --show-leak-kinds=all \
  --track-origins=yes --log-file=valgrind.log \
  ./ircserv 6667 pass
```

Se cerraron clientes, después se detuvo el servidor con `Ctrl+C`. Para leer la evidencia:

```bash
tail -n 90 valgrind.log
grep -E 'ERROR SUMMARY|definitely lost|indirectly lost|possibly lost|still reachable|Invalid read|Invalid write|Invalid free' valgrind.log
```

### Última salida comunicada — PID 108518

| Métrica | Resultado |
|---|---:|
| `ERROR SUMMARY` | **0 errors from 0 contexts** |
| `definitely lost` | **0 bytes** |
| `indirectly lost` | **0 bytes** |
| `possibly lost` | **0 bytes** |
| `still reachable` | **73,760 bytes in 3 blocks** |
| Vector de `pollfd` | **32 bytes** |
| Buffer de salida estándar de C | **1,024 bytes** |
| Asignación de la biblioteca C++ | **72,704 bytes** |

**Interpretación rigurosa:** `still reachable` significa que Valgrind aún podía encontrar referencias a esa memoria al finalizar; **no la clasifica como memoria perdida**. El servidor fue interrumpido por la acción predeterminada de `SIGINT` (Ctrl+C) mientras estaba en `poll()`, así que no siguió un camino normal de salida con destrucción del objeto `Server`. Los **32 bytes del vector** son memoria del proyecto que probablemente habría liberado su destructor si se ejecutara. Los **1.024 + 72.704 bytes** son buffers/asignaciones de bibliotecas. No se debe prometer que esos valores desaparecerán con seguridad en todos los sistemas.

**¿Puede hacer suspender?** La escala exige revisar leaks y puede penalizar fugas reales. **Este informe no muestra pérdidas ni errores de memoria**, por tanto es una evidencia favorable, pero **no es garantía de nota** ni prueba de que el apagado controlado funciona. Si el evaluador pregunta, explicar la diferencia entre `still reachable` y `definitely lost` y que se usó Ctrl+C.

**Respuesta breve en inglés:**

> Valgrind reported zero memory errors and zero definitely, indirectly or possibly lost bytes. It reported 73,760 bytes as still reachable, mostly from the C/C++ runtime, plus 32 bytes for our poll vector. We stopped the server with Ctrl+C while it was inside poll, so the normal Server destructor did not run. Still reachable does not mean the memory was lost, but this run does not prove graceful shutdown cleanup.

**Opcional, si el evaluador pide revisar FDs:** repetir con `--track-fds=yes`; analizar con cuidado la terminación por señal. No cambiar la gestión de señales justo antes de evaluación solo para eliminar “still reachable” sin probar cuidadosamente los efectos.

---

## 9. Comprobaciones finales de entrega y defensa

### Entrega

- [ ] La versión vigente del subject y escala en Intra se corresponde con lo presentado.
- [ ] Compila con los flags obligatorios desde **clon Git vacío**.
- [ ] `make` sin cambios no relinka.
- [ ] `poll`/`fcntl`/`errno` cumplen el texto vigente.
- [ ] README en inglés: primera línea en cursiva con logins exactos, Description, Instructions, Resources, AI usage.
- [ ] No se han añadido por accidente binarios, logs, pruebas privadas o PDFs innecesarios.
- [ ] Ambos colaboradores conocen commit, rama y remoto final.
- [ ] Referencia HexChat reproduce los comandos obligatorios fundamentales.
- [ ] CPU / fallos permanentes de envío investigados si la evaluación lo exige.
- [ ] Preguntas orales practicadas **por ambos**.

### Para que el evaluador vea rápidamente el proyecto

1. `make fclean && make`.
2. `./ircserv 6667 pass`.
3. Abrir `nc -C` y HexChat.
4. Registrar usuario, `JOIN`, mensajes de canal y directo.
5. Mostrar KICK y MODE con permisos.
6. Explicar `poll`, buffers, manejo de desconexión.
7. Mostrar el log de Valgrind con interpretación prudente.
8. Estar preparado para un cambio pequeño **solo si la escala vigente lo pide**.

---

## 10. Preguntas que el evaluador puede formular después de cada test

- **¿Dónde se crea `Client`?** Tras un `accept()` exitoso, antes de completar PASS/NICK/USER.
- **¿Por qué solo un `poll()`?** Un bucle central vigila tanto listening fd como clientes para lecturas y escrituras.
- **¿Por qué no hay `fork()`?** El subject lo prohíbe; se maneja concurrencia con I/O no bloqueante.
- **¿Por qué `POLLIN` antes de `recv()`?** Evita lecturas sin evento de disponibilidad; en socket no bloqueante el `recv` no espera indefinidamente.
- **¿Por qué `POLLOUT` solo cuando hay datos?** Evita despertar el ciclo continuamente por sockets casi siempre escribibles.
- **¿Qué ocurre si `send` acepta 300 de 1000 bytes?** Se borran únicamente los 300 aceptados; quedan 700 en cola.
- **¿Qué sucede si `recv` devuelve 0?** El peer cerró el sentido de envío; se limpia ese cliente.
- **¿Por qué `MODE #canal` devuelve `+` tras `+o`?** `+o` es privilegio de miembro, no modo general del canal.
- **¿Por qué no está implementado `QUIT`?** No está enumerado entre las funciones obligatorias del subject; la desconexión TCP sí se gestiona.
- **¿Qué es `SIGPIPE`?** Señal de escritura a conexión rota; `MSG_NOSIGNAL` en Linux evita que ese `send` termine el proceso por la acción por defecto.
- **¿Qué indica `still reachable`?** Memoria todavía referenciable, distinta de memoria perdida; Ctrl+C impidió salida normal.

Para respuestas extensas, ejercicios de modificación y recorrido de archivos, consultar **`ft_irc_DEFENSE_MASTERCLASS_INTERNAL.md`**.

---

## 11. Fuentes y alcance

- **Subject 11.0 aportado**, `en.subject.pdf`: introducción y reglas (págs. 3–5), Mandatory y red (págs. 6–9), README (pág. 10), evaluación/modificación (pág. 12).
- **Subject 10.0 aportado**, `ft_irc.pdf`: comparación histórica; para las decisiones actuales priorizar 11.0 y la versión que figure en Intra.
- **Escala aportada** `evaluation_sheet.pdf` (42 EvalHub, 5 págs.): Basic checks, Networking, Networking specials, Commands, Bonus.
- **Evidencia real del equipo:** salidas de compilación, `nc`, HexChat, scripts Python y Valgrind compartidas en la conversación. Donde falta evidencia se indica expresamente.
- **Referencias técnicas externas de estudio (no requisitos nuevos del subject):** `man 2 poll`, `man 2 recv`, `man 2 send`, RFC 1459 / RFC 2812.
