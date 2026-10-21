/* vita-elf-create puts the import/export tables (a few KB of "SCE data") at
 * the end of the code segment (segment 0). The linker starts the data
 * segment at the next 1 MiB boundary, so when the code happens to end just
 * below a boundary there's no room left and the build fails with
 *   "Cannot allocate N bytes for SCE data at end of segment 0; segment 1
 *    overlaps".
 * This read-only block (in segment 0) moves the end of the code past that
 * boundary. If the error ever comes back after the code grows or shrinks,
 * change the size (or remove this file) so the end lands elsewhere. */
__attribute__((used, aligned(16))) const unsigned char tmc_vita_segment_pad[16 * 1024] = { 1 };
