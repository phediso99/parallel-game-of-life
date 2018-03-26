#PBS -q pdlab
#PBS -N game_of_life_4
#PBS -j oe
#PBS -l nodes=4:ppn=8,mem=60GB
#PBS -l walltime=2:00:00
#PBS -m abe -M rosevoul@gmail.com

PROCESSES=2

module load mpi/mpich3-x86_64

N=40000
threshold=0.6
generations=3
display=0
threads=8          

cd $PBS_O_WORKDIR
LOGFILE=game_$PROCESSES.txt
rm $LOGFILE
echo "N = " $N "Processes = " $PROCESSES >> $LOGFILE
mpiexec -ppn 1 ./src/game-of-life $N $threshold $generations $display | grep "GAME" >> $LOGFILE

