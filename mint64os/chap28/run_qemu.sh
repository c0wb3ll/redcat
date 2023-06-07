O_DEBUG=""
O_ARCH="x86_64"
O_DISK="-hda ${PWD}/HDD.img"

while getopts da: opts; do
    case $opts in
    v) O_DEBUG="-s -S"
        ;;
    a) O_ARCH=$OPTARG
        ;;
    d) O_DISK=""
    esac
done

qemu-img create HDD.img 20M
qemu-system-${O_ARCH} -L . -m 64 -fda ${PWD}/Disk.img ${O_DISK} -rtc base=localtime -M pc -serial tcp::4444,server,nowait ${O_DEBUG}
