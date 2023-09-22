# picoFi
A wifi helper for the Raspberry Pi Pico W meant to help develop IoT devices *FAST*.

## What it does

- Allows users to set which AP to connect to
- Creates its own AP when the set one is unavailable
- Opens a web server on port 80

## Blink patterns

```
🟩🟩🟩🟩🟩🟩🟩🟩🟩🟩⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛ = about to connect

🟩⬛⬛⬛⬛🟩⬛⬛⬛⬛🟩⬛⬛⬛⬛🟩⬛⬛⬛⬛... = connecting

🟩█🟩█🟩█🟩█ = error

🟩█🟩█🟩█🟩█🟩█🟩█🟩█🟩█🟩█🟩█🟩█🟩█🟩█... = erasing flash

🟩🟩🟩🟩🟩🟩🟩🟩🟩🟩🟩🟩🟩🟩🟩🟩🟩🟩🟩🟩... = server up

⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛⬛... = setup server up
```
