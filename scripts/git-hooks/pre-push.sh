# pre push hook script from
# http://codeinthehole.com/writing/tips-for-using-a-Git-pre-commit-hook/

# works on linux
NUM_CORES=$(grep "^core id" /proc/cpuinfo | sort -u | wc -l)

git stash -q --keep-index
scons test=1 -j$NUM_CORES
./mixxx-test
RESULT=$?
git stash pop -q
[ $RESULT -ne 0 ] && exit 1
exit 0
