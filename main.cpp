#include "VPRNetParser.hpp"
#include "BlockHandler.hpp"
#include <fstream>

void count_pass(VPRNetParser &p, ifstream &file1,
                int &net_count, int &block_count) {
    CountPass b;

    b.parse(p, file1);

    cout << "------------------------------------------------------------------------" << endl;
    cout << "PASS 1:" << endl;
    // If we had simply passed "b" to bind above, all of the following counts
    // would be zero, since the state of instance "b" would remain unchanged.
    cout << "  Global count:       " << b.global_count << endl;

    cout << "  Funcblock count:    " << b.funcblock_count << endl;
    cout << "  Input count:        " << b.input_count << endl;
    cout << "  Output count:       " << b.output_count << endl;

    block_count = b.funcblock_count + b.input_count + b.output_count;
    net_count = b.net_count;

    cout << "  --------------------------" << endl;
    cout << "  Block count:        " 
        << b.funcblock_count + b.input_count + b.output_count
        << endl;
    cout << "  Net count:          " << b.net_count << endl;
}

void load_pass(VPRNetParser &p, ifstream &file1,
                int net_count, int block_count, 
                struct parse_data_t &d) {
    LoadPass v(net_count, block_count);
    cout << "------------------------------------------------------------------------" << endl;
    cout << "PASS 2:" << endl;
    v.parse(p, file1, d);
    v.display_contents();
}


void map_pass(VPRNetParser &p, ifstream &file1,
                int net_count, int block_count, 
                struct parse_data_t &d) {
    MapPass v(net_count, block_count, d);
    cout << "------------------------------------------------------------------------" << endl;
    cout << "PASS 3:" << endl;
    v.parse(p, file1);
    v.display_contents();
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
    int net_count, block_count;
    count_pass(parser, file1, net_count, block_count);


    file1.clear();
    file1.seekg(0, ios::beg);

    struct parse_data_t d;
    load_pass(parser, file1, net_count, block_count, d);

    file1.clear();
    file1.seekg(0, ios::beg);

    map_pass(parser, file1, net_count, block_count, d);


    file1.close();
	return 0;
}
