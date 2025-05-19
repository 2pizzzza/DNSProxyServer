# DNS Proxy Server

This is a DNS proxy server written in C that forwards DNS requests to an upstream server and filters requests based on a blacklist. Blacklisted domains receive a configurable response (`REFUSED`, `NXDOMAIN`, or a custom IP address). The server uses the UDP protocol and reads its configuration from a YAML file.

## Features
- Forwards DNS requests to an upstream DNS server (e.g., `8.8.8.8`).
- Filters requests for blacklisted domains, responding with `REFUSED`, `NXDOMAIN`, or a custom IP (e.g., `0.0.0.0`).
- Supports configuration via a YAML file (`config.yml`).
- Logs client requests, including client IP, requested domain, timestamp, and response type, to both console and file (`dns_proxy.log`).
- Case-insensitive blacklist matching.
- Minimal DNS parser supporting single-question A-record queries.

## Project Structure
```
.
├── CMakeLists.txt          # CMake configuration
├── config.yml             # Sample configuration file
├── dns_proxy.log          # Log file for client requests
├── Makefile               # Makefile for direct compilation
├── README.md              # Project documentation
└── src                    # Source code
    ├── blacklist          # Blacklist functionality
    │   ├── blacklist.c
    │   └── blacklist.h
    ├── config             # Configuration handling
    │   ├── config.c
    │   └── config.h
    ├── core               # Core DNS functionality
    │   ├── dns.c
    │   └── dns.h
    ├── log                # Logging functionality
    │   ├── log.c
    │   └── log.h
    └── main.c             # Main program entry
```

## Prerequisites
- **Compiler**: GCC or compatible C compiler.
- **Library**: `libyaml` for parsing YAML configuration files.
- **Operating System**: Linux (tested on Ubuntu/Debian).
- **Tools**: `make`, `cmake`, `dig` (for testing), `tcpdump` (optional for traffic analysis).

Install dependencies on Debian/Ubuntu:
```bash
sudo apt-get install build-essential cmake libyaml-dev
```

## Building the Project

### Using CMake
1. Create a build directory:
   ```bash
   mkdir build && cd build
   ```
2. Run CMake to generate build files:
   ```bash
   cmake ..
   ```
3. Build the project:
   ```bash
   make
   ```
4. The executable `DNSProxyServer` will be created in the `build` directory.

### Using Makefile
Build directly using the provided `Makefile`:
```bash
make
```
This will produce the executable `DNSProxyServer` in the root directory.

To clean up:
```bash
make clean
```

## Configuration
The server reads its configuration from `config.yml`. Below is the default configuration:

```yaml
upstream: 8.8.8.8
port: 53
blocked_response: REFUSED # Options: REFUSED, NXDOMAIN, or an IP address (e.g., 0.0.0.0)
blacklist:
  - google.com
  - example.com
  - xxx.com
  - amit.com
  - hooli.net
```

- `upstream`: IP address of the upstream DNS server (e.g., `8.8.8.8` for Google DNS).
- `port`: Port to listen on (default: 53, requires root privileges).
- `blocked_response`: Response for blacklisted domains (`REFUSED`, `NXDOMAIN`, or an IP address).
- `blacklist`: List of domains to block.

To use a different configuration file, specify its path when running the server.

## Running the Server
Run the server with root privileges (required for binding to port 53):
```bash
sudo ./build/DNSProxyServer config.yml
# or, if built with Makefile:
sudo ./DNSProxyServer config.yml
```

The server will:
- Load the configuration from `config.yml`.
- Start listening on the specified port (default: 53).
- Log requests to `dns_proxy.log` and the console.

Example output:
```
Loaded blacklist[0]: google.com
Loaded blacklist[1]: example.com
Loaded blacklist[2]: xxx.com
Loaded blacklist[3]: amit.com
Loaded blacklist[4]: hooli.net
DNS proxy running on port 53, upstream 8.8.8.8
```

## Testing
The server was tested using multiple tools to verify functionality for blacklisted and non-blacklisted domains, as well as different response types.

### Test Tools
1. **`dig`**:
   - Test a blacklisted domain:
     ```bash
     dig @127.0.0.1 google.com
     ```
     Expected output (with `blocked_response: REFUSED`):
     ```
     ;; ->>HEADER<<- opcode: QUERY, status: REFUSED, id: <id>
     ;; QUESTION SECTION:
     ;google.com.                    IN      A
     ```
     With `blocked_response: 0.0.0.0`:
     ```
     ;; ANSWER SECTION:
     google.com.             3600    IN      A       0.0.0.0
     ```
   - Test a non-blacklisted domain:
     ```bash
     dig @127.0.0.1 kernel.org
     ```
     Expected output:
     ```
     ;; ANSWER SECTION:
     kernel.org.             <ttl>   IN      A       139.178.84.217
     ```

2. **`nslookup`**:
   ```bash
   nslookup google.com 127.0.0.1
   nslookup kernel.org 127.0.0.1
   ```

3. **`host`**:
   ```bash
   host google.com 127.0.0.1
   host kernel.org 127.0.0.1
   ```

4. **`tcpdump`** (for network traffic):
   ```bash
   sudo tcpdump -i lo udp port 53
   ```
   Verifies that blacklisted domains do not generate upstream traffic, while non-blacklisted domains do.

5. **`dnsperf`** (for load testing):
   Create a `queries.txt` file:
   ```
   google.com A
   kernel.org A
   example.com A
   github.com A
   ```
   Run:
   ```bash
   dnsperf -s 127.0.0.1 -p 53 -d queries.txt -n 100
   ```

### Test Scenarios
- **Blacklist**: Verified that all blacklisted domains (`google.com`, `example.com`, etc.) return the configured response (`REFUSED`, `NXDOMAIN`, or IP).
- **Non-blacklisted domains**: Confirmed that requests are forwarded to the upstream server (`8.8.8.8`) and return valid IP addresses.
- **Response types**: Tested all response types:
  - `REFUSED`: Returns `status: REFUSED` with no answer section.
  - `NXDOMAIN`: Returns `status: NXDOMAIN` with no answer section.
  - IP (e.g., `0.0.0.0`): Returns `status: NOERROR` with the specified IP.
- **Stability**: Sent 1000+ requests using `dnsperf` to ensure no crashes or memory leaks.
- **Logging**: Checked `dns_proxy.log` for correct logging of client IP, domain, timestamp, and response type.

## Usage
1. Configure `config.yml` as needed (e.g., change `blocked_response` or add domains to `blacklist`).
2. Build and run the server (see above).
3. Send DNS queries using `dig`, `nslookup`, or configure a client to use `127.0.0.1:53` as the DNS server.
4. Monitor logs in `dns_proxy.log` or the console for request details.

Example log entry:
```
[2025-05-19 23:02:45] Client: 127.0.0.1, Domain: google.com, Response: REFUSED
[2025-05-19 23:02:50] Client: 127.0.0.1, Domain: kernel.org, Response: Forwarded to upstream
```

## Limitations
- Supports only UDP (no TCP).
- Handles single-question A-record queries (other types, like AAAA, may not work).
- No response caching.
- No multi-threading; processes requests sequentially.
- Limited error handling for malformed packets.

## Additional Features
- Case-insensitive blacklist matching.
- Detailed logging of all requests to both console and file.

## License
This project uses `libyaml` (MIT License). Ensure compliance with its terms. The rest of the code is provided under the MIT License (see `LICENSE` file, if added).
