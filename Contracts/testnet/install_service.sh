#!/bin/bash -x

# SYSTEMD_USER_DIR=$HOME/.config/systemd/user
SYSTEMD_USER_DIR=/usr/lib/systemd/user

sudo mkdir -p $SYSTEMD_USER_DIR
sudo cp geth.service $SYSTEMD_USER_DIR

sudo mkdir -p /opt/tc
sudo cp gethtc /opt/tc
