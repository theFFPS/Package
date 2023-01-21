set -e
export HOME_PATH_TO_USE=/root
if [ "$EUID" -ne 0 ]; then
  export HOME_PATH_TO_USE=/home/$(whoami)
fi

if test -f "${HOME_PATH_TO_USE}/.package_postbuild_lock"; then
    exit
fi
echo "for IPATH in \$(ls -d /Apps/*/)
do
    export CPATH=\${CPATH}:\${IPATH}include
    export LIBRARY_PATH=\${LIBRARY_PATH}:\${IPATH}lib
    export LIBRARY_PATH=\${LIBRARY_PATH}:\${IPATH}lib64
    export PATH=\${PATH}:\${IPATH}bin
    export PATH=\${PATH}:\${IPATH}sbin
done" >> ${HOME_PATH_TO_USE}/.zshrc
echo "for IPATH in \$(ls -d /Apps/*/)
do
    export CPATH=\${CPATH}:\${IPATH}include
    export LIBRARY_PATH=\${LIBRARY_PATH}:\${IPATH}lib
    export LIBRARY_PATH=\${LIBRARY_PATH}:\${IPATH}lib64
    export PATH=\${PATH}:\${IPATH}bin
    export PATH=\${PATH}:\${IPATH}sbin
done" >> ${HOME_PATH_TO_USE}/.bashrc
touch ${HOME_PATH_TO_USE}/.package_postbuild_lock
unset HOME_PATH_TO_USE