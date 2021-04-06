if [ -f /etc/crypto-policies/config ] && cmp -s /etc/defaults/etc/crypto-policies/config /etc/crypto-policies/config
then
    rm /etc/crypto-policies/config
fi

