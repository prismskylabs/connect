# uncomment lines below to enable debugging
#echo "Entering gen-jam.sh"
#pwd
cat <<EOF >tools/build/src/user-config.jam
using gcc : arm : "$WN/arm-linux-gnueabi-g++" ;
EOF
#echo "Exiting gen-jam.sh"
