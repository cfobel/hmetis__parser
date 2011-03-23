#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

%%{
	machine ParseVPR;

	action start_word {
		ws[nwords] = fpc;
	}

	action end_word {
		we[nwords++] = fpc;
	}

	action start_line {
		ls = fpc;
		ts = fpc;
	}

	action end_input {
		printf("found input: ");
		fwrite( ls, 1, p - ls, stdout );
		printf("\n");
	}

	action end_output {
		printf("found output: ");
		fwrite( ls, 1, p - ls, stdout );
		printf("\n");
	}

	action end_clb {
        if(ts != be) {
            cout << "We are continuing a previous token" << endl;
            ls = buf;
        }
		printf("found clb: (%d)\n", (te - ls));
		fwrite( ls, 1, p - ls, stdout );
		printf("\n");
        ts = 0;
	}

	action end_global {
		printf("found global: ");
		fwrite( ls, 1, p - ls, stdout );
		printf("\n");
	}

	action end_block {
        be = fpc;
        te = fpc;
	}

	# Words in a line.
	word = ^[ \t\n]+;

	# The whitespace separating words in a line.
	whitespace = [ \t];

	label = (alnum | [\[\]_:])+;
	paddedlabel = whitespace+ label whitespace*;
	pinlabel = 'open' | label;

	emptyline = ( whitespace* '\n');
	pins = whitespace+ pinlabel ('\\\n' | whitespace | pinlabel)**;
	pinlist = ( 'pinlist:' pins );
	subblock = ( 'subblock:' pins );
    comment = ( '#' (whitespace* word)** );
    endofline = ( comment? whitespace* '\n' );

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

%% write data nofinal;

#define MAXWORDS 256
#define BUFSIZE 512
char buf[BUFSIZE];

int main()
{
	int i, nwords = 0;
	char *ls = 0;
	char *ts, *te = 0;
	char *be = 0;
	char *ws[MAXWORDS];
	char *we[MAXWORDS];

	int cs;
	int have = 0;

	%% write init;

    bool done = false;
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
        cin.read( p, space );
        int len = cin.gcount();
        char *pe = p + len;
        char *eof = 0;

        /* If no data was read indicate EOF. */
        if ( len == 0 ) {
            eof = pe;
            done = true;
        } else {
            cout << "Executing..." << endl;
            cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
            fwrite( buf, 1, pe - buf, stdout );
            cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
            %% write exec;

            if ( cs == ParseVPR_error ) {
                /* Machine failed before finding a token. */
                cerr << "PARSE ERROR" << endl;
                exit(1);
            }
            if ( ts == 0 ) {
                cout << "Have none left..." << endl;
                have = 0;
            } else {
                /* There is a prefix to preserve, shift it over. */
                have = pe - ts;
                cout << "Have some left: " << have << endl;
                memmove( buf, ts, have );
                te = buf + (te-ts);
                ts = buf;
            }
        }
    }
}
