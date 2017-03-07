#!/bin/bash
source env.sh

read -p "Create new accounts? [y/N] " yn
case $yn in
	[Yy]*) $GETH --DATADIR $DATADIR account new;; 
	*    ) echo "No account created";;
esac
