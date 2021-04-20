if [ -f /etc/nanorc ] && cmp -s /etc/defaults/etc/nanorc /etc/nanorc
then
    rm /etc/nanorc
fi

