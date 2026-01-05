#!/bin/bash

sudo rm -rf /etc/OpenCL/vendors/*.icd
sudo cp /etc/OpenCL/pocl.icd  /etc/OpenCL/vendors
