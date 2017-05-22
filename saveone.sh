TURN="$1";
ME_BALLS="$2";
THEM_BALLS="$3";
ME_DUCKS="$4";
THEM_DUCKS="$5";
MAX_BALLS="$6";

if (( ME_BALLS > 1 )); then
	echo "1";
elif (( THEM_BALLS == 0 )); then
	if (( THEM_DUCKS == 0 )); then
		echo "1";
	else
		echo "0";
	fi;
elif (( ME_DUCKS > 1 )); then
	echo "2";
else
	echo "0";
fi;
