# store arguments in a special array 
args=("$@") 
# get number of elements 
ELEMENTS=${#args[@]} 
 
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

echo ${args[@]} > dvrescue.args
dvrescue "${args[@]}"

#set "${args[@]}"

#echo $@ > dvrescue.args
#dvrescue $@