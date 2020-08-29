# runs rps
FLAGS=$@
CIN=
COUT=
NUM_RAND=2000 # change to control how many rounds rand_gen plays
RPS_PROJ_DIR=..
cd ${RPS_PROJ_DIR}
# check if graphing or not
if [ ! $(echo ${FLAGS} | grep -o -s '\-G' | wc -w) = 0 ]; then
    COUT="./src/graph_progress.py"
fi
if [ ! $(echo ${FLAGS} | grep -o -s '\-R' | wc -w) = 0 ]; then
    CIN="./src/rand_gen"
fi

if [ ! -z ${CIN} ]; then
    ${CIN} ${NUM_RAND} | ./src/rps ${FLAGS} | ${COUT}
else
    ./src/rps ${FLAGS} | ${COUT}
fi    
