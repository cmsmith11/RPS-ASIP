/*********************************************************************
* rand_gen.cpp
* Charlie Smith
* Generates a random 'move' (1, 2, or 3) and prints to stdout
*********************************************************************/

#include <iostream>
#include <random>
using namespace std;

int main(int argc, char *argv[]) {
    random_device rand;
    int num_rounds;
    cout << "Randy" << endl;
    if (argc != 2) {
        num_rounds = 10;
    } else {
        try {
            num_rounds = stoi(argv[1]);
        } catch(invalid_argument e) {
            cerr << "usage: ./rand_gen [num rounds]" << endl;
            cerr << "Please specify number of rounds to simulate" << endl;
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < num_rounds; i++) {
        cout << (rand() % 3) + 1 << endl;
    }
    cout << 0 << endl;
    return 0;
}
