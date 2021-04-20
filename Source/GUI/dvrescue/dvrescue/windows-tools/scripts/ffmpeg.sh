# store arguments in a special array 
args=("$@") 

echo $@ > ffmpeg.args.orig

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

echo ${args[@]} > ffmpeg.args
ffmpeg ${args[@]}

#set "${args[@]}"

#echo $@ > ffmpeg.args
#ffmpeg $@