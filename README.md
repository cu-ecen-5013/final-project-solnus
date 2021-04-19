# IoT Addressable LED Controller
This repository contains final project submission for ECEN 5713 AESD by Jiabin Lin and Steve Rizor

# Building device service outside buildroot
Building the applications outside buildroot requires manual installation of some tools/libraries. Below is a list of some that are needed:

1. Install build tools

```sudo apt-get install install cmake build-essential scons git libcurl4-openssl-dev libssl-dev uuid-dev libconfig-dev```

2. Install Azure IoT SDK for C
```
// See https://github.com/Azure/azure-iot-sdk-c/blob/master/doc/ubuntu_apt-get_sample_setup.md
sudo apt-get install -y software-properties-common
sudo add-apt-repository ppa:aziotsdklinux/ppa-azureiot
sudo apt-get update
sudo apt-get install -y azure-iot-sdk-c-dev
```

# Running webapp
1. Testing webapp locally

```cd webapp && node start```
```[open localhost:3000 in browser]```

```[Deployed Web App](https://ecen-ledcontrol.herokuapp.com/)```
# Repository Overview
[Project Overview](https://github.com/cu-ecen-5013/final-project-JiabinLin12/wiki/Project-Overview)

[Project Schedule](https://github.com/cu-ecen-5013/final-project-JiabinLin12/wiki/Final-Project-Assignment-Schedule-Page)
