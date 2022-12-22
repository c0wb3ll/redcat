O_DEBUG="no"
O_ARCH="x86_64"

while getopts da: opts; do
    case $opts in
    d) O_DEBUG="yes"
        ;;
    a) O_ARCH=$OPTARG
        ;;
    esac
done

if [ "${O_DEBUG}" = "yes" ]; then
    qemu-system-${O_ARCH} -L . -m 64 -fda ${PWD}/Disk.img -rtc base=localtime -M pc -s -S
fi
if [ "${O_DEBUG}" = "no" ]; then
    qemu-system-${O_ARCH} -L . -m 64 -fda ${PWD}/Disk.img -rtc base=localtime -M pc
fi
