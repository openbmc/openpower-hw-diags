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
    // test setField() and getField() - using BitString class
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
        cout << "maskString() failed!" << endl;
#endif
    }

    //
    // test copy-constructor, ==, assignment
    //
    
    // test copy-constructor
    BitString<T> bs2 = bs;
    if ( ! ( bs2 == bs ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "copy constructor failed!" << endl;
#endif        
    }

    // test == operator
    T buffer3 [ 16 / sizeof(T) ] = { 0 };
    BitString<T> bs3( ( sizeof( buffer3 ) * 8 ), buffer3 );
    
    bs.setAll();
    if ( ( bs3 == bs ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout <<"'==' operator failed!" << endl;
#endif        
    }

    // test assignment operator
    bs3 = bs;
    if ( ! ( bs3 == bs ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout <<"assignment operator failed!" << endl;
#endif        
    }

    //
    // test bitwise operators (not, and, or, shr, shl)
    //
    uint32_t bsb_size;
    bsb_size = ( sizeof( T ) * 8 * 2 ); // buffer size = 2 bit-fields
    BitStringBuffer<T> bsb1( bsb_size );
    BitStringBuffer<T> bsb2( bsb_size );
    BitStringBuffer<T> bsbzero( bsb_size );
    BitStringBuffer<T> bsbones( bsb_size );

    bsb1.clearAll();
    bsb1.setField( 0, ( bsb_size / 2 ) , ALLBITS ); // set bottom half
    bsb2.clearAll();
    bsb2.setField( ( bsb_size / 2 ), ( bsb_size / 2 ), ALLBITS ); // set top haf

    bsbzero.clearAll(); // bit-string of all ones
    bsbones.setAll();   // bit-string of all zeros

    // test ~
    if ( ! ( ~bsb1 == bsb2 ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "operator ~ failed!" << endl;
#endif // CONSOLE_OUTPUT
    }
    
    // test <<
    if ( ! ( ( bsb2 << MAXBITS ) == bsb1 ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "operator << failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    // test >>
    if ( ! ( ( bsb1 >> MAXBITS ) == bsb2 ) ) 
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "operator >>  failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    // test &
    if ( ! ( bsbzero == ( bsb1 & bsb2 ) ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "operator & failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    // test |
    if ( ! ( bsbones == ( bsb1 | bsb2 ) ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "operator | failed!" << endl;
#endif // CONSOLE_OUTPUT
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
