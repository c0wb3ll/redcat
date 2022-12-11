O_MSG=""

while getopts m: opts; do
    case $opts in
    m) O_MSG=$OPTARG
        ;;
    esac
done

if [ "${O_MSG}" = "" ]; then
    echo "usage:"
    echo "\t./test.sh -m {commit_message}"
fi
if [ "${O_MSG}" != "" ]; then
    git add .
    git commit -m "${O_MSG}"
    git push -u origin main
fi
