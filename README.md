# AM-Robotics Hardware-Layer Code

An EtherCAT-Based Real-Time Control System Architecture for Robotic Applications.

## Goals

The goal of this project is to provide a framework for the low-level control of robots via an EtherCAT bus. It defines the concept of bus variables, which is a general wrapper for the representation of variables on bus slaves. Using this concept of bus variables, an interface to the commercial EtherCAT master stack from acontis technologies (http://www.acontis.com) is provided. Several device abstractions for Beckhoff EtherCAT devices and the Gold series from Elmo Motion Control (http://elmomc.com) are provided within the framework. This framework may be used (together with the acontis stack or a different EtherCAT master stack) to control the joints of robots with a high number of DoFs.

## Publication

The performance of the bus variable concept is evaluated and discussed in our following publication:

F. Sygulla et al., "An EtherCAT-Based Real-Time Control System Architecture for Humanoid Robots," 2018 IEEE 14th International Conference on Automation Science and Engineering (CASE), Munich, 2018, pp. 483-490, doi: 10.1109/COASE.2018.8560532.

## Folder Structure

- *common*: Templates used for the definition of general BusSlave, BusMaster objects as well as the Bus Variables.
- *devices*: Includes Device Abstractions for some common slave classes (EL1012, EL3104, EL2004 on a EK1100 as well as Elmo Gold).
- *iface*: Headers with some definitions
- *utils*: Some utility classes. Note that this code is optimized for QNX Neutrino 6.6 and may not run on other platforms.

The bus variable concept allows full support of SDO/PDO communication with slaves.

## License

The project is published under the terms of the
[CRAPL License](http://matt.might.net/articles/crapl/).
