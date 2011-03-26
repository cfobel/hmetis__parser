#include "VPRNetParser.hpp"
#include "BlockHandler.hpp"
#include <fstream>

BlockHandler count_pass(VPRNetParser &p, ifstream &file1) {
    BlockHandler b;

	p.init();
    p.register_input_process_func(boost::bind(&BlockHandler::process_input, &b, _1, _2));
    p.register_output_process_func(boost::bind(&BlockHandler::process_output, &b, _1, _2));
    p.register_funcblock_process_func(boost::bind(&BlockHandler::process_funcblock, &b, _1, _2, _3, _4));
    p.register_global_process_func(boost::bind(&BlockHandler::process_global, &b, _1));
    p.parse(file1);

    cout << "------------------------------------------------------------------------" << endl;
    cout << "PASS 1:" << endl;
    // If we had simply passed "b" to bind above, all of the following counts
    // would be zero, since the state of instance "b" would remain unchanged.
    cout << "  Funcblock count:    " << b.funcblock_count << endl;
    cout << "  Input count:        " << b.input_count << endl;
    cout << "  Output count:       " << b.output_count << endl;
    cout << "  Global count:       " << b.global_count << endl;

    return b;
}

VerboseHandler verbose_pass(VPRNetParser &p, ifstream &file1) {
    VerboseHandler v;

    p.register_input_process_func(boost::bind(&VerboseHandler::process_input, &v, _1, _2));
    p.register_output_process_func(boost::bind(&VerboseHandler::process_output, &v, _1, _2));
    p.register_funcblock_process_func(boost::bind(&VerboseHandler::process_funcblock, &v, _1, _2, _3, _4));
    p.register_global_process_func(boost::bind(&VerboseHandler::process_global, &v, _1));

    cout << "------------------------------------------------------------------------" << endl;
    cout << "PASS 2:" << endl;
    p.parse(file1);

    return v;
}


int main(int argc, char** argv) {
    VPRNetParser parser(32 << 10);

    if(argc != 2) {
        cerr << "Invalid number of arguments." << endl;
        cerr << "usage: " << argv[0] << " <benchmark path>" << endl;
        exit(-1);
    }

    ifstream file1(argv[1]);

    BlockHandler count_b = count_pass(parser, file1);

    file1.clear();
    file1.seekg(0, ios::beg);

    VerboseHandler verbose_b = verbose_pass(parser, file1);
    file1.close();
	return 0;
}
