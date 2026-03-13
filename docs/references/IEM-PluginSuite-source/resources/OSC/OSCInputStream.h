/*
 ==============================================================================

 This file is part of the JUCE library.
 Copyright (c) 2017 - ROLI Ltd.

 JUCE is an open source library subject to commercial or open-source
 licensing.

 By using JUCE, you agree to the terms of both the JUCE 5 End-User License
 Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
 27th April 2017).

 End User License Agreement: www.juce.com/juce-5-licence
 Privacy Policy: www.juce.com/juce-5-privacy-policy

 Or: You may also use this code under the terms of the GPL v3 (see
 www.gnu.org/licenses).

 JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
 EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
 DISCLAIMED.

 ==============================================================================
 */

#include <sys/types.h>

#if defined(_MSC_VER)
    #include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

class MyOSCInputStream
{
public:
    /** Creates an MyOSCInputStream.

     @param sourceData               the block of data to use as the stream's source
     @param sourceDataSize           the number of bytes in the source data block
     */
    MyOSCInputStream (const void* sourceData, size_t sourceDataSize) :
        input (sourceData, sourceDataSize, false)
    {
    }

    //==============================================================================
    /** Returns a pointer to the source data block from which this stream is reading. */
    const void* getData() const noexcept { return input.getData(); }

    /** Returns the number of bytes of source data in the block from which this stream is reading. */
    size_t getDataSize() const noexcept { return input.getDataSize(); }

    /** Returns the current position of the stream. */
    juce::uint64 getPosition() { return (juce::uint64) input.getPosition(); }

    /** Attempts to set the current position of the stream. Returns true if this was successful. */
    bool setPosition (juce::int64 pos) { return input.setPosition (pos); }

    /** Returns the total amount of data in bytes accessible by this stream. */
    juce::int64 getTotalLength() { return input.getTotalLength(); }

    /** Returns true if the stream has no more data to read. */
    bool isExhausted() { return input.isExhausted(); }

    //==============================================================================
    juce::int32 readInt32()
    {
        checkBytesAvailable (4, "OSC input stream exhausted while reading int32");
        return input.readIntBigEndian();
    }

    juce::uint64 readUint64()
    {
        checkBytesAvailable (8, "OSC input stream exhausted while reading uint64");
        return (juce::uint64) input.readInt64BigEndian();
    }

    float readFloat32()
    {
        checkBytesAvailable (4, "OSC input stream exhausted while reading float");
        return input.readFloatBigEndian();
    }

    juce::String readString()
    {
        checkBytesAvailable (4, "OSC input stream exhausted while reading string");

        auto posBegin = (size_t) getPosition();
        auto s = input.readString();
        auto posEnd = (size_t) getPosition();

        if (static_cast<const char*> (getData())[posEnd - 1] != '\0')
            throw juce::OSCFormatError (
                "OSC input stream exhausted before finding null terminator of string");

        size_t bytesRead = posEnd - posBegin;
        readPaddingZeros (bytesRead);

        return s;
    }

    juce::MemoryBlock readBlob()
    {
        checkBytesAvailable (4, "OSC input stream exhausted while reading blob");

        auto blobDataSize = input.readIntBigEndian();
        checkBytesAvailable ((blobDataSize + 3) % 4,
                             "OSC input stream exhausted before reaching end of blob");

        juce::MemoryBlock blob;
        auto bytesRead = input.readIntoMemoryBlock (blob, (ssize_t) blobDataSize);
        readPaddingZeros (bytesRead);

        return blob;
    }

    juce::OSCColour readColour()
    {
        checkBytesAvailable (4, "OSC input stream exhausted while reading colour");
        return juce::OSCColour::fromInt32 ((juce::uint32) input.readIntBigEndian());
    }

    juce::OSCTimeTag readTimeTag()
    {
        checkBytesAvailable (8, "OSC input stream exhausted while reading time tag");
        return juce::OSCTimeTag (juce::uint64 (input.readInt64BigEndian()));
    }

    juce::OSCAddress readAddress() { return juce::OSCAddress (readString()); }

    juce::OSCAddressPattern readAddressPattern() { return juce::OSCAddressPattern (readString()); }

    //==============================================================================
    juce::OSCTypeList readTypeTagString()
    {
        juce::OSCTypeList typeList;

        checkBytesAvailable (4, "OSC input stream exhausted while reading type tag string");

        if (input.readByte() != ',')
            throw juce::OSCFormatError ("OSC input stream format error: expected type tag string");

        for (;;)
        {
            if (isExhausted())
                throw juce::OSCFormatError (
                    "OSC input stream exhausted while reading type tag string");

            const juce::OSCType type = input.readByte();

            if (type == 0)
                break; // encountered null terminator. list is complete.

            if (! juce::OSCTypes::isSupportedType (type))
                throw juce::OSCFormatError (
                    "OSC input stream format error: encountered unsupported type tag");

            typeList.add (type);
        }

        auto bytesRead = (size_t) typeList.size() + 2;
        readPaddingZeros (bytesRead);

        return typeList;
    }

    //==============================================================================
    juce::OSCArgument readArgument (juce::OSCType type)
    {
        switch (type)
        {
            case 'i':
                return juce::OSCArgument (readInt32());
            case 'f':
                return juce::OSCArgument (readFloat32());
            case 's':
                return juce::OSCArgument (readString());
            case 'b':
                return juce::OSCArgument (readBlob());
            case 'r':
                return juce::OSCArgument (readColour());

            default:
                // You supplied an invalid OSCType when calling readArgument! This should never happen.
                jassertfalse;
                throw juce::OSCInternalError (
                    "OSC input stream: internal error while reading message argument");
        }
    }

    //==============================================================================
    juce::OSCMessage readMessage()
    {
        auto ap = readAddressPattern();
        auto types = readTypeTagString();

        juce::OSCMessage msg (ap);

        for (auto& type : types)
            msg.addArgument (readArgument (type));

        return msg;
    }

    //==============================================================================
    juce::OSCBundle readBundle (size_t maxBytesToRead = std::numeric_limits<size_t>::max())
    {
        // maxBytesToRead is only passed in here in case this bundle is a nested
        // bundle, so we know when to consider the next element *not* part of this
        // bundle anymore (but part of the outer bundle) and return.

        checkBytesAvailable (16, "OSC input stream exhausted while reading bundle");

        if (readString() != "#bundle")
            throw juce::OSCFormatError (
                "OSC input stream format error: bundle does not start with string '#bundle'");

        juce::OSCBundle bundle (readTimeTag());

        size_t bytesRead = 16; // already read "#bundle" and timeTag
        auto pos = getPosition();

        while (! isExhausted() && bytesRead < maxBytesToRead)
        {
            bundle.addElement (readElement());

            auto newPos = getPosition();
            bytesRead += (size_t) (newPos - pos);
            pos = newPos;
        }

        return bundle;
    }

    //==============================================================================
    juce::OSCBundle::Element readElement()
    {
        checkBytesAvailable (4, "OSC input stream exhausted while reading bundle element size");

        auto elementSize = (size_t) readInt32();

        if (elementSize < 4)
            throw juce::OSCFormatError (
                "OSC input stream format error: invalid bundle element size");

        return readElementWithKnownSize (elementSize);
    }

    //==============================================================================
    juce::OSCBundle::Element readElementWithKnownSize (size_t elementSize)
    {
        checkBytesAvailable ((juce::int64) elementSize,
                             "OSC input stream exhausted while reading bundle element content");

        auto firstContentChar = static_cast<const char*> (getData())[getPosition()];

        if (firstContentChar == '/')
            return juce::OSCBundle::Element (readMessageWithCheckedSize (elementSize));
        if (firstContentChar == '#')
            return juce::OSCBundle::Element (readBundleWithCheckedSize (elementSize));

        throw juce::OSCFormatError ("OSC input stream: invalid bundle element content");
    }

private:
    juce::MemoryInputStream input;

    //==============================================================================
    void readPaddingZeros (size_t bytesRead)
    {
        size_t numZeros = ~(bytesRead - 1) & 0x03;

        while (numZeros > 0)
        {
            if (isExhausted() || input.readByte() != 0)
                throw juce::OSCFormatError ("OSC input stream format error: missing padding zeros");

            --numZeros;
        }
    }

    juce::OSCBundle readBundleWithCheckedSize (size_t size)
    {
        auto begin = (size_t) getPosition();
        auto maxBytesToRead = size - 4; // we've already read 4 bytes (the bundle size)

        juce::OSCBundle bundle (readBundle (maxBytesToRead));

        if (getPosition() - begin != size)
            throw juce::OSCFormatError (
                "OSC input stream format error: wrong element content size encountered while reading");

        return bundle;
    }

    juce::OSCMessage readMessageWithCheckedSize (size_t size)
    {
        auto begin = (size_t) getPosition();
        auto message = readMessage();

        if (getPosition() - begin != size)
            throw juce::OSCFormatError (
                "OSC input stream format error: wrong element content size encountered while reading");

        return message;
    }

    void checkBytesAvailable (juce::int64 requiredBytes, const char* message)
    {
        if (input.getNumBytesRemaining() < requiredBytes)
            throw juce::OSCFormatError (message);
    }
};
