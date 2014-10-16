# define out /*
# <peychart@mail.pf> - 20140202
# see: http://home.gna.org/ldapcppei/miniHowto-1.en.html#ss1.6.3
  out=`basename $0|sed -e 's/\.[^.]*$//'`
  echo "c++ -std=c++0x -o $out $0 -ldap -ldapcppei ..."
  c++ -std=c++0x -o $out $0 -lldap -lldapcppei && strip $out && chmod 111 $out
  exit $?
# */
# undef out
/****************************************************************************************
The next tool must give a (rather - with dn sorted) similar result to :
  ldapsearch -x -LLL -h "ldap" -D "cn=Manager,dc=myDomain" -w "password" -b "dc=myDomain"
*****************************************************************************************/
#include <libldapcppei/ldapEasyInterface.h>
#include <iostream>
#include <sstream>

using namespace ldapcppei;

std::string getarg( std::string s, int argc, char* argv[] ) {
	for ( int i(1); i < argc; )
		if ( s.compare( argv[i++] ) == 0 )
			return argv[i];
	return std::string();
}

void print78 ( std::stringstream& s ) {
	for ( size_t i(0), j(0); i < s.str().size(); i++ ) {
		if ( j++ >= 78 ) { j=2;
			std::cout << "\n ";
		} std::cout << s.str().at(i);
	} std::cout << std::endl;
}

int    main ( int argc, char* argv[] ) {
	std::string	passwd;
	std::string	syntaxe = std::string( "Syntaxe: " ) + std::string( argv[0] )
				+ std::string(  " -h server_name[:port]" )
				+ std::string( " [-D binddn [-w password] ]" )
				+ std::string(  " -b searchbase" )
				+ std::string( " [-s {base|one|sub|children}]" )
				+ std::string( " [-f ldapfilter]" )
				+ std::string( " [-a attrname1[,attrname2...]]" );

	if ( argc < 5 || argc > 15 || ! argc % 2
			||   !*getarg( "-h", argc, argv ).c_str()
			|| ( !*getarg( "-D", argc, argv ).c_str() && *getarg( "-w", argc, argv ).c_str() )
			||   !*getarg( "-b", argc, argv ).c_str() )  {	// Don't care about exotic options... :(
		std::cerr << syntaxe << std::endl;
		return 1;
	}

	if ( *getarg( "-D", argc, argv ).c_str() && ! *getarg( "-w", argc, argv ).c_str() ) {
		std::cerr << "Enter the password: ";
		std::cin  >> passwd;
	}else	passwd = getarg( "-w", argc, argv );

	ldapEasyInterface ldapDesc( "ldap://" + getarg( "-h", argc, argv ) );

	if ( ! ldapDesc.setBindDn( getarg( "-D", argc, argv ).c_str(), passwd.c_str() ).open() ) {
		std::cerr << ldapDesc.getErrorString().c_str() << std::endl;
		return ldapDesc.errNo();
	}

	switch ( *getarg( "-s", argc, argv ).c_str() ) {
		case '\0':
		case 's' : ldapDesc.doSearchScopeSubTree( getarg( "-b", argc, argv ).c_str(), getarg( "-a", argc, argv ).c_str(), getarg( "-f", argc, argv ).c_str() );
			break;
		case 'b' : ldapDesc.doSearchScopeBase( getarg( "-b", argc, argv ).c_str(), getarg( "-a", argc, argv ).c_str(), getarg( "-f", argc, argv ).c_str() );
			break;
		case 'o' : ldapDesc.doSearchScopeOneLevel( getarg( "-b", argc, argv ).c_str(), getarg( "-a", argc, argv ).c_str(), getarg( "-f", argc, argv ).c_str() );
			break;
		case 'c' : ldapDesc.doSearchScopeChildren( getarg( "-b", argc, argv ).c_str(), getarg( "-a", argc, argv ).c_str(), getarg( "-f", argc, argv ).c_str() );
			break;
		default  : std::cerr << syntaxe << std::endl;
			return 1;
	} if ( ldapDesc.errNo() != LDAP_SUCCESS ) {
		if ( ldapDesc.errNo() == LDAP_SIZELIMIT_EXCEEDED ) {
			std::cout << "Size limit exceeded!..." << std::endl;
			std::cout << "Need \"sizelimit unlimited\" in your slapd configuration?...)" << std::endl;
		}else	std::cout << "Not found!..." << std::endl;
		return ldapDesc.errNo();
	}

	for ( entry *e; ! ldapDesc.end(); ldapDesc.gotoNext() ) {
		std::stringstream	ss;
		e = ldapDesc.getEntry();
		ss << "dn: " << e; print78( ss );
		for ( attribute *a; ! e->end(); e->gotoNext() ) {
			a = e->getAttribute();
			for ( berValue *b; ! a->end(); a->gotoNext() ) {
				std::stringstream	ss;
				b = a->getBerValue();
				ss << a << ":";
				if ( b->size() ) {
					if ( b->mustBeEncoded64() )
						ss << ": " << b->toCode64();
					else	ss << " "  << b->c_str();
				} print78( ss );
			}
		} std::cout << std::endl;
	}

	return 0;
}
