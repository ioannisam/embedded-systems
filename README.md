# Real-Time Crypto Analysis

**Author:** Ioannis Michalainas

This project aims to implement a real-time cryptocurrency analysis system that connects to OKX WebSocket API to process transaction data for major cryptocurrency pairs (BTC-USDT, ETH-USDT, etc.).

## Tools

- **C Programming Language**
- **gcc**
- **Make**
- **arm-linux-gnueabihf-gcc**
- **POSIX Threads (pthreads)**
- **libwebsockets**
- **OKX WebSocket API**

## Features

- Asynchronously receives and logs transaction data for 8 cryptocurrency pairs
- Calculates 15-minute moving averages for prices and volumes every minute
- Computes Pearson correlation coefficients to identify patterns between cryptocurrencies
- Runs on Raspberry Pi using C with libwebsockets for efficient, long-term operation

The implementation focuses on minimizing processing delays and maintaining accuracy over extended runtime periods, making it suitable for continuous market analysis and potential trading strategy development.

## Building

To build use the included `makefile`.
- For standard architecture:

    ```bash
    make
    ```
- For Raspberry Pi:

    ```bash
    make rpi
    ```