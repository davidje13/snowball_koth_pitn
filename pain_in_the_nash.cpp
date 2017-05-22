#include <fstream>
#include <iostream>
#include <vector>
#include <array>
#include <random>
#include <utility>

typedef double NumT;
static const NumT EPSILON = 1e-5;

struct Index {
	int me;
	int them;

	Index(int me, int them) : me(me), them(them) {}
};

struct Value {
	NumT me;
	NumT them;

	Value(void) : me(0), them(0) {}

	Value(NumT me, NumT them) : me(me), them(them) {}
};

template <int subDimMe, int subDimThem>
struct Game {
	const std::array<NumT, 9> *valuesMe;
	const std::array<NumT, 9> *valuesThemT;

	std::array<int, subDimMe> coordsMe;
	std::array<int, subDimThem> coordsThem;

	Game(
		const std::array<NumT, 9> *valuesMe,
		const std::array<NumT, 9> *valuesThemT
	)
		: valuesMe(valuesMe)
		, valuesThemT(valuesThemT)
		, coordsMe{}
		, coordsThem{}
	{}

	Index baseIndex(Index i) const {
		return Index(coordsMe[i.me], coordsThem[i.them]);
	}

	Value at(Index i) const {
		Index i2 = baseIndex(i);
		return Value(
			(*valuesMe)[i2.me * 3 + i2.them],
			(*valuesThemT)[i2.me + i2.them * 3]
		);
	}

	Game<2, 2> subgame22(int me0, int me1, int them0, int them1) const {
		Game<2, 2> b(valuesMe, valuesThemT);
		b.coordsMe[0] = coordsMe[me0];
		b.coordsMe[1] = coordsMe[me1];
		b.coordsThem[0] = coordsThem[them0];
		b.coordsThem[1] = coordsThem[them1];
		return b;
	}
};

struct Strategy {
	std::array<NumT, 3> probMe;
	std::array<NumT, 3> probThem;
	Value expectedValue;
	bool valid;

	Strategy(void)
		: probMe{}
		, probThem{}
		, expectedValue()
		, valid(false)
	{}

	void findBestMe(const Strategy &b) {
		if(b.valid && (!valid || b.expectedValue.me > expectedValue.me)) {
			*this = b;
		}
	}
};

template <int dimMe, int dimThem>
void debugGame(const Game<dimMe, dimThem> &g) {
	std::cerr << "Subgame " << dimMe << 'x' << dimThem << std::endl;
	std::cerr << '.';
	for(int them = 0; them < dimThem; ++ them) {
		std::cerr << "\t" << char('a' + them);
	}
	std::cerr << std::endl;
	for(int me = 0; me < dimMe; ++ me) {
		std::cerr << char('A' + me);
		for(int them = 0; them < dimThem; ++ them) {
			std::cerr
				<< "\t" << g.at(Index(me, them)).me
				<< "|" << g.at(Index(me, them)).them;
		}
		std::cerr << std::endl;
	}
	std::cerr << std::endl;
}

void debugStrategy(const Strategy &s) {
	std::cerr << "Strategy:" << std::endl;
	std::cerr << "  Me:  ";
	for(int i = 0; i < s.probMe.size(); ++ i) {
		std::cerr << ' ' << s.probMe[i];
	}
	std::cerr << " for expected payoff " << s.expectedValue.me << std::endl;
	std::cerr << "  Them:";
	for(int i = 0; i < s.probThem.size(); ++ i) {
		std::cerr << ' ' << s.probThem[i];
	}
	std::cerr << " for expected payoff " << s.expectedValue.them << std::endl;
	std::cerr << std::endl;
}

template <int dimMe, int dimThem>
Strategy nash_pure(const Game<dimMe, dimThem> &g) {
	Strategy s;
	int choiceMe = -1;
	int choiceThem = 0;
	for(int me = 0; me < dimMe; ++ me) {
		for(int them = 0; them < dimThem; ++ them) {
			const Value &v = g.at(Index(me, them));
			bool valid = true;
			for(int me2 = 0; me2 < dimMe; ++ me2) {
				if(g.at(Index(me2, them)).me > v.me) {
					valid = false;
				}
			}
			for(int them2 = 0; them2 < dimThem; ++ them2) {
				if(g.at(Index(me, them2)).them > v.them) {
					valid = false;
				}
			}
			if(valid) {
				if(choiceMe == -1 || v.me > s.expectedValue.me) {
					s.expectedValue = v;
					choiceMe = me;
					choiceThem = them;
				}
			}
		}
	}
	if(choiceMe != -1) {
		Index iBase = g.baseIndex(Index(choiceMe, choiceThem));
		s.probMe[iBase.me] = 1;
		s.probThem[iBase.them] = 1;
		s.valid = true;
	}
	return s;
}

Strategy nash_mixed(const Game<2, 2> &g) {
	//    P    Q
	// p a A  b B
	// q c C  d D

	Value A = g.at(Index(0, 0));
	Value B = g.at(Index(0, 1));
	Value C = g.at(Index(1, 0));
	Value D = g.at(Index(1, 1));

	// q = 1-p, Q = 1-P
	// Pick p such that choice of P,Q is arbitrary

	// p*A+(1-p)*C = p*B+(1-p)*D
	// p*A+C-p*C = p*B+D-p*D
	// p*(A+D-B-C) = D-C
	// p = (D-C) / (A+D-B-C)

	NumT p = (D.them - C.them) / (A.them + D.them - B.them - C.them);

	// P*a+(1-P)*b = P*c+(1-P)*d
	// P*a+b-P*b = P*c+d-P*d
	// P*(a+d-b-c) = d-b
	// P = (d-b) / (a+d-b-c)

	NumT P = (D.me - B.me) / (A.me + D.me - B.me - C.me);

	Strategy s;
	if(p >= -EPSILON && p <= 1 + EPSILON && P >= -EPSILON && P <= 1 + EPSILON) {
		if(p <= 0) {
			p = 0;
		} else if(p >= 1) {
			p = 1;
		}
		if(P <= 0) {
			P = 0;
		} else if(P >= 1) {
			P = 1;
		}
		Index iBase0 = g.baseIndex(Index(0, 0));
		Index iBase1 = g.baseIndex(Index(1, 1));
		s.probMe[iBase0.me] = p;
		s.probMe[iBase1.me] = 1 - p;
		s.probThem[iBase0.them] = P;
		s.probThem[iBase1.them] = 1 - P;
		s.expectedValue = Value(
			P * A.me + (1 - P) * B.me,
			p * A.them + (1 - p) * C.them
		);
		s.valid = true;
	}
	return s;
}

Strategy nash_mixed(const Game<3, 3> &g) {
	//    P    Q    R
	// p a A  b B  c C
	// q d D  e E  f F
	// r g G  h H  i I

	Value A = g.at(Index(0, 0));
	Value B = g.at(Index(0, 1));
	Value C = g.at(Index(0, 2));
	Value D = g.at(Index(1, 0));
	Value E = g.at(Index(1, 1));
	Value F = g.at(Index(1, 2));
	Value G = g.at(Index(2, 0));
	Value H = g.at(Index(2, 1));
	Value I = g.at(Index(2, 2));

	// r = 1-p-q, R = 1-P-Q
	// Pick p,q such that choice of P,Q,R is arbitrary

	NumT q = ((
		+ A.them * (I.them-H.them)
		+ G.them * (B.them-C.them)
		- B.them*I.them
		+ H.them*C.them
	) / (
		(G.them+E.them-D.them-H.them) * (B.them+I.them-H.them-C.them) -
		(H.them+F.them-E.them-I.them) * (A.them+H.them-G.them-B.them)
	));

	NumT p = (
		((G.them+E.them-D.them-H.them) * q + (H.them-G.them)) /
		(A.them+H.them-G.them-B.them)
	);

	NumT Q = ((
		+ A.me * (I.me-F.me)
		+ C.me * (D.me-G.me)
		- D.me*I.me
		+ F.me*G.me
	) / (
		(C.me+E.me-B.me-F.me) * (D.me+I.me-F.me-G.me) -
		(F.me+H.me-E.me-I.me) * (A.me+F.me-C.me-D.me)
	));

	NumT P = (
		((C.me+E.me-B.me-F.me) * Q + (F.me-C.me)) /
		(A.me+F.me-C.me-D.me)
	);

	Strategy s;
	if(
		p >= -EPSILON && q >= -EPSILON && p + q <= 1 + EPSILON &&
		P >= -EPSILON && Q >= -EPSILON && P + Q <= 1 + EPSILON
	) {
		if(p <= 0) { p = 0; }
		if(q <= 0) { q = 0; }
		if(P <= 0) { P = 0; }
		if(Q <= 0) { Q = 0; }
		if(p + q >= 1) {
			if(p > q) {
				p = 1 - q;
			} else {
				q = 1 - p;
			}
		}
		if(P + Q >= 1) {
			if(P > Q) {
				P = 1 - Q;
			} else {
				Q = 1 - P;
			}
		}
		Index iBase0 = g.baseIndex(Index(0, 0));
		s.probMe[iBase0.me] = p;
		s.probThem[iBase0.them] = P;
		Index iBase1 = g.baseIndex(Index(1, 1));
		s.probMe[iBase1.me] = q;
		s.probThem[iBase1.them] = Q;
		Index iBase2 = g.baseIndex(Index(2, 2));
		s.probMe[iBase2.me] = 1 - p - q;
		s.probThem[iBase2.them] = 1 - P - Q;
		s.expectedValue = Value(
			A.me * P + B.me * Q + C.me * (1 - P - Q),
			A.them * p + D.them * q + G.them * (1 - p - q)
		);
		s.valid = true;
	}
	return s;
}

template <int dimMe, int dimThem>
Strategy nash_validate(Strategy &&s, const Game<dimMe, dimThem> &g, Index unused) {
	if(!s.valid) {
		return s;
	}

	NumT exp;

	exp = 0;
	for(int them = 0; them < dimThem; ++ them) {
		exp += s.probThem[them] * g.at(Index(unused.me, them)).me;
	}
	if(exp > s.expectedValue.me) {
		s.valid = false;
		return s;
	}

	exp = 0;
	for(int me = 0; me < dimMe; ++ me) {
		exp += s.probMe[me] * g.at(Index(me, unused.them)).them;
	}
	if(exp > s.expectedValue.them) {
		s.valid = false;
		return s;
	}

	return s;
}

Strategy nash(const Game<2, 2> &g, bool verbose) {
	Strategy s = nash_mixed(g);
	s.findBestMe(nash_pure(g));
	if(!s.valid && verbose) {
		std::cerr << "No nash equilibrium found!" << std::endl;
		debugGame(g);
	}
	return s;
}

Strategy nash(const Game<3, 3> &g, bool verbose) {
	Strategy s = nash_mixed(g);
	s.findBestMe(nash_validate(nash_mixed(g.subgame22(1, 2,  1, 2)), g, Index(0, 0)));
	s.findBestMe(nash_validate(nash_mixed(g.subgame22(1, 2,  0, 2)), g, Index(0, 1)));
	s.findBestMe(nash_validate(nash_mixed(g.subgame22(1, 2,  0, 1)), g, Index(0, 2)));
	s.findBestMe(nash_validate(nash_mixed(g.subgame22(0, 2,  1, 2)), g, Index(1, 0)));
	s.findBestMe(nash_validate(nash_mixed(g.subgame22(0, 2,  0, 2)), g, Index(1, 1)));
	s.findBestMe(nash_validate(nash_mixed(g.subgame22(0, 2,  0, 1)), g, Index(1, 2)));
	s.findBestMe(nash_validate(nash_mixed(g.subgame22(0, 1,  1, 2)), g, Index(2, 0)));
	s.findBestMe(nash_validate(nash_mixed(g.subgame22(0, 1,  0, 2)), g, Index(2, 1)));
	s.findBestMe(nash_validate(nash_mixed(g.subgame22(0, 1,  0, 1)), g, Index(2, 2)));
	s.findBestMe(nash_pure(g));
	if(!s.valid && verbose) {
		// theory says this should never happen, but fp precision makes it possible
		std::cerr << "No nash equilibrium found!" << std::endl;
		debugGame(g);
	}
	return s;
}

struct PlayerState {
	int balls;
	int ducks;

	PlayerState(int balls, int ducks) : balls(balls), ducks(ducks) {}

	PlayerState doReload(int maxBalls) const {
		return PlayerState(std::min(balls + 1, maxBalls), ducks);
	}

	PlayerState doThrow(void) const {
		return PlayerState(std::max(balls - 1, 0), ducks);
	}

	PlayerState doDuck(void) const {
		return PlayerState(balls, std::max(ducks - 1, 0));
	}

	std::array<double,3> flail(int maxBalls) const {
		// opponent has obvious win;
		// try stuff at random and hope the opponent is bad

		(void) ducks;

		int options = 0;
		if(balls > 0) {
			++ options;
		}
		if(balls < maxBalls) {
			++ options;
		}
		if(ducks > 0) {
			++ options;
		}

		std::array<double,3> p{};
		if(balls < balls) {
			p[0] = 1.0f / options;
		}
		if(balls > 0) {
			p[1] = 1.0f / options;
		}
		return p;
	}
};

class GameStore {
protected:
	const int balls;
	const int ducks;
	const std::size_t playerStates;
	const std::size_t gameStates;

public:
	static std::string filename(int turn) {
		return "nashdata_" + std::to_string(turn) + ".dat";
	}

	GameStore(int maxBalls, int maxDucks)
		: balls(maxBalls)
		, ducks(maxDucks)
		, playerStates((balls + 1) * (ducks + 1))
		, gameStates(playerStates * playerStates)
	{}

	std::size_t playerIndex(const PlayerState &p) const {
		return p.balls * (ducks + 1) + p.ducks;
	}

	std::size_t gameIndex(const PlayerState &me, const PlayerState &them) const {
		return playerIndex(me) * playerStates + playerIndex(them);
	}

	std::size_t fileIndex(const PlayerState &me, const PlayerState &them) const {
		return 2 + gameIndex(me, them) * 2;
	}

	PlayerState stateFromPlayerIndex(std::size_t i) const {
		return PlayerState(i / (ducks + 1), i % (ducks + 1));
	}

	std::pair<PlayerState, PlayerState> stateFromGameIndex(std::size_t i) const {
		return std::make_pair(
			stateFromPlayerIndex(i / playerStates),
			stateFromPlayerIndex(i % playerStates)
		);
	}

	std::pair<PlayerState, PlayerState> stateFromFileIndex(std::size_t i) const {
		return stateFromGameIndex((i - 2) / 2);
	}
};

class Generator : public GameStore {
	static char toDat(NumT v) {
		int iv = int(v * 256.0);
		return char(std::max(std::min(iv, 255), 0));
	}

	std::vector<Value> next;

public:
	Generator(int maxBalls, int maxDucks)
		: GameStore(maxBalls, maxDucks)
		, next()
	{}

	const Value &nextGame(const PlayerState &me, const PlayerState &them) const {
		return next[gameIndex(me, them)];
	}

	void make_probabilities(
		std::array<NumT, 9> &g,
		const PlayerState &me,
		const PlayerState &them
	) const {
		const int RELOAD = 0;
		const int THROW = 1;
		const int DUCK = 2;

		g[RELOAD * 3 + RELOAD] =
			nextGame(me.doReload(balls), them.doReload(balls)).me;

		g[RELOAD * 3 + THROW] =
			(them.balls > 0) ? -1
			: nextGame(me.doReload(balls), them.doThrow()).me;

		g[RELOAD * 3 + DUCK] =
			nextGame(me.doReload(balls), them.doDuck()).me;

		g[THROW * 3 + RELOAD] =
			(me.balls > 0) ? 1
			: nextGame(me.doThrow(), them.doReload(balls)).me;

		g[THROW * 3 + THROW] =
			((me.balls > 0) == (them.balls > 0))
			? nextGame(me.doThrow(), them.doThrow()).me
			: (me.balls > 0) ? 1 : -1;

		g[THROW * 3 + DUCK] =
			(me.balls > 0 && them.ducks == 0) ? 1
			: nextGame(me.doThrow(), them.doDuck()).me;

		g[DUCK * 3 + RELOAD] =
			nextGame(me.doDuck(), them.doReload(balls)).me;

		g[DUCK * 3 + THROW] =
			(them.balls > 0 && me.ducks == 0) ? -1
			: nextGame(me.doDuck(), them.doThrow()).me;

		g[DUCK * 3 + DUCK] =
			nextGame(me.doDuck(), them.doDuck()).me;
	}

	Game<3, 3> make_game(const PlayerState &me, const PlayerState &them) const {
		static std::array<NumT, 9> globalValuesMe;
		static std::array<NumT, 9> globalValuesThemT;
		#pragma omp threadprivate(globalValuesMe)
		#pragma omp threadprivate(globalValuesThemT)

		make_probabilities(globalValuesMe, me, them);
		make_probabilities(globalValuesThemT, them, me);
		Game<3, 3> g(&globalValuesMe, &globalValuesThemT);
		for(int i = 0; i < 3; ++ i) {
			g.coordsMe[i] = i;
			g.coordsThem[i] = i;
		}
		return g;
	}

	Strategy solve(const PlayerState &me, const PlayerState &them, bool verbose) const {
		if(me.balls > them.balls + them.ducks) { // obvious answer
			Strategy s;
			s.probMe[1] = 1;
			s.probThem = them.flail(balls);
			s.expectedValue = Value(1, -1);
			return s;
		} else if(them.balls > me.balls + me.ducks) { // uh-oh
			Strategy s;
			s.probThem[1] = 1;
			s.probMe = me.flail(balls);
			s.expectedValue = Value(-1, 1);
			return s;
		} else if(me.balls == 0 && them.balls == 0) { // obvious answer
			Strategy s;
			s.probMe[0] = 1;
			s.probThem[0] = 1;
			s.expectedValue = nextGame(me.doReload(balls), them.doReload(balls));
			return s;
		} else {
			return nash(make_game(me, them), verbose);
		}
	}

	void generate(int turns, bool saveAll, bool verbose) {
		next.clear();
		next.resize(gameStates);
		std::vector<Value> current(gameStates);
		std::vector<char> data(2 + gameStates * 2);

		for(std::size_t turn = turns; (turn --) > 0;) {
			if(verbose) {
				std::cerr << "Generating for turn " << turn << "..." << std::endl;
			}
			NumT maxDiff = 0;
			NumT msd = 0;
			data[0] = balls;
			data[1] = ducks;
			#pragma omp parallel for reduction(+:msd), reduction(max:maxDiff)
			for(std::size_t meBalls = 0; meBalls < balls + 1; ++ meBalls) {
				for(std::size_t meDucks = 0; meDucks < ducks + 1; ++ meDucks) {
					const PlayerState me(meBalls, meDucks);
					for(std::size_t themBalls = 0; themBalls < balls + 1; ++ themBalls) {
						for(std::size_t themDucks = 0; themDucks < ducks + 1; ++ themDucks) {
							const PlayerState them(themBalls, themDucks);
							const std::size_t p1 = gameIndex(me, them);

							Strategy s = solve(me, them, verbose);

							NumT diff;

							data[2+p1*2  ] = toDat(s.probMe[0]);
							data[2+p1*2+1] = toDat(s.probMe[0] + s.probMe[1]);
							current[p1] = s.expectedValue;
							diff = current[p1].me - next[p1].me;
							msd += diff * diff;
							maxDiff = std::max(maxDiff, std::abs(diff));
						}
					}
				}
			}

			if(saveAll) {
				std::ofstream fs(filename(turn).c_str(), std::ios_base::binary);
				fs.write(&data[0], data.size());
				fs.close();
			}

			if(verbose) {
				std::cerr
					<< "Expectations changed by at most " << maxDiff
					<< " (RMSD: " << std::sqrt(msd / gameStates) << ")" << std::endl;
			}
			if(maxDiff < 0.0001f) {
				if(verbose) {
					std::cerr << "Expectations have converged. Stopping." << std::endl;
				}
				break;
			}
			std::swap(next, current);
		}

		// Always save turn 0 with the final converged expectations
		std::ofstream fs(filename(0).c_str(), std::ios_base::binary);
		fs.write(&data[0], data.size());
		fs.close();
	}
};

void test(void) {
	std::array<NumT, 9> valuesMe;
	std::array<NumT, 9> valuesThemT;
	Game<3, 3> g(&valuesMe, &valuesThemT);
	for(int i = 0; i < 3; ++ i) {
		g.coordsMe[i] = i;
		g.coordsThem[i] = i;
	}

	Game<2, 2> g2(&valuesMe, &valuesThemT);
	for(int i = 0; i < 2; ++ i) {
		g2.coordsMe[i] = i;
		g2.coordsThem[i] = i;
	}

	std::cerr << "Testing rock, paper, scissors:" << std::endl;
	valuesMe[0] = 0; valuesMe[1] =-1; valuesMe[2] = 1;
	valuesMe[3] = 1; valuesMe[4] = 0; valuesMe[5] =-1;
	valuesMe[6] =-1; valuesMe[7] = 1; valuesMe[8] = 0;

	valuesThemT[0] = 0; valuesThemT[1] =-1; valuesThemT[2] = 1;
	valuesThemT[3] = 1; valuesThemT[4] = 0; valuesThemT[5] =-1;
	valuesThemT[6] =-1; valuesThemT[7] = 1; valuesThemT[8] = 0;

	debugGame(g);
	debugStrategy(nash(g, true));

	std::cerr << "Testing chicken (L, C, R):" << std::endl;
	valuesMe[0] = 0; valuesMe[1] =-1; valuesMe[2] =-10;
	valuesMe[3] = 1; valuesMe[4] =-10; valuesMe[5] = 1;
	valuesMe[6] =-10; valuesMe[7] =-1; valuesMe[8] = 0;

	valuesThemT[0] = 0; valuesThemT[1] =-1; valuesThemT[2] =-10;
	valuesThemT[3] = 1; valuesThemT[4] =-10; valuesThemT[5] = 1;
	valuesThemT[6] =-10; valuesThemT[7] =-1; valuesThemT[8] = 0;

	debugGame(g);
	debugStrategy(nash(g, true));

	std::cerr << "Testing chicken (L, C):" << std::endl;
	debugGame(g2);
	debugStrategy(nash(g2, true));

	std::cerr << "Testing online example:" << std::endl;
	valuesMe[0] = 1; valuesMe[1] = 10; valuesMe[2] =-10;
	valuesMe[3] = 0; valuesMe[4] = 1; valuesMe[5] = 10;
	valuesMe[6] = 1; valuesMe[7] = 1; valuesMe[8] = 1;

	valuesThemT[0] = 1; valuesThemT[1] = 10; valuesThemT[2] =-10;
	valuesThemT[3] = 0; valuesThemT[4] = 1; valuesThemT[5] = 10;
	valuesThemT[6] = 1; valuesThemT[7] = 1; valuesThemT[8] = 1;

	debugGame(g);
	debugStrategy(nash(g, true));

	std::cerr << "Testing online example 2 (clear pure strategy):" << std::endl;
	valuesMe[0] = 3; valuesMe[1] = 3; valuesMe[2] = 2;
	valuesMe[3] = 1; valuesMe[4] = 3; valuesMe[5] = 0;
	valuesMe[6] = 0; valuesMe[7] = 0; valuesMe[8] = 3;

	valuesThemT[0] = 2; valuesThemT[1] = 0; valuesThemT[2] = 2;
	valuesThemT[3] = 0; valuesThemT[4] = 3; valuesThemT[5] = 0;
	valuesThemT[6] = 2; valuesThemT[7] = 3; valuesThemT[8] = 2;

	debugGame(g);
	debugStrategy(nash(g, true));

	std::cerr << "Testing online example 3 (mixed > pure):" << std::endl;
	valuesMe[0] = 1; valuesMe[1] = 0; valuesMe[2] = 0;
	valuesMe[3] = 0; valuesMe[4] = 0; valuesMe[5] = 3;
	valuesMe[6] = 0; valuesMe[7] = 2; valuesMe[8] = 0;

	valuesThemT[0] = 1; valuesThemT[1] = 0; valuesThemT[2] = 0;
	valuesThemT[3] = 0; valuesThemT[4] = 2; valuesThemT[5] = 0;
	valuesThemT[6] = 0; valuesThemT[7] = 0; valuesThemT[8] = 3;

	debugGame(g);
	debugStrategy(nash(g, true));

	std::cerr << "Testing prisoner's dilemma:" << std::endl;
	valuesMe[0] =-1; valuesMe[1] =-3;
	valuesMe[3] = 0; valuesMe[4] =-2;

	valuesThemT[0] =-1; valuesThemT[1] =-3;
	valuesThemT[3] = 0; valuesThemT[4] =-2;

	debugGame(g2);
	debugStrategy(nash(g2, true));

	std::cerr << "Testing problematic grid:" << std::endl;
	valuesMe[0] =-0.5; valuesMe[1] =-1; valuesMe[2] = 0;
	valuesMe[3] = 1; valuesMe[4] =-0.5; valuesMe[5] =-0.75;
	valuesMe[6] =-1; valuesMe[7] =-0.5; valuesMe[8] =-0.75;

	valuesThemT[0] = 0.5; valuesThemT[1] =-1; valuesThemT[2] = 1;
	valuesThemT[3] = 1; valuesThemT[4] = 0.5; valuesThemT[5] = 0.5;
	valuesThemT[6] = 0; valuesThemT[7] = 0.75; valuesThemT[8] = 0.75;

	debugGame(g);
	debugStrategy(nash(g, true));
}

void open_file(std::ifstream &target, int turn, int maxDucks, int maxBalls) {
	target.open(GameStore::filename(turn).c_str(), std::ios::binary);
	if(target.is_open()) {
		return;
	}

	target.open(GameStore::filename(0).c_str(), std::ios::binary);
	if(target.is_open()) {
		return;
	}

	Generator(maxBalls, maxDucks).generate(200, false, false);
	target.open(GameStore::filename(0).c_str(), std::ios::binary);
}

int choose(int turn, const PlayerState &me, const PlayerState &them, int maxBalls) {
	std::ifstream fs;
	open_file(fs, turn, std::max(me.ducks, them.ducks), maxBalls);

	unsigned char balls = fs.get();
	unsigned char ducks = fs.get();
	fs.seekg(GameStore(balls, ducks).fileIndex(me, them));
	unsigned char p0 = fs.get();
	unsigned char p1 = fs.get();
	fs.close();

	// only 1 random number per execution; no need to seed a PRNG
	std::random_device rand;
	int v = std::uniform_int_distribution<int>(0, 254)(rand);
	if(v < p0) {
		return 0;
	} else if(v < p1) {
		return 1;
	} else {
		return 2;
	}
}

void debugFilePos(std::size_t pos) {
	std::ifstream fs(GameStore::filename(0).c_str(), std::ios::binary);
	unsigned char balls = fs.get();
	unsigned char ducks = fs.get();

	auto state = GameStore(balls, ducks).stateFromFileIndex(pos);
	std::cout
		<< "Me:   balls = " << state.first.balls
		<< ", ducks = " << state.first.ducks << std::endl
		<< "Them: balls = " << state.second.balls
		<< ", ducks = " << state.second.ducks << std::endl;
}

int main(int argc, const char *const *argv) {
	if(argc == 1) {
		test();
		return 0;
	}

	if(argc == 2) { // game state
		debugFilePos(atoi(argv[1]));
		return 0;
	}

	if(argc == 4) { // maxTurns, maxBalls, maxDucks
		Generator(atoi(argv[2]), atoi(argv[3])).generate(atoi(argv[1]), true, true);
		return 0;
	}

	if(argc == 7) { // turn, meBalls, themBalls, meDucks, themDucks, maxBalls
		std::cout << choose(
			atoi(argv[1]),
			PlayerState(atoi(argv[2]), atoi(argv[4])),
			PlayerState(atoi(argv[3]), atoi(argv[5])),
			atoi(argv[6])
		) << std::endl;
		return 0;
	}

	return 1;
}
