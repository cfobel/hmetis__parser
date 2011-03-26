#include "VPRNetParser.hpp"
#include "BlockHandler.hpp"
#include <fstream>

void count_pass(VPRNetParser &p, ifstream &file1) {
    BlockHandler b;

    b.parse(p, file1);

    cout << "------------------------------------------------------------------------" << endl;
    cout << "PASS 1:" << endl;
    // If we had simply passed "b" to bind above, all of the following counts
    // would be zero, since the state of instance "b" would remain unchanged.
    cout << "  Funcblock count:    " << b.funcblock_count << endl;
    cout << "  Input count:        " << b.input_count << endl;
    cout << "  Output count:       " << b.output_count << endl;
    cout << "  Global count:       " << b.global_count << endl;
}

void verbose_pass(VPRNetParser &p, ifstream &file1) {
    VerboseHandler v;
    cout << "------------------------------------------------------------------------" << endl;
    cout << "PASS 2:" << endl;
    v.parse(p, file1);
}


int main(int argc, char** argv) {
    VPRNetParser parser(32 << 10);

    if(argc != 2) {
        cerr << "Invalid number of arguments." << endl;
        cerr << "usage: " << argv[0] << " <benchmark path>" << endl;
        exit(-1);
    }

    ifstream file1(argv[1]);

    // Count number of blocks and nets
    count_pass(parser, file1);


    file1.clear();
    file1.seekg(0, ios::beg);

    verbose_pass(parser, file1);
    file1.close();
	return 0;
}
