#include "VPRNetParser.hpp"

%%{
	machine VPRNetParser;

	action start_line {
		ls = fpc;
		ts = fpc;
	}

    action end_global {
        process_global();
    }

    action start_subblock {
        subblocks.push_back(SubBlock());
        p_subblock = &subblocks[subblocks.size() - 1];
        in_subblock_pin_list = true;
    }

    action end_subblock {
        in_subblock_pin_list = false;
        int num_pins = p_subblock->pins.size();
        p_subblock->clock_pin = p_subblock->pins[num_pins - 1];
        p_subblock->pins.resize(num_pins - 1);
    }

	action end_input {
        process_input();
	}

	action end_output {
        process_output();
	}

	action start_funcblocktype {
        label_start = fpc;
    }
    
    action end_funcblocktype {
        length = fpc - label_start;
        if(length < 0) {
            label_start = buf + (label_start - be);
        }
        funcblocktype = string(label_start, fpc - label_start);
    }

	action start_funcblock {
        subblocks.clear();
        ls = fpc;
        ts = fpc;
    }

	action end_funcblock {
        if(ts != be) {
            ls = buf;
        }
        ts = 0;
        process_funcblock();
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
        if(in_pin_list) {
            pin_list.push_back( 
                string(pin_start, fpc - pin_start));
        } else if(in_subblock_pin_list) {
            p_subblock->pins.push_back( 
                string(pin_start, fpc - pin_start));
        }
    }

    action start_subblock_label {
        subblock_label_start = fpc;
    }
    
    action end_subblock_label {
        length = fpc - subblock_label_start;
        if(length < 0) {
            subblock_label_start = buf + (subblock_label_start - be);
        }
        p_subblock->label = string(subblock_label_start, fpc - subblock_label_start);
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
    label = (('\\' label_char) | label_char - '\\') (label_char)* $1 %0;
    funcblocktype = ('.' label) >start_funcblocktype %end_funcblocktype;
    block_label = label >start_label %end_label;
    subblocklabel = label >start_subblock_label %end_subblock_label;
	paddedlabel = whitespace+ block_label whitespace*;
	paddedsubblocklabel = whitespace+ subblocklabel;
	pinlabel = (label) >start_pin %end_pin;

    separator = ('\\\n' | whitespace);

    pins = separator+ pinlabel (separator+ | pinlabel)* $1 %0;
	pinlist = ( 'pinlist:' pins ) >start_pinlist %end_pinlist;
	subblock = ( 'subblock:' paddedsubblocklabel pins ) >start_subblock %end_subblock;
    comment = ( '#' (whitespace* word)** );
    endofline = ( comment? whitespace* '\n' );
	emptyline = whitespace* endofline;

	global =     ( '.global' paddedlabel endofline )
                >start_line %end_global;
	input =      ( '.input'  paddedlabel endofline 
                        whitespace* pinlist endofline ) 
                >start_line %end_input;
	output =     ( '.output' paddedlabel endofline 
                        whitespace* pinlist endofline ) 
                >start_line %end_output;
	funcblock = ( funcblocktype paddedlabel endofline 
                        whitespace* pinlist endofline 
                        (whitespace* subblock endofline)+ ) 
                >start_funcblock %end_funcblock;

	# Any number of lines.
	main := (emptyline %end_block | global | input | output | funcblock)+;
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
