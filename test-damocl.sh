#!/bin/bash

sudo rm -rf /etc/OpenCL/vendors/*.icd
sudo cp /etc/OpenCL/damocl.64.icd  /etc/OpenCL/vendors
