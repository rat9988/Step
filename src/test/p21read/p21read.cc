/** \file p21read.cc
** 21-Jul-1995
** This code was developed with the support of the United States Government,
** and is not subject to copyright.

** This file contains a simple little program to read in a file and
** write it back out to a new file.  The purpose is to demonstrate the
** use of a Schema Class Library.  The name of the file to be read in
** can be supplied as a command line argument.  If no argument is
** provided, the program tries to read the file testfile.stp.  The
** name of the file to be output may also be provided, if no name is
** provided the file written out is called file.out
*/

extern void SchemaInit( class Registry & );
#include <STEPfile.h>
#include <sdai.h>
#include <STEPattribute.h>
#include <ExpDict.h>
#include <Registry.h>
#include <algorithm>
#include <string>
#include <unistd.h>

/**
 * Compare the schema names from the lib (generated by fedex_plus) and
 * the current exchange file. Both strings are converted to upper case
 * before comparison. If they are not identical, attempt to strip any
 * ASN.1 identifiers and compare again. Returns true for a match.
 */
bool compareOneSchName( std::string lib, std::string file ) {
    size_t b,e,ls,fs;
    b = lib.find_first_of('\'')+1;
    e = lib.find_last_of('\'');
    lib = lib.substr(b,e-b);
    std::transform(lib.begin(), lib.end(), lib.begin(), ::toupper);
    std::transform(file.begin(), file.end(), file.begin(), ::toupper);
    if(lib == file) {
        return true;
    }

    //There are no spaces, unless there is an ASN.1 identifier. If
    //the strings don't already match, try to remove this identifier.
    ls = lib.find_first_of(' ');
    fs = file.find_first_of(' ');
    if(  lib.substr(0,ls) == file.substr(0,fs) ) {
        return true;
    }
    std::cerr << "This pair of schema names do not match - " << lib << " and " << file << std::endl;
    return false;
}

/**
 * Loop through all available schemas and attributes, looking
 * for a match. If match not found, print error and exit(1)
 */
void checkSchemaName( Registry & reg, STEPfile & sf, bool ignoreErr ) {
    bool match = false;
    std::string sname;
    STEPattribute *attr;
    const Schema * sc;
    reg.ResetSchemas();
    //file id 3 is always the schema name
    SDAI_Application_instance *ai =
                    sf.HeaderInstances()->FindFileId(3)->GetApplication_instance();
    while( ( attr = ai->NextAttribute() ) ) {
        attr->asStr(sname);
        while( (sc = reg.NextSchema() ) ) {
            if( compareOneSchName(sname, sc->Name() ) ) {
                match = true;
                break;
            }
        }
        if(match) break;
    }
    if (!match) {
        std::cerr << "ERROR - schema name mismatch. Tried all available combinations." << std::endl;
        if( !ignoreErr ) {
            exit(1);
        }
    }
}

void printUse(const char * exe) {
    std::cout << "p21read - read a STEP Part 21 exchange file using SCL, and write the data to another file." << std::endl;
    std::cout << "Syntax:  " << exe << " [-i] infile [outfile]" << std::endl;
    std::cout << "Use '-i' to ignore a schema name mismatch." << std::endl;
    std::cout << "Use '--' as the last argument if a file name starts with a dash." << std::endl;
    exit( 1 );
}

int main( int argc, char * argv[] ) {
    bool ignoreErr = false;
    char c;
    if( argc > 4 || argc < 2 ) {
        printUse(argv[0]);
    }
    while( ( c = getopt( argc, argv, "i" ) ) != -1 ) {
        switch( c ) {
            case 'i':
                ignoreErr = true;
                break;
            case '?':
            default:
                printUse(argv[0]);
        }
    }


///////////////////////////////////////////////////////////////////////////////
// You have to initialize the schema before you do anything else.
// This initializes all of the registry information for the schema you
// plan to use.  The SchemaInit() function is generated by fedex_plus.
// See the 'extern' stmt above.
//
// The registry is always going to be in memory.
//
///////////////////////////////////////////////////////////////////////////////

    Registry  registry( SchemaInit );
    InstMgr   instance_list;
    STEPfile  sfile( registry, instance_list );
    char   *  flnm;

    cout << "\nEXAMPLE :  load file ..." << endl;
    if( argc >= ( optind + 1 ) ) {
        flnm = argv[optind];
    } else {
        flnm = ( char * )"testfile.step";
    }
    sfile.ReadExchangeFile( flnm );
    sfile.Error().PrintContents(cout);

    checkSchemaName( registry, sfile, ignoreErr );

    cout << "EXAMPLE :  write file ..." << endl;
    if( argc == optind+2 ) {
        flnm = argv[optind+1];
    } else {
        flnm = ( char * )"file.out";
    }
    sfile.WriteExchangeFile( flnm );
    sfile.Error().PrintContents(cout);
    cout << flnm << " written"  << endl;
}
