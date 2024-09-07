WavPack Seeking
===============

WavPack is a block-based format.


Default Behavior
----------------

A lot of useful information is found in `::WavpackSeekSample64()`.

The behavior is such that it will seek to the block holding the requested
sample, then allocate a temporary buffer(!) and unpack the number of samples
that need to be skipped in the block.

It means we can do a naive implementation with accurate seeks.


Finding Block Start Indices by Digging into `::WavpackContext`
--------------------------------------------------------------

Most of the useful information is in 'WavpackStream`, which could be accessed
from the `WavpackContext`, but both are private implementations only found in
`wavpack_local.h`.

    struct WavpackContext {

      int num_streams, max_streams, stream_version;
      WavpackStream **streams;
      void *stream3;

    };

    struct WavpackStream {

      unsigned char *blockbuff, *blockend;
      unsigned char *block2buff, *block2end;

    };


Finding Block Start Indices by Parsing the File
-----------------------------------------------

- The structure `WavpackHeader` inside `wavpack.h` is not used by the API,
  but part of the file format (so changing it would break compatibility).

    WavpackHeader::block_index    Index of the first sample in the block.
    WavpackHeader::block_samples  Number of samples stored in the block.

  In multi-channel WavPack files, blocks for the same sample range happen,
  because channels are encoded individually or as stereo pairs.

- `::find_sample()` in `unpack_seek.c` searches for the block containing
  a sample using the following method:

  - If there's a current block (most recently decoded one), check if it
    lies before or after the target sample.

  - Then do something similar to a binary search, but the middle point
    is calculated from sample count between left and right block.

