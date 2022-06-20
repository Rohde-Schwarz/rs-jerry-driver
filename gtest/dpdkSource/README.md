# File overview

## HRZR Header

### controltypes.bin
Includes all `control`-types with Bit 2 unset and increasing `sequence number`

### controltypesWithTimestamp.bin
`controltypes.bin` with Bit 2 set.

### controltypesReverseSequenceNumber.bin
`controltypes.bin` with decreasing `sequenenz number`

## HRZR Metadata

### metadata_time_clock_types.bin
Includes all `Clock Sync Source`-types and all `Time Sync Source`-types

## HRZR Header+Metadata

### header_metadata.bin
Includes control:metadata and one whole metadata example

## Misc

### floatsamples.in
Includes complex float samples to be proccessed.

### floatsamples.norm
Includes normed `floatsamples.in` to compare to.
