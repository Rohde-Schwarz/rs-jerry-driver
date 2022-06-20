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

## Unit Tests

The unit tests are using the [GoogleTest Framework](https://github.com/google/googletest).

To enable/disable building the unit tests, switch the option `BUILD_TESTS` to ON/OFF in the [CMakeLists.txt](CMakeLists.txt).

## Preperation
This project requires the system to be set up correctly according to the RS Jerry Setup Repo.

## Build & Install

```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
```
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
