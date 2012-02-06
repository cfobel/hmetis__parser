#include "HMetisResultParser.hpp"

%%{
	machine HMetisResultParser;

	action start_parse {
        vertex_count = 0;
	}

	action start_line {
		ls = fpc;
		ts = fpc;
	}

    action start_vertex_id {
        vertex_id_start = fpc;
    }

    action end_vertex_id {
        length = fpc - vertex_id_start;
        if(length < 0) {
            vertex_id_start = buf + (vertex_id_start - be);
        }
        vertex_id = boost::lexical_cast<int>(
                string(vertex_id_start, fpc - vertex_id_start));
    }

    action end_vertex {
        process_vertex();
        vertex_count++;
    }

	# Words in a line.
	word = ^[ \t\n]+;

	# The whitespace separating words in a line.
	whitespace = [ \t];

    comment = ( '#' (whitespace* word)** );
    endofline = ( comment? whitespace* '\n' );
	emptyline = whitespace* endofline;
    vertex_id = (digit+) >start_vertex_id %end_vertex_id;

	vertex = ( vertex_id endofline ) >start_line %end_vertex;

	# Any number of lines.
	main := (emptyline | vertex)+ >start_parse;
}%%


/* Regal data ****************************************/
%% write data nofinal;
/* Regal data: end ***********************************/


void HMetisResultParser::init() {
    buf = &buf_vector[0];
    _BUFSIZE = buf_vector.size();

	%% write init;
}


void HMetisResultParser::parse(std::istream &in_stream) {
    bool done = false;
    int i = 0;
    have = 0;
    while ( !done ) {
        /* How much space is in the buffer? */
        int space = _BUFSIZE - have;
        if ( space == 0 ) {
            /* Buffer is full. */
            cerr << "TOKEN TOO BIG" << endl;
            exit(1);
        }
        /* Read in a block after any data we already have. */
        char *p = buf + have;
        in_stream.read( p, space );
        int len = in_stream.gcount();
        char *pe = p + len;
        char *eof = 0;

        /* If no data was read indicate EOF. */
        if ( len == 0 ) {
            eof = pe;
            done = true;
        } else {
            %% write exec;

            if ( cs == HMetisResultParser_error ) {
                /* Machine failed before finding a token. */
                cerr << "PARSE ERROR" << endl;
                exit(1);
            }
            if ( ts == 0 ) {
                have = 0;
            } else {
                /* There is a prefix to preserve, shift it over. */
                have = pe - ts;
                memmove( buf, ts, have );
                te = buf + (te-ts);
                ts = buf;
            }
        }
    }
}
