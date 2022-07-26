#!/bin/bash
shell_dir=$(cd "$(dirname "$0")";pwd)/
library_dir=${shell_dir}lib/

ldconfig -n ${library_dir}
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${library_dir}
ulimit -n 65536

cd ${library_dir}
for file in *.so.*
do
    if [[ ${file}"x" != "x" ]]; then
        realname=$(echo ${file} | rev |cut -d '/' -f 1 | rev)
        libname=$(echo ${realname} |cut -d '.' -f 1)
        if [ ! -f ${libname}.so ]; then
            ln -sf ${realname} ${libname}.so
        fi
    fi
done

cd ${shell_dir}
if [ -f runlog ]; then
    runlog_size=$(ls -l runlog | awk '{print $5}')
    if [ ${runlog_size} -gt 10000000 ]; then
            echo -e "runlog too big, restart at $(date)" > runlog
    fi
fi

function TrapSigint()
{
    :
}
trap TrapSigint 2

cd ${shell_dir}
echo -e "${project}-valgrind start at $(date)" >> runlog
chmod +x ${project}
valgrind --time-stamp=yes --log-file=${project}_memcheck.log --leak-check=full --show-leak-kinds=all --tool=memcheck ./${project}
echo -e "${project}-valgrind stop at $(date)" >> runlog