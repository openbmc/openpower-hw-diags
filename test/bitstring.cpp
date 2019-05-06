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
int test_BitString( BitString<T> bs ) {

    int retval = 0;         // test result

    //
    // test setFIeld() and getField()
    //
    T val1 = 0, val2 = 0;   // test values

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
            if (val1 != val2) { retval++; }
            
            // Output data
#if CONSOLE_OUTPUT
            cout << "(" << dec << pos << "," << dec << len << ")";
            printval( val1 ); cout << ":"; printval( val2 );
            if ( val1 != val2 ) { cout << "<- mismatch! "; } cout << endl;
#endif // CONSOLE_OUTPUT
        }
    }

    //
    // test setAll()
    //
    bs.setAll();
    if ( ( bs.getField( 0, MAXBITS ) ) != ALLBITS )
    { 
        retval++;
#if CONSOLE_OUTPUT        
        cout << "setAll() failed!" << endl;
#endif // CONSOLE_OUTPUT        
    }

    //
    // test clearAll()
    //
    bs.clearAll();
    if ( ( bs.getField( 0, MAXBITS ) ) != 0 )
    { 
        retval++;
#if CONSOLE_OUTPUT        
        cout << "clearAll() failed!" << endl;
#endif // CONSOLE_OUTPUT        
    }
 
    //
    // test setBit();
    //
    for ( pos = 0; pos <= MAXBITS; pos++ ) {
        bs.setBit(pos);
    }
    if ( bs.getField( 0, MAXBITS ) != ALLBITS ) 
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "setBit() failed!" << endl;
//        cout << (int)bs.getField( 0, MAXBITS ) << endl;
//        cout << (int)MAXBITS << endl;
//        cout << (int)ALLBITS << endl;
#endif // CONSOLE_OUTPUT        
    }
    
    //
    // test clearBit();
    //
    for ( pos = 0; pos <= MAXBITS; pos++ ) {
        bs.clearBit(pos);
    }
    if ( bs.getField( 0, MAXBITS ) != 0 ) 
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "clearBit() failed!" << endl;
#endif // CONSOLE_OUTPUT        
    }

    //
    // test isBitSet()
    //
    bs.setAll();

    for ( pos = 0; pos < MAXBITS ; pos++ ) {
        if ( bs.isBitSet(pos) != 1 ) 
        {
            retval++;
#if CONSOLE_OUTPUT
        cout << "isBitSet(1) failed!" << pos << endl;
#endif // CONSOLE_OUTPUT
        }
    }

    bs.clearAll();

    for ( pos = 0; pos < MAXBITS; pos++ ) {
        if ( bs.isBitSet(pos) != 0 ) 
        {
            retval++;
#if CONSOLE_OUTPUT
        cout << "isBitSet(0) failed!" << pos << endl;
#endif // CONSOLE_OUTPUT
        }
    }

    return retval;
}

/**
 * @brief Test the BitString functions.
 */
int main() {
    uint32_t retval = 0;

    uint8_t buffer [16] = {0};  // buffer for testing

    // test with 8-bit fields
    BitString<uint8_t> \
        bs8( ( sizeof( buffer ) * 8 ), buffer );
    retval += test_BitString( bs8 );

    // test with 16-bit fields
    BitString<uint16_t> \
        bs16( ( sizeof( buffer ) * 8 ), reinterpret_cast< uint16_t * >(buffer) );
    retval += test_BitString( bs16 );

    // test with 32-bit fields
    BitString<uint32_t> \
        bs32( ( sizeof( buffer ) * 8 ), reinterpret_cast< uint32_t * >(buffer) );
    retval += test_BitString( bs32 );

    // test with 64-bit fields
    BitString<uint64_t> \
        bs64( ( sizeof( buffer ) * 8 ), reinterpret_cast< uint64_t * >(buffer) );
    retval += test_BitString( bs64 );

    if (retval)
    {
#if CONSOLE_OUTPUT
        cout << "errors: " << retval << endl;
#endif // CONSOLE_OUT        
    }

    return retval;
}
