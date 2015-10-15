/*
 * GLUS - Modern OpenGL, OpenGL ES and OpenVG Utilities. Copyright (C) since 2010 Norbert Nopper
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GL/glus.h"

// Data type used for the memory array.
#define GLUS_MEMORY_DATA_TYPE uint32_t

// Data size of the data type.
#define GLUS_MEMORY_DATA_SIZE sizeof(GLUS_MEMORY_DATA_TYPE)

// Alignment in memory.
#define GLUS_MEMORY_DATA_ALIGNMENT_SIZE (GLUS_MEMORY_DATA_SIZE * 2)

// Factor to get the right alignment.
#define GLUS_MEMORY_DATAT_ALIGNMENT_FACTOR (GLUS_MEMORY_DATA_ALIGNMENT_SIZE / GLUS_MEMORY_DATA_SIZE)

// 128 MB of memory. Change only here, if more or less is needed.
#define GLUS_MEMORY_SIZE (128*1024*1024/GLUS_MEMORY_DATA_SIZE)

// Number of memory table entries.
#define GLUS_MEMORY_TABLE_ENTRIES	2048

// States of a memory block entry.
#define GLUS_VALID_AND_FREE		1
#define GLUS_VALID_AND_LOCKED   2
#define GLUS_INVALID			3

/**
 * Structure for the memory table entry.
 */
typedef struct _GLUSmemoryTable {

	/**
	 * Flag, if entry is valid and/or free.
	 */
	GLUSubyte status;

	/**
	 * Start index into the memory.
	 */
	size_t startIndex;

	/**
	 * Length of the indices from the allocated memory.
	 */
	size_t lengthIndices;

} GLUSmemoryTable;

/**
 * Available memory with given data type.
 */
static GLUS_MEMORY_DATA_TYPE g_memory[GLUS_MEMORY_SIZE];

/**
 * Memory table used to manage the memory array.
 */
static GLUSmemoryTable g_memoryTable[GLUS_MEMORY_TABLE_ENTRIES] = {{GLUS_VALID_AND_FREE, 0, GLUS_MEMORY_SIZE}};

/**
 * Current amount of initialized memory table entries.
 */
static size_t g_memoryTableEntries = 1;

static GLUSboolean glusMemoryInitTableEntry(size_t startIndex, size_t lengthIndices)
{
    size_t tableIndex = 0;

	//

    while (tableIndex < g_memoryTableEntries)
    {
        // If not valid, the table entry can be reused.
        if (g_memoryTable[tableIndex].status == GLUS_INVALID)
        {
            break;
        }

        tableIndex++;
    }

    if (tableIndex >= GLUS_MEMORY_TABLE_ENTRIES)
    {
        return GLUS_FALSE;
    }

	//

	g_memoryTable[tableIndex].status = GLUS_VALID_AND_FREE;
	g_memoryTable[tableIndex].startIndex = startIndex;
	g_memoryTable[tableIndex].lengthIndices = lengthIndices;

	if (tableIndex == g_memoryTableEntries)
	{
		g_memoryTableEntries++;
	}

	return GLUS_TRUE;
}

static GLUSvoid glusMemoryGarbageCollect()
{
	GLUSboolean continueGC = GLUS_TRUE;

	// Do garbage collection, until no memory has been merged.
	while(continueGC)
	{
		GLUSuint tableIndex = 0;

		continueGC = GLUS_FALSE;

		while (tableIndex < g_memoryTableEntries)
		{
			if (g_memoryTable[tableIndex].status == GLUS_VALID_AND_FREE)
			{
				GLUSuint otherTableIndex = 0;

				while (otherTableIndex < g_memoryTableEntries)
				{
					if (otherTableIndex == tableIndex)
					{
						otherTableIndex++;

						continue;
					}

					if (g_memoryTable[otherTableIndex].status == GLUS_VALID_AND_FREE)
					{
						// Check, if two entries are adjacent.
						if (g_memoryTable[tableIndex].startIndex + g_memoryTable[tableIndex].lengthIndices == g_memoryTable[otherTableIndex].startIndex)
						{
							g_memoryTable[tableIndex].lengthIndices += g_memoryTable[otherTableIndex].lengthIndices;

							g_memoryTable[otherTableIndex].status = GLUS_INVALID;

							// Continue to try to merge.
							continueGC = GLUS_TRUE;
						}
					}

					otherTableIndex++;
				}
			}

			tableIndex++;
		}
	}
}

static void* glusMemoryInternalMalloc(size_t size)
{
	GLUSuint tableIndex = 0;

	// Calculate needed indecies.
	size_t lengthIndices = size % GLUS_MEMORY_DATA_SIZE == 0 ? (size / GLUS_MEMORY_DATA_SIZE) : (size / GLUS_MEMORY_DATA_SIZE + 1);

	size_t alignmentIndicesRest = lengthIndices % GLUS_MEMORY_DATAT_ALIGNMENT_FACTOR;

	// Needed for overflow of lengthIndices.
	if (lengthIndices * GLUS_MEMORY_DATA_SIZE < size)
	{
		return 0;
	}

	// Test an adjust alignment.
	if (alignmentIndicesRest != 0)
	{
	    lengthIndices += GLUS_MEMORY_DATAT_ALIGNMENT_FACTOR - alignmentIndicesRest;
	}

	while (tableIndex < g_memoryTableEntries)
	{
		// Search for a memory table entry, where the size fits in.
		if (g_memoryTable[tableIndex].status == GLUS_VALID_AND_FREE && g_memoryTable[tableIndex].lengthIndices >= lengthIndices)
		{
		    // Is a split needed?
		    if (lengthIndices < g_memoryTable[tableIndex].lengthIndices)
		    {
                // Assign the rest of the available memory to another table entry.
                if (!glusMemoryInitTableEntry(g_memoryTable[tableIndex].startIndex + lengthIndices, g_memoryTable[tableIndex].lengthIndices - lengthIndices))
                {
                    // No empty entry could be found, so do not split and use all memory.
                    lengthIndices = g_memoryTable[tableIndex].lengthIndices;
                }
		    }

			// Entry now manages the requested memory.
			g_memoryTable[tableIndex].status = GLUS_VALID_AND_LOCKED;
			g_memoryTable[tableIndex].lengthIndices = lengthIndices;

			return (void*)&g_memory[g_memoryTable[tableIndex].startIndex];
		}

		tableIndex++;
	}

	return 0;
}

//

void* GLUSAPIENTRY glusMemoryMalloc(size_t size)
{
	void* pointer = 0;

	if (size == 0)
	{
		return pointer;
	}

	pointer = glusMemoryInternalMalloc(size);

	// If no memory was allocated ...
	if (!pointer)
	{
		// ... do garbage collection ...
		glusMemoryGarbageCollect();

		// ... and try to allocate again.
		pointer = glusMemoryInternalMalloc(size);
	}

	return pointer;
}

void GLUSAPIENTRY glusMemoryFree(void* pointer)
{
	GLUSuint tableIndex = 0;

	if (!pointer)
	{
		return;
	}

	// Search pointer ...
	while (tableIndex < g_memoryTableEntries)
	{
		// ... and free memory by setting flag in table entry.
		if (g_memoryTable[tableIndex].status == GLUS_VALID_AND_LOCKED && (void*)&g_memory[g_memoryTable[tableIndex].startIndex] == pointer)
		{
			g_memoryTable[tableIndex].status = GLUS_VALID_AND_FREE;

			return;
		}

		tableIndex++;
	}
}
