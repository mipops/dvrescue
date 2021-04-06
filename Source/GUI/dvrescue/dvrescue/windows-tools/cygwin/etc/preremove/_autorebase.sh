#!/bin/dash
export PATH=/bin
b=/var/cache/rebase
echo -n "_autorebase cache directory ${b} removal... "
rm -rf "${b}" && echo "succeeded!" || echo "failed!"
