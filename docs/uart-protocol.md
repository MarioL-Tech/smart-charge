# UART Protocol — ESP32 ↔ Raspberry Pi

Line-based ASCII protocol over the UART link (115200 baud, `\n` terminated).
This is the contract between the ESP32 and the Raspberry Pi. See
`docs/pin-connection.md` for the wiring.

## ESP32 → Raspberry Pi

| Message | Meaning |
|---|---|
| `EVSE:STATUS:CHARGING:ON:SRC:<source>` | Charging started |
| `EVSE:STATUS:CHARGING:OFF:SRC:<source>` | Charging stopped |
| `EVSE:STATUS:ANTITHEFT:ACTIVE:SRC:<source>` | Anti-theft lock engaged (servo 90°) |
| `EVSE:STATUS:ANTITHEFT:INACTIVE:SRC:<source>` | Anti-theft lock released (servo 0°) |
| `EVSE:RFID:CARD:UID:<uid>` | RFID card detected (`uid` = hex, e.g. `AB:CD:EF:12`) |
| `EVSE:ERROR:UNKNOWN_CMD:<cmd>` | Unknown command received |

`<source>` is who changed the state: `boot`, `pi`, `rfid` or `query`
(status was requested).

## Raspberry Pi → ESP32

| Command | Meaning |
|---|---|
| `CMD:CHARGE:ON` | Start charging |
| `CMD:CHARGE:OFF` | Stop charging |
| `CMD:STATUS` | Request current status (ESP32 replies with `EVSE:STATUS:...`) |

## Behavior rules

1. The ESP32 sends `EVSE:STATUS:...` on **every state change** (including boot).
2. Repeated identical commands are **not** re-sent (state did not change).
3. RFID tap = **manual override**: toggles the charging state, reported as
   `SRC:rfid`. Cards held on the reader are debounced (800 ms).
4. **UID whitelist:** currently *any* card toggles the state. A whitelist of
   allowed UIDs is a planned extension.
5. Unknown commands are answered with `EVSE:ERROR:UNKNOWN_CMD:<cmd>`.

## Charging station interface

TBD — the physical interface to the charging station (relay/contactor,
CP/PP signal, Modbus, ...) is still under discussion. The ESP32 code isolates
this behind the `applyChargingState()` function so the interface can be added
without touching the protocol.
