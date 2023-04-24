# rs-jerry-driver

An example IQ Streaming client for use with streaming devices, like the MSR4.

## Hardware

- system with SR-IOV support
- ixgbe driver

As of writing this, only Intel® Ethernet Controller X520 and Intel® Ethernet Controller X710 have been tested and used. Though it should work with other NICs aswell provided the system is set up correctly according to the RS Jerry Setup Repo

## Software

The following versions are required to build and run:
| Name  | Version | Purpose | Get It |
| --- | --- | --- | --- |
| C++  | >17  | Compiling | --- |
| CMake  | >3.14  | Building | `sudo apt-get install cmake` |
| gRPC  | 1.44.0  | Configure MSR4 | [Quickstart](https://grpc.io/docs/languages/cpp/quickstart/) |
| jsoncpp  | 1.7.4 | Alternative to configure MSR4 | `sudo apt-get install libjsoncpp-dev` |
| DPDK  | 21.02.0  | Speed | [Download](https://core.dpdk.org/download/) + [Building](https://doc.dpdk.org/guides/linux_gsg/build_dpdk.html) |
| pkg-config  | 0.29.1  | Finding DPDK | `sudo apt-get install pkg-config` |
| gTest (optional)  | 1.10.0  | Running UnitTests | `sudo apt-get install libgtest-dev` |

Transitive requirements will be reported by CMake and should be installed accordingly.

## Unit Tests

The unit tests are using the [GoogleTest Framework](https://github.com/google/googletest).

To enable/disable building the unit tests, switch the option `BUILD_TESTS` to ON/OFF in the [CMakeLists.txt](CMakeLists.txt).


## Preperation
This project requires the system to be set up correctly according to the RS Jerry Setup Repo.

## Build & Install

Before building, adapt the path to the GRPC installation to your local build path in CMakeLists.txt, for example:

    set(LOCAL_GRPC_PATH /home/GnuRadio/local)

Then, create a build directory and start the installation.

    mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make
    sudo make install

This installs the library `libjerryDriver.a` into `/usr/local/lib` as well as the headers into `/usr/local/include/`.

## Usage

After the installation you can `#include <iqClient/iqClient.h>` in your project and use its contents.

### IqClient
The common approach is to instantiate a new empty iqClient and modifying it afterwards to your needs.

_Example Code:_
```c++
std::unique_ptr<IqClient> iqclient = std::make_unique<IqClient>();
```

### Configure MSR4 via gRPC
Using gRPC requires you to first log into the MSR4 with your given credentials.

_Example Code: Login_
```c++
iqClient->SetMSR4Ip("some.device.net");
iqClient->SetMSR4Credentials("user", "password");
iqClient->MSR4Login();
```

Setting single values in the MSR4 is as simple as calling the corresponding function of the `iqClient`.
The return value of Set-Functions is usually an ErrorMessage. See the errors proto file for further information.

_Example Code: gRPC and Errors_
```c++
iqClient->SetDestinationAddress("127.0.0.1");
RsIcpxGrpcService::ErrorMessage err = iqClient->SetSatFrequencyHz(1500000000);
if(err.errorcode() != 0)
      throw InvalidValueError(err.errormessage());
```

Another approach to configure the MSR4 is to specify settings in a `.json` file and loading it. Further details on `.json` can be found in the `configFiles/README.md` as well as an `example.json` and `schema.json`.

_Example Code: JSON_
```c++
iqClient->SetMSR4ByJson("my/awesome/path/to/example.json");
```

### Getting Samples (via DPDK)
The created VFs from the RS Jerry Setup Repo are enumerated in order of creation starting at 0. 
The VF to listen to needs to be specified.
The incoming samples can be divided by a norm. The norm defaults to 1.

_Example Code: Specify DPDK Settings_
```c++
iqClient->SetPortID(0);
iqClient->SetNorm(10);
```

After having everything set up correctly, start DPDK and the MSR4 IQ-streamer.

Note: MSR4 settings can only be changed while the IQ-streamer is not streaming, so make sure to call `iqClient->SetStreamingStatus(false);` beforehand.

_Example Code: Starting_
```c++
iqClient->SetupDpdkSource();
iqClient->SetStreamingStatus(true);
```

Now calling `int GetSamples(int number_of_samples, std::complex<float> *samples)` retrievs a maximum of `number_of_samples` items into `samples`, and returns the actual number of items succesfully stored in `samples`.
```c++
int number_of_samples       = 10;
std::complex<float> *output = (std::complex<float> *) malloc(sizeof(std::complex<float>) * number_of_samples);
int nsamples                = 0;

nsamples = iqClient->GetSamples(number_of_samples, output);
```
`GetSamples(...)` can be and probably should be called in an endless loop to keep the samples coming.


After you are done, you should call
```c++
iqClient->TeardownDpdkSource();
```
to free the memory.

**Please note:**\
Due to how dpdk works, `SetupDpdkSource()` (which calls `rte_eal_init()`) can only be called **once per process, even after `TeardownDpdkSource()`** since calling `rte_eal_cleanup()` internally does not shutdown dpdk "cleanly".
Further information on this: [Issue#263](https://github.com/intel-go/nff-go/issues/263).

In essence:
> [ rte_eal_cleanup() ] was added as it was required to release hugepage memory from secondary processes when shutting down. In the long term, it is intended to provide a "clean" shutdown of all of DPDK resources, however it is not that today. In current code, rte_eal_init() can still only be called once per process.


## Examples
You can find more examples on how to use the iqClient either
- in the unit tests directly or
- in a little more elaborated example using gnuradio as receiver in the rs-jerry-gnuradio repo

## Troubleshooting

**Problem:** The MSR4 sends HRZR packets to my device and the packets are visible in `watch -n 0.5 -d 'ethtool -S <myDevice> | grep -v ": 0"'` but I do not get any samples by calling `GetSamples(...)`.\
**Solution:** This usually means the packets are not send to the VF. When using the Intel® Ethernet Controller X710 make sure the latest compatible [i40e Driver](https://www.intel.com/content/www/us/en/download/18026/intel-network-adapter-driver-for-pcie-40-gigabit-ethernet-network-connections-under-linux.html) is installed for your system.

## Sidenote: HRZR
### Header
<table border="1">
  <tr>
    <th>Byte</th>
    <th scope="col" colspan="8" style="text-align: center">0</th>
    <th scope="col" colspan="8" style="text-align: center">1</th>
    <th scope="col" colspan="8" style="text-align: center">2</th>
    <th scope="col" colspan="8" style="text-align: center">3</th>
  </tr>
  <tr>
    <!-- The following eight cells will appear under the same header -->
    <th>Bit</td>
    <th>0</th>
    <th>1</t>
    <th>2</th>
    <th>3</th>
    <th>4</th>
    <th>5</th>
    <th>6</th>
    <th>7</th>
    <th>0</th>
    <th>1</th>
    <th>2</th>
    <th>3</th>
    <th>4</th>
    <th>5</th>
    <th>6</th>
    <th>7</th>
    <th>0</th>
    <th>1</th>
    <th>2</th>
    <th>3</th>
    <th>4</th>
    <th>5</th>
    <th>6</th>
    <th>7</th>
    <th>0</th>
    <th>1</th>
    <th>2</th>
    <th>3</th>
    <th>4</th>
    <th>5</th>
    <th>6</th>
    <th>7</th>
  </tr>
  <tr>
    <th scope="row" rowspan="3" style="text-align: center">Content</th>
    <td scope="col" colspan="4" style="text-align: center">Control</td>
    <td scope="col" colspan="12" style="text-align: center">Sequence number</td>
    <td scope="col" colspan="16" style="text-align: center">Total packet length (including header)</td>
  </tr>
  <tr>
    <td scope="col" colspan="32" style="text-align: center">Receiver Address</td>
  </tr>
  <tr>
    <td scope="col" colspan="32" style="text-align: center">Data</td>
  </tr>
</table>

### Control
<table border="1">
  <tr>
    <th scope="col" colspan="8">Bit 0</th>
    <th scope="col" colspan="8">Bit 1</th>
    <th scope="col" colspan="8">Bit 2</th>
    <th scope="col" colspan="8">Bit 3</th>
    <th scope="col" colspan="8">Packet type</th>
  </tr>
  <tr>
    <td scope="col" colspan="8">0</td>
    <td scope="col" colspan="8">0</td>
    <td scope="col" colspan="8">x</td>
    <td scope="col" colspan="8">0</td>
    <td scope="col" colspan="8">Data</td>
  </tr>
  <tr>
    <td scope="col" colspan="8">0</td>
    <td scope="col" colspan="8">1</td>
    <td scope="col" colspan="8">x</td>
    <td scope="col" colspan="8">1</td>
    <td scope="col" colspan="8">Metadata</td>
  </tr>
</table>

### Payload
Transmission in little-endian format, i.e. the least significant byte first. A sample consists of I and Q, each with 16 bits per component, i.e. a total of 32 bits per sample. I always comes first, then Q.
<table border="1">
<tr>
    <th>Byte</th>
    <th scope="col" colspan="8" style="text-align: center">0</th>
    <th scope="col" colspan="8" style="text-align: center">1</th>
    <th scope="col" colspan="8" style="text-align: center">2</th>
    <th scope="col" colspan="8" style="text-align: center">3</th>
  </tr>
  <tr>
    <th>Bit</th>
    <th>0</th>
    <th>1</th>
    <th>2</th>
    <th>3</th>
    <th>4</th>
    <th>5</th>
    <th>6</th>
    <th>7</th>
    <th>0</th>
    <th>1</th>
    <th>2</th>
    <th>3</th>
    <th>4</th>
    <th>5</th>
    <th>6</th>
    <th>7</th>
    <th>0</th>
    <th>1</th>
    <th>2</th>
    <th>3</th>
    <th>4</th>
    <th>5</th>
    <th>6</th>
    <th>7</th>
    <th>0</th>
    <th>1</th>
    <th>2</th>
    <th>3</th>
    <th>4</th>
    <th>5</th>
    <th>6</th>
    <th>7</th>
  </tr>
  <tr>
    <th scope="row" rowspan="3" style="text-align: center">Content</th>
    <td scope="col" colspan="16" style="text-align: center">I</td>
    <td scope="col" colspan="16" style="text-align: center">Q</td>
  </tr>
  <tr>
    <td scope="col" colspan="16" style="text-align: center">I</td>
    <td scope="col" colspan="16" style="text-align: center">Q</td>
  </tr>
  <tr>
    <td scope="col" colspan="16" style="text-align: center">...</td>
    <td scope="col" colspan="16" style="text-align: center">...</td>
  </tr>
</table>

### Metadata
Metadata is sent once per second.
<table border="1">
  <tr>
    <th>Byte</th>
    <th scope="col" colspan="8" style="text-align: center">0</th>
    <th scope="col" colspan="8" style="text-align: center">1</th>
    <th scope="col" colspan="8" style="text-align: center">2</th>
    <th scope="col" colspan="8" style="text-align: center">3</th>
  </tr>
  <tr>
    <th>Bit</th>
    <th>0</th>
    <th>1</th>
    <th>2</th>
    <th>3</th>
    <th>4</th>
    <th>5</th>
    <th>6</th>
    <th>7</th>
    <th>0</th>
    <th>1</th>
    <th>2</th>
    <th>3</th>
    <th>4</th>
    <th>5</th>
    <th>6</th>
    <th>7</th>
    <th>0</th>
    <th>1</th>
    <th>2</th>
    <th>3</th>
    <th>4</th>
    <th>5</th>
    <th>6</th>
    <th>7</th>
    <th>0</th>
    <th>1</th>
    <th>2</th>
    <th>3</th>
    <th>4</th>
    <th>5</th>
    <th>6</th>
    <th>7</th>
  </tr>
  <tr>
    <th scope="row" rowspan="10" style="text-align: center">Content</th>
    <td scope="col" colspan="4" style="text-align: center">Version</td>
    <td scope="col" colspan="4" style="text-align: center">Time sync source</td>
    <td scope="col" colspan="2" style="text-align: center">Clock sync source</td>
    <td scope="col" colspan="22" style="text-align: center">Zero Padding</td>
  </tr>
  <tr>
    <td scope="col" colspan="64" style="text-align: center">Zero Padding</td>
  </tr>
  <tr>
    <td scope="col" colspan="64" style="text-align: center">Unix timestamp in nanosecond-precision (MSBs) *</td>
  </tr>
  <tr>
    <td scope="col" colspan="64" style="text-align: center">Unix timestamp in nanosecond-precision (LSBs) *</td>
  </tr>
  <tr>
    <td scope="col" colspan="64" style="text-align: center">Sample frequency</td>
  </tr>
  <tr>
    <td scope="col" colspan="64" style="text-align: center">Center frequency (max 4.2 GHz)</td>
  </tr>
  <tr>
    <td scope="col" colspan="64" style="text-align: center">Preamp gain float (IEEE 754) **</td>
  </tr>
  <tr>
    <td scope="col" colspan="64" style="text-align: center">ISO 6709 latitude*** in degree float (IEEE 754)</td>
  </tr>
  <tr>
    <td scope="col" colspan="64" style="text-align: center">ISO 6709 longitude*** in degree float (IEEE 754)</td>
  </tr>
  <tr>
    <td scope="col" colspan="64" style="text-align: center">ISO 6709 elevation*** in meter float (IEEE 754)</td>
  </tr>
</table>

\* unix timestap in nanoseconds since 01.01.1970, 0:00  
** defines the total 200MHz channel power correction value in dB  
*** the default value or no GPS fix is available: -256.0

### Version
Current: 0000

### Time Sync Source
How we get the timestamps:

If GPS or Network is selected but not synchronized (no GPS fix, no connection to NTP Master), internal RTC Time should be transmitted as timestamp and bit 3 should be zero.
<table border="1">
  <tr>
    <th scope="col" colspan="8">Bit 0</th>
    <th scope="col" colspan="8">Bit 1</th>
    <th scope="col" colspan="8">Bit 2</th>
    <th scope="col" colspan="8">Bit 3 - SYNCED</th>
    <th scope="col" colspan="8">Packet type</th>
  </tr>
  <tr>
    <td scope="col" colspan="8">0</td>
    <td scope="col" colspan="8">0</td>
    <td scope="col" colspan="8">x</td>
    <td scope="col" colspan="8">0</td>
    <td scope="col" colspan="8">No time sync</td>
  </tr>
  <tr>
    <td scope="col" colspan="8">1</td>
    <td scope="col" colspan="8">0</td>
    <td scope="col" colspan="8">x</td>
    <td scope="col" colspan="8">0/1</td>
    <td scope="col" colspan="8">GPS</td>
  </tr>
  <tr>
    <td scope="col" colspan="8">0</td>
    <td scope="col" colspan="8">1</td>
    <td scope="col" colspan="8">x</td>
    <td scope="col" colspan="8">0/1</td>
    <td scope="col" colspan="8">Network</td>
  </tr>
  <tr>
    <td scope="col" colspan="8">1</td>
    <td scope="col" colspan="8">1</td>
    <td scope="col" colspan="8">x</td>
    <td scope="col" colspan="8">x</td>
    <td scope="col" colspan="8">Unknown</td>
  </tr> 
</table>

### Clock Sync Sources
How we synchronize:
<table border="1">
  <tr>
    <th scope="col" colspan="8">Bit 0</th>
    <th scope="col" colspan="8">Bit 1</th>
    <th scope="col" colspan="8">Packet type</th>
  </tr>
  <tr>
    <td scope="col" colspan="8">0</td>
    <td scope="col" colspan="8">0</td>
    <td scope="col" colspan="8">Internal clock</td>
  </tr>
  <tr>
    <td scope="col" colspan="8">1</td>
    <td scope="col" colspan="8">0</td>
    <td scope="col" colspan="8">External clock</td>
  </tr>
  <tr>
    <td scope="col" colspan="8">0</td>
    <td scope="col" colspan="8">1</td>
    <td scope="col" colspan="8">Unknown</td>
  </tr>
  <tr>
    <td scope="col" colspan="8">1</td>
    <td scope="col" colspan="8">1</td>
    <td scope="col" colspan="8">Unknown</td>
  </tr>
</table>
