nb_clients=$1;
nb_servers=$2;
if [ -d "build" ];
then 
    echo "build directory already exists"l
else
    mkdir "build";
fi
echo "build created";
sum=$(( $nb_clients + $nb_servers + 1))
echo "$sum"
cd "build"
cmake ..
make
mpirun -n "$sum" ./algorep --servers $nb_servers --clients $nb_clients 