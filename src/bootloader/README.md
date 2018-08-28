# Hostboot Bootloader (HBBL)
The purpose of HBBL is to find hostboot images in the PNOR, load the images in
the cache, and start hostboot. HBBL is a compact image, which is delivered as
part of the SBE SEEPROM image.

## Terminology
- **SBE**: Self Boot Engine (Runs prior to hostboot)
- **PNOR**: Flash memory where hostboot code is stored
- **HRMOR**: Offset register that tells hypervisor where real memory starts
- **L3 Cache**: In this context, it is local memory
- **Boot side**: Devices store memory on redundant “sides” in case one side is
 corrupted, the other is available
- **HBB**: Hostboot Base Image, section of PNOR flash that holds the code that
starts hostboot
- **TOC**: Table of Contents, section of PNOR flash that describes the layout
of the rest of the flash chip
