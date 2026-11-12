# UART Echo Driver

## Hardware Platform
* **MCU:** STM32C031C6 (ARM Cortex-M0+)
* **Board:** Nucleo-C031C6

## Overview
A bare-metal, polling-based UART driver designed to echo single ASCII characters back to a host terminal application.

## Description
This UART driver is written for the STM32C031C6 MCU. It configures the necessary GPIO alternate function pins for UART TX and RX, sets up the baud rate, and utilizes the hardware USART Read Data Register (`USART_RDR`) to poll for incoming character input. Once a character is received, it immediately writes the same byte to the Transmit Data Register (`USART_TDR`) for an instant echo.

## Limitations & Weaknesses
Because this driver is built purely on **polling** and lacks advanced hardware abstractions, it has a few inherent limitations:
1. **CPU Blocking:** The CPU is trapped inside a `while` loop waiting for flags (`TXE` / `RXNE`). It cannot execute other background application logic or enter low-power sleep modes while waiting.
2. **No Data Buffering:** There is no queue or ring buffer. If data arrives faster than `receiveUART()` can read it, or if the CPU is busy elsewhere, new bytes will overwrite unread data, causing an **Overrun Error (ORE)** and data loss.
3. **Power Inefficient:** Continuous polling keeps the CPU fully active, making this approach unsuitable for battery-powered or energy-sensitive embedded designs.

## Roadmap
1. Implement ring buffers for asynchronous data handling
2. Transition from polling to Interrupt-Driven USART operation
3. Implement Direct Memory Access (DMA) for continuous communication
