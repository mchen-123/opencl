#!/bin/bash

sudo rm -rf /etc/OpenCL/vendors/*.icd
sudo cp /etc/OpenCL/thivecl.64.icd  /etc/OpenCL/vendors
