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
    ifstream &in_stream;
    partition_t partition;
    int vertex_count;
    int min_vertex;
    int max_vertex;
public:
    PartitionReader(ifstream &in_stream) : in_stream(in_stream), vertex_count(0) {}

    partition_t get_partition() {
        min_vertex = 999999999;
        max_vertex = -999999999;
        vertex_count = 0;
        partition = partition_t();
        int set_id;
        while(in_stream >> set_id) {
            process_vertex(set_id, vertex_count);
        }
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
            partition.resize(set_id + 1);
        }
        vertex_list_t &vertices = partition[set_id];
        vertices.push_back(vertex_id);
        min_vertex = min(min_vertex, vertex_id);
        max_vertex = max(max_vertex, vertex_id);
    }
};

#endif
