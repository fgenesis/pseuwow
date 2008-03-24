#ifndef _DEBUGSTUFF_H
#define _DEBUGSTUFF_H


#ifdef _DEBUG
    #define DEBUG(code) code;
    #define DEBUG_APPENDIX " - DEBUG"
    #define CODEDEB(code) fprintf(stderr,"[[ %s ]]\n",#code); code;
#else 
    #define DEBUG(code) /* code */
    #define DEBUG_APPENDIX
    #define CODEDEB(code) (code;)
#endif

#define ASSERT( assertion ) { if( !(assertion) ) { fprintf( stderr, "\n%s:%i ASSERTION FAILED:\n  %s\n", __FILE__, __LINE__, #assertion ); throw "Assertion Failed"; }}



#endif
