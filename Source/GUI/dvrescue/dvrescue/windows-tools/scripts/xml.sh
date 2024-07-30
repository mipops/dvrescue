# store arguments in a special array 
args=("$@") 
# get number of elements 
ELEMENTS=${#args[@]} 

# echo "$@" > args.txt
 
# echo each element in array  
# for loop 

for (( i=0;i<$ELEMENTS;i++)); do 
	# echo "${args[${i}]}"
	if [[ ${args[${i}]} == /cygdrive* ]]; then
		path=$(cygpath -w ${args[${i}]})
		# echo "original path: ${args[${i}]}, updated path: $path"
		args[${i}]=$path
	fi
	if [[ ${args[${i}]} == /tmp* ]]; then
		path=$(cygpath -w ${args[${i}]})
		# echo "original path: ${args[${i}]}, updated path: $path"
		args[${i}]=$path
	fi

	args[${i}]="${args[${i}]}"
done

echo ${args[@]} > xml.args
xml "${args[@]}"

# set "${args[@]}"

# echo $@ > xml.args

# OUTPUT=$(xml $@)
# echo $OUTPUT