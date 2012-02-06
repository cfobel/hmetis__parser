#ifndef ___PARTITION_READER__HPP___
#define ___PARTITION_READER__HPP___

#include <vector>
#include <fstream>
#include <boost/format.hpp>
#include "HMetisResultParser.hpp"

using namespace std;

class PartitionReader {
    HMetisResultParser p;
    ifstream &in_stream;

    typedef vector<int> vertex_list_t;
    vector<vertex_list_t> partition;
public:
    PartitionReader(ifstream &in_stream) : in_stream(in_stream) {
        p = HMetisResultParser(32 << 10);
    }

    vector<vertex_list_t> get_partition() {
        vector<vertex_list_t> partition;
        p.init();
        p.register_vertex_process_func(boost::bind(&PartitionReader::process_vertex, this, partition, _1, _2));
        p.parse(in_stream);
        return partition;
    }

    void process_vertex(vector<vertex_list_t> &partition, int vertex_id, int vertex_count) {
        if(vertex_id + 1 > partition.size()) {
#if 0
            cout << boost::format("Resizing partition list: %d->%d")
                % partition.size() % (vertex_id + 1) << endl;
#endif
            partition.resize(vertex_id + 1);
        }
        vertex_list_t &vertices = partition[vertex_id];
        vertices.push_back(vertex_count);
    }
};

#endif
