#!/bin/sh

current_dir=$(pwd)
make_configure=0
enable_debug=0
cp_binfmt=0
help() {
	echo "Usage:"
	echo "	-c              configure"
	echo "	-d              --enable-debug"
	echo "	-h              help"
	echo "	-r              rm -rf build && mkdir build"
	echo "	-u              cp build/mips64el-linux-user/qemu-mips64el  /home/loongson/binfmt-qemu"
}

rm_build() {
    rm -rf build;
    mkdir build;
}

parseArgs() {
	while getopts "cdhru" opt; do
		case ${opt} in
		c)
                        make_configure=1
			;;
		d)
                        enable_debug=1
			;;
		h)
			help
			exit
			;;
		r)
			rm_build
			;;
		u)
			cp_binfmt=1
			;;
		# 若选项需要参数但未收到，则走冒号分支
		:)	
			echo "invalid args, exit..."
			help
			exit
			;;
		# 若遇到未指定的选项，会走问号分支
		?)
			echo "invalid args, exit..."
			help
			exit
			;;
		esac
	done
}

make_cmd() {
    cd build

    if [ $make_configure -eq 1 ] ; then
        if [ $enable_debug -eq 1 ] ; then
            ../configure --target-list=i386-linux-user --enable-latx --disable-guest-base --enable-debug --disable-werror
        else
            ../configure --target-list=i386-linux-user --enable-latx --disable-guest-base --disable-debug-info --disable-werror
        fi
    fi

    make -j8
    cd ${current_dir}
#    cp build/i386-linux-user/qemu-i386 /home/loongson/latx/test-app/

    #if [ $cp_binfmt -eq 1 ] ; then
    #fi
}

parseArgs "$@"
make_cmd
