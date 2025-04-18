# Real-Time Crypto Analysis

**Author:** Ioannis Michalainas

This project implements a real-time cryptocurrency analysis system that connects to OKX WebSocket API to process transaction data for major cryptocurrency pairs:

<table>
  <tr>
    <td>BTC‑USDT</td><td>ADA‑USDT</td><td>ETH‑USDT</td><td>DOGE‑USDT</td>
  </tr>
  <tr>
    <td>XRP‑USDT</td><td>SOL‑USDT</td><td>LTC‑USDT</td><td>BNB‑USDT</td>
  </tr>
</table>

## Tools

- **C**
- **gcc**
- **make**
- **pthreads**
- **libwebsockets**
- **Python**

## Features

- Asynchronously receives and logs transaction data for 8 cryptocurrency pairs
- Calculates 15-minute moving averages for prices and volumes every minute
- Computes Pearson correlation coefficients to identify patterns between cryptocurrencies
- Runs on Raspberry Pi using C with libwebsockets for efficient, long-term operation

The implementation focuses on minimizing processing delays and maintaining connection and accuracy over extended runtime periods, making it suitable for continuous market analysis and potential trading strategy development.

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

Verify CA Certificates Exist:
```bash
ls /etc/ssl/certs/ca-certificates.crt
```
If missing, install certificates:
```bash
sudo pacman -S ca-certificates
```
*(Use the package manager specific to your host system.)*

## Appendix

For more in depth information and results refer to [`docs/report.pdf`](docs/report.pdf).