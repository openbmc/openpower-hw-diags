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
 * @param bsb BitStringBuffer to test with.
 */
template< typename T >
int test_BitStringClass( BitStringBuffer<T> & bsb ) {

    int retval = 0;         // test result

    BitStringBuffer<T> bstest( 128 );

    bsb.clearAll();

    // Test case parameters
    uint32_t len = 1;
    uint32_t pos = 0;

    // Test values
    T val1 = 0, val2 = 0; 
   
    //
    // test setField() and getField()
    //

    // For every valid field position
    for (pos = 0; pos <= bsb.getBitLen(); pos++) {
        // For every valid field length
        for (len = 1;  ( ( len <=  MAXBITS ) && ( ( len + pos ) <= bsb.getBitLen() ) ); len++) {
            
            // Set the bit field
            bsb.setField( pos, len, ALLBITS );

            // Get the bit field
            val1 = bsb.getField( pos, len );

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
    bsb.setAll();

    if ( ALLBITS != ( bsb.getField( 0, MAXBITS ) ) )
    { 
        retval++;
#if CONSOLE_OUTPUT        
        cout << "setAll() failed!" << endl;
#endif // CONSOLE_OUTPUT        
    }

    //
    // test clearAll()
    //
    bsb.clearAll();

    if ( ( 0 != bsb.getField( 0, MAXBITS ) ) )
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
        bsb.setBit(pos);
    }

    if ( ALLBITS != bsb.getField( 0, MAXBITS ) ) 
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
        bsb.clearBit(pos);
    }

    if ( 0 != bsb.getField( 0, MAXBITS ) ) 
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "clearBit() failed!" << endl;
#endif // CONSOLE_OUTPUT        
    }

    //
    // test isBitSet()
    //
    bsb.setAll();
    for ( pos = 0; pos < bsb.getBitLen(); pos++ ) {
        if ( 1 != bsb.isBitSet(pos) ) 
        {
            retval++;
#if CONSOLE_OUTPUT
        cout << "isBitSet(1) failed!" << pos << endl;
#endif // CONSOLE_OUTPUT
        }
    }

    bsb.clearAll();
    for ( pos = 0; pos < bsb.getBitLen(); pos++ ) {
        if ( 0 != bsb.isBitSet(pos) ) 
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
    bsb.clearAll();
    if ( true != bsb.isZero() ) 
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "isZero(0) failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    bsb.setAll();
    if ( false != bsb.isZero() ) 
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "isZero(1) failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    //
    // test isEqual()
    //
    bsb.clearAll();
    bstest.clearAll();
    if ( !( bsb.isEqual( bstest ) ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "isEqual(0) failed!" << endl;
#endif // COUNSOLE_OUTPUT
    }
    bstest.setAll();
    if ( bsb.isEqual( bstest ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "isEqual(1) failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    //
    // test getSetCount()
    //
    bsb.setAll();
    if ( bsb.getBitLen() != bsb.getSetCount() )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "getSetCount(1) failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    bsb.clearAll();
    if ( 0 != bsb.getSetCount() )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "getSetCount(0) failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    for( pos = 0; pos < MAXBITS; pos += 2 ) { bsb.setBit(pos); }
    if ( ( MAXBITS / 2 )  != bsb.getSetCount(0, MAXBITS) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "getSetCount(MAXBITS) failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    //
    // test setString()
    //
    bsb.clearAll();
    bstest.setAll();

    bsb.setString( bstest );
    if( ! ( bsb.isEqual( bstest ) ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "setString() failed!" << endl;
#endif // CONSOLET_OUTPUT
    }

    // clear both strings
    bsb.clearAll();
    bstest.clearAll();

    // set bottom half of one string and top half of other string
    bsb.setField( 0, MAXBITS, ALLBITS );
    bstest.setField( MAXBITS, MAXBITS, ALLBITS );

    // copy top half of one string to bottom half of other string (vice versa)
    bsb.setString( bstest, MAXBITS, MAXBITS, MAXBITS );
    bstest.setString( bsb, 0, MAXBITS, 0 );
    if ( ! ( bsb.isEqual( bstest ) ) ) 
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "setString(0) failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    //
    // test maskString()
    //
    bsb.setAll();

    // create mask string
    bstest.clearAll();
    for ( pos = 0; pos < bstest.getBitLen(); pos += 2 ) { bstest.setBit( pos ); }

    //  apply mask
    bsb.maskString( bstest );

    val1 = bsb.getField( 0, MAXBITS );
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
    // test bitwise operators (==, not, and, or, shr, shl)
    //
    BitStringBuffer<T> bsbtest1( MAXBITS * 2 );
    BitStringBuffer<T> bsbtest2( MAXBITS * 2 );
    BitStringBuffer<T> bsbzero( MAXBITS * 2 );
    BitStringBuffer<T> bsbones( MAXBITS * 2 );

    bsbtest1.clearAll();
    bsbtest2.clearAll();

    // test == operator
    bsb.setAll();
    if ( ! ( bsbtest1 == bsbtest2 ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout <<"'==' operator failed!" << endl;
#endif        
    }

    bsbtest1.setField( 0, MAXBITS , ALLBITS );      // set bottom half
    bsbtest2.setField( MAXBITS, MAXBITS, ALLBITS ); // set top haf

    // test ~
    if ( ! ( ~bsbtest1 == bsbtest2 ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "operator ~ failed!" << endl;
#endif // CONSOLE_OUTPUT
    }
    
    // test <<
    if ( ! ( ( bsbtest2 << MAXBITS ) == bsbtest1 ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "operator << failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    // test >>
    if ( ! ( ( bsbtest1 >> MAXBITS ) == bsbtest2 ) ) 
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "operator >>  failed!" << endl;
#endif // CONSOLE_OUTPUT
    }
    
    bsbzero.clearAll(); // bit-string of all ones
    bsbones.setAll();   // bit-string of all zeros

    // test &
    if ( ! ( bsbzero == ( bsbtest1 & bsbtest2 ) ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "operator & failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    // test |
    if ( ! ( bsbones == ( bsbtest1 | bsbtest2 ) ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "operator | failed!" << endl;
#endif // CONSOLE_OUTPUT
    }
   
    //
    // test BitString and BitStringBuffers constructors and assignments
    //

    // BitString constructor
    T buffer [MAXBITS] = { 0 };
    BitString<T> bs1( ( sizeof( buffer ) * 8 ), buffer );
    if ( ( nullptr == bs1.getBufAddr() ) || \
           MAXBITS == bs1.getBitLen() || \
            ( ! ( bs1.isZero() ) ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "BitString constructor failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    // BitstringBuffer constructor
    BitStringBuffer<T> bsb1( 128 );
    if ( ( nullptr == bsb1.getBufAddr() ) || \
           MAXBITS == bsb1.getBitLen() || \
            ( ! ( bsb1.isZero() ) ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "BitStringBuffer constructor failed!" << endl;
#endif // CONSOLE_OUTPUT
    }
    
    // BitStringBuffer copy constructor from BitStringBuffer
    BitStringBuffer<T> bsb2 = bsb1;
    if ( ! ( bsb2 == bsb1 ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "BitStringBuffer copy constructor =BitStringBuffer failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    
    // BitStringBuffer constructor from BitString
    BitStringBuffer<T> bsb3 = bs1;
    if ( ! ( bsb3 == bs1 ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "BitStringBuffer copy constructor =BitString failed!" << endl;
#endif // CONSOLE_OUTPUT
    }


    // assignment operator BitStringBuffer from BitstringBuffer
    bsb2 = bsb3;
    if ( ! ( bsb2 == bsb3 ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "BitStringBuffer assignment =BitStringBuffer failed!" << endl;
#endif // CONSOLE_OUTPUT
    }


    // assignment operator BitStringBuffer from BitString
    bsb3 = bs1;
    if ( ! ( bsb3 == bs1 ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "BitStringBuffer assignment =BitString failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    
    // BitStringBuffer constructor with BitStringBuffer initializer
    BitStringBuffer<T> bsb4( bsb1 );
    if ( ! ( bsb4 == bsb1 ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "BitStringBuffer constructor (BitStringBuffer) failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    // BitStringBuffer constructor with BitString initializer
    BitStringBuffer<T> bsb5( bs1 );
    if ( ! ( bsb5 == bs1 ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "BitStringBuffer constructor (BitString) failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    // copy constructor BitString from BitString
    BitString<T> bs2 = bs1;
    if ( ! ( bs2 == bs1 ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "BitString copy constructor =BitString failed!" << endl;
#endif // CONSOLE_OUTPUT
    }


    // copy constructor BitString from BitStringBuffer - not supported
    //BitString<T> bs3 = bsb1;
    bs1.setAll();

    // assignment operator BitString from BitString
    bs2 = bs1;
    if ( ! ( bs2 == bs1 ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "BitString assignment =BitString failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    // assignment operator BitString from BitStringBuffer - not supported
    //bs3 = bsb1;

    // BitString constructor with BitString initializer
    BitString<T> bs4( bs1 );
    if ( ! ( bs4 == bs1 ) )
    {
        retval++;
#if CONSOLE_OUTPUT
        cout << "BitString constructor (BitString) failed!" << endl;
#endif // CONSOLE_OUTPUT
    }

    // BitString constructor with BitStringBuffer initializer - not supported
    //BitString<T> bs5( bsb1);

    // BitStringBuffer destructor
    ~bsb2;

    // BitString destructor
    ~bs2;

    return retval;
}

/**
 * @brief Test the BitString based classes.
 */
int main() {
    uint32_t retval = 0;

    
    // test with 8-bit fields
    BitStringBuffer<uint8_t> bs8(128); 
    retval += test_BitStringClass( bs8 );

    // test with 16-bit fields
    BitStringBuffer<uint16_t> bs16(128);
    retval += test_BitStringClass( bs16 );

    // test with 32-bit fields
    BitStringBuffer<uint32_t> bs32(128);
    retval += test_BitStringClass( bs32 );

    // test with 64-bit fields
    BitStringBuffer<uint64_t> bs64(128);
    retval += test_BitStringClass( bs64 );

    if (retval)
    {
#if CONSOLE_OUTPUT
        cout << "errors: " << retval << endl;
#endif // CONSOLE_OUTPUT
    }

    return retval;
}
