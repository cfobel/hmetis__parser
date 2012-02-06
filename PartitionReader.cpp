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
    pr.get_partition();

#if 0
    cout << boost::format("vertex %d is in set %d")
        % vertex_count % vertex_id
        << endl;
    copy(vertices.begin(), vertices.end(),
            ostream_iterator<int>(cout, ", "));
    cout << endl;
#endif

    return 0;
}
