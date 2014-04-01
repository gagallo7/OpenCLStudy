#!/usr/bin/python

import sys

def load_sections(filename):
    with open(filename, 'r') as infile:
        line = ''
        while True:
            while not line.startswith('root'):
                line = next(infile)  # raises StopIteration, ending the generator
                continue  # find next entry

            entry = {}
            i = 0
            for line in infile:
                line = line.strip()
                if not line: break

                key, value = map(str.strip, line.split(':', 2))
                value = line.split ( ' ', 5 )
                entry[i] = value[5].split ( ' ', 5 )[-1].split( ' ', 5 )[2]
                i += 1

            yield entry

toks = load_sections ( str ( sys.argv [1] ) ) 

for i in toks:
    print (i[0])
