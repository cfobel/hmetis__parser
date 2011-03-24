#include "VPRNetParser.hpp"

%%{
	machine VPRNetParser;

	action start_line {
		ls = fpc;
		ts = fpc;
	}

	action end_input {
        process_input();
	}

	action end_output {
        process_output();
	}

	action end_clb {
        if(ts != be) {
            ls = buf;
        }
        ts = 0;
        process_clb();
	}

	action end_block {
        be = fpc;
        te = fpc;
	}
    
    action start_pinlist {
        pin_list.clear();
        in_pin_list = true;
    }
    
    action end_pinlist {
        in_pin_list = false;
    }
    
    action start_pin {
        pin_start = fpc;
    }
    
    action end_pin {
        length = fpc - pin_start;
        if(length < 0) {
            pin_start = buf + (pin_start - be);
        }
        if(in_pin_list) pin_list.push_back(string(pin_start, fpc - pin_start));
    }

    action start_label {
        label_start = fpc;
    }
    
    action end_label {
        length = fpc - label_start;
        if(length < 0) {
            label_start = buf + (label_start - be);
        }
        label = string(label_start, fpc - label_start);
    }

	# Words in a line.
	word = ^[ \t\n]+;

	# The whitespace separating words in a line.
	whitespace = [ \t];

	label_char = alnum | [\[\]_:\\];
	label = label_char (label_char)* $1 %0;
    block_label = label >start_label %end_label;
	paddedlabel = whitespace+ block_label whitespace*;
	properlabel = (label - "open") >start_pin %end_pin;
	pinlabel = ("open" | properlabel);

	pins = whitespace+ pinlabel ('\\\n' | whitespace+ | pinlabel)* $1 %0;
	pinlist = ( 'pinlist:' pins ) >start_pinlist %end_pinlist;
	subblock = ( 'subblock:' pins );
    comment = ( '#' (whitespace* word)** );
    endofline = ( comment? whitespace* '\n' );
	emptyline = whitespace* endofline;

	global =     ( '.global' paddedlabel endofline );
	input =      ( '.input'  paddedlabel endofline 
                        whitespace* pinlist endofline ) 
                >start_line %end_input;
	output =     ( '.output' paddedlabel endofline 
                        whitespace* pinlist endofline ) 
                >start_line %end_output;
	logicblock = ( '.clb'    paddedlabel endofline 
                        whitespace* pinlist endofline 
                        (whitespace* subblock endofline)+ ) 
                >start_line %end_clb;

	# Any number of lines.
	main := (emptyline %end_block | global | input | output | logicblock)+;
}%%


/* Regal data ****************************************/
%% write data nofinal;
/* Regal data: end ***********************************/


void VPRNetParser::init() {
    buf = &buf_vector[0];
    BUFSIZE = buf_vector.size();

	%% write init;
}


void VPRNetParser::parse(std::istream &in_stream) {
    bool done = false;
    int i = 0;
    have = 0;
    while ( !done ) {
        /* How much space is in the buffer? */
        int space = BUFSIZE - have;
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

            if ( cs == VPRNetParser_error ) {
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