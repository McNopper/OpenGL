/*
 * GLUS - Modern OpenGL, OpenGL ES and OpenVG Utilities. Copyright (C) 2010 - 2014 Norbert Nopper
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

#ifndef GLUS_MEMORY_H_
#define GLUS_MEMORY_H_

/**
 * Allocate memory block.
 *
 * @param size Size of the memory block in bytes.
 *
 * @return Returns on success the pointer to allocated memory. Otherwise null is returned.
 */
GLUSAPI void* GLUSAPIENTRY glusMalloc(size_t size);

/**
 * Deallocate memory block.
 *
 * @param pointer Pointer to a memory block previously allocated with glusMalloc.
 */
GLUSAPI void GLUSAPIENTRY glusFree(void* pointer);

#endif /* GLUS_MEMORY_H_ */
