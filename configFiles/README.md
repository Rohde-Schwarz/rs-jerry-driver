# JSON

A MSR4 configuration consists of 4 sections:

## LoginSettings

The `LoginSettings` are used to log into the designated MSR4.

`Ip` as `string` is the address of the MSR4 you want to connect to.\
`User` as `string` is the username to log into the MSR4.\
`Password` as `string` is the password to log into the MSR4.\

## RxChannelSettings

The `RxChannelSettings` is used to know which channel to edit.

`RxChannel` as `int` can only be 1-4.

## StreamingSettings

The `StreamingSettings` are used to set the RF settings in the MSR4.\
The RF settings configure the RF input signal that provides the I/Q data.

`SatFrequency` as `int` measured in `[Hz]`:\
Center frequency of the carrier at the satellite.

`DownFrequency` as `int` measured in `[Hz]`:\
Conversion frequency of the downconverter. Required to calculate the L-band frequency in the receiver from the carrier frequency at the satellite.

`BandwidthByAnalysisBandwidth` as `int` measured in `[Hz]`:\
Displayed frequency range of the RF input channel, centered around the "L-Band frequency".\
The maximum bandwidth is the full span of the channel, which is 200 MHz.

## ChannelSettings

The `ChannelSettings` are used to set the UDP/IP settings in the MSR4.\
The UDP/IP settings determine the data receiver. Note that the connection settings to the receivers are configured in the "Network settings".

`Port` as `int`:\
Port on the receiver the data is streamed to.

`DestinationAddress` as `string`:\
The address on the receiver the data is streamed to, e.g. an IP address.

`Protocol` as `string` either `HRZR` or `AMMOS`:\
The protocol defines the communication format used to stream the data from the R&S MSR4 to the connected device. Currently, only HRZR protocol is supported for I/Q data streaming. For more information on the protocol, see the application note.