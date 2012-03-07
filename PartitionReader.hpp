#ifndef ___PARTITION_READER__HPP___
#define ___PARTITION_READER__HPP___

#include <vector>
#include <fstream>
#include <boost/format.hpp>
#include "HMetisResultParser.hpp"

using namespace std;

class PartitionReader {
public:
    typedef vector<int> vertex_list_t;
    typedef vector<vertex_list_t> partition_t;
private:
    HMetisResultParser p;
    ifstream &in_stream;
    partition_t partition;
    int vertex_count;
    int min_vertex;
    int max_vertex;
public:
    PartitionReader(ifstream &in_stream) : in_stream(in_stream), vertex_count(0) {
        p = HMetisResultParser(32 << 10);
    }

    partition_t get_partition() {
        min_vertex = 999999999;
        max_vertex = -999999999;
        vertex_count = 0;
        partition = partition_t();
        p.init();
        p.register_vertex_process_func(boost::bind(&PartitionReader::process_vertex, this, _1, _2));
        p.parse(in_stream);
        cout << boost::format("[PartitionReader] vertex_count=%d min_vertex=%d max_vertex=%d")
            % vertex_count
            % min_vertex
            % max_vertex
            << endl;
        return partition;
    }

    void process_vertex(int set_id, int vertex_id) {
        vertex_count++;
        if(set_id + 1 > partition.size()) {
#if 0
            cout << boost::format("Resizing partition list: %d->%d")
                % partition.size() % (set_id + 1) << endl;
#endif
            partition.resize(set_id + 1);
        }
        vertex_list_t &vertices = partition[set_id];
        vertices.push_back(vertex_id);
        min_vertex = min(min_vertex, vertex_id);
        max_vertex = max(max_vertex, vertex_id);
    }
};

#endif
