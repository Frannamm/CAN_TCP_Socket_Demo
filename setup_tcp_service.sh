#!/bin/bash
cd /home/dali/Documents/CAN_TCP_Socket_Demo

sudo systemctl stop TCP_demo
make clean
make
ls -l server

sudo cp TCP_demo.service /etc/systemd/system/TCP_demo.service

sudo systemctl daemon-reload
sudo systemctl enable TCP_demo
sudo systemctl start TCP_demo
sudo systemctl status TCP_demo --no-pager