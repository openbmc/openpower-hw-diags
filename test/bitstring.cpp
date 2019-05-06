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

    // Secondary BitString for test cases
    T buffer [ 16 / sizeof(T)  ] = { 0 };
    BitString<T> bstest( ( sizeof( buffer ) * 8 ), buffer );

    // Test case parameters
    uint32_t len = 1;
    uint32_t pos = 0;
    uint32_t maxoffset = bs.getBitLen();
    uint32_t maxlength = ( sizeof( * bs.getBufAddr() ) * 8);

    // Test values
    T val1 = 0, val2 = 0; 
   
    //
    // test setField() and getField()
    //

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
            
            if ( val1 != val2 ) 
            { 
                retval++;
#if CONSOLE_OUTPUT
                cout << "(" << dec << pos << "," << dec << len << ")";
                printval( val1 ); cout << ":"; printval( val2 );
                cout << "<- mismatch! ";  cout << endl; 
#endif // CONSOLE_OUTPUT
            }
        }
    }

    //
    // test setAll()
    //
    bs.setAll();

    if ( ALLBITS != ( bs.getField( 0, MAXBITS ) ) )
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

    if ( ( 0 != bs.getField( 0, MAXBITS ) ) )
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

    if ( ALLBITS != bs.getField( 0, MAXBITS ) ) 
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "setBit() failed!" << endl;
#endif // CONSOLE_OUTPUT        
    }
    
    //
    // test clearBit();
    //
    for ( pos = 0; pos <= MAXBITS; pos++ ) {
        bs.clearBit(pos);
    }

    if ( 0 != bs.getField( 0, MAXBITS ) ) 
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
    for ( pos = 0; pos < bs.getBitLen(); pos++ ) {
        if ( 1 != bs.isBitSet(pos) ) 
        {
            retval++;
#if CONSOLE_OUTPUT
        cout << "isBitSet(1) failed!" << pos << endl;
#endif // CONSOLE_OUTPUT
        }
    }

    bs.clearAll();
    for ( pos = 0; pos < bs.getBitLen(); pos++ ) {
        if ( 0 != bs.isBitSet(pos) ) 
        {
            retval++;
#if CONSOLE_OUTPUT
        cout << "isBitSet(0) failed!" << pos << endl;
#endif // CONSOLE_OUTPUT
        }
    }

    //
    // test isZero()
    //
    
    bs.clearAll();
    if ( true != bs.isZero() ) 
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "isZero(0) failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    bs.setAll();
    if ( false != bs.isZero() ) 
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "isZero(1) failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    //
    // test isEqual()
    //
    bs.clearAll();
    bstest.clearAll();
    if ( !( bs.isEqual( bstest ) ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "isEqual(0) failed!" << endl;
#endif // COUNSOLE_OUTPUT
    }
    bstest.setAll();
    if ( bs.isEqual( bstest ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "isEqual(1) failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    //
    // test getSetCount()
    //
    bs.setAll();
    if ( bs.getBitLen() != bs.getSetCount() )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "getSetCount(1) failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    bs.clearAll();
    if ( 0 != bs.getSetCount() )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "getSetCount(0) failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    for( pos = 0; pos < MAXBITS; pos += 2 ) { bs.setBit(pos); }
    if ( ( MAXBITS / 2 )  != bs.getSetCount(0, MAXBITS) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "getSetCount(MAXBITS) failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    //
    // test setString()
    //
    bs.clearAll();
    bstest.setAll();

    bs.setString( bstest );
    if( ! ( bs.isEqual( bstest ) ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "setString() failed!" << endl;
#endif // CONSOLET_OUTPUT
    }

    // clear both strings
    bs.clearAll();
    bstest.clearAll();

    // set bottom half of one string and top half of other string
    bs.setField( 0, MAXBITS, ALLBITS );
    bstest.setField( MAXBITS, MAXBITS, ALLBITS );

    // copy top half of one string to bottom half of other string (vice versa)
    bs.setString( bstest, MAXBITS, MAXBITS, MAXBITS );
    bstest.setString( bs, 0, MAXBITS, 0 );
    if ( ! ( bs.isEqual( bstest ) ) ) 
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "setString(0) failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    //
    // test maskString()
    //
    bs.setAll();

    // create mask string
    bstest.clearAll();
    for ( pos = 0; pos < bstest.getBitLen(); pos += 2 ) { bstest.setBit( pos ); }

    //  apply mask
    bs.maskString( bstest );

    val1 = bs.getField( 0, MAXBITS );
    val2 = bstest.getField( 0, MAXBITS );
    val1 ^= val2;

    if ( ALLBITS != val1 ) 
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << hex << (int)bs.getField( 0, MAXBITS ) << endl;
        cout << hex << (int)bstest.getField( 0, MAXBITS ) << endl;
        cout << "maskString() failed!" << endl;
#endif
    }

    return retval;
}

/**
 * @brief Test the BitString functions.
 */
int main() {
    uint32_t retval = 0;

    uint8_t buffer [16] = { 0 };  // buffer for testing

    // test with 8-bit fields
    BitString<uint8_t> \
        bs8( ( sizeof( buffer ) * 8 ), buffer );
    retval += test_BitString( bs8 );

/*
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
*/
    if (retval)
    {
#if CONSOLE_OUTPUT
        cout << "errors: " << retval << endl;
#endif // CONSOLE_OUTPUT
    }

    return retval;
}
