# AM-Robotics Hardware-Layer Code

An EtherCAT-Based Real-Time Control System Architecture for Robotic Applications.

## Goals

The goal of this project is to provide a framework for the low-level control of robots via an EtherCAT bus. It defines the concept of bus variables, which is a general wrapper for the representation of variables on bus slaves. Using this concept of bus variables, an interface to the commercial EtherCAT master stack from acontis technologies (http://www.acontis.com) is provided. Several device abstractions for Beckhoff EtherCAT devices and the Gold series from Elmo Motion Control (http://elmomc.com) are provided within the framework. This framework may be used (together with the acontis stack or a different EtherCAT master stack) to control the joints of robots with a high number of DoFs.

## Background

The code shown here is presented in an international publication, which is currently under review. In this publication, we show the high performance of our implementation and the general concept of bus variables. Furthermore, we show how this hardware layer can be used for extended joint-control concepts.

Once published, we'll provide a link to the publication in this Readme.

## Folder Structure

- *common*: Templates used for the definition of general BusSlave, BusMaster objects as well as the Bus Variables.
- *devices*: Includes Device Abstractions for some common slave classes (EL1012, EL3104, EL2004 on a EK1100 as well as Elmo Gold).
- *iface*: Headers with some definitions
- *utils*: Some utility classes. Note that this code is optimized for QNX Neutrino 6.6 and may not run on other platforms.

The bus variable concept allows full support of SDO/PDO communication with slaves.

## License

The project is published under the terms of the
[CRAPL License](http://matt.might.net/articles/crapl/).