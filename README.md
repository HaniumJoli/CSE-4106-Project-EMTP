# EMTP — Enhanced Mail Transfer Protocol Simulation

## Overview

EMTP (Enhanced Mail Transfer Protocol) is an OMNeT++ simulation project that extends the Simple Mail Transfer Protocol (SMTP) with:

- **End-to-End Encryption**: RSA key-exchange at the application layer
- **Priority-Based Delivery**: HIGH vs LOW priority mail queuing and delivery
- **Application-Layer Filtering**: Security and reliability checks before acceptance

This project demonstrates understanding of SMTP commands and flow while showcasing protocol enhancements at the application level.

## Features

### Core SMTP Implementation
- Complete SMTP command flow: `HELO`, `MAIL FROM`, `RCPT TO`, `DATA`, `QUIT`
- Client-server architecture with proper state management
- Message sequencing and response handling

### Enhanced Features
1. **RSA Encryption**
   - Server generates RSA key-pair on startup
   - Public key shared during HELO handshake
   - Client encrypts message body with server's public key
   - Server decrypts messages using private key

2. **Priority Queuing**
   - Two-tier priority system (HIGH/LOW)
   - Separate queues for each priority level
   - Preferential processing of high-priority mails in constrained mode

3. **Mail Filtering**
   - Subject header validation
   - Empty body detection
   - Priority header validation
   - Encryption verification
   - Configurable filtering rules

## Project Structure

```
EMTP/
├── src/
│   ├── SMTPMessage.msg          # Protocol message definitions
│   ├── EncryptionHelper.h/.cc   # RSA encryption implementation
│   ├── ClientModule.ned/.h/.cc  # SMTP client module
│   ├── ServerModule.ned/.h/.cc  # SMTP server with priority queues
│   └── package.ned              # Package definition
├── simulations/
│   ├── EMTPNetwork.ned          # Network topology
│   ├── omnetpp.ini              # Simulation configurations
│   └── package.ned              # Simulation package definition
├── Makefile                      # Build automation
└── README.md                     # This file
```

## Building the Project

### Prerequisites
- OMNeT++ 6.0 or later
- C++14 compatible compiler
- Standard OMNeT++ libraries

### Build Instructions

1. **Generate Makefiles**:
   ```bash
   make makefiles
   ```

2. **Build the project**:
   ```bash
   make
   ```

3. **Clean build artifacts**:
   ```bash
   make clean
   ```

## Running Simulations

### Available Configurations

The project includes six pre-configured simulation scenarios in `omnetpp.ini`:

#### 1. **Baseline** (Default)
Normal operation with unlimited capacity and no priority discrimination.
```bash
cd simulations
./run -u Cmdenv -c Baseline
```
- 5 clients (3 HIGH, 2 LOW priority)
- Unlimited queue capacity
- Equal treatment of all priorities
- Measures baseline latencies

#### 2. **Constrained**
Bandwidth-limited scenario demonstrating priority-based delivery.
```bash
./run -u Cmdenv -c Constrained
```
- 10 clients (5 HIGH, 5 LOW priority)
- Reduced delivery rate (2s vs 0.5s)
- Smaller queue (20 vs 100)
- HIGH priority mails processed first

#### 3. **NoEncryption**
Tests overhead of encryption by disabling it.
```bash
./run -u Cmdenv -c NoEncryption
```
- Same as Baseline but without encryption
- Measures encryption overhead

#### 4. **StrictFiltering**
Tests filtering rules with intentionally invalid mails.
```bash
./run -u Cmdenv -c StrictFiltering
```
- 8 clients, some with missing subjects
- Strict filtering enabled
- Measures rejection rates

#### 5. **HighLoad**
High-load scenario with many concurrent clients.
```bash
./run -u Cmdenv -c HighLoad
```
- 20 clients (10 HIGH, 10 LOW priority)
- Tests queue behavior under load
- Measures queue saturation effects

#### 6. **SingleClient**
Simple test with one client for debugging.
```bash
./run -u Cmdenv -c SingleClient
```
- Single client
- Useful for protocol flow verification

### Running with GUI

To run simulations in the OMNeT++ IDE or Qtenv:
```bash
cd simulations
./run
```
Then select the desired configuration from the GUI.

## Collected Metrics

### Client Statistics
- **mailsSent**: Total number of mail sending attempts
- **mailsAccepted**: Number of mails accepted by server
- **mailsRejected**: Number of mails rejected by server
- **rtt**: Round-trip time from HELO to QUIT

### Server Statistics
- **mailsReceived**: Total mails received
- **mailsAccepted**: Mails passed filtering and accepted
- **mailsRejected**: Mails rejected by filtering rules
- **mailsDelivered**: Mails successfully delivered
- **highPriorityDelivered**: High-priority mails delivered
- **lowPriorityDelivered**: Low-priority mails delivered
- **totalQueueSize**: Combined queue size over time
- **highPriorityQueue**: High-priority queue size
- **lowPriorityQueue**: Low-priority queue size
- **deliveryLatency**: Overall delivery latency
- **highPriorityLatency**: High-priority delivery latency
- **lowPriorityLatency**: Low-priority delivery latency

## SMTP Protocol Flow

```
Client                                  Server
  |                                       |
  |--- HELO clientName ------------------>|
  |<-- 250 Hello + PublicKey -------------|
  |                                       |
  |--- MAIL FROM:<sender> --------------->|
  |<-- 250 OK ----------------------------|
  |                                       |
  |--- RCPT TO:<recipient> -------------->|
  |<-- 354 Start mail input --------------|
  |                                       |
  |--- DATA (headers + encrypted body) -->|
  |    - PRIORITY: HIGH/LOW               |
  |    - Subject: ...                     |
  |    - Encrypted body                   |
  |<-- 250 OK (or 550 Rejected) ----------|
  |                                       |
  |--- QUIT ----------------------------->|
  |<-- 221 Bye ----------------------------|
```

## Key Implementation Details

### Encryption
- Simplified RSA implementation for simulation purposes
- **NOT cryptographically secure** - for demonstration only
- Uses small primes (251-367 range) for fast computation
- Character-by-character encryption

### Priority Queuing
- **Normal Mode**: Both queues processed fairly
- **Constrained Mode**: HIGH priority queue always processed first
- Queue capacity limits trigger rejection when full

### Filtering Rules
1. Subject header must be present (if `requireSubject = true`)
2. Message body cannot be empty
3. Priority header must be valid (HIGH or LOW)
4. Decryption must succeed (if encrypted)

## Expected Results

### Baseline vs Constrained
- **Baseline**: Similar latencies for HIGH and LOW priority
- **Constrained**: Significantly lower latency for HIGH priority mails

### Encryption Overhead
- Comparing Baseline vs NoEncryption shows encryption processing time
- Should see slight increase in processing time with encryption

### Filtering Impact
- StrictFiltering config shows rejection rate
- Validates that invalid mails are caught before queuing

## Extension Ideas

1. **Multiple Servers**: Add server-to-server mail forwarding
2. **Attachments**: Extend mail format to support file attachments
3. **Authentication**: Add SMTP AUTH command support
4. **Queue Scheduling**: Implement different queue scheduling algorithms
5. **Network Conditions**: Add packet loss, delay variation simulations
6. **TLS/SSL**: Simulate transport-layer encryption handshake

## Troubleshooting

### Build Errors
- Ensure OMNeT++ environment variables are set: `source setenv`
- Regenerate makefiles: `make makefiles`
- Check C++ compiler version (C++14 or later required)

### Runtime Errors
- Check that message files are compiled: `.msg` → `_m.h` and `_m.cc`
- Verify network topology in EMTPNetwork.ned
- Check parameter values in omnetpp.ini

### No Results
- Ensure statistics recording is enabled in omnetpp.ini
- Check sim-time-limit is sufficient
- Verify clients have different start times

## References

- [OMNeT++ Documentation](https://docs.omnetpp.org/)
- [SMTP RFC 5321](https://tools.ietf.org/html/rfc5321)
- [RSA Cryptography](https://en.wikipedia.org/wiki/RSA_(cryptosystem))

## License

LGPL - See project license for details.

**Note**: This is an educational simulation. The encryption implementation is simplified and should NOT be used for actual security purposes.
