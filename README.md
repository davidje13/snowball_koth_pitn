# Pain in the Nash

This is the [Pain in the Nash](https://codegolf.stackexchange.com/a/121943/8927) entry for
[Snowball Fight KoTH](https://codegolf.stackexchange.com/q/120688/8927), so called because the fact that I had to write my own
Nash equilibrium solver was a real pain.

Compile as C++11 or better. For performance, it's good to compile with OpenMP support (but this is just for speed; it's not
required)

```
g++ -std=c++11 -fopenmp pain_in_the_nash.cpp -o pain_in_the_nash
```

Run one turn with:

```
./pain_in_the_nash <turn> <my_snowball_count> <their_snowball_count> <my_duck_count> <their_duck_count> <max_snowballs>

# e.g.:
./pain_in_the_nash 10 20 20 5 7 50
```

Or (re)generate the data files with:

```
./pain_in_the_nash <max_turns> <max_balls> <max_ducks>

# e.g.:
./pain_in_the_nash 1000 50 25
```

This uses Nash equilibria to decide what to do on each turn, which means that *in theory* it will always win or draw in the
long run (over many games), no matter what strategy the opponent uses. Whether that's the case in practice depends on whether
I made any mistakes in the implementation. However, since this KoTH competition only has a single round against each opponent,
it probably won't do very well on the leaderboard.

My original idea was to have a simple valuation function for each game state (e.g. each ball is worth +b, each duck is +d),
but this leads to obvious problems figuring out what those valuations should be, and means it can't act on diminishing returns
of gathering more and more balls, etc. So instead, this will analyse the *entire game tree*, working backwards from turn 1000,
and fill in the actual valuations based on how each game could pan out.

The result is that I have absolutely no idea what strategy this uses, except for a couple of hard-coded "obvious" behaviours
(throw snowballs if you have more balls than your opponent has balls+ducks, and reload if you're both out of snowballs). If
anybody wants to analyse the dataset it produces I imagine there's some interesting behaviour to discover!

Testing this against "Save One" shows that it does indeed win in the long-run, but only by a small margin (514 wins, 486
losses, 0 draws in the first batch of 1000 games, and 509 wins, 491 losses, 0 draws in the second).

## manager.sh

This is a simple script which runs a number of games between 2 agents and records the winner. It will always run games with
max_snowballs = 50, initial_ducks = 25, and max_turns = 1000.

Usage:

```
./manager.sh <agent_1> <agent_2> <total_games>

# e.g.:
./manager.sh ./pain_in_the_nash ./save_one.sh 1000
```

The test competitors are:

* save_one.sh (based on the [Save One](https://codegolf.stackexchange.com/a/120848/8927) entry)
* simple.sh (based on obvious rules with randomisation for any non-clear states)
