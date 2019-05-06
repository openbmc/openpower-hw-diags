#include "bitstring.hpp"

/**
 * @brief Print formatted hex value
 * @param val   Hex value to print
 */
template< typename T >
void printval(T val) {
    cout << setfill('0') << setw(sizeof(T)*2) << hex << (int)val << ":";
}

/**
 * @brief Exercise getField
 * @param bs BitString to exercise getField on.
 */
template< typename T >
void test_getField( BitString<T> bs ) {

    unsigned int len = 1;
    unsigned int pos = 0;

    unsigned int maxoffset = bs.getBitLen();
    unsigned int maxlength = ( sizeof( * bs.getBufAddr() ) * 8);
   
    // For every valid field position
    for (pos = 0; pos < maxoffset; pos++) {
        // For every valid field length
        for (len = 1;  ( ( len <=  maxlength ) && ( ( len + pos ) <= maxoffset ) ); len++) {

            // Retrieve and display the bit field
            cout << "(" << pos << "," << len << ")"; 
            printval( bs.getField( pos, len ) );
        }
        cout << endl;
    }
    cout << endl;
}

/**
 * @brief Test the BitString functions.
 */
int main() {

    // data for tests
    unsigned char buffer [] = {0xde, 0xad, 0xbe, 0xef, 0xba, 0xad, 0xf0, 0x0d, \
                            0xca, 0xfe, 0xd0, 0x0d, 0xb1, 0x05, 0xc0, 0xde };

    // test with byte size fields
    BitString<unsigned char> \
        bs8( ( sizeof( buffer ) * 8 ), buffer );
    test_getField( bs8 );

    // test with word size fields
    BitString<unsigned short> \
        bs16( ( sizeof( buffer ) * 8 ), reinterpret_cast< unsigned short * >(buffer) );
    test_getField ( bs16 );

    // test with dword size fields
    BitString<unsigned int> \
        bs32( ( sizeof( buffer ) * 8 ), reinterpret_cast< unsigned int * >(buffer) );
    test_getField ( bs32 );

    // test with qword size fields
    BitString<unsigned long> \
        bs64( ( sizeof( buffer ) * 8 ), reinterpret_cast< unsigned long * >(buffer) );
    test_getField ( bs64 );

    return 0;
}
