# Prerequisites 

1. Install ESPHome into the container user environment (pinning to the project-tested release):
   ```bash
   pip install --user esphome==2025.8.1
   ```
2. Ensure `~/.local/bin` is on your `PATH` for the session (or call the binaries explicitly):
   ```bash
   export PATH="$HOME/.local/bin:$PATH"
   ```
3. When compiling, PlatformIO may need to download ESP-IDF toolchains. Behind the proxy this can require patience; rerun the command if transient HTTP 403 errors appear.

#Workflo

Use `esphome compile ./thebox.yaml` often to check your intermediate work results.
Use `esphome config ./thebox.yaml` even more ofthen to check intermediate work results.

Do NOT spawn new components or features, if there are simillar ones available on esphome official docs. 
