# define out /*
# <peychart@mail.pf> - 20140202
# Attribute [un]codeBase64 from ldif
#
  out=`basename $0|sed -e 's/\.[^.]*$//'`
  echo "c++ -o $out $0 ..."
  c++ -o $out $0 ; strip $out; chmod 111 $out
  exit $?
# */
# undef out
#include <string>
//using namespace std;

// Table de translation RFC1113
static const unsigned char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Table de Translation inverse
static const unsigned char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

static void encode64block( unsigned char *in, unsigned char *out, int len ) {
	out[0] = (unsigned char) cb64[ (int)(in[0] >> 2) ];
	out[1] = (unsigned char) cb64[ (int)(((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)) ];
	out[2] = (unsigned char) (len > 1 ? cb64[ (int)(((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)) ] : '=');
	out[3] = (unsigned char) (len > 2 ? cb64[ (int)(in[2] & 0x3f) ] : '=');
}

static void decode64block( unsigned char in[4], unsigned char out[3] )
{	out[0] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
	out[1] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
	out[2] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
}

unsigned char getcForDecode64 ( std::string& s, size_t& i ) {
	while ( i < s.size() && (s.at(i) == '=' || s.at(i) == '\r') )
		i++;
	if ( i < s.size() && s.at(i) == '\n' ) {
		if ( s.at(i) == ' ' || s.at(i) == '\t')
			i++;
		else	i = s.size();
	} return( i<s.size() ? s.at(i++) : 0 );
}

std::string decode64( std::string s )
{	unsigned char	in[4], out[3], v;
	std::string	ret("");

	for ( size_t pt(0); pt < s.size(); ) {
		size_t	len(0);
		for ( size_t i(0); i < 4 && pt < s.size(); i++ ) {
			for ( v=0; pt < s.size() && v==0; ) {
				if ( (v = getcForDecode64( s, pt )) ) {
					v = ((v < 43 || v > 122) ? 0 : cd64[v - 43]);
					if ( v )
						v = ((v == '$') ? 0 : v - 61);
			}	}
			if ( v ) {
				in[i] = v - 1;
				len++;
			}else   in[i] = 0;
		}
		if ( len ) {
			decode64block( in, out );
			for ( size_t i(0); i < len-1; i++ ) {
				ret += out[i];
		}	}
	} return ret;
}

std::string encode64( std::string s ) {
	unsigned char	in[3], out[4];
	std::string	ret;

	for ( size_t i(0); i < s.size(); ) {
		size_t	len=0;
		for ( size_t j(0); j < 3; j++ ) {
			if ( i < s.size() ) {
				in[j] = s.at(i++);
				len++;
			}else	in[j] = 0;
		}
		if ( len ) {
			encode64block( in, out, len );
			for ( size_t j=0; j < 4; j++ )
				ret += out[j];
	}	}
	return ret;
}

bool isBinary( std::string& s ) {
	if ( s.size() && (s.at(0)=='<' || s.at(0)==':' || s.at(0)==' ' || s.at( s.size()-1 )==' ' )  )
		return true;
	for ( size_t i=0; i < s.size(); i++ )
		if ( s.at(i) < ' ' || s.at(i) > '~' )
			return true;
	return false;
}

inline char getType( std::string& s ) {
	if ( s.size() && s.at(0) == ':' ) {
		s.erase( 0, 1 );
		if ( s.size() && s.at(0) == ':' ) {
			s.erase( 0, 1 );
			return 'b';
		} return 'e';
	} return isBinary( s ) ? 'b' : 'r';
}

void printLine( std::string& s ) {
	size_t	pos = s.find(":");
	if ( pos++ != std::string::npos ) {
		char	type;

		std::cout << s.substr( 0, pos ); s.erase( 0, pos );	// print the attribute name...

		type = getType( s );
		if ( s.size() && s.at(0)==' ' )
			s.erase( 0, 1 );

		switch( type ) {
//			case 'e': s = ":: " + ldiffNewLine( decode64( s ) );	break;
			case 'r': s = " " + s;					break;
			case 'b': s = ": " + encode64( s );
	}	}
	std::cout << s << std::endl;
}

inline bool isBinaryAttribute( std::string& s ) {
	size_t pos  = s.find ( ":" );
	return pos != std::string::npos && ( s.substr( pos, 3 ).compare( ":::" )==0 );
}

int main( int argc, char *argv[] ) {
	std::string	current, next;

	if ( argc > 1 ) {
		std::cerr << "Syntaxe: " << argv[0] << " <filename..." << std::endl;
		std::cerr << "  Convert a LDIF format into a format readable by (some) humans and vice versa." << std::endl;
		std::cerr << "  eg:\tldapsearch | " << argv[0] << " | " << argv[0] << std::endl;
		return 1;
	}

	for ( bool ended=getline(std::cin, current).eof(), isBinary; !ended; current=next ) {
		isBinary = isBinaryAttribute( current );
		do {	ended |= getline(std::cin, next).eof();
			if ( ! next.size() || next.at(0) != ' ' )
				break;
			current.append( isBinary ? "\n" + next.substr( 1 ) : next.substr( 1 ) );
		} while ( !ended );

		printLine( current );
	}
	return 0;
}
