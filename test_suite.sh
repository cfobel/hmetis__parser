#/bin/bash

function error {
    if [ "$1" != "" ]
        then echo "Error";
        echo "$2";
    fi
}

NETLIST_PARSER="./VPRNetParser"

echo -ne "`date`: Processing MCNC benchmarks\t";
mcnc_result=`(for bm in ../../pycuda/starplus_cuda_c/benchmarks/mcnc/*.net ; do echo "Processing ${bm}"; ${NETLIST_PARSER} < ${bm}; echo "-------------------------------"; done;)`
echo "DONE (`date`)"
mcnc_error=`echo -n "${mcnc_result}" | grep "Error"`
error "${mcnc_error}" "${mcnc_result}";

echo -ne "`date`: Processing large benchmarks\t";
large_result=`(for bm in ../../pycuda/starplus_cuda_c/benchmarks/large/*.net ; do echo "Processing ${bm}"; ${NETLIST_PARSER} < ${bm}; echo "-------------------------------"; done;)`
echo "DONE (`date`)"
large_error=`echo -n "${large_result}" | grep "Error"`
error "${large_error}" "${large_result}";
