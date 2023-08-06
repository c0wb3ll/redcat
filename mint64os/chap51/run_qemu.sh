O_DEBUG=""
O_ARCH="x86_64"
O_DISK="-hda ${PWD}/HDD.img"
O_SINGLE_CORE="-smp sockets=4"

while getopts da: opts; do
    case $opts in
    v) O_DEBUG="-s -S"
        ;;
    a) O_ARCH=$OPTARG
        ;;
    d) O_DISK=""
        ;;
    s) O_SINGLE_CORE=""
        ;;
    r) qemu-img craete HDD.img 20M
	;;
    esac
done

qemu-system-${O_ARCH} -L . -m 64 -fda ${PWD}/Disk.img ${O_DISK} --boot a -rtc base=localtime -M pc -serial tcp::4444,server,nowait ${O_SINGLE_CORE} ${O_DEBUG}
