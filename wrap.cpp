# define out /*
# <peychart@mail.pf> - 20140202
# see: http://home.gna.org/ldapcppei/miniHowto-1.en.html#ss1.6.3
  out=`basename $0|sed -e 's/\.[^.]*$//'`
  echo "c++ -std=c++0x -o $out $0..."
  c++ -std=c++0x -o $out $0 && strip $out && chmod 111 $out
  exit $?
# */
# undef out
/***********************************************************************************************
This tool unwraps the ldif format on stdin and rewraps to the specified lenght (default is 78)...
************************************************************************************************/
#include <iostream>
#include <sstream>

#define	DEFAULT_LEN		78

int    main ( int argc, char* argv[] ) {
	size_t			n(0);
	std::string		current, next;

	if ( argc == 2 ) {
		std::stringstream	s( argv[1] );
		s >> n;
	} if ( argc != 1 && (argc!=2 || n < 2) ) {
		std::cerr << "Syntaxe: " << argv[0] << " [maxlen]\t# must be >1, default is " << DEFAULT_LEN << ", -1 -> unwrap..." << std::endl;
		return 1;
	} if ( n < 2 ) n = DEFAULT_LEN;

	for ( bool ended=getline(std::cin, current).eof(); !ended; current=next ) {
		do {    ended |= getline(std::cin, next).eof();
			if ( ! next.size() || next.at(0) != ' ' )
				break;
			current.append( next.substr( 1 ) );
		} while ( !ended );

		for ( size_t i(0), j(0); i < current.size(); i++ ) {
			if ( j++ >= n ) { j=2;
				std::cout << "\n ";
			} std::cout << current.at(i);
		} std::cout << std::endl;
	}

	return 0;
}
