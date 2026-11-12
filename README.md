# UART Driver Examples

This repository is a growing collection of UART examples for the NUCLEO-C031C6 development board, using the STM32C031C6 microcontroller.

The project is intended to increase in complexity over time, beginning with simple polling-based UART communication and progressing toward interrupt-driven implementations.

## Projects

### UART_Echo

A basic polling-based UART example. Received data is read and transmitted
back using blocking UART operations.

### UART_EchoString

Received data is read and transmitted back using blocking UART operations. When user presses 'Enter' the entire string is printed out with additional implementation of Overrun flag to simulate missed characters. 

## Hardware

- Development board: NUCLEO-C031C6
- Microcontroller: STM32C031C6
- Development environment: STM32CubeIDE

## Project Goals

- Demonstrate UART communication fundamentals
- Compare polling and interrupt-driven implementations
- Provide progressively more advanced STM32 UART examples
- Serve as a foundation for future UART features and experiments
