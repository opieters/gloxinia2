# gloxinia2

This is the second version of [Gloxinia](https://github.com/opieters/gloxinia), a research-focussed measurement platform that is tailored for deployment in greenhouses. Its goals are simple interconnectivity, flexbibility and cost-effectiveness.

## Architecture

The general architecture from the inital version remained, but some boards are not separate noted in place of add-on components. At it's core, the system comprises of a set of communicating nodes. Three possible options are:
- dicio: a node with serial interconnectivity, such that the system can interface with a computer. This node also features storage (16GB). This node can also send commands to other nodes in case of errors or crashes.
- sylvatica: a node with eight 16-bit ADCs
- planalta: a node with four LIAs (advanced analogue sensors)

All nodes communicate over a CAN bus and dicio nodes also have UART connection for communication computers. Plugging in a USB-C cable in the right port should suffice to connect to the system. Make sure to only plug cables to matching interfaces, since the hardware can be damaged otherwise. The interfaces are detailed in the PCB design and the interface section below. 

Note that the CAN bus requires termination. No automatic termination is implemented, so the use needs to manually terminate the CAN lines!

Each board interface also has an I2C port (not that some of the ports share the same physical interface), and analogue ports and a One Wire port. One Wire is currently not supported in the software. 

## Interfacing

Each node consists of a stack of boards. The base board with the analogue and digital processing, a botton connectivity board to interconnect different nodes and an optional top interfacing board (only for sylvatica).

### Dicio

Base board: [IF1] [IF2] [IF3] [IF4]
IF board: [Serial/USB] [] [CAN] [CAN]

### Sylvatica

Top board: [IF4] [IF5] [IF6] [IF7]
Base board: [IF1] [IF2] [IF3] [IF4]
IF board: [] [] [CAN] [CAN]

### Planalta

Base board: [IF1] [IF2] [IF3] [IF4]
IF board: [] [] [CAN] [CAN]

Note that IF1-4 are not feature-identical, IF1 supports a faster LIA (when others disabled) than IF2, 3 and 4 resp. See software implementation for details. These interfaces to no feature a 1W port.

## Software

The software consists of several parts: the embedded C code, the C++ GUI and the Python readout script. 

The embedded code is in the `firmware` folder. A separate MPLAB X project is created per hardware board (dicio, planalta and sylvatica). The code is written in C and compiled with the XC16 compiler. The code is written in a modular fashion, such that it is easy to add new features. The code is documented in the source files. Common files are included in the `lib2` folder. 

A bootloader project was also created to update the devices over CAN and UART, but this was not tested and should not yet be used in production. The UART version was slightly tested and first results were promising.

The GUI code is written in C++ using the Qt framework. A recent version of Qt 6 is necessary to build the project. Qt Designer files are included with in the `gui_qt` folder to get started with this project. 

## Enclousure

A 3D printable enclosure design is included in the `enclosure` folder, along with the STL files. The design is made in OpenSCAD.

A radiation shield was also designed for the environemental sensors, which is included in the 