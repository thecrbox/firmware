# Prerequisites 

1. Install ESPHome into the container user environment (pinning to the project-tested release):
   ```bash
   pip install --user esphome==2026.0.1
   ```
2. Ensure `~/.local/bin` is on your `PATH` for the session (or call the binaries explicitly):
   ```bash
   export PATH="$HOME/.local/bin:$PATH"
   ```
3. When compiling, PlatformIO may need to download ESP-IDF toolchains. Behind the proxy this can require patience; rerun the command if transient HTTP 403 errors appear.

# Source code

Use `.yml` files in current dir and contents of ./src/ dir
Ignore `.git`, `.idea`; always exclude them
Go to `.esphome` only to check current upstream implementation; when you need to figure out the impl of a class there is no project source code there, only upstream code

# Workflow

Use `esphome compile ./thebox4.yaml` often to check your intermediate work results.
Use `esphome config ./thebox4.yaml` even more ofthen to check intermediate work results.
Use `esphome run ./thebox4.yaml --device box2-0004.local` to flash a real device; note: logs will become visible 20s after flashing; whole compilation+flashing takes at least 1 minute

Do NOT manually create new components or features, if there are similar ones available on esphome official docs.

