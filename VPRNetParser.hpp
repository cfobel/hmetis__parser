#ifndef ___VPR_NET_PARSER__HPP___
#define ___VPR_NET_PARSER__HPP___

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#include <stdlib.h>

using namespace std;

#define DEF_BUFSIZE 8192

typedef boost::function<void (const string &label, const vector<string> &pins)> 
            process_func_t;
typedef boost::function<void (const string &label, const vector<string> &pins,
                                        const vector< vector<string> > &subblocks)> 
            clb_process_func_t;

class VPRNetParser {
    vector<char> buf_vector;
    char* buf;
    int BUFSIZE;

	const char *ls;
	const char *ts;
	const char *te;
	const char *be;
	const char *pin_start;
	const char *label_start;

	int cs;
	int have;
	int length;

    vector<string> clb_pin_list;
    vector<string> *p_subblock_pin_list;
    vector<vector<string> > subblocks;
    string label;
    bool in_pin_list;
    bool in_subblock_pin_list;
    clb_process_func_t clb_process_func;
    process_func_t input_process_func;
    process_func_t output_process_func;

public:
    VPRNetParser() {
        buf_vector = vector<char>(DEF_BUFSIZE);
    }
    VPRNetParser(int buffer_size) {
        buf_vector = vector<char>(buffer_size);
    }
	void init();
	void parse(std::istream &in_stream);

    void register_input_process_func(process_func_t fun) {
        input_process_func = fun; }
    void register_output_process_func(process_func_t fun) { 
        output_process_func = fun; }
    void register_clb_process_func(clb_process_func_t fun) { 
        clb_process_func = fun; }

    void process_input() { input_process_func(label, clb_pin_list); }
    void process_output() { output_process_func(label, clb_pin_list); }
    void process_clb() { 
        clb_process_func(label, clb_pin_list, subblocks); 
    }
    void process_global() { cout << "global: " << label << endl; }
};

#endif
