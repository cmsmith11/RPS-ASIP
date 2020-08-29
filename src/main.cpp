/************************************************************************************
* Charlie Smith
* rps_proj/scripts/run.sh
* Artificial intelligent Rock, Paper, Scissors program. Learns play style as it
* goes. Saves history in a file, so can resume play across multiple sessions. 
* Enable debug print statements with -D flag and 'fairplay mode' with -P flag.
************************************************************************************/
#include <iostream>
#include <fstream>
#include <string>
#include <deque>
#include <utility>
#include <list>
#include <cmath>
#include <time.h>
#include <unistd.h>
using namespace std;

// options flags
bool DEBUG = false;
bool FAIRPLAY = false;
bool GRAPHING = false;

// Tuning values            /* <--earlier...recent--> */
static float LOCAL_DIST[] = {0.1, 0.15, 0.25, 0.25, 0.25};
static float WEIGHT[] = {0, 0.8, 0.2}; // {time, their move, my move}

enum {END, ROCK, PAPER, SCISSORS};
static deque<float*> move_deque;
static int rel_hist_count = 10;
static int dec_factor = 1;

void process_flags(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-D"))
            DEBUG = true;
        else if (!strcmp(argv[i], "-P"))
            FAIRPLAY = true;
        else if (!strcmp(argv[i], "-G"))
            GRAPHING = true;
        else if (!strcmp(argv[i], "-R"))
            NULL; // no var set needed
        else {
            cerr << "usage: ./run.sh [-DGhPR]" << endl;
            cerr << "\t-D:\tRun with debug print statements" << endl;
            cerr << "\t-G:\tRun with graphing progress. Must have Matplotlib to work." << endl;
            cerr << "\t-h, --help:\n\t\tPrint this help message." << endl;
            cerr << "\t-P:\tRun in 'Fairplay' mode." << endl;
            cerr << "\t-R:\tUse random move generator, Randy, to play against CPU." << endl;
            exit(0);
        }
    }
}

void setup(fstream *file)
{
    string name;
    cerr << "Enter username: ";
    cin >> name;
    file->open("data/" + name + "_hist.txt");
    if (!file->is_open()) {
        file->open("data/" + name + "_hist.txt", ios::out);
        if (!file->is_open()) {
            cerr << "Error opening file for: " << name << endl;
            exit(EXIT_FAILURE);
        }
        cerr << "Welcome new user, " << name << "!" << endl;
    } else {
        cerr << "Welcome back, " << name << "!" << endl;
    }
    if (GRAPHING) cout << name << endl;
}

int str_to_arr(string str, float *arr) {
    if (str.length() < 5) // arbitrary
        return 0;
    // assume form of line: "132.2903 1 2", for ex.
    size_t idx1, idx2;
    arr[0] = stof(str, &idx1); // time
    arr[1] = stof(str.substr(idx1), &idx2); // their move
    arr[2] = stof(str.substr(idx1 + idx2)); // my move
    return 1;
}

// loss = 0, tie = 1, win = 2
int ltw(int them, int me) {
    if (them == me) return 1;
    return (them - 1) == (me % 3) ? 2 : 0;
}

void parse_file(fstream *file, int *rec, list<int> *prog) {
    file->seekg(0, ios::beg);
    string line;
    while (getline(*file, line)) {
        // add move to deque
        int size = sizeof(WEIGHT) / sizeof(WEIGHT[0]);
        float *move = (float *) malloc(size * sizeof(float));
        if (str_to_arr(line, move)) {
            if (move[1]) {
                rec[ltw(move[1], move[2])]++;
                int new_score = prog->back() + ltw(move[1], move[2]) - 1;
                prog->push_back(new_score);
                if (GRAPHING) cout << new_score << endl;
            }
            move_deque.push_back(move);
        }
    }
    file->clear();
    int tot = rec[0] + rec[1] + rec[2];
    if (!tot) return;
    cerr << "User's play record: " << endl;
    cerr << "   wins:   " << rec[2] << " = " << float(rec[2])/tot*100 << "%" << endl;
    cerr << "   losses: " << rec[0] << " = " << float(rec[0])/tot*100 << "%" << endl;
    cerr << "   ties:   " << rec[1] << " = " << float(rec[1])/tot*100 << "%" << endl;
}

// [1, 0, 2, 1, 0, 3, 1, 2]->[0, 0, 0, 0, 0, 3, 1, 2]
// by returning index = 4
int dummy_cuttoff(int beg) {
    int length = sizeof(LOCAL_DIST) / sizeof(LOCAL_DIST[0]);
    for (int i = beg + length - 1; i >= beg; i--) {
        if (i < 0 || move_deque[i][1] == 0)
            return i;
    }
    return -1;
}

float global_decay_sim(float sim, int moves_ago) {
    // not an exact science here
    float new_sim = moves_ago < rel_hist_count ? sim : 0;
    return new_sim;
}

float calculate_sim(int test, int *nxt) {
    float sim;
    int length = sizeof(LOCAL_DIST) / sizeof(LOCAL_DIST[0]);
    int curr = move_deque.size() - length;
    int i;
    float dummy[] = {0, 0, 0};
    for (i = 0; i < length; i++) {
        float *past = (test + i) > dummy_cuttoff(test) ? move_deque[test + i] : dummy;
        float *pres = (curr + i) > dummy_cuttoff(curr) ? move_deque[curr + i] : dummy;
        // calc per move
        float time_sim = 1 / (1 + abs(past[0] - pres[0]));
        if (time_sim == 1) time_sim = 0; // when time is forced to 0 for both
        float their_sim = (past[1] == pres[1] ? 1 : 0);
        float my_sim = (past[2] == pres[2] ? 1 : 0);
        float local_sim = WEIGHT[0]*time_sim + WEIGHT[1]*their_sim + WEIGHT[2]*my_sim;
        sim += local_sim * LOCAL_DIST[i];
        if (DEBUG) cerr << "        [" << past[0] << ", " << past[1] << ", " << past[2] << "] - ";
        if (DEBUG) cerr << "[" << pres[0] << ", " << pres[1] << ", " << pres[2] << "] = ";
        if (DEBUG) cerr << local_sim << endl;
    }
    *nxt = (int) move_deque[test + i][1];
    if (DEBUG) cerr << "        nxt: " << move_deque[test + i][0] << " -> " << move_deque[test + i][1] << endl;
    // apply global decay
    sim = global_decay_sim(sim, move_deque.size() - (test + length + 1));
    return sim;
}

bool compare_pairs(pair<float,int> &first, pair<float,int> &second) {
    return first.first > second.first; // sort greatest to least
}

int weighted_vote(list< pair<float,int> > *sm_ptr) {
    float votes[3]; // hold r, p, & s votes
    votes[0] = votes[1] = votes[2] = 0;
    int tot_votes = sm_ptr->size() < rel_hist_count ? sm_ptr->size() : rel_hist_count;
    while (!sm_ptr->empty()) {
        votes[sm_ptr->front().second - 1] += sm_ptr->front().first / tot_votes;
        sm_ptr->pop_front();
    }

    int vote = SCISSORS; // default pred
    float max = votes[SCISSORS - 1];
    for (int i = 0; i < sizeof(votes)/sizeof(votes[0]); i++) {
        if (DEBUG) cerr << "Vote for " << i + 1 << ": " << votes[i] << endl;
        if (votes[i] > max) {
            max = votes[i];
            vote = i + 1;
        }
    }
    return vote;
}

int generate_move() {
    if (DEBUG) cerr << "GENERATING MOVE: move_deque.size() = " << move_deque.size() << endl;
    list< pair<float,int> > sim_movs;
    int loc_dist_len = sizeof(LOCAL_DIST) / sizeof(LOCAL_DIST[0]);
    if (DEBUG) cerr << "    for i = [" << -loc_dist_len << ", " << (int)(move_deque.size()-loc_dist_len) << ")" << endl;
    for (int i = -loc_dist_len; i < (int)(move_deque.size() - loc_dist_len); i++) {
        if (DEBUG) cerr << "    Loop, index = " << i << endl;
        int nxt;
        float sim = calculate_sim(i, &nxt);
        if (DEBUG) cerr << "    Tot. Sim: " << sim << endl;
        pair<float,int> *new_pair = (pair<float,int> *) malloc(sizeof(pair<float,int>));
        *new_pair = make_pair(sim, nxt);
        sim_movs.push_back(*new_pair);
    }
    if (DEBUG) cerr << "rel_hist_count = " << rel_hist_count << endl;
    sim_movs.sort(compare_pairs);
    int pred = weighted_vote(&sim_movs);
    if (DEBUG) cerr << "Pred: " << pred << endl;
    // now beat pred
    return (pred == ROCK) ? PAPER : ((pred == PAPER) ? SCISSORS : ROCK);
}

string mvtostr(int mv) {
    string str[] = {"END", "ROCK", "PAPER", "SCISSORS"};
    return str[mv];
}

int obtain_move(int my_move, float *move_time) {
    cerr << "ROCK(1), PAPER(2), SCISSORS(3), SHOOT!: ";
    int their_move_int;
    bool first_nonmv = true;
    while(true) {
        string their_move;
        chrono::system_clock::time_point start = chrono::system_clock::now();
        cin >> their_move;
        chrono::system_clock::time_point end = chrono::system_clock::now();
        chrono::milliseconds dtn = chrono::duration_cast<chrono::milliseconds>(end - start);
        *move_time = ((float) dtn.count()) / 1000;
        try {
            their_move_int = stoi(their_move);
            if (their_move_int < 0 || their_move_int > 3)
                int dumb = stoi("bad code design"); // designed to print error message and continue loop
            break;
        } catch(invalid_argument e) {
            if (!FAIRPLAY || (FAIRPLAY && !first_nonmv)) {
                cerr << "Press (1) for Rock, (2) for Paper, (3) for Scissors, or (0) to end game." << endl;
                if (FAIRPLAY)
                    cerr << "Press any other non whitespace key to view CPU's move." << endl;
            } else {
                cerr << "CPU: " << mvtostr(my_move) << endl;
                first_nonmv = false;
            }
            continue;
        }
    }
    return their_move_int;
}

string outcome(int you, int me) {
    if (me == you) {
        rel_hist_count += 2;
        dec_factor / 2 > 0 ? dec_factor /= 2 : dec_factor = 1;
        return "TIE!";
    }
    if (me - you == 1 || me - you == -2) {
        rel_hist_count += 2; // slow growth
        dec_factor / 2 > 0 ? dec_factor /= 2 : dec_factor = 1;
        return "LOSE!";
    }
    int len = sizeof(LOCAL_DIST) / sizeof(LOCAL_DIST[0]);
    rel_hist_count - dec_factor > len ? rel_hist_count -= dec_factor : rel_hist_count = len;
    dec_factor *= 2;
    return "WIN!";
}

int rec_hist(string *str, float time, int their_move, int my_move) {
    if (!their_move) my_move = 0;
    string new_line = to_string(time) + " " + to_string(their_move) + " " + to_string(my_move) + "\n";
    if (DEBUG) cerr << "Adding line of hist!:\n" << new_line;
    *str += new_line;
    
    // add move to deque
    int size = sizeof(WEIGHT) / sizeof(WEIGHT[0]);
    float *move = (float *) malloc(size * sizeof(float));
    if (str_to_arr(new_line, move))
        move_deque.push_back(move);

    cerr << "You: " << mvtostr(their_move) << endl;
    cerr << "CPU: " << mvtostr(my_move) << endl;
    cerr << "*** YOU " << outcome(their_move, my_move) << " ***\n\n";
    return their_move;
}

void play_rps(string *new_hist, list<int> *prog) {
    int another_round = 1;
    float time;
    while (another_round) {
        int my_move = generate_move();
        int their_move = obtain_move(my_move, &time);
        int new_score = prog->back() + ltw(their_move, my_move) - 1;
        prog->push_back(new_score);
        if (GRAPHING)
            cout << new_score << endl;
        another_round = rec_hist(new_hist, time, their_move, my_move);
    }
}

void save_hist(fstream *file, string *new_hist) {
    file->seekp(0, ios::end);
    (*file) << *new_hist;
    if (DEBUG) cerr << "Just added this:" << "\n" << *new_hist << endl;
}

int main(int argc, char *argv[])
{
    fstream file;
    int record[3];
    list<int> progress;
    
    // parse flags
    process_flags(argc, argv);
    if (DEBUG) cerr << "rps away! -- DEBUG ON --" << endl;
    
    // prologue
    setup(&file);

    // parse file
    parse_file(&file, record, &progress); // after this point, move_deque is populated

    // begin play
    string new_hist;
    play_rps(&new_hist, &progress);

    // write and save play hist
    save_hist(&file, &new_hist);

    file.close();
    if (DEBUG) cerr << "done" << endl;
    return 0;
}
