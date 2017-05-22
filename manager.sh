INITIAL_DUCKS="25";
MAX_BALLS="50";
MAX_TURNS="1000";

A_EXEC="$1";
B_EXEC="$2";
GAME_COUNT="$3";

A_WINS="0";
B_WINS="0";
DRAWS="0";

GAME="0";
while (( "$GAME" < "$GAME_COUNT" )); do
	TURN="0";
	A_BALLS="0";
	B_BALLS="0";
	A_DUCKS="$INITIAL_DUCKS";
	B_DUCKS="$INITIAL_DUCKS";
	WINNER="DRAW";
	while [[ "$TURN" != "$MAX_TURNS" ]]; do
		echo "Game $GAME/$GAME_COUNT, turn $TURN/$MAX_TURNS:";
		A_ACT="$("$A_EXEC" "$TURN" "$A_BALLS" "$B_BALLS" "$A_DUCKS" "$B_DUCKS" "$MAX_BALLS")";
		B_ACT="$("$B_EXEC" "$TURN" "$B_BALLS" "$A_BALLS" "$B_DUCKS" "$A_DUCKS" "$MAX_BALLS")";
		echo "  A = {Balls: $A_BALLS, Ducks: $A_DUCKS, Action: $A_ACT}";
		echo "  B = {Balls: $B_BALLS, Ducks: $B_DUCKS, Action: $B_ACT}";
		# reload, throw, duck
		if [[ "$A_ACT" == "0" ]] && (( "$A_BALLS" < "$MAX_BALLS" )); then
			if [[ "$B_ACT" == "0" ]] && (( "$B_BALLS" < "$MAX_BALLS" )); then
				(( A_BALLS += 1 ));
				(( B_BALLS += 1 ));
			elif [[ "$B_ACT" == "1" ]] && (( "$B_BALLS" > "0" )); then
				WINNER="B";
				break;
			elif [[ "$B_ACT" == "2" ]] && (( "$B_DUCKS" > "0" )); then
				(( A_BALLS += 1 ));
				(( B_DUCKS -= 1 ));
			else
				(( A_BALLS += 1 ));
			fi;
		elif [[ "$A_ACT" == "1" ]] && (( "$A_BALLS" > "0" )); then
			if [[ "$B_ACT" == "0" ]] && (( "$B_BALLS" < "$MAX_BALLS" )); then
				WINNER="A";
				break;
			elif [[ "$B_ACT" == "1" ]] && (( "$B_BALLS" > "0" )); then
				(( A_BALLS -= 1 ));
				(( B_BALLS -= 1 ));
			elif [[ "$B_ACT" == "2" ]] && (( "$B_DUCKS" > "0" )); then
				(( A_BALLS -= 1 ));
				(( B_DUCKS -= 1 ));
			else
				WINNER="A";
				break;
			fi;
		elif [[ "$A_ACT" == "2" ]] && (( "$A_DUCKS" > "0" )); then
			if [[ "$B_ACT" == "0" ]] && (( "$B_BALLS" < "$MAX_BALLS" )); then
				(( A_DUCKS -= 1 ));
				(( B_BALLS += 1 ));
			elif [[ "$B_ACT" == "1" ]] && (( "$B_BALLS" > "0" )); then
				(( A_DUCKS -= 1 ));
				(( B_BALLS -= 1 ));
			elif [[ "$B_ACT" == "2" ]] && (( "$B_DUCKS" > "0" )); then
				(( A_DUCKS -= 1 ));
				(( B_DUCKS -= 1 ));
			else
				(( A_DUCKS -= 1 ));
			fi;
		else
			if [[ "$B_ACT" == "0" ]] && (( "$B_BALLS" < "$MAX_BALLS" )); then
				(( B_BALLS += 1 ));
			elif [[ "$B_ACT" == "1" ]] && (( "$B_BALLS" > "0" )); then
				WINNER="B";
				break;
			elif [[ "$B_ACT" == "2" ]] && (( "$B_DUCKS" > "0" )); then
				(( B_DUCKS -= 1 ));
			fi;
		fi;
		(( TURN += 1 ));
	done;

	if [[ "$WINNER" == "A" ]]; then
		(( A_WINS += 1 ));
	elif [[ "$WINNER" == "B" ]]; then
		(( B_WINS += 1 ));
	else
		(( DRAWS += 1 ));
	fi;

	echo "Result: $WINNER";
	echo;

	(( GAME += 1 ));
done;

echo;
echo "Summary: A: $A_WINS  B: $B_WINS  DRAW: $DRAWS";
