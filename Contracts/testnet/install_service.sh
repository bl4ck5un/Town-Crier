#!/bin/bash

SYSTEMD_USER_DIR=$HOME/.config/systemd/user

mkdir -p $SYSTEMD_USER_DIR
cp geth.service $SYSTEMD_USER_DIR
