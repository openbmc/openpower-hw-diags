/**
 * Copyright © 2019 IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdint>

#define CONSOLE_OUTPUT 1

using namespace std;

#if CONSOLE_OUTPUT
#include <iostream>
#include <iomanip>
#define PRDF_ASSERT(x) { if(!(x)) { cout << endl << "ASSERT! "<< #x << endl; } }
#else
#define PRDF_ASSERT(x) { if(!(x)) { } }
#endif // CONSOLE_OUTPUT

// Packed structure with a single element to force compile-time fixups of
// potential unaligned memory access.
struct __attribute__((packed)) packed_buffer
{
    uint8_t val;
};

// BitStringBuffer Class
template< class T = uint32_t >
class BitStringBuffer;

//##############################################################################
//                      BitString class definition
//##############################################################################

// BitString Class
template< class T = uint32_t >
class BitString
{
  #define ALLBITS (static_cast<T>(-1))  // all bits = 1
  #define MAXBITS (sizeof(T)*8)         // all bits count
  #define MINBITS (sizeof(uint8_t)*8)

  public: // functions

    /**
     * @brief Constructor
     * @param i_bitLen  The number of bits in the bit string.
     * @param i_bufAddr The starting address of the memory buffer.
     * @param i_offset  Optional input to indicate the actual starting position
     *                  of the bit string within the memory buffer.
     * @post A BitString structure is created
     */
    BitString( uint32_t i_bitLen,  void * i_bufAddr,
               uint32_t i_offset = 0 ) :
        iv_bitLen(i_bitLen), iv_bufAddr(i_bufAddr), iv_offset(i_offset)
    {}

    /** @brief Destructor */
    virtual ~BitString() {}

    /** @return The number of bits in the bit string buffer. */
    uint32_t getBitLen() const { return iv_bitLen; }

    /** @return The address of the bit string buffer. Note that this may
     *          return nullptr. */
     packed_buffer * getBufAddr() const
     {
        return reinterpret_cast<packed_buffer *>(iv_bufAddr);
     }

    /**
     * @brief  Returns the bit field of the given length located at the given
     *         position whithin the bit string.
     * @param  i_pos The starting position of the target range.
     * @param  i_len The number of bits of the target range.
     * @return The value of the bit field range specified .
     * @pre    nullptr != getBufAddr()
     * @pre    0 < i_len
     * @pre    i_len <= MAXBITS
     * @pre    i_pos + i_len <= getBitLen()
     */
    T getField( uint32_t i_pos, uint32_t i_len ) const;

    /**
     * @brief Sets the bit field of the given length located at the given
     *        position within the bit string.
     * @param i_pos The starting position of the target range.
     * @param i_len The number of bits of the target range.
     * @param i_val The value to set.
     * @pre   nullptr != getBufAddr()
     * @pre   0 < i_len
     * @pre   i_len <= MAXBITS
     * @pre   i_pos + i_len <= getBitLen()
     */
    void setField( uint32_t i_pos, uint32_t i_len, T i_val );

    /**
     * @param  i_pos The target position.
     * @return True if the bit at the given position is set(1), false otherwise.
     * @pre    i_pos < getBitLen().
     */
    bool isBitSet( uint32_t i_pos ) const { return 0 != getField(i_pos, 1); }

    /**
     * @brief Sets the target position to 1.
     * @param i_pos The target position.
     * @pre   i_pos < getBitLen().
     */
    void setBit( uint32_t i_pos ) { setField( i_pos, 1, 1 ); }

    /** @brief Sets the entire bit string to 1's. */
    void setAll()
    {
        for( uint32_t i = 0; i < getBitLen(); i++ )
        {
            setBit(i);
        }
    }

    /**
     * @brief Sets the target position to 0.
     * @param i_pos The target position.
     * @pre   i_pos < getBitLen
     */
    void clearBit( uint32_t i_pos ) { setField( i_pos, 1, 0 ); }

    /** @brief Sets the entire bit string to 0's. */
    void clearAll()
    {
        for ( uint32_t i = 0; i < getBitLen(); i++ )
        {
            clearBit(i);
        }
    }

    /**
     * @brief Set bits in this string based on the given string.
     * @param i_sStr The source string.
     * @param i_sPos The starting position of the source string.
     * @param i_sLen The number of bits to copy from the source string.
     * @param i_dPos The starting position of the this string.
     * @pre   nullptr != getBufAddr()
     * @pre   nullptr != i_sStr.getBufAddr()
     * @pre   0 < i_sLen
     * @pre   i_sPos + i_sLen <= i_sStr.getBitLen()
     * @pre   i_dPos < getBitLen()
     * @post  Source bits in given range are copied to this starting at i_dPos.
     * @note  If the length of the given string is greater than the length of
     *        this string, then the extra bits are ignored.
     * @note  If the length of the given string is less than the length of this
     *        string, then the extra bits in this string are not modified.
     * @note  This string and the source string may specify overlapping memory.
     */
    void setString( const BitString & i_sStr, uint32_t i_sPos,
                    uint32_t i_sLen, uint32_t i_dPos = 0 );

    /**
     * @brief Set bits in this string based on the provided string.
     * @param i_sStr The source string.
     * @note  This will try to copy as much of the source as possible to this
     *        string, starting with the first bit in each string.
     * @note  See the other definition of this function for details and
     *        restrictions.
     */
    void setString( const BitString & i_sStr )
    {
        setString( i_sStr, 0, i_sStr.getBitLen() );
    }

    /**
     * @brief Masks (clears) any bits set in this string that correspond to bits
     *        set in the given string (this & ~mask).
     * @param i_mask The mask string.
     * @note  If the length of the given string is greater than the length of
     *        this string, then the extra bits are ignored.
     * @note  If the length of the given string is less than the length of this
     *        string, then the extra bits in this string are not modified.
     */
    void maskString( const BitString & i_mask );

    /**
     * @param  i_str The string to compare.
     * @return True if the strings are equivalent, false otherwise.
     * @pre    Both strings must be of equal length and have same values to be
     *         equal.
     */
    bool isEqual( const BitString & i_str ) const;

    /** @return True if there are no bit set(1) in this bit string, false
     *          otherwise. */
    bool isZero() const;

    /**
     * @param  i_pos The starting position of the target range.
     * @param  i_len The length of the target range.
     * @return The number of bits that are set(1) in given range of this string.
     * @pre    nullptr != getBufAddr()
     * @pre    i_pos + i_len <= getBitLen()
     */
    uint32_t getSetCount( uint32_t i_pos, uint32_t i_len ) const;

    /** @return The number of bits that are set(1) in this string. */
    uint32_t getSetCount() const { return getSetCount( 0, getBitLen() ); }

    /** @brief Comparison operator. */
    bool operator==( const BitString & i_str ) const { return isEqual(i_str); }

    /** @brief Bitwise NOT operator. */
    BitStringBuffer<T> operator~() const;

    /** @brief Bitwise AND operator. */
    BitStringBuffer<T> operator&( const BitString & i_bs ) const;

    /** @brief Bitwise OR operator. */
    BitStringBuffer<T>    operator|( const BitString & i_bs ) const;

    /** @brief Right shift operator. */
    BitStringBuffer<T>  operator>>( uint32_t i_shift ) const;

    /** @brief Left shift operator. */
    BitStringBuffer<T>  operator<<( uint32_t i_shift ) const;

  protected: // functions

    /**
     * @param i_newBufAddr The starting address of the new bit string buffer.
     * @pre   Before calling this function, make sure you deallocate the old
     *        buffer to avoid memory leaks.
     */
    void setBufAddr( void * i_newBufAddr ) { iv_bufAddr = i_newBufAddr; }

    /** @param i_newBitLen The new bit length of this bit string buffer. */
    void setBitLen( uint32_t i_newBitLen ) { iv_bitLen = i_newBitLen; }

  private: // functions

    // Prevent the assignment operator and copy constructor from a
    // BitStringBuffer. While technically these could be done. We run into
    // serious problems like with the operator functions above that all return
    // a BitStringBuffer. If we allowed these, the BitString would end up
    // pointing to memory that is no longer in context.
    BitString & operator=( const BitStringBuffer<T>  & i_bsb );
    BitString( const BitStringBuffer<T>  & i_bsb );

    /**
     * @brief  Given a bit position within the bit string, this function returns
     *         the address that contains the bit position and the bit position
     *         relative to that address.
     * @param  o_relPos The returned relative position.
     * @param  i_absPos The inputted absolute position.
     * @return The relative address.
     * @pre    nullptr != getBufAddr()
     * @pre    i_absPos < getBitLen()
     */
    packed_buffer * getRelativePosition( uint32_t & o_relPos,
                                    uint32_t   i_absPos ) const;

  private: // instance variables

    uint32_t   iv_bitLen;  ///< The bit length of this buffer.
    void       *iv_bufAddr; ///< The beginning address of this buffer.
    uint32_t   iv_offset;  ///< Start position offset
};

/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/

template< typename T >
packed_buffer * BitString<T>::getRelativePosition( uint32_t & o_relPos,
                            uint32_t   i_absPos ) const
{
    PRDF_ASSERT( nullptr != getBufAddr() );     // must have a valid address
    PRDF_ASSERT( i_absPos < getBitLen() );      // must be a valid position

    o_relPos = (i_absPos + iv_offset) % MINBITS;

    return reinterpret_cast<packed_buffer *>(iv_bufAddr) + ((i_absPos + iv_offset) / MINBITS);
}

//------------------------------------------------------------------------------

template< typename T >
T BitString<T>::getField( uint32_t i_pos, uint32_t i_len ) const
{
    PRDF_ASSERT( nullptr != getBufAddr() );     // must have a valid address
    PRDF_ASSERT( 0 < i_len );                   // must have at least one bit
    PRDF_ASSERT( i_len <= MAXBITS );            // i_len length must be valid
    PRDF_ASSERT( i_pos + i_len <= getBitLen() );// field must be within range

    // Get the relative address and position of the field.
    uint32_t relPos = 0;
    packed_buffer * relAddr = 0;
    T o_val = 0;

    // Retrieve the field of bits from the bit string
    for (uint32_t i = 0; i < i_len; i++) {
        relAddr = getRelativePosition( relPos, i_pos + i );
        if ( relAddr->val & ( static_cast<T>(1) << relPos ) ) { \
            o_val |= ( static_cast<T>(1) << i ); }
    }

    return o_val;
}

//------------------------------------------------------------------------------

template< typename T >
void BitString<T>::setField( uint32_t i_pos, uint32_t i_len, T i_val )
{
    PRDF_ASSERT( nullptr != getBufAddr() );      // must to have a valid address
    PRDF_ASSERT( 0 < i_len );                    // must have at least one bit
    PRDF_ASSERT( i_len <= MAXBITS );             // i_len length must be valid
    PRDF_ASSERT( i_pos + i_len <= getBitLen() ); // field must be within range

    // Get the relative address and position of the field.
    uint32_t relPos = 0;
    packed_buffer * relAddr = 0;

    // Set the field of bits in the bit string
    for ( uint32_t i = 0; i < i_len; i++) {
        relAddr = getRelativePosition( relPos, i_pos + i );

        if ( i_val & ( static_cast<T>(1) << i ) ) { \
            relAddr->val |= ( static_cast<T>(1) << ( relPos ) ); }
        else { relAddr->val &= ~( static_cast<T>(1) << ( relPos ) ); }

    }
}

//------------------------------------------------------------------------------

template< typename T >
void BitString<T>::setString( const BitString & i_sStr, uint32_t i_sPos,
                           uint32_t i_sLen, uint32_t i_dPos )
{
    // Ensure the source parameters are valid.
    PRDF_ASSERT( nullptr != i_sStr.getBufAddr() );
    PRDF_ASSERT( 0 < i_sLen ); // at least one bit to copy
    PRDF_ASSERT( i_sPos + i_sLen <= i_sStr.getBitLen() );

    // Ensure the destination has at least one bit available to copy.
    PRDF_ASSERT( nullptr != getBufAddr() );
    PRDF_ASSERT( i_dPos < getBitLen() );

    // If the source length is greater than the destination length then
    // ignore extra source bits.
    uint32_t actLen = i_sLen;
    if ( i_sLen > ( getBitLen() - i_dPos ) ) {
        actLen =  ( getBitLen() - i_dPos );
    }

    // The bit strings may be in overlapping memory spaces. So we need to copy
    // the data in the correct direction to prevent overlapping.
    uint32_t sRelOffset = 0, dRelOffset = 0;

    packed_buffer * sRelAddr = i_sStr.getRelativePosition( sRelOffset, i_sPos );
    packed_buffer * dRelAddr =        getRelativePosition( dRelOffset, i_dPos );

    // Copy the data.
    if ( (dRelAddr == sRelAddr) && (dRelOffset == sRelOffset) )
    {
        // Do nothing. The source and destination are the same.
    }
    else if ( (dRelAddr < sRelAddr) ||
              ((dRelAddr == sRelAddr) && (dRelOffset < sRelOffset)) )
    {
        // Copy the data forward.
        for ( uint32_t pos = 0; pos < actLen; pos += MAXBITS )
        {
            uint32_t len = MAXBITS;
            if( MAXBITS > ( actLen - pos ) ) {
                len = ( actLen - pos );
            }
            T value = i_sStr.getField( i_sPos + pos, len );
            setField( i_dPos + pos, len, value );
        }
    }
    else // Copy the data backwards.
    {
        // Get the first position of the last chunk (naturally aligned).
        uint32_t lastPos = ((actLen-1) / MAXBITS) * MAXBITS;

        // Start with the last chunk and work backwards.
        for ( int32_t pos = lastPos; 0 <= pos; pos -= MAXBITS )
        {
            uint32_t len = MAXBITS;
            if( MAXBITS > ( actLen - pos ) ) {
                len = ( actLen - pos );
            }
            T value = i_sStr.getField( i_sPos + pos, len );
            setField( i_dPos + pos, len, value );
        }
    }
}

//------------------------------------------------------------------------------

template< typename T >
void BitString<T>::maskString( const BitString & i_mask )
{
    // Get the length of the smallest string.
    uint32_t actLen = i_mask.getBitLen();
    if ( i_mask.getBitLen() > getBitLen() ) {
        actLen = getBitLen();
    }

    for ( uint32_t pos = 0; pos < actLen; pos += MAXBITS )
    {
        uint32_t len = MAXBITS;
        if ( MAXBITS > actLen - pos ) {
            len = ( actLen - pos );
        }

        T dVal =        getField( pos, len );
        T sVal = i_mask.getField( pos, len );

        setField( pos, len, dVal & ~sVal );
    }
}

//------------------------------------------------------------------------------

template< typename T >
bool BitString<T>::isEqual( const BitString & i_str ) const
{
    if ( getBitLen() != i_str.getBitLen() )
        return false; // size not equal

    for ( uint32_t pos = 0; pos < getBitLen(); pos += MAXBITS )
    {
        uint32_t len = MAXBITS;
        if ( MAXBITS > ( getBitLen() - pos ) ) {
            len = ( getBitLen() - pos );
        }

        if ( getField(pos, len) != i_str.getField(pos, len) )
            return false; // bit strings do not match
    }

    return true; // bit strings match
}

//------------------------------------------------------------------------------

template< typename T >
bool BitString<T>::isZero() const
{
    for ( uint32_t pos = 0; pos < getBitLen(); pos += MAXBITS )
    {
        uint32_t len = MAXBITS;
        if ( MAXBITS > ( getBitLen() - pos ) ) {
            len = ( getBitLen() - pos );
        }

        if ( 0 != getField(pos, len) )
            return false; // something is non-zero
    }

    return true; // everything was zero
}

//------------------------------------------------------------------------------

template< typename T >
uint32_t BitString<T>::getSetCount( uint32_t i_pos, uint32_t i_len ) const
{
    uint32_t endPos = i_pos + i_len;

    PRDF_ASSERT( endPos <= getBitLen() );

    uint32_t count = 0;

    for ( uint32_t i = i_pos; i < endPos; i++ )
    {
        if ( isBitSet(i) ) count++;
    }

    return count;
}

//##############################################################################
//                      BitStringBuffer class definition
//##############################################################################

/** A BitStringBuffer is a BitString that maintains its own buffer in memory.
 *  It guarantees that sufficient memory is allocated and deallocated in the
 *  constructor and destructor, respectively. In addition, the assignment
 *  operator will adjust the amount of memory needed, as necessary, for the
 *  assignment.
 */
template< class T >
class BitStringBuffer : public BitString<T>
{
  public: // functions

    /**
     * @brief Create a BitStringBuffer structure
     * @param i_bitLen Number of bits in the string.
     */
    explicit BitStringBuffer( uint32_t i_bitLen );

    /** @brief Destructor */
    ~BitStringBuffer();

    /** @brief Copy constructor from BitString */
    BitStringBuffer ( const BitString<T>  & i_bs );

    /** @brief Copy constructor from BitStringBuffer */
    BitStringBuffer ( const BitStringBuffer<T>  & i_bsb );

    /** @brief Assignment from BitString */
    BitStringBuffer  & operator=( const BitString<T>  & i_bs );

    /** @brief Assignment from BitStringBuffer */
    BitStringBuffer  & operator=( const BitStringBuffer<T>  & i_bsb );

  private: // functions

    /** @brief Deallocates the old buffer, if needed, and initializes the new
     *         buffer. */
    void initBuffer();
};

//------------------------------------------------------------------------------
//                      BitStringBuffer class implementation
//------------------------------------------------------------------------------

// BitStringBuffer Constructor From Length - public
// Use: BitStringBuffer<> bitstringbuffer( int )

template< typename T >
BitStringBuffer<T>::BitStringBuffer( uint32_t i_bitLen ) :
    BitString<T>( i_bitLen, nullptr )
{
    initBuffer();
}

//------------------------------------------------------------------------------
// BitStringBuffer Destructor - public
// Use: delete bitstringbuffer

template< typename T >
BitStringBuffer<T>::~BitStringBuffer()
{
    delete [] this->getBufAddr();
}

//------------------------------------------------------------------------------
// BitStrinBuffer Copy Constructor From BitString - public
// Use: BitStringBuffer<> bitstringbuffer = bitstring

template< typename T >
BitStringBuffer<T>::BitStringBuffer( const BitString<T> & i_bs ) :
    BitString<T>( i_bs.getBitLen(), nullptr )
{
    initBuffer();
    if ( ! ( nullptr == i_bs.getBufAddr() ) ) {
        if ( !i_bs.isZero() ) this->setString( i_bs );
    }
}

//------------------------------------------------------------------------------
// BitStringBuffer Copy Constructor From BitStringBuffer - public
// Use: BitStringBuffer<> bitstringbuffer' = bitstringbuffer

template< typename T >
BitStringBuffer<T>::BitStringBuffer( const BitStringBuffer<T> & i_bsb ) :
    BitString<T>( i_bsb.getBitLen(), nullptr )
{
    initBuffer();
    if ( !i_bsb.isZero() ) this->setString( i_bsb );
}

//------------------------------------------------------------------------------
// BitstringBuffer Assignment From BitString - public
// Use: BitStringBuffer<> bitstringbuffer = bitstring

template< typename T >
BitStringBuffer<T> & BitStringBuffer<T>::operator=( const BitString<T> & i_bs )
{
    // The initBuffer() function will deallocate the buffer as well, however we
    // also need to deallocate the buffer here before we set the length.
    delete [] this->getBufAddr();
    this->setBufAddr( nullptr );

    this->setBitLen( i_bs.getBitLen() );
    initBuffer();
    if ( ! ( nullptr == i_bs.getBufAddr() ) ) {
        if ( !i_bs.isZero() ) this->setString( i_bs );
    }

    return *this;
}

//------------------------------------------------------------------------------
// BitStringBuffer Assignment From BitStringBuffer - public
// Use: BitStringBuffer<> bitstringbuffer' = bitstringbuffer

template< typename T >
BitStringBuffer<T> & BitStringBuffer<T>::operator=( const BitStringBuffer<T> & i_bsb )
{
    if ( this != &i_bsb ) // Check for assignment to self
    {
        // The initBuffer() function will deallocate the buffer as well, however
        // we also need to deallocate the buffer here before we set the length.
        delete [] this->getBufAddr();
        this->setBufAddr( nullptr );

        this->setBitLen( i_bsb.getBitLen() );
        initBuffer();
        if ( !i_bsb.isZero() ) this->setString( i_bsb );
    }

    return *this;
}

//------------------------------------------------------------------------------
// BitStringBuffer Operator Overload Not - public
// Use: ~bitstringbuffer

template< typename T >
BitStringBuffer<T> BitString<T>::operator~() const
{
    BitStringBuffer<T> bsb( getBitLen() );

    for ( uint32_t pos = 0; pos < getBitLen(); pos += MAXBITS )
    {
        uint32_t len = MAXBITS;
        if ( MAXBITS > ( getBitLen() - pos ) ) { len = ( getBitLen() - pos ); }

        T dVal = getField( pos, len );

        bsb.setField( pos, len, ~dVal );
    }

    return bsb;
}

//------------------------------------------------------------------------------
// BitStringBuffer Operator Overload And - public
// Use: bitstringbuffer & bitstring

template< typename T >
BitStringBuffer<T> BitString<T>::operator&( const BitString<T> & i_bs ) const
{
    // Get the length of the smallest string.
    uint32_t actLen = getBitLen();
    if ( getBitLen() > i_bs.getBitLen() ) { actLen = i_bs.getBitLen(); }

    BitStringBuffer<T> bsb( actLen );

    for ( uint32_t pos = 0; pos < actLen; pos += MAXBITS )
    {
        uint32_t len = MAXBITS;
        if ( MAXBITS > ( actLen - pos ) ) { len = ( actLen - pos ); }

        T dVal =      getField( pos, len );
        T sVal = i_bs.getField( pos, len );

        bsb.setField( pos, len, dVal & sVal );
    }

    return bsb;
}

//------------------------------------------------------------------------------
// BitStringBuffer Operator Overload Or - public
// Use: bitstringbuffer | bitstring

template< typename T >
BitStringBuffer<T> BitString<T>::operator|( const BitString<T> & i_bs ) const
{
    // Get the length of the smallest string.
    uint32_t actLen = getBitLen();
    if ( getBitLen() > i_bs.getBitLen() ) { actLen = i_bs.getBitLen(); }

    BitStringBuffer<T> bsb( actLen );

    for ( uint32_t pos = 0; pos < actLen; pos += MAXBITS )
    {
        uint32_t len = MAXBITS;
        if ( MAXBITS > ( actLen - pos ) ) { len = ( actLen - pos ); }

        T dVal =      getField( pos, len );
        T sVal = i_bs.getField( pos, len );

        bsb.setField( pos, len, dVal | sVal );
    }

    return bsb;
}

//------------------------------------------------------------------------------
// BitStringBuffer Operator Overload ShiftRight - public
// Use: bitstringbuffer >> int

template< typename T >
BitStringBuffer<T> BitString<T>::operator>>( uint32_t i_shift ) const
{
    BitStringBuffer<T> bsb( getBitLen() ); // default all zeros

    if ( i_shift < getBitLen() ) // would be zero after shift > getBitLen
    {
        // extract string that would remain after shift
        BitString<T> bs ( this->getBitLen() - i_shift, this->getBufAddr(), i_shift );
        // copy string to shifted position
        bsb.setString( bs, 0, bs.getBitLen(), 0 );
    }
    return bsb;
}

//------------------------------------------------------------------------------
// BitStringBuffer Operator Overload ShiftLeft - public
// Use: bitstringbuffer << int

template< typename T >
BitStringBuffer<T> BitString<T>::operator<<( uint32_t i_shift ) const
{
    BitStringBuffer<T> bsb( getBitLen() ); // default all zeros

    if ( i_shift < getBitLen() ) // would be zero after shift > getBitLen
    {
        // extract string that would remain after shift
        BitString<T> bs ( this->getBitLen() - i_shift, this->getBufAddr(), 0 );
        // copy string to shifted position
        bsb.setString( bs, 0, bs.getBitLen(), i_shift );
    }
    return bsb;
}

//------------------------------------------------------------------------------
// BitStringBuffer initBuffer - private

template< typename T >
void BitStringBuffer<T>::initBuffer()
{
    // Deallocate the current buffer.
    delete [] this->getBufAddr();

    // Allocate the new buffer.
    this->setBufAddr( new T [ ( this->getBitLen() / 8 ) / sizeof( T ) ] );

    // Clear the new buffer.
    if ( ! ( this->isZero() ) ) this->clearAll();
}

//##############################################################################
//                       bsb_t class
//##############################################################################

//------------------------------------------------------------------------------
// bsb_t constructor - public
// Use: bsb_t bsb( bitstring_val );

template< class T >
class bsb_t : public BitString<T>
{
  public: // function

    explicit bsb_t( T &value ) : BitString<T>( sizeof(T)*8, &value ) {}
};
