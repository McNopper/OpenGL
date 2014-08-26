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

// 4 byte alignment.
#define GLUS_ALIGNMENT 4

// 128 MB of memory. Change only here, if more or less is needed.
// Division by 4 bytes, as uint32_t is used.
#define GLUS_MEMORY_SIZE (128*1024*1024/GLUS_ALIGNMENT)

// Number of memory table entries.
#define GLUS_MEMORY_TABLE_ENTRIES	1024

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
 * Available memory with 4 byte alignment. If alignment is changed, do also adapt size.
 */
static uint32_t g_memory[GLUS_MEMORY_SIZE];

/**
 * Memory table used to manage the memory array.
 */
static GLUSmemoryTable g_memoryTable[GLUS_MEMORY_TABLE_ENTRIES] = {{GLUS_VALID_AND_FREE, 0, GLUS_MEMORY_SIZE}};

/**
 * Current amount of initialized memory table entries.
 */
static size_t g_memoryTableEntries = 1;

static GLUSboolean glusMemoryFindTableEntry(size_t* foundTableIndex)
{
	GLUSuint tableIndex = 0;

	if (!foundTableIndex)
	{
		return GLUS_FALSE;
	}

	while (tableIndex < g_memoryTableEntries)
	{
		// If not valid, the table entry can be reused.
		if (g_memoryTable[tableIndex].status == GLUS_INVALID)
		{
			*foundTableIndex = tableIndex;

			return GLUS_TRUE;
		}

		tableIndex++;
	}

	return GLUS_FALSE;
}

static GLUSboolean glusMemoryInitTableEntry(size_t tableIndex, size_t startIndex, size_t lengthIndices)
{
	if (tableIndex > g_memoryTableEntries || tableIndex >= GLUS_MEMORY_TABLE_ENTRIES)
	{
		return GLUS_FALSE;
	}

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

	// 4 byte alignment.
	size_t lengthIndices = size % GLUS_ALIGNMENT == 0 ? (size / GLUS_ALIGNMENT) : (size / GLUS_ALIGNMENT + 1);

	if (lengthIndices * GLUS_ALIGNMENT < size)
	{
		return 0;
	}

	while (tableIndex < g_memoryTableEntries)
	{
		// Search for a memory table entry, where the size fits in.
		if (g_memoryTable[tableIndex].status == GLUS_VALID_AND_FREE && g_memoryTable[tableIndex].lengthIndices >= lengthIndices)
		{
			size_t otherTableIndex;

			// Try to reuse an entry.
			if (!glusMemoryFindTableEntry(&otherTableIndex))
			{
				otherTableIndex = tableIndex + 1;
			}

			// Assign the rest of the available memory to another table entry.
			if (!glusMemoryInitTableEntry(otherTableIndex, g_memoryTable[tableIndex].startIndex + lengthIndices, g_memoryTable[tableIndex].lengthIndices - lengthIndices))
			{
				// No empty entry could be found, so do not split and use all memory.
				lengthIndices = g_memoryTable[tableIndex].lengthIndices;
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
