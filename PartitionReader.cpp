#include <fstream>
#include <iostream>
#include <iterator>
#include <boost/format.hpp>
#include "PartitionReader.hpp"

using namespace std;

int main() {
    string input_path("test.hmt.part.4");
    ifstream input(input_path.c_str());
    PartitionReader pr(input);
    PartitionReader::partition_t p = pr.get_partition();

    for(int set_id = 0; set_id < p.size(); set_id++) {
        PartitionReader::vertex_list_t &vertices = p[set_id];
        cout << boost::format("[%d] ") % set_id;
        copy(vertices.begin(), vertices.end(),
                ostream_iterator<int>(cout, ", "));
        cout << endl;
    }

    return 0;
}
