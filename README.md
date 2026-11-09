# Colosseum vs. ns-3: Wi-Fi Ad Hoc Link Measurement and Reconciliation

A two-node IEEE 802.11 ad hoc link measured on real software-defined radios through the Colosseum hardware-in-the-loop wireless emulator, then compared against an equivalent ns-3.48 simulation. Course project for EECE5155 (Wireless Sensor Networks & IoT), Northeastern University, Fall 2026.

**Authors:** Shashank Meghashyam, Riya Rawal

## What we did

- **RF emulator verification:** Sent a tone between two USRP X310 radio nodes through Colosseum RF scenario 1009 ("All Paths 0 dB"). The tone was received at 1010 MHz and 990 MHz, and was absent at 900 MHz, outside the scenario's 960–1040 MHz passband.
- **Wi-Fi link bring-up:** Ran a GNU Radio IEEE 802.11a/g/p transceiver on both nodes and used tcpdump to identify the control traffic exchanged at startup (IPv6 MLDv2 reports, Duplicate Address Detection, router solicitations, and ARP).
- **Measurements:** Measured round-trip time with ping (30 packets per direction) and UDP throughput with iperf3 at a 5 Mbps offered load.
- **ns-3 counterpart:** Ran a two-node 802.11g ad hoc model with a single 5 Mbps UDP flow (1400-byte payload), instrumented with FlowMonitor.
- **Reconciliation:** Audited an AI-generated explanation of the ns-3 vs. Colosseum gap against the scenario's documented channel specification.

## Key results

| Metric | ns-3.48 simulation | Colosseum (real SDR) |
|---|---|---|
| Offered UDP rate | 5 Mbps | 5 Mbps |
| Delivered throughput | 5.10 Mbps | ≈ 2.11 Mbps |
| Latency | 0.32 ms mean one-way (FlowMonitor) | ≈ 15.0 ms ping RTT (≈ 7.5 ms one-way, assuming a symmetric link) |
| Loss | 2 of 4,017 packets | 0 of 30 pings in each direction |

The two latency figures are different metrics (one-way UDP flow delay vs. ICMP round-trip time), and the iperf3 run ended without a final summary, so Colosseum throughput is the mean of the server's per-interval reports.

**Main finding:** Scenario 1009 applies a constant 0 dB channel, so path loss and fading cannot explain the gap. The difference comes mainly from the real SDR processing, buffering, and hardware-path overhead that a discrete-event simulator does not model by default.

## Files

- `lab2-wifi-adhoc.cc` – ns-3.48 simulation (base program provided by the course)
- `Colosseum-ns3-Report.pdf` – full report with terminal captures, spectrum plots, and analysis

## How to run the ns-3 model

Copy `lab2-wifi-adhoc.cc` into the `scratch/` folder of an ns-3.48 tree, then from the ns-3 root:

```
./ns3 run scratch/lab2-wifi-adhoc
./ns3 run 'scratch/lab2-wifi-adhoc --distance=20'
```

## Tools

Colosseum wireless emulator, USRP X310, GNU Radio, UHD, ns-3.48, C++, iperf3, ping, tcpdump, Linux
