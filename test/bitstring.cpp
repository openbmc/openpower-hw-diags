#include "bitstring.hpp"

/**
 * @brief Print formatted hex value
 * @param val   Hex value to print
 */
template< typename T >
void printval(T val) {
#if CONSOLE_OUTPUT
    cout << setfill('0') << setw(sizeof(T)*2) << hex << (long long int)val;
#endif // CONSOLE_OUTPUT
}

/**
 * @brief Exercise getField and setField
 * @param bs BitString to test with.
 */
template< typename T >
int test_setgetField( BitString<T> bs ) {

    int retval = 0;         // test result
    T val1 = 0, val2 = 0;   // test values

    // setup test parameters
    uint32_t len = 1;
    uint32_t pos = 0;
    uint32_t maxoffset = bs.getBitLen();
    uint32_t maxlength = ( sizeof( * bs.getBufAddr() ) * 8);
   
    // For every valid field position
    for (pos = 0; pos <= maxoffset; pos++) {
        // For every valid field length
        for (len = 1;  ( ( len <=  maxlength ) && ( ( len + pos ) <= maxoffset ) ); len++) {
            
            // Set the bit field
            bs.setField( pos, len, ALLBITS );

            // Get the bit field
            val1 = bs.getField( pos, len );

            // Test the bit field
            val2 = ( ALLBITS >> ( MAXBITS - len ) );
            if (val1 != val2) { retval = 1; }
            
            // Output data
#if CONSOLE_OUTPUT
            cout << "(" << dec << pos << "," << dec << len << ")";
            printval( val1 ); cout << ":"; printval( val2 );
            if ( val1 != val2 ) { cout << "<- mismatch! "; } cout << endl;
#endif // CONSOLE_OUTPUT
        }
    }
    return retval;
}

/**
 * @brief Test the BitString functions.
 */
int main() {

    uint8_t buffer [16] = {0};  // buffer for testing

    // test with 8-bit fields
    BitString<uint8_t> \
        bs8( ( sizeof( buffer ) * 8 ), buffer );
    test_setgetField( bs8 );

    // test with 16-bit fields
    BitString<uint16_t> \
        bs16( ( sizeof( buffer ) * 8 ), reinterpret_cast< uint16_t * >(buffer) );
    test_setgetField( bs16 );

    // test with 32-bit fields
    BitString<uint32_t> \
        bs32( ( sizeof( buffer ) * 8 ), reinterpret_cast< uint32_t * >(buffer) );
    test_setgetField( bs32 );

    // test with 64-bit fields
    BitString<uint64_t> \
        bs64( ( sizeof( buffer ) * 8 ), reinterpret_cast< uint64_t * >(buffer) );
    test_setgetField( bs64 );

    return 0;
}
