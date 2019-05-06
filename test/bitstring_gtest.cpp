#include "bitstring.hpp"
#include "gtest/gtest.h"

// test case parameters
#define num_fields 2
#define field_size ( sizeof( ParamType ) * 8 )
#define bitstring_size ( field_size * num_fields )

// userful test patterns
#define max_field_size ( sizeof( uint64_t) * 8 )
#define pattern_55 ( 0x5555555555555555ULL >> ( max_field_size - field_size ) )
#define pattern_aa ( 0xaaaaaaaaaaaaaaaaULL >> ( max_field_size - field_size ) )
#define pattern_ff ( 0xffffffffffffffffULL >> ( max_field_size - field_size ) )
#define pattern_00 ( 0x0000000000000000ULL >> ( max_field_size - field_size ) )


// test fixture
template<typename T>
    struct BitStringTest : public testing::Test
    {
        using ParamType = T;
    };

    // test all field sizes supported
    using TestTypes = testing::Types<uint8_t, uint16_t, uint32_t, uint64_t>;
    TYPED_TEST_CASE(BitStringTest, TestTypes);

//
// BitString and BitStringBuffer tests
//
TYPED_TEST( BitStringTest, BitStringBufferConstructorFromLength )
{
    using ParamType  = typename TestFixture::ParamType;

    // test BitStringBuffer constructor
    BitStringBuffer<ParamType> bsb ( bitstring_size );
    ASSERT_NE( nullptr, bsb.getBufAddr() );
    ASSERT_EQ( bitstring_size, bsb.getBitLen() );
    ASSERT_TRUE( bsb.isZero() );
}

TYPED_TEST( BitStringTest, CopyConstructorFromBitString )
{
    using ParamType  = typename TestFixture::ParamType;

    // test with un-initialized buffer
    BitString<ParamType> bs1 ( bitstring_size, nullptr, 0 );
    BitStringBuffer<ParamType> bsb1 = bs1;
    ASSERT_NE( nullptr, bsb1.getBufAddr() );
    ASSERT_EQ( bs1.getBitLen(), bsb1.getBitLen() );
    ASSERT_TRUE( bsb1.isZero() );

    // test with initialized buffer
    ParamType buffer2 [ num_fields ] = { 0 };
    BitString<ParamType> bs2 ( bitstring_size, buffer2, 0 );
    BitStringBuffer<ParamType> bsb2 = bs2;
    ASSERT_NE( nullptr, bsb2.getBufAddr() );
    ASSERT_EQ( bs2.getBitLen() , bsb2.getBitLen() );
    ASSERT_TRUE( bsb2.isEqual(bs2) );

    // test with initialized buffer
    ParamType buffer3 [] = { pattern_aa, pattern_55 };
    BitString<ParamType> bs3 ( bitstring_size, buffer3, 0 );
    BitStringBuffer<ParamType> bsb3 = bs3;
    ASSERT_NE( nullptr, bsb3.getBufAddr() );
    ASSERT_EQ( bs3.getBitLen(), bsb3.getBitLen() );
    ASSERT_FALSE( bsb3.isZero() );
    ASSERT_TRUE( bsb3.isEqual(bs3) );
}

TYPED_TEST( BitStringTest, CopyConstructorFromBitStringBuffer )
{
    using ParamType  = typename TestFixture::ParamType;

    // test with default bitstringbuffer
    BitStringBuffer<ParamType> bsb1 ( bitstring_size );
    BitStringBuffer<ParamType> bsb2 = bsb1;
    ASSERT_NE( nullptr, bsb2.getBufAddr() );
    ASSERT_EQ( bsb1.getBitLen(), bsb2.getBitLen() );
    ASSERT_TRUE( bsb2.isZero() );

    // test with un-initialized buffer -> bitstringbuffer
    BitString<ParamType> bs3 ( bitstring_size, nullptr, 0 );
    BitStringBuffer<ParamType> bsb3 = bs3;
    BitStringBuffer<ParamType> bsb4 = bsb3;
    ASSERT_NE( nullptr, bsb4.getBufAddr() );
    ASSERT_EQ( bsb3.getBitLen(), bsb4.getBitLen() );
    ASSERT_TRUE( bsb4.isZero() );

    // test with initialized buffer -> bitstringbuffer
    ParamType buffer4 [ num_fields ] = { 0 };
    BitString<ParamType> bs4 ( bitstring_size, buffer4, 0 );
    BitStringBuffer<ParamType> bsb5 = bs4;
    BitStringBuffer<ParamType> bsb6 = bsb5;
    ASSERT_NE( nullptr, bsb6.getBufAddr() );
    ASSERT_EQ( bsb5.getBitLen(), bsb6.getBitLen() );
    ASSERT_TRUE( bsb6.isEqual( bsb5 ) );

    // test with initialized buffer -> bitstringbuffer
    ParamType buffer6 [] = { pattern_aa, pattern_55 };
    BitString<ParamType> bs6 ( bitstring_size, buffer6, 0 );
    BitStringBuffer<ParamType> bsb7 = bs6;
    BitStringBuffer<ParamType> bsb8 = bsb7;
    ASSERT_NE( nullptr, bsb8.getBufAddr() );
    ASSERT_EQ( bsb7.getBitLen(), bsb8.getBitLen() );
    ASSERT_FALSE( bsb8.isZero() );
    ASSERT_TRUE( bsb8.isEqual( bsb7 ) );
}

TYPED_TEST( BitStringTest, AssignmentFromBitString )
{
    using ParamType  = typename TestFixture::ParamType;

    // test with un-initialized buffer
    BitString<ParamType> bs1 ( bitstring_size, nullptr, 0 );
    BitStringBuffer<ParamType> bsb1 ( bitstring_size );
    bsb1 = bs1;
    ASSERT_NE( nullptr, bsb1.getBufAddr() );
    ASSERT_EQ( bs1.getBitLen(), bsb1.getBitLen() );
    ASSERT_TRUE( bsb1.isZero() );

    // test with fields cleared
    ParamType buffer2 [ num_fields ] = { 0 };
    BitString<ParamType> bs2 ( bitstring_size, buffer2, 0 );
    BitStringBuffer<ParamType> bsb2 ( bitstring_size );
    bsb2 = bs2;
    ASSERT_NE( nullptr, bsb2.getBufAddr() );
    ASSERT_EQ( bs2.getBitLen(), bsb2.getBitLen() );
    ASSERT_TRUE( bsb2.isEqual(bs2) );

    // test with fields set
    ParamType buffer3 [] = { pattern_aa, pattern_55 };
    BitString<ParamType> bs3 ( bitstring_size, buffer3, 0 );
    BitStringBuffer<ParamType> bsb3 ( bitstring_size );
    bsb3 = bs3;
    ASSERT_NE( nullptr, bsb3.getBufAddr() );
    ASSERT_EQ( bs3.getBitLen(), bsb3.getBitLen() );
    ASSERT_FALSE( bsb3.isZero() );
    ASSERT_TRUE( bsb3.isEqual(bs3) );
}

TYPED_TEST( BitStringTest, AssignmentFromBitStringBuffer )
{
    using ParamType  = typename TestFixture::ParamType;

    // test with fields cleared bitstringbuffer -> bitstringbuffer
    BitStringBuffer<ParamType> bsb1 ( bitstring_size );
    BitStringBuffer<ParamType> bsb2 ( bitstring_size );
    bsb2 = bsb1;
    ASSERT_NE( nullptr, bsb2.getBufAddr() );
    ASSERT_EQ( bsb1.getBitLen(), bsb2.getBitLen() );
    ASSERT_TRUE( bsb2.isZero() );

    // test with un-initialized buffer -> bitstringbuffer
    BitString<ParamType> bs3 ( bitstring_size, nullptr, 0 );
    BitStringBuffer<ParamType> bsb3 = bs3;
    BitStringBuffer<ParamType> bsb4 ( bitstring_size );
    bsb4 = bsb3;
    ASSERT_NE( nullptr, bsb4.getBufAddr() );
    ASSERT_EQ( bsb3.getBitLen(), bsb4.getBitLen() );
    ASSERT_TRUE( bsb4.isZero() );

    // test with initialized buffer -> bitstringbuffer
    ParamType buffer4 [ num_fields ] = { 0 };
    BitString<ParamType> bs4 ( bitstring_size, buffer4, 0 );
    BitStringBuffer<ParamType> bsb5 = bs4;
    BitStringBuffer<ParamType> bsb6 ( bitstring_size );
    bsb6 = bsb5;
    ASSERT_NE( nullptr, bsb6.getBufAddr() );
    ASSERT_EQ( bsb5.getBitLen(), bsb6.getBitLen() );
    ASSERT_TRUE( bsb6.isEqual( bsb5 ) );

    // test with initialized buffer -> bitstringbuffer
    ParamType buffer6 [] = { pattern_aa, pattern_55 };
    BitString<ParamType> bs6( bitstring_size, buffer6, 0 );
    BitStringBuffer<ParamType> bsb7 = bs6;
    BitStringBuffer<ParamType> bsb8 ( bitstring_size ) ;
    bsb8 = bsb7;
    ASSERT_NE( nullptr, bsb8.getBufAddr() );
    ASSERT_EQ( bsb7.getBitLen(), bsb8.getBitLen() );
    ASSERT_FALSE( bsb8.isZero() );
    ASSERT_TRUE( bsb8.isEqual( bsb7 ) );
}

TYPED_TEST( BitStringTest, BitStringBufferOperatorOverloadNot )
{
    using ParamType  = typename TestFixture::ParamType;

    // tests with fields set
    ParamType buffer1 [ num_fields ] = { pattern_aa, pattern_aa };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    BitStringBuffer<ParamType> bsb1 = bs1;
    ParamType buffer2 [ num_fields ] = { pattern_55, pattern_55 };
    BitString<ParamType> bs2( bitstring_size, buffer2 );
    BitStringBuffer<ParamType> bsb2 = bs2;
    // ~(0x55) = 0xaa
    bsb2 = ~( bsb2 );
    ASSERT_TRUE( bsb1.isEqual( bsb2 ) );
}

TYPED_TEST( BitStringTest, BitStringBufferOperatorOverloadAnd )
{
    using ParamType  = typename TestFixture::ParamType;

    // test with fields set
    ParamType buffer1 [ num_fields ] = { pattern_aa, pattern_aa };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    BitStringBuffer<ParamType> bsb1 = bs1;
    ParamType buffer2 [ num_fields ] = { pattern_55, pattern_55 };
    BitString<ParamType> bs2( bitstring_size, buffer2 );
    BitStringBuffer<ParamType> bsb2 = bs2;
    // 0xaa & 0x55 = 0xaa
    bsb1 = ( bsb1 & bsb2 );
    ASSERT_TRUE( bsb1.isZero() );
}

TYPED_TEST( BitStringTest, BitStringBufferOperatorOverloadOr )
{
    using ParamType  = typename TestFixture::ParamType;

    // test with fields set
    ParamType buffer1 [ num_fields ] = { pattern_aa, pattern_aa };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    BitStringBuffer<ParamType> bsb1 = bs1;
    ParamType buffer2 [ num_fields ] = { pattern_55, pattern_55 };
    BitString<ParamType> bs2( bitstring_size, buffer2 );
    BitStringBuffer<ParamType> bsb2 = bs2;
    // 0x55 | 0xaa = 0xff
    bsb1 = ~( bsb1 | bsb2 ); // ~(0xff) = 0x00
    ASSERT_TRUE( bsb1.isZero() );
}

TYPED_TEST( BitStringTest, BitStringBufferOperatorOverloadShiftRight )
{
    using ParamType  = typename TestFixture::ParamType;

    // test with fields set
    ParamType buffer1 [ num_fields ] = { pattern_aa, pattern_aa };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    BitStringBuffer<ParamType> bsb1 = bs1;
    ParamType buffer2 [ num_fields ] = { pattern_55, pattern_55 };
    BitString<ParamType> bs2( bitstring_size, buffer2 );
    BitStringBuffer<ParamType> bsb2 = bs2;
    // 0xaa >> 1 = 0x55
    bsb1 = ( bsb1 >> 1 );
    ASSERT_TRUE( bsb1.isEqual( bsb2 ) );
    // clear with shift
    bsb1 = ( bsb1 >> bsb1.getBitLen() );
    ASSERT_TRUE( bsb1.isZero() );
}

TYPED_TEST( BitStringTest, BitStringBufferOperatorOverloadShiftLeft )
{
    using ParamType  = typename TestFixture::ParamType;

    // test with fields set
    ParamType buffer1 [ num_fields ] = { pattern_aa, pattern_aa };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    BitStringBuffer<ParamType> bsb1 = bs1;
    ParamType buffer2 [ num_fields ] = { pattern_55, pattern_55 };
    BitString<ParamType> bs2( bitstring_size, buffer2 );
    BitStringBuffer<ParamType> bsb2 = bs2;
    // 0x55 << 1 = 0xaa
    bsb2 = ( bsb2 << 1 );
    ASSERT_TRUE( bsb2.isEqual( bsb1 ) );
    // clear with shift
    bsb2 = ( bsb2 <<  bsb2.getBitLen() );
    ASSERT_TRUE( bsb2.isZero() );
}

TYPED_TEST( BitStringTest, BitStringConstructor )
{
    using ParamType  = typename TestFixture::ParamType;

    // test with unitialized buffer
    BitString<ParamType> bs1( bitstring_size, nullptr );
    ASSERT_EQ( nullptr, bs1.getBufAddr() );
    ASSERT_EQ( bitstring_size, bs1.getBitLen() );

    // test with initialized bufffer
    ParamType buffer2 [ num_fields ] = {};
    BitString<ParamType> bs2( bitstring_size, buffer2 );
    ASSERT_NE( nullptr, bs2.getBufAddr() );
    ASSERT_EQ( bitstring_size, bs2.getBitLen() );
}

TYPED_TEST( BitStringTest, BitStringGetBitLen )
{
    using ParamType  = typename TestFixture::ParamType;

    // test with unitialized buffer
    BitString<ParamType> bs1( bitstring_size, nullptr );
    ASSERT_EQ( bitstring_size, bs1.getBitLen() );
}

TYPED_TEST( BitStringTest, BitStringGetBufAddr )
{
    using ParamType  = typename TestFixture::ParamType;

    // test with uninitialized buffer
    BitString<ParamType> bs1( bitstring_size, nullptr );
    ASSERT_EQ( nullptr, bs1.getBufAddr() );

    // test initialized buffer
    ParamType buffer2 [ num_fields ] = {};
    BitString<ParamType> bs2( bitstring_size, buffer2 );
    ASSERT_NE( nullptr, bs2.getBufAddr() );
}

TYPED_TEST( BitStringTest, BitStringGetField )
{
    using ParamType  = typename TestFixture::ParamType;

    // test with initialized bufffer
    ParamType buffer1 [ num_fields ] = { pattern_aa, pattern_55 };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    ASSERT_EQ( pattern_aa, bs1.getField( 0, field_size ) );
    ASSERT_EQ( pattern_55, bs1.getField( field_size, field_size ) );
    // todo, test 0x55aa (verified works, needs test case)
}

TYPED_TEST( BitStringTest, BitStringGetFieldJustify )
{
    // todo, need test case
}

TYPED_TEST( BitStringTest, BitStringSetField )
{
    using ParamType  = typename TestFixture::ParamType;

    // start with empty field
    ParamType buffer1 [ num_fields ] = {};
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    ASSERT_TRUE( bs1.isZero() );
    // set pattern
    bs1.setField( 0, field_size, pattern_aa );
    ASSERT_EQ( pattern_aa, bs1.getField( 0, field_size ) );
}

TYPED_TEST( BitStringTest, BitStringSetFieldJustify )
{
    // todo, need test case
}

TYPED_TEST( BitStringTest, BitStringIsBitSet )
{
    using ParamType  = typename TestFixture::ParamType;

    // start with bit set
    ParamType buffer1 [ num_fields ] = { 1 };
    // test bit
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    ASSERT_TRUE( bs1.isBitSet( 0 ) );
    //  todo, add more test patterns
}

TYPED_TEST( BitStringTest, BitStringSetBit )
{
    using ParamType  = typename TestFixture::ParamType;

    // start with empty fields
    ParamType buffer1 [ num_fields ] = { 0 };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    ASSERT_FALSE( bs1.isBitSet( 0 ) );
    // set bit
    bs1.setBit( 0 );
    ASSERT_TRUE( bs1.isBitSet( 0 ) );
    // todo, add more test patterns
}

TYPED_TEST( BitStringTest, BitStringSetAll )
{
    using ParamType  = typename TestFixture::ParamType;

    // start with clear fields
    ParamType buffer1 [ num_fields ] = { 0 };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    ASSERT_TRUE( bs1.isZero() );
    // set bits
    bs1.setAll();
    ASSERT_EQ( pattern_ff, bs1.getField( 0, field_size ) );
}

TYPED_TEST( BitStringTest, BitStringClearBit )
{
    using ParamType  = typename TestFixture::ParamType;

    // start with fields set
    ParamType buffer1 [ num_fields ] = { 1 };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    ASSERT_TRUE( bs1.isBitSet( 0 ) );
    // clear bit
    bs1.clearBit( 0 );
    ASSERT_FALSE( bs1.isBitSet( 0 ) );
    // todo, add more test patterns
}

TYPED_TEST( BitStringTest, BitStringClearAll )
{
    using ParamType  = typename TestFixture::ParamType;

    // test with fields set
    ParamType buffer1 [ num_fields ] = { pattern_ff, pattern_ff };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    ASSERT_FALSE( bs1.isZero() );
    // clear bits
    bs1.clearAll();
    ASSERT_TRUE( bs1.isZero() );
}

TYPED_TEST( BitStringTest, BitStringSetString )
{
    using ParamType  = typename TestFixture::ParamType;

    // start with fields set
    ParamType buffer1 [ num_fields ] = { pattern_ff, pattern_ff };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    ParamType buffer2 [ num_fields ] = { 0 };
    BitString<ParamType> bs2( bitstring_size, buffer2 );
    ASSERT_TRUE( bs2.isZero() );
    // set bits
    bs2.setString( bs1, 0, bitstring_size, 0 );
    ASSERT_EQ( pattern_ff, bs1.getField( 0, field_size ) );
    ASSERT_EQ( pattern_ff, bs1.getField( field_size, field_size ) );
    // todo, add more test patterns
}

TYPED_TEST( BitStringTest, BitStringMaskString )
{
    using ParamType  = typename TestFixture::ParamType;

    // test with fields set
    ParamType buffer1 [ num_fields ] = { pattern_ff, pattern_ff };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    ParamType buffer2 [ num_fields ] = { pattern_55, pattern_55 };
    BitString<ParamType> bs2( bitstring_size, buffer2 );
    // 0xff & 0xff = 0xaa
    bs1.maskString( bs2 );
    ASSERT_EQ( pattern_aa, bs1.getField( 0, field_size ) );
    ASSERT_EQ( pattern_aa, bs1.getField( field_size, field_size ) );
}

TYPED_TEST( BitStringTest, BitStringIsEqual )
{
    using ParamType  = typename TestFixture::ParamType;

    // test with fields set
    ParamType buffer1 [ num_fields ] = { pattern_55, pattern_55 };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    ParamType buffer2 [ num_fields ] = { pattern_55, pattern_55 };
    BitString<ParamType> bs2( bitstring_size, buffer2 );
    ASSERT_TRUE( bs1.isEqual( bs2 ) );
}

TYPED_TEST( BitStringTest, BitStringIsZero )
{
    using ParamType  = typename TestFixture::ParamType;

    // test with fields clear
    ParamType buffer1 [ num_fields ] = { 0 };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    ASSERT_TRUE( bs1.isZero() );
    // test with fields set
    ParamType buffer2 [ num_fields ] = { pattern_55, pattern_55 };
    BitString<ParamType> bs2( bitstring_size, buffer2 );
    ASSERT_FALSE( bs2.isZero() );
}

TYPED_TEST( BitStringTest, BitStringGetSetCount )
{
    using ParamType  = typename TestFixture::ParamType;

    // test with no clear field
    ParamType buffer1 [ num_fields ] = { 0 };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    ASSERT_EQ( 0, bs1.getSetCount() );
    // test with bits set
    ParamType buffer2 [ num_fields ] = { pattern_ff, pattern_ff };
    BitString<ParamType> bs2( bitstring_size, buffer2 );
    ASSERT_EQ( bitstring_size, bs2.getSetCount() );
}

TYPED_TEST( BitStringTest, BitStringOperatorOverloadEqEq )
{
    using ParamType  = typename TestFixture::ParamType;

    // test == buffers
    ParamType buffer1 [ num_fields ] = { pattern_55, pattern_55 };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    ParamType buffer2 [ num_fields ] = { pattern_55, pattern_55 };
    BitString<ParamType> bs2( bitstring_size, buffer2 );
    ASSERT_TRUE( bs1 == bs2 );
    // test with != buffers
    ParamType buffer3 [ num_fields ] = { 0 };
    BitString<ParamType> bs3( bitstring_size, buffer3 );
    ASSERT_FALSE( bs2 == bs3 );
}

TYPED_TEST( BitStringTest, ExerciseBitString )
{
    using ParamType  = typename TestFixture::ParamType;

    // start with unititialized buffer
    ParamType buffer1 [ num_fields ] = { 0 };
    BitString<ParamType> bs1( bitstring_size, buffer1 );

    // initialize test parameters
    ParamType allbits = -1;
    ParamType val1 = 0;
    ParamType val2 = 0;
    ParamType pos = 0;
    ParamType len = 1;

    // test set/get ...
    // for every valid field position
    for (pos = 0; pos <= bs1.getBitLen(); pos++) {
        // For every valid field length
        for (len = 1;  ( ( len <= field_size ) && ( ( len + pos ) \
                        <= bs1.getBitLen() ) ); len++) {

            bs1.setField( pos, len, allbits );

            // Get the bit field
            val1 = bs1.getField( pos, len );

            // Test the bit field
            val2 = ( allbits >> ( field_size - len ) );

            ASSERT_EQ( val1, val2 );
        }
    }
}

/*
TYPED_TEST( BitStringTest, BitStringDestructor )
{
    // using ParamType  = typename TestFixture::ParamType;
    // todo - overload new and delete is one option
}
*/

/*
TYPED_TEST( BitStringTest, BitStringBufferDestructor )
{
    // using ParamType  = typename TestFixture::ParamType;
    // overload new and delete is one option
}
*/

/* not supported
 *
TYPED_TEST( BitStringTest, BitStringOperatorOverloadNot )
{
    ParamType buffer1 [ num_fields ] = { pattern_55, pattern_55 };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    bs1 = ~( bs1 );
    ASSERT_EQ( pattern_aa, bs1.getField( 0, field_size ) );
    ASSERT_EQ( pattern_aa, bs1.getField( field_size, field_size ) );
}

TYPED_TEST( BitStringTest, BitStringOperatorOverloadAnd )
{
    ParamType buffer1 [ num_fields ] = { pattern_55, pattern_55 };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    ParamType buffer2 [ num_fields ] = { pattern_aa, pattern_aa };
    BitString<ParamType> bs2( bitstring_size, buffer2 );
    bs1 = ( bs1 & bs2 );
    ASSERT_TRUE( bs1.isZero() );
}

TYPED_TEST( BitStringTest, BitStringOperatorOverloadOr )
{
    ParamType buffer1 [ num_fields ] = { pattern_55, pattern_55 };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    ParamType buffer2 [ num_fields ] = { pattern_aa, pattern_aa };
    BitString<ParamType> bs2( bitstring_size, buffer2 );
    bs1 = ( bs1 | bs2 );
    ASSERT_EQ( pattern_ff, bs1.getField( 0, field_size ) );
    ASSERT_EQ( pattern_ff, bs1.getField( field_size, field_size ) );
}

TYPED_TEST( BitStringTest, BitStringOperatorOverloadShiftRight )
{
    ParamType buffer1 [ num_fields ] = { pattern_55, pattern_55 };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    ParamType buffer2 [ num_fields ] = { pattern_aa, pattern_aa };
    BitString<ParamType> bs2( bitstring_size, buffer2 );
    bs2 = ( bs2 >> 1 );
    ASSERT_FALSE( bs2.isZero() );
    ASSERT_TRUE( bs1 == bs2 );
}

TYPED_TEST( BitStringTest, BitStringOperatorOverloadShiftLeft )
{
    ParamType buffer1 [ num_fields ] = { pattern_55, pattern_55 };
    BitString<ParamType> bs1( bitstring_size, buffer1 );
    ParamType buffer2 [ num_fields ] = { pattern_aa, pattern_aa };
    BitString<ParamType> bs2( bitstring_size, buffer2 );
    bs1 = ( bs1 << 1 );
    ASSERT_FALSE( bs1.isZero() );
    ASSERT_TRUE( bs1 == bs2 );
}
*/
