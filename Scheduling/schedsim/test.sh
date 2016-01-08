#!/bin/bash

#The script will simulate a given input file under all the implemented schedulers and all possible CPU counts (until a given maximum CPU count)
#Create a shell script that does not accept arguments, but instead asks the user to enter the following information interactively:
#– Theinputfilenametobesimulated.Thescriptwillverifythatthefileexistsandisregular. If that is not the case, the script will display an error message and will ask the user to enter a file name again.
#– Maximum number of CPUs to use during the simulation. This number should not be greater than 8. If the number is greater than 8, the script will display an error message and will ask the user to enter a new CPU count.
#Then, a results directory will be created and the simulator will be executed for each available scheduling algorithm and for each CPU count from 1 to the maximum CPU count provided by the used. The script will ensure that all simulation results are safely backed up (without overwriting files) in the results directory. Finally, for each file as- sociated with the various simulations, a different chart will be generated. The charts will be stored in the results directory as well.
#A possible pseudocode representation of the core processing of the script is as follows:


directory="results"

# Ask the user a valid filename to make the simulation

filename=""
valid=false

while [ "$valid" != "true" ]
do
	echo "(?) Filename (without extension): "
	read auxFilename

	filename=("./examples/"$auxFilename".txt")
	
	if [ -f "$filename" ]
	then
	valid=true
	else
		echo "$filename not found."
	fi
done

# Ask the user a valid maximun Number of CPUS to be executed

maxCPUs=0
valid=false

while [ "$valid" != "true" ]
do
	echo "(?) Maximum CPU's: "
	read maxCPUs
 
	if (( maxCPUs >= 0 &&  maxCPUs <= 8 )); then
		valid=true
	else
		echo "(Error) You're number is not valid (range: 0-8). Try again"
	fi
done

# Create a results directory

if [ -d "$directory" ]; then
	# Removes Directory if the folder exists
	rm -rf -- $directory  # INFO: http://stackoverflow.com/questions/820760/in-unix-how-do-you-remove-everything-in-the-current-directory-and-below-it
fi

mkdir $directory # creates an empty directory

# simulator will be executed for each available scheduling algorithm.
# and for each CPU count from 1 to the maximum CPU count provided by the used. 
 
array2=()
i=0
for line in $(./schedsim -L); do
	if (( i >= 2 )); then
		cpus=1
		algorithmName=$line

		echo "=========================================================="
		echo "Executing $maxCPUs times $algorithmName algorithm"
		echo "=========================================================="

		while (( cpus <= maxCPUs )); do
			./schedsim -n $cpus -i $filename
			echo "---"
			((cpus+=1))
		done

		# Backup files. Moving to other directory and with a special name
		# Filename will be: $algorithmName"_"$cpus".log"
		for f_name in *.log; do 
			auxFilename=($directory"/"$algorithmName"__"$f_name)
			mv $f_name $auxFilename

			# Generate Chart generate chart
			.././gantt-gplot/generate_gantt_chart $auxFilename
		done

	else
		# This line is header. Not the algorithm name
		# print on screen
		#echo $line
		echo ""
	fi
	((i+=1))
done

exit 0 
 
#foreach nameSched in listOfAvailableSchedulers
#do
#    for cpus = 1 to maxCPUs
#	do
#	  ./sched-sim -n cpus -i .......
#	for i=1 to cpus
#	do
#		move CPU_$i.log to  results/nameSched-CPU-$i.log
#	done
#	generate chart
#done done

