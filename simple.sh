TURN="$1";
ME_BALLS="$2";
THEM_BALLS="$3";
ME_DUCKS="$4";
THEM_DUCKS="$5";
MAX_BALLS="$6";

if (( ME_BALLS > THEM_BALLS + THEM_DUCKS )); then
	echo "1";
elif (( ME_BALLS == 0 )) && (( THEM_BALLS == 0 )); then
	echo "0";
else
	RND="$RANDOM";
	(( RND %= 3 ));
	echo "$RND";
fi;
