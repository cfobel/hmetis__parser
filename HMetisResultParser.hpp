#ifndef ___H_METIS_RESULT_PARSER__HPP___
#define ___H_METIS_RESULT_PARSER__HPP___

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#include <stdlib.h>

using namespace std;

#define DEF_BUFSIZE 8192

typedef boost::function<void (int vertex_id, int vertex_count)> 
        vertex_process_func_t;

class HMetisResultParser {
    vector<char> buf_vector;
    char* buf;
    int _BUFSIZE;

	const char *ls;
	const char *ts;
	const char *te;
	const char *be;
	const char *vertex_id_start;

	int cs;
	int have;
	int length;

    int vertex_count;
    int vertex_id;
    vertex_process_func_t vertex_process_func;
public:
    HMetisResultParser() {
        buf_vector = vector<char>(DEF_BUFSIZE);
    }
    HMetisResultParser(int buffer_size) {
        buf_vector = vector<char>(buffer_size);
    }
	void init();
	void parse(std::istream &in_stream);

    void register_vertex_process_func(vertex_process_func_t fun) {
        vertex_process_func = fun;
    }
    void process_vertex() {
        if(vertex_process_func) vertex_process_func(vertex_id, vertex_count); 
    }
};

#endif
