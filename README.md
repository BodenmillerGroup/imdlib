# imdlib

IMD file parser library

## Features
* Full data and metadata access (read-only)
* Compressed row storage (CSR) of in-memory data
* Python 3 bindings for interactive data access

## Prerequisites

* **pugixml** <br />
http://www.pugixml.org <br />
Tested with pugixml 1.8.1

* **pybind11** (optional, for Python support) <br />
https://github.com/pybind/pybind11 <br />
Configured as a Git submodule, no additional setup required

## Installation

```bash
git clone --recursive https://github.com/BodenmillerGroup/imdlib.git
mkdir imdlib/cmake-build-release
cd imdlib/cmake-build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
sudo ldconfig
```

## Usage

This is a C++11 example of the full functionality of the library:

```C++
#include <iostream>
#include <imdlib/IMDFile.h>

int main(int argc, char *argv[]) {
    imd::IMDFile imdFile("/path/to/file");

    std::string metadata = imdFile.readMetadata();
    std::cout << metadata << std::endl;

    std::size_t pushIndex = 123;
    std::size_t markerIndex = 12;
    std::string markerName = "191Ir";
    const auto data = imdFile.readData();
    const auto &pulses = data.getPulses();
    const auto &intensities = data.getIntensities();
    uint16_t pulseValue = pulses(pushIndex, markerIndex);
    uint16_t intensityValue = intensities(pushIndex, markerName);
    std::cout << pulseValue << ", " << intensityValue << std::endl;

    return 0;
}
```

For interactive/scripting usage, this is a Python 3 example:

```python3
import imdpy

imd = imdpy.IMDFile('/path/to/file')

metadata = imd.read_metadata()
print(metadata)

push_index = 123
marker_index = 12
marker_name = "191Ir"
data = imd.read_data()
pulseValue = data.pulses[push_index, marker_index]
intensityValue = data.pulses[push_index, marker_name]
print(pulseValue, intensityValue)
```

At any time, a brief documentation is available using Python's built-in help functionality.

## License

Copyright 2019 Bodenmiller Lab

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this project except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
