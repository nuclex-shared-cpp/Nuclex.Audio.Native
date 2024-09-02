Container Formats, an Overview
==============================

Some preliminary investigation into the structure of various audio file formats
in order to be able to find the right approach to the public API of Nuclex.Audio.Native
and decide whether to expose only a streaming interface of a random access interface.


OGG (Vorbis and Opus)
=====================

At first, Ogg looks like a rather deeply layered container:

    Physical Bitstream           The data of one or more audio files concatenated
      Logical Bitstream          A single audio file
        Pages                    0-255 byte fragments of one audio packet
          Packets                Individually decodable chunk of audio data

The entire "Physical Bitstream" layer can usually be disregarded. It allows multiple
OGG files to be concatenated (as in `cat song1.ogg song2.ogg > bothsongs.ogg`).
This means audio format, channel layout and tagging metadata could suddenly change.

Inside the logical bitstream, pages are a slightly heaver framing element, containing
a header indicating the absolute sample position, a stream id and checksum and such.
Normal page sizes are mentioned to be around 8 KiB.

Packets are another level that can probably disregarded. Audio data is split into
0-255 byte packets. The page header lists packet sizes, but the packets themselves
have no header, so the concept of packets might as well not exist for our purposes.

So it comes down to:

    Pages
      Audio chunks               Each can extend to multiple pages

Audio chunks are split into packets (I don't see the reason, perhaps UDP transmissions?)
and into pages, but a independentlich decodeable audio chunk can cover multiple packets
and multiple pages, too.

I haven't figured out how to do that. Or if pages are, after all, decodable individually
in some way, since each page records the absolute sample / frame position.


FLAC
====

FLAC can be encapsulated in OGG, but usually stands alone:

  File
    Blocks
      Subblocks

Audio can be stored in non-interlaved channels, changing per block/subblock, as each
block can have a different compression algorithm.

It sounded as if different channels can have different compression algorithms applied
to them, too, so putting together a full audio frame might require decoding multiple
blocks in a row (or is that the purpose of subblocks?)


WavPack
=======

WavPack is really simple:

  File
    Blocks

