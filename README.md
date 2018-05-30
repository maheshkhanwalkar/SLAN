# SLAN
Simulated Ethernet-powered LAN

## Introduction
To put it simply, SLAN is a simulator. It allows for the creation of LANs that driven by an underlying (simulated)
Ethernet connection.

There are two main components of SLAN:
   * Ethernet backbone
   * Ethernet devices

Both of these components are simulated using Unix daemons: there are no *virtual* network devices and there is no
traffic routed through the kernel.

This makes SLAN relatively simple and also quite flexible. Having a kernel interface complicates custom routing and can
possibly add the problem of dealing with the kernel's filtering and firewall policies.

## Setup
TODO

## Caveats
SLAN is modelled to be as close to actual Ethernet hardware as possible. However, SLAN is **software**, so hardware
Ethernet policies, like CSMA/CD are not implemented.

In addition, it is quite possible that SLAN diverges from the Ethernet specification for simplicity reasons. These
divergences will be noted in detail, as best as possible.
